#include "duplo-util/storage.h"

Storage::Storage(uint32_t num_max_execs, bool ram_only) :
  buffers(NUM_DATA_TYPES,
          std::vector<std::array<uint8_t, STORAGE_BUFFER_SIZE>>(num_max_execs)),
  ram_only(ram_only),
  buffered_components(NUM_DATA_TYPES,
                      std::vector<std::string>(num_max_execs)),
  buffered_interval(NUM_DATA_TYPES,
                    std::vector<std::pair<uint32_t, uint32_t>>(num_max_execs)) {
}

void Storage::PrepareFile(std::string file_name, DATA_TYPE data_type, uint64_t num_prepare_bytes) {

  std::string full_file_name = GetFullFileName(file_name, data_type);
  if (!ram_only) {
    int fd = open(full_file_name.c_str(), O_RDWR | O_CREAT, S_IRWXU);

    // Stretch the file size to the required size
    if (lseek(fd, num_prepare_bytes, SEEK_SET) == -1) {
      //below is not good practice. See comment for explanation.
      close(fd);
      perror("Error calling lseek() to 'stretch' the file");
      exit(EXIT_FAILURE);
    }

    if (write(fd, "", 1) == -1) {
      close(fd);
      perror("Error writing last byte of the file");
      exit(EXIT_FAILURE);
    }

    close(fd);
  } else {
    buffer_map.emplace(full_file_name, std::vector<uint8_t>(num_prepare_bytes));
  }
  map_mutex.lock();

  file_size_map.emplace(full_file_name, num_prepare_bytes);

  map_mutex.unlock();
}

void Storage::WriteBuckets(std::string file_name, DATA_TYPE data_type, uint32_t buckets_from, uint32_t num_buckets, uint8_t data_to_write[], uint64_t write_pos, uint64_t num_bytes, uint32_t bucket_size) {

  if (num_bytes == 0) {
    return;
  }

  std::string full_file_name = GetFullFileName(file_name, data_type);

  uint64_t num_prepare_bytes = file_size_map[full_file_name];
  if ((write_pos + num_bytes) > num_prepare_bytes) {
    perror("Writing outside file");
    exit(EXIT_FAILURE);
  }

  if (!ram_only) {

    int fd = open(full_file_name.c_str(), O_WRONLY, S_IRWXU);

    if (lseek(fd, write_pos, SEEK_SET) == -1) {
      //below is not good practice. See comment for explanation.
      close(fd);
      perror("Error calling lseek() to 'stretch' the file");
      exit(EXIT_FAILURE);
    }

    uint64_t q = num_bytes / max_num_write_bytes;
    uint64_t r = num_bytes % max_num_write_bytes;

    for (uint64_t i = 0; i < q; ++i) {

      if (write(fd, data_to_write + i * max_num_write_bytes, max_num_write_bytes) == -1) {
        close(fd);
        perror("Error writing last byte of the file");
        exit(EXIT_FAILURE);
      }
    }

    if (write(fd, data_to_write + q * max_num_write_bytes, r) == -1) {
      close(fd);
      perror("Error writing last byte of the file");
      exit(EXIT_FAILURE);
    }

    close(fd);
  } else {
    std::copy(data_to_write, data_to_write + num_bytes, buffer_map[full_file_name].begin() + write_pos);
  }

  uint64_t bucket_bytes = num_bytes / num_buckets;
  map_mutex.lock();
  for (uint64_t i = 0; i < num_buckets; ++i) {
    filepos_map.emplace(std::make_tuple(full_file_name, buckets_from + i), write_pos + i * bucket_bytes);
    bucket_bytes_map.emplace(std::make_tuple(full_file_name, buckets_from + i), bucket_bytes);
  }

  map_mutex.unlock();
}

void Storage::ReadBuckets(std::string file_name, DATA_TYPE data_type, uint32_t buckets_from, uint32_t num_buckets, BYTEArrayVector& res, bool use_buffer, uint32_t exec_id) {

  if (num_buckets == 0) {
    return;
  }

  std::string full_file_name = GetFullFileName(file_name, data_type);

  map_mutex.lock();
  uint64_t read_location = filepos_map[std::make_tuple(full_file_name, buckets_from)];

  uint64_t num_bucket_bytes = bucket_bytes_map[std::make_tuple(full_file_name, buckets_from)];
  map_mutex.unlock();

  res = BYTEArrayVector(num_buckets, num_bucket_bytes);
  if (!ram_only) {
    if (use_buffer &&
        (buffered_components[data_type][exec_id] == full_file_name) &&
        (buffered_interval[data_type][exec_id].first <= read_location) &&
        ((read_location + res.size()) <= buffered_interval[data_type][exec_id].second)) { //bytes are buffered

      uint64_t offset_from = read_location - buffered_interval[data_type][exec_id].first;

      std::copy(buffers[data_type][exec_id].begin() + offset_from, buffers[data_type][exec_id].begin() + offset_from + res.size(), res.data());

    } else {

      int fd = open(full_file_name.c_str(), O_RDONLY, S_IRWXU);

      lseek(fd, read_location, SEEK_SET);

      uint64_t file_size = file_size_map[full_file_name];
      uint64_t num_bytes_remaining_of_file = file_size - read_location;

      uint64_t required_bytes = res.size();
      if (use_buffer &&
          (required_bytes < STORAGE_BUFFER_SIZE)) { //buffering
        required_bytes = STORAGE_BUFFER_SIZE;
        uint64_t read_to_pos = std::min(required_bytes, num_bytes_remaining_of_file); //ensure we do not read past file_size

        read(fd, buffers[data_type][exec_id].data(), read_to_pos);

        std::copy(buffers[data_type][exec_id].begin(), buffers[data_type][exec_id].begin() + res.size(), res.data());

        buffered_components[data_type][exec_id] = full_file_name;
        buffered_interval[data_type][exec_id] = std::make_pair(read_location, read_to_pos);

      } else { //no buffering

        read(fd, res.data(), required_bytes);
      }

      close(fd);

    }
  } else {
    std::copy(buffer_map[full_file_name].begin() + read_location, buffer_map[full_file_name].begin() + read_location + res.size(), res.data());
  }
}

std::string Storage::GetFullFileName(std::string file_name, DATA_TYPE data_type) {
  std::string full_file_name;
  if (data_type == TABLES) {
    full_file_name =  std::string(STORAGE_PREFIX + file_name + TABLES_SUFFIX);
  } else if (data_type == SOLDERINGS) {
    full_file_name =  std::string(STORAGE_PREFIX + file_name + SOLDERING_SUFFIX);
  } else if (data_type == AUXDATA) {
    full_file_name =  std::string(STORAGE_PREFIX + file_name + AUXDATA_SUFFIX);
  } else if (data_type == VERTICAL_SOLDERINGS) {
    full_file_name =  std::string(STORAGE_PREFIX + file_name + VERTICAL_SOLDERINGS_SUFFIX);
  } else if (data_type == AUTHS) {
    full_file_name =  std::string(STORAGE_PREFIX + file_name + AUTHS_SUFFIX);
  } else if (data_type == AUTHS_SOLDERINGS) {
    full_file_name =  std::string(STORAGE_PREFIX + file_name + AUTHS_SOLDERINGS_SUFFIX);
  } else if (data_type == AUTHS_DELTA_SOLDERINGS) {
    full_file_name =  std::string(STORAGE_PREFIX + file_name + AUTHS_DELTA_SOLDERINGS_SUFFIX);
  } else if (data_type == AUTHS_IDS) {
    full_file_name =  std::string(STORAGE_PREFIX + file_name + AUTHS_IDS_SUFFIX);
  } else if (data_type == INPUT_MASKS_AUXDATA) {
    full_file_name =  std::string(STORAGE_PREFIX + file_name + INPUT_MASKS_AUXDATA_SUFFIX);
  } else if (data_type == INPUT_MASKS_DELTA_AUXDATA) {
    full_file_name =  std::string(STORAGE_PREFIX + file_name + INPUT_MASKS_DELTA_AUXDATA_SUFFIX);
  } else if (data_type == INPUT_MASKS_CORRECTIONS) {
    full_file_name =  std::string(STORAGE_PREFIX + file_name + INPUT_MASKS_CORRECTIONS_SUFFIX);
  } else if (data_type == INPUT_MASKS_DELTA_CORRECTION) {
    full_file_name =  std::string(STORAGE_PREFIX + file_name + INPUT_MASKS_DELTA_CORRECTION_SUFFIX);
  } else if (data_type == INPUT_PERM_BITS) {
    full_file_name =  std::string(STORAGE_PREFIX + file_name + INPUT_PERM_BITS_SUFFIX);
  } else if (data_type == RECOV) {
    full_file_name =  std::string(STORAGE_PREFIX + file_name + RECOV_SUFFIX);
  } else if (data_type == RECOV_SOLD) {
    full_file_name =  std::string(STORAGE_PREFIX + file_name + RECOV_DATA_SUFFIX);
  } else if (data_type == OUTPUT_MASKS_AUXDATA) {
    full_file_name =  std::string(STORAGE_PREFIX + file_name + OUTPUT_MASKS_AUXDATA_SUFFIX);
  } else {
    std::cout << "error reading/writing file. Bad filename." << std::endl;
    exit(EXIT_FAILURE);
  }

  return full_file_name;
}
