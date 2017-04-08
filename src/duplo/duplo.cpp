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
  uint64_t res = chan.getTotalDataSent();

  for (int i = 0; i < exec_channels.size(); ++i) {
    res += exec_channels[i].getTotalDataSent();
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