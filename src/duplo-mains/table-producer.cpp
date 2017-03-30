#include "duplo/duplo.h"
#include <gmpxx.h>

void MaxSingleProb(int b, uint32_t num_buckets, mpf_class& curr_p, mpf_class& max_prob) {

  mpf_class one(1.0);
  mpf_class curr_b(b);
  mpz_class curr_num_buckets(num_buckets);
  bool max_t_found = false;
  for (int t_inc = b; t_inc <= (num_buckets * b); ++t_inc) {

    if (max_t_found) {
      break;
    }

    //Compute max t
    mpf_class t(t_inc);

    //Compute M
    mpf_class M = curr_num_buckets * curr_b;
    mpf_div(M.get_mpf_t(), M.get_mpf_t(), curr_p.get_mpf_t());
    M = ceil(M);

    //Compute m
    mpf_class m = curr_num_buckets * curr_b;

    //tmp object for binomial expressions
    mpz_class tmp3_mpz;

    // // Compute (M-t choose m-t)
    // mpf_class tmp0 = M - t;
    // mpf_class tmp1 = m - t;
    // mpz_bin_uiui(tmp3_mpz.get_mpz_t(), tmp0.get_ui(), tmp1.get_ui());
    // mpf_class tmp3 = tmp3_mpz;

    // // Compute (M choose m)
    // mpz_bin_uiui (tmp3_mpz.get_mpz_t(), M.get_ui(), m.get_ui());
    // mpf_class tmp4 = tmp3_mpz;

    // //Compute (M-t choose m-t) / (M choose m)
    // mpf_class tmp5 = tmp3 / tmp4;

    // Compute (1 - p)^t
    mpf_class tmp5 = one - curr_p;
    mpf_pow_ui(tmp5.get_mpf_t(), tmp5.get_mpf_t(), t.get_ui());

    //Compute (t choose b)
    mpf_class tmp6;
    mpz_bin_uiui (tmp3_mpz.get_mpz_t(), t.get_ui(), curr_b.get_ui());
    tmp6 = tmp3_mpz;

    //Compute 1/(m choose b)
    mpf_class tmp7;
    mpz_bin_uiui (tmp3_mpz.get_mpz_t(), m.get_ui(), curr_b.get_ui());
    tmp7 = one / tmp3_mpz;

    //Compute the result
    mpf_class res = tmp5 * curr_num_buckets * tmp6 * tmp7;

    if (mpf_cmp(res.get_mpf_t(), max_prob.get_mpf_t()) != -1) {
      max_prob = res;
    } else {
      max_t_found = true;
    }
  }
}

void FindBestSingleParams(uint32_t num_buckets, uint32_t& res_bucket, long double& check_val, bool& check_val_negate) {

  uint32_t upper_bucket_bound = 41;
  uint32_t lower_check_p_bound = 10;
  uint32_t lower_check_p_inv_bound = 2;
  uint32_t num_inv_iterations = 0;
  mpz_class target_prob_inv;
  mpz_class base(2);
  mpf_class base_mpf(2);
  mpf_class one(1.0);
  mpz_pow_ui(target_prob_inv.get_mpz_t(), base.get_mpz_t(), SSEC);
  mpf_class target_prob = one / target_prob_inv;


  for (uint32_t b = upper_bucket_bound; b > 1; --b) {
    bool success = false;
    mpf_class curr_p;
    for (int check_p = lower_check_p_bound; check_p > 0; --check_p) {

      if (success) {
        //Ensures that we always choose the largest p that satisfies security.
        //Skip any consecutive smaller p values as we already found one that works and performance decreases as p decreases.
        break;
      }

      mpf_pow_ui(curr_p.get_mpf_t(), base_mpf.get_mpf_t(), check_p);
      curr_p = one / curr_p;
      mpf_class max_prob(0.0); //set/reset max_prob

      MaxSingleProb(b, num_buckets, curr_p, max_prob);

      //Check if current params satisfy security
      if ((max_prob != 0) && (max_prob < target_prob)) {
        res_bucket = b;
        check_val = check_p;
        check_val_negate = false;
        success = true;

        double check_prob = (1 / pow(2, check_val));

        // std::cout << "bucket size b=" << res_bucket << ", with check_prob=" << check_prob << " possible" << std::endl;
      }
    }

    for (int check_p = lower_check_p_inv_bound; check_p < (lower_check_p_inv_bound + num_inv_iterations); ++check_p) {

      if (success) {
        //Ensures that we always choose the largest p that satisfies security.
        //Skip any consecutive smaller p values as we already found one that works and performance decreases as p decreases.
        break;
      }

      mpf_pow_ui(curr_p.get_mpf_t(), base_mpf.get_mpf_t(), check_p);
      curr_p = one - (one / curr_p); //Not sure works!
      mpf_class max_prob(0.0);
      MaxSingleProb(b, num_buckets, curr_p, max_prob);

      //Check if current params satisfy security
      if ((max_prob != 0) && (max_prob < target_prob)) {
        res_bucket = b;
        check_val = check_p;
        check_val_negate = true;
        success = true;

        double check_prob = 1 - (1 / pow(2, check_val));

        // std::cout << "bucket size b=" << res_bucket << ", with check_prob=" << check_prob << " possible" << std::endl;
      }
    }
  }
}

void MaxProbMajority(int b, uint32_t num_buckets, mpf_class& curr_p, mpf_class& max_prob, uint32_t catch_reciproc_prob) {

  mpf_class one(1.0);
  mpf_class curr_b(b);
  mpf_class base_mpf(2);
  mpf_class catch_prob(catch_reciproc_prob);
  mpz_class curr_num_buckets(num_buckets);
  bool max_t_found = false;
  for (int t_inc = ceil(b / 2); t_inc <= (num_buckets * b); ++t_inc) {

    if (max_t_found) {
      break;
    }

    mpf_class t(t_inc);

    // Compute (1 - p/2)^t
    mpf_class tmp0 = one - (curr_p / catch_prob);
    mpf_pow_ui(tmp0.get_mpf_t(), tmp0.get_mpf_t(), t.get_ui());

    // Compute 2^(b-1)
    mpf_class tmp1;
    mpf_pow_ui(tmp1.get_mpf_t(), base_mpf.get_mpf_t(), b - 1);

    //Compute m
    mpf_class m = curr_num_buckets * curr_b;

    //Compute (t / m)
    mpf_class tmp2 = t / m;

    //Compute (t / m)^(b/2)
    mpf_class tmp3 = ceil(curr_b / 2);
    mpf_pow_ui(tmp2.get_mpf_t(), tmp2.get_mpf_t(), tmp3.get_ui());

    //Compute the result
    mpf_class res = tmp0 * curr_num_buckets * tmp1 * tmp2;

    if (mpf_cmp(res.get_mpf_t(), max_prob.get_mpf_t()) != -1) {
      max_prob = res;
    } else {
      max_t_found = true;
    }
  }
}

void FindBestMajorityParams(uint32_t num_buckets, uint32_t& res_bucket, long double& check_val, bool& check_val_negate, uint32_t catch_reciproc_prob) {

  uint32_t upper_bucket_bound = 87;
  uint32_t lower_check_p_bound = 10;
  uint32_t lower_check_p_inv_bound = 2;
  uint32_t num_inv_iterations = 0;
  mpz_class target_prob_inv;
  mpz_class base(2);
  mpf_class base_mpf(2);
  mpf_class one(1.0);
  mpz_pow_ui(target_prob_inv.get_mpz_t(), base.get_mpz_t(), SSEC);
  mpf_class target_prob = one / target_prob_inv;

  for (uint32_t b = upper_bucket_bound; b > 1; --b) {
    bool success = false;
    mpf_class curr_p;
    for (int check_p = lower_check_p_bound; check_p > 0; --check_p) {

      if (success) {
        //Ensures that we always choose the largest p that satisfies security.
        //Skip any consecutive smaller p values as we already found one that works and performance decreases as p decreases.
        break;
      }

      //Compute 2^check_p
      mpf_pow_ui(curr_p.get_mpf_t(), base_mpf.get_mpf_t(), check_p);
      curr_p = one / curr_p;
      mpf_class max_prob(0.0); //set/reset max_prob
      MaxProbMajority(b, num_buckets, curr_p, max_prob, catch_reciproc_prob);

      //Check if current params satisfy security
      if ((max_prob != 0) && (max_prob < target_prob)) {
        res_bucket = b;
        check_val = check_p;
        check_val_negate = false;
        success = true;

        double check_prob = (1 / pow(2, check_val));

        // std::cout << "bucket size b=" << res_bucket << ", with check_prob=" << check_prob << " possible" << std::endl;
      }
    }

    for (int check_p = lower_check_p_inv_bound; check_p < (lower_check_p_inv_bound + num_inv_iterations); ++check_p) {

      if (success) {
        //Ensures that we always choose the largest p that satisfies security.
        //Skip any consecutive smaller p values as we already found one that works and performance decreases as p decreases.
        break;
      }

      mpf_pow_ui(curr_p.get_mpf_t(), base_mpf.get_mpf_t(), check_p);
      curr_p = one - (one / curr_p);
      mpf_class max_prob(0.0);
      MaxProbMajority(b, num_buckets, curr_p, max_prob, catch_reciproc_prob);

      //Check if current params satisfy security
      if ((max_prob != 0) && (max_prob < target_prob)) {
        res_bucket = b;
        check_val = check_p;
        check_val_negate = true;
        success = true;

        double check_prob = 1 - (1 / pow(2, check_val));

        // std::cout << "bucket size b=" << res_bucket << ", with check_prob=" << check_prob << " possible" << std::endl;
      }
    }
  }
}

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