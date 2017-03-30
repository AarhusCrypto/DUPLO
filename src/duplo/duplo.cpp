#include "duplo/duplo.h"

Duplo::Duplo(uint8_t seed[], uint32_t num_max_parallel_execs, bool ram_only) :
  thread_pool(TP_MUL_FACTOR * std::thread::hardware_concurrency()),
  curr_num_ready_inputs(0),
  inputs_used(0),
  exec_rnds(num_max_parallel_execs),
  persistent_storage(num_max_parallel_execs, ram_only),
  ios(0) {

  rnd.SetSeed(load_block(seed));

  uint8_t tmp_seed[CSEC_BYTES];
  for (int e = 0; e < num_max_parallel_execs; ++e) {
    rnd.get<uint8_t>(tmp_seed, CSEC_BYTES);
    exec_rnds[e].SetSeed(load_block(tmp_seed));
  }

  inp_bucket_circuit.num_inp_wires = 1;
  inp_bucket_circuit.num_wires = 1;
  inp_bucket_circuit.num_out_wires = 0;
  inp_bucket_circuit.num_non_free_gates = 1; //makes space for the wire auths. 1 non_free_gate is equal one wire auth
  inp_bucket_circuit.num_gates = 0;

  //Remove any potential previously constructed garbling material
  std::string delete_storage;
  delete_storage = "rm -rf " + STORAGE_PREFIX;
  system(delete_storage.c_str());
  std::string create_storage;
  create_storage = "mkdir " + STORAGE_PREFIX;
  system(create_storage.c_str());
}

Duplo::~Duplo() {
  ios.stop();
}

uint64_t Duplo::GetTotalDataSent() {
  uint64_t res = chan->getTotalDataSent();

  for (int i = 0; i < exec_channels.size(); ++i) {
    res += exec_channels[i]->getTotalDataSent();
  }

  return res;
}

uint32_t Duplo::GetTotalNumberOfConstInputWires(std::vector<std::pair<std::string, uint32_t>>& input_components, uint32_t from, uint32_t to) {

  if (from == -1 && to == -1) {
    from = 0;
    to = input_components.size();
  }

  uint32_t num_total_const_inputs = 0;
  for (int i = from; i < to; ++i) {
    std::string component_name = std::get<0>(input_components[i]);
    Circuit& circuit = string_to_circuit_map[component_name];
    num_total_const_inputs += circuit.num_const_inp_wires;
  }

  return num_total_const_inputs;
}

uint32_t Duplo::GetTotalNumberOfEvalInputWires(std::vector<std::pair<std::string, uint32_t>>& input_components, uint32_t from, uint32_t to) {

  if (from == -1 && to == -1) {
    from = 0;
    to = input_components.size();
  }

  uint32_t num_total_eval_inputs = 0;
  for (int i = from; i < to; ++i) {
    std::string component_name = std::get<0>(input_components[i]);
    Circuit& circuit = string_to_circuit_map[component_name];
    num_total_eval_inputs += circuit.num_eval_inp_wires;
  }

  return num_total_eval_inputs;
}

uint32_t Duplo::GetTotalNumberOfInputWires(std::vector<std::pair<std::string, uint32_t>>& input_components, uint32_t from, uint32_t to) {

  return GetTotalNumberOfConstInputWires(input_components, from, to) + GetTotalNumberOfEvalInputWires(input_components, from, to);
}

uint32_t Duplo::GetTotalNumberOfConstOutputWires(std::vector<std::pair<std::string, uint32_t>>& output_components, uint32_t from, uint32_t to) {

  if (from == -1 && to == -1) {
    from = 0;
    to = output_components.size();
  }

  uint32_t num_total_const_outputs = 0;
  for (int i = from; i < to; ++i) {
    std::string component_name = std::get<0>(output_components[i]);
    Circuit& circuit = string_to_circuit_map[component_name];
    num_total_const_outputs += circuit.num_const_out_wires;
  }

  return num_total_const_outputs;
}

uint32_t Duplo::GetTotalNumberOfEvalOutputWires(std::vector<std::pair<std::string, uint32_t>>& output_components, uint32_t from, uint32_t to) {

  if (from == -1 && to == -1) {
    from = 0;
    to = output_components.size();
  }

  uint32_t num_total_eval_outputs = 0;
  for (int i = from; i < to; ++i) {
    std::string component_name = std::get<0>(output_components[i]);
    Circuit& circuit = string_to_circuit_map[component_name];
    num_total_eval_outputs += circuit.num_eval_out_wires;
  }

  return num_total_eval_outputs;
}

uint32_t Duplo::GetTotalNumberOfOutputWires(std::vector<std::pair<std::string, uint32_t>>& output_components, uint32_t from, uint32_t to) {

  if (from == -1 && to == -1) {
    from = 0;
    to = output_components.size();
  }

  uint32_t num_total_outputs = 0;
  for (int i = from; i < to; ++i) {
    std::string component_name = std::get<0>(output_components[i]);
    Circuit& circuit = string_to_circuit_map[component_name];
    num_total_outputs += circuit.num_out_wires;
  }

  return num_total_outputs;
}

uint32_t Duplo::GetNumberOfOutputWires(std::vector<std::vector<uint32_t>>& indices, uint32_t from, uint32_t to) {

  uint32_t num_total_outputs = 0;
  for (int i = from; i < to; ++i) {
    num_total_outputs += indices[i].size();
  }

  return num_total_outputs;
}

void WeightedRandomString(uint8_t res[], int weight, int res_length, osuCrypto::PRNG& rnd, bool negate_probability) {

  uint8_t temp[res_length];
  std::fill(res, res + res_length, 0xFF);
  while (weight > 0) {
    rnd.get<uint8_t>(temp, res_length);
    for (unsigned int i = 0; i < res_length; ++i) {
      res[i] &= temp[i];
    }
    weight--;
  }
  if (negate_probability) {
    for (int i = 0; i < res_length; ++i) {
      res[i] = ~res[i];
    }
  }
}

void ComputeIndices(uint32_t num_circuits, Circuit& circuit, uint32_t& num_inp_keys, uint32_t& num_out_keys, uint32_t& num_deltas, uint32_t& num_commit_keys, uint32_t& num_base_keys, uint32_t& input_keys_idx, uint32_t& output_keys_idx, uint32_t& deltas_idx) {
  num_inp_keys = num_circuits * circuit.num_inp_wires;
  num_out_keys = num_circuits * circuit.num_out_wires;
  num_deltas = num_circuits;
  num_commit_keys = num_inp_keys + num_out_keys + num_deltas;
  num_base_keys = num_inp_keys + num_out_keys;

  input_keys_idx = 0;
  output_keys_idx = num_circuits * circuit.num_inp_wires;
  deltas_idx = num_circuits * (circuit.num_inp_wires + circuit.num_out_wires);
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



void ComputeCheckFraction(int check_frac, int num_items, float &final_rep_frac, bool negate_check_probability) {

  float slack_frac;

  int p_g_nom = 1;

  float p_g_tmp;
  if (negate_check_probability) {
    p_g_tmp = 1.0 - (float) p_g_nom / (float) (1 << check_frac);

  } else {
    p_g_tmp = (float) p_g_nom / (float) (1 << check_frac);
  }

  const double _a_g = 2 * num_items;
  const double _b = SSEC * log(2);

  double _c = SSEC * log(2) * (p_g_tmp - 1);
  double det = _b * _b - 4 * _a_g * _c;
  if (det < 0.0) {
    printf("ERROR: Bad parameters, check_frac slack equation has no real roots");
    exit(EXIT_FAILURE);
  } else {
    // two real roots (possibly equal)
    double r1 = ( -_b + sqrt(det)) / (2 * _a_g);
    double r2 = ( -_b - sqrt(det)) / (2 * _a_g);
//        debug("slack_frac two roots: %f, %f", r1, r2);
    if (r1 > 0 && r2 > 0) {
      slack_frac = r1 < r2 ? r1 : r2;
    } else if (r1 <= 0 && r2 > 0) {
      slack_frac = r2;
    } else if (r1 > 0 && r2 <= 0) {
      slack_frac = r1;
    } else {
      printf("ERROR: Bad parameters, p_g slack equation has no real roots");
      exit(EXIT_FAILURE);
    }
  }

  // std::cout << "For " << num_items << " items, slack is " << slack_frac << " which is, " << (p_g_tmp+slack_frac)/p_g_tmp << " more than " << p_g_tmp  << std::endl;

  final_rep_frac = 1 / (1 - p_g_tmp - slack_frac);
}

void PickBestSingleBucketParams(uint32_t num_buckets, uint32_t& bucket_size, long double& check_factor, bool& negate_check_factor) {
  bool found = false;
  for (int i = single_bucket_param_table_size - 1; i >= 0; --i) {
    if (num_buckets >= single_bucket_param_table[i][0]) {
      bucket_size = single_bucket_param_table[i][1];
      check_factor = single_bucket_param_table[i][2];
      negate_check_factor = single_bucket_param_table[i][3];
      found = true;
      break; //Do not try worse parameters
    }
  }

  if (!found) {
    throw std::runtime_error("Num buckets too low!");
    std::cout << "Num buckets too low: " << num_buckets << std::endl;
  }
}

void PickBestMajorityBucketParams(uint32_t num_buckets, uint32_t& bucket_size, long double& check_factor, bool& negate_check_factor) {
  bool found = false;
  for (int i = majority_bucket_param_table_size - 1; i >= 0; --i) {
    if (num_buckets >= majority_bucket_param_table[i][0]) {
      bucket_size = majority_bucket_param_table[i][1];
      check_factor = majority_bucket_param_table[i][2];
      negate_check_factor = majority_bucket_param_table[i][3];
      found = true;
      break; //Do not try worse parameters
    }
  }

  if (!found) {
    throw std::runtime_error("Num buckets too low!");
    std::cout << "Num buckets too low: " << num_buckets << std::endl;
  }
}

void PickBestMajorityAuthParams(uint32_t num_buckets, uint32_t& auth_size, long double& check_factor, bool& negate_check_factor) {
  bool found = false;
  for (int i = majority_auth_param_table_size - 1; i >= 0; --i) {
    if (num_buckets >= majority_auth_param_table[i][0]) {
      auth_size = majority_auth_param_table[i][1];
      check_factor = majority_auth_param_table[i][2];
      negate_check_factor = majority_auth_param_table[i][3];
      found = true;
      break; //Do not try worse parameters
    }
  }

  if (!found) {
    throw std::runtime_error("Num buckets too low!");
    std::cout << "Num buckets too low: " << num_buckets << std::endl;
  }
}