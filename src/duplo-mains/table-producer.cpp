#include "duplo/duplo.h"

int main(int argc, const char* argv[]) {
  uint32_t num_parallel_execs = std::thread::hardware_concurrency();
  ctpl::thread_pool thread_pool(num_parallel_execs);
  std::mutex mutex;
  
  if (*argv[1] == '0') {
    std::cout << "single" << std::endl;
    uint32_t single_upper_bound = 10000000;
    uint32_t inc_number = 100;
    std::unordered_map<uint32_t, std::string> single_map(single_upper_bound);
    std::vector<std::future<void>> single_futures(num_parallel_execs);
    std::vector<int> single_i_from, single_i_to;
    PartitionBufferFixedNum(single_i_from, single_i_to, num_parallel_execs, single_upper_bound);
    for (int e = 0; e < num_parallel_execs; ++e) {

      single_futures[e] = thread_pool.push([&single_map, &single_i_from, &single_i_to, &mutex, e, inc_number] (int id) {
        long double prev_check_factor = 0;
        uint32_t prev_bucket_size = 0;
        bool prev_negate_check_factor = 0;
        uint32_t num_calls = single_i_to[e] - single_i_from[e];

        for (int i = single_i_from[e]; i < single_i_to[e]; ++i) {
          long double check_factor;
          uint32_t bucket_size;
          bool negate_check_factor;
          FindBestSingleParams(i, bucket_size, check_factor, negate_check_factor);

          if (check_factor != prev_check_factor ||
              bucket_size != prev_bucket_size ||
              negate_check_factor != prev_negate_check_factor) {
            mutex.lock();
            single_map.emplace(i, "{" + std::to_string(i) + ", " + std::to_string(bucket_size) + ", " + std::to_string((int) check_factor) + ", " + std::to_string(negate_check_factor) + "},");
            mutex.unlock();
            prev_check_factor = check_factor;
            prev_bucket_size = bucket_size;
            prev_negate_check_factor = negate_check_factor;
          }

          if ((i % 100) == 0) {
            mutex.lock();
            std::cout << "exec " << e << ": " << i << "/" << single_i_to[e] << std::endl;
            mutex.unlock();
          }

          if (i > 150 && i <= 1000) {
            i += inc_number;
          } else if (i > 1000 && i <= 10000) {
            i += 5 * inc_number;
          } else if (i > 10000) {
            i += 50 * inc_number;
          }
        }
      });
    }

    for (std::future<void>& f : single_futures) {
      f.wait();
    }

    std::vector<uint32_t> single_keys;
    for (auto& it : single_map) {
      single_keys.emplace_back(it.first);
    }
    std::sort(single_keys.begin(), single_keys.end());
    for (auto& it : single_keys) {
      std::cout << single_map[it] << std::endl;
    }

  } else if (*argv[1] == '1') {
    std::cout << "majority" << std::endl;
    uint32_t majority_upper_bound = 10000000;
    uint32_t inc_number = 100;
    std::unordered_map<uint32_t, std::string> majority_map(majority_upper_bound);
    std::vector<std::future<void>> majority_futures(num_parallel_execs);
    std::vector<int> majority_i_from, majority_i_to;
    PartitionBufferFixedNum(majority_i_from, majority_i_to, num_parallel_execs, majority_upper_bound);
    for (int e = 0; e < num_parallel_execs; ++e) {

      majority_futures[e] = thread_pool.push([&majority_map, &majority_i_from, &majority_i_to, &mutex, e, inc_number] (int id) {
        long double prev_check_factor = 0;
        uint32_t prev_bucket_size = 0;
        bool prev_negate_check_factor = 0;
        for (int i = majority_i_from[e]; i < majority_i_to[e]; ++i) {
          long double check_factor;
          uint32_t bucket_size;
          bool negate_check_factor;
          FindBestMajorityParams(i, bucket_size, check_factor, negate_check_factor, 1);

          if (check_factor != prev_check_factor ||
              bucket_size != prev_bucket_size ||
              negate_check_factor != prev_negate_check_factor) {
            mutex.lock();
            majority_map.emplace(i, "{" + std::to_string(i) + ", " + std::to_string(bucket_size) + ", " + std::to_string((int) check_factor) + ", " + std::to_string(negate_check_factor) + "},");
            mutex.unlock();
            prev_check_factor = check_factor;
            prev_bucket_size = bucket_size;
            prev_negate_check_factor = negate_check_factor;
          }

          if ((i % 100) == 0) {
            mutex.lock();
            std::cout << "exec " << e << ": " << i << "/" << majority_i_to[e] << std::endl;
            mutex.unlock();
          }

          if (i > 150 && i <= 1000) {
            i += inc_number;
          } else if (i > 1000 && i <= 10000) {
            i += 5 * inc_number;
          } else if (i > 10000) {
            i += 50 * inc_number;
          }
        }
      });
    }

    for (std::future<void>& f : majority_futures) {
      f.wait();
    }

    std::vector<uint32_t> majority_keys;
    for (auto& it : majority_map) {
      majority_keys.emplace_back(it.first);
    }
    std::sort(majority_keys.begin(), majority_keys.end());
    for (auto& it : majority_keys) {
      std::cout << majority_map[it] << std::endl;
    }

  } else if (*argv[1] == '2') {
    std::cout << "auths" << std::endl;
    uint32_t auths_upper_bound = 10000000;
    uint32_t inc_number = 100;
    //Majority
    std::unordered_map<uint32_t, std::string> auths_map(auths_upper_bound);
    std::vector<std::future<void>> auths_futures(num_parallel_execs);
    std::vector<int> auths_i_from, auths_i_to;
    PartitionBufferFixedNum(auths_i_from, auths_i_to, num_parallel_execs, auths_upper_bound);
    for (int e = 0; e < num_parallel_execs; ++e) {

      auths_futures[e] = thread_pool.push([&auths_map, &auths_i_from, &auths_i_to, &mutex, e, inc_number] (int id) {
        long double prev_check_factor = 0;
        uint32_t prev_bucket_size = 0;
        bool prev_negate_check_factor = 0;
        for (int i = auths_i_from[e]; i < auths_i_to[e]; ++i) {
          long double check_factor;
          uint32_t bucket_size;
          bool negate_check_factor;
          FindBestMajorityParams(i, bucket_size, check_factor, negate_check_factor, 2);

          if (check_factor != prev_check_factor ||
              bucket_size != prev_bucket_size ||
              negate_check_factor != prev_negate_check_factor) {
            mutex.lock();
            auths_map.emplace(i, "{" + std::to_string(i) + ", " + std::to_string(bucket_size) + ", " + std::to_string((int) check_factor) + ", " + std::to_string(negate_check_factor) + "},");
            mutex.unlock();
            prev_check_factor = check_factor;
            prev_bucket_size = bucket_size;
            prev_negate_check_factor = negate_check_factor;
          }

          if ((i % 100) == 0) {
            mutex.lock();
            std::cout << "exec " << e << ": " << i << "/" << auths_i_to[e] << std::endl;
            mutex.unlock();
          }

          if (i > 150 && i <= 1000) {
            i += inc_number;
          } else if (i > 1000 && i <= 10000) {
            i += 5 * inc_number;
          } else if (i > 10000) {
            i += 50 * inc_number;
          }
        }
      });
    }

    for (std::future<void>& f : auths_futures) {
      f.wait();
    }

    std::vector<uint32_t> auths_keys;
    for (auto& it : auths_map) {
      auths_keys.emplace_back(it.first);
    }
    std::sort(auths_keys.begin(), auths_keys.end());
    for (auto& it : auths_keys) {
      std::cout << auths_map[it] << std::endl;
    }
  } else {
    std::cout << "invalid param" << std::endl;
  }
}