#ifndef DUPLO_DUPLO_STORAGE_H_
#define DUPLO_DUPLO_STORAGE_H_

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#include "duplo-util/util.h"

enum DATA_TYPE {
  TABLES = 0,
  SOLDERINGS = 1,
  AUXDATA = 2,
  VERTICAL_SOLDERINGS = 3,
  AUTHS = 4,
  AUTHS_SOLDERINGS = 5,
  AUTHS_DELTA_SOLDERINGS = 6,
  AUTHS_IDS = 7,
  INPUT_MASKS_AUXDATA = 8,
  INPUT_MASKS_DELTA_AUXDATA = 9,
  INPUT_MASKS_CORRECTIONS = 10,
  INPUT_MASKS_DELTA_CORRECTION = 11,
  INPUT_PERM_BITS = 12,
  RECOV = 13,
  RECOV_SOLD = 14,
  OUTPUT_MASKS_AUXDATA = 15
};

const uint32_t NUM_DATA_TYPES = 16;
const uint32_t STORAGE_BUFFER_SIZE = 8192;

class Storage {

public:
  
  Storage(uint32_t num_max_execs, bool ram_only);

  void PrepareFile(std::string file_name, DATA_TYPE data_type, uint64_t num_prepare_bytes);

  void WriteBuckets(std::string file_name, DATA_TYPE data_type, uint32_t buckets_from, uint32_t num_buckets, uint8_t data_to_write[], uint64_t write_pos, uint64_t num_bytes, uint32_t bucket_size);
  
  // void OverWriteBuckets(std::string file_name, DATA_TYPE data_type, uint32_t buckets_from, uint32_t num_buckets, uint8_t data_to_write[], uint64_t write_pos, uint64_t num_bytes, uint32_t bucket_size);

  void ReadBuckets(std::string file_name, DATA_TYPE data_type, uint32_t buckets_from, uint32_t num_buckets, BYTEArrayVector& res, bool use_buffer = false, uint32_t exec_id = std::numeric_limits<uint32_t>::max());

  bool ram_only;
  std::unordered_map<std::string, std::vector<uint8_t>> buffer_map;

  std::mutex map_mutex;
  
  std::unordered_map<std::string, uint64_t> file_size_map;
  
  std::unordered_map<std::tuple<std::string, uint32_t>, uint64_t> filepos_map;
  std::unordered_map<std::tuple<std::string, uint32_t>, uint64_t> bucket_bytes_map;

  std::vector<std::vector<std::array<uint8_t, STORAGE_BUFFER_SIZE>>> buffers;
  std::vector<std::vector<std::string>> buffered_components;
  std::vector<std::vector<std::pair<uint32_t, uint32_t>>> buffered_interval;


  const uint64_t max_num_write_bytes = std::pow(2,30);

private:
  std::string GetFullFileName(std::string file_name, DATA_TYPE data_type);
};


#endif /* DUPLO_DUPLO_STORAGE_H_ */