#include "duplo/duplo-evaluator.h"

DuploEvaluator::DuploEvaluator(uint8_t seed[], uint32_t num_max_parallel_execs, bool ram_only) :
  Duplo(seed, num_max_parallel_execs, ram_only),
  commit_seed_OTs(NUM_COMMIT_SEED_OT),
  commit_seed_choices(NUM_COMMIT_SEED_OT) {

}

DuploEvaluator::~DuploEvaluator() {

  chan.close();

  for (int e = 0; e < exec_channels.size(); ++e) {
    exec_channels[e].close();
  }

  end_point.stop();
}

void DuploEvaluator::Connect(std::string ip_address, uint16_t port) {

  end_point.start(ios, ip_address, port, osuCrypto::EpMode::Client, "ep");

  chan = end_point.addChannel("chan", "chan");

  for (int e = 0; e < exec_rnds.size(); ++e) {
    exec_channels.emplace_back(end_point.addChannel("exec_channel_" + std::to_string(e), "exec_channel_" + std::to_string(e)));
  }
}

void DuploEvaluator::Setup() {

  //BaseOTs
  osuCrypto::u64 num_base_OTs = CSEC + SSEC;
  std::vector<std::array<osuCrypto::block, 2>> base_ots(num_base_OTs);

  osuCrypto::NaorPinkas baseOTs;

  baseOTs.send(base_ots, rnd, chan, 1);

  //Extended the base ots and set them for each dot_receiver
  osuCrypto::KosDotExtReceiver temp_dot_reciever;
  temp_dot_reciever.setBaseOts(base_ots);

  for (int exec_id = 0; exec_id < exec_rnds.size(); ++exec_id) {
    dot_receivers.emplace_back(temp_dot_reciever.split());
  }

  //Extended one last time to setup a kos receiver
  osuCrypto::KosOtExtReceiver kos_receiver;
  std::vector<std::array<osuCrypto::block, 2>> currBaseRecvOts(CSEC);
  for (uint32_t i = 0; i < CSEC; ++i) {
    currBaseRecvOts[i][0] = temp_dot_reciever.mGens[i][0].get<osuCrypto::block>();
    currBaseRecvOts[i][1] = temp_dot_reciever.mGens[i][1].get<osuCrypto::block>();
  }
  kos_receiver.setBaseOts(currBaseRecvOts);

  //Run kos OTX and store the resulting NUM_COMMIT_SEED_OT OTs appropriately
  commit_seed_choices.randomize(rnd);

  kos_receiver.receive(commit_seed_choices, commit_seed_OTs, rnd, chan);

  //Setup tmp commit_receiver
  SplitCommitReceiver string_tmp_receiver(CSEC);

  std::vector<osuCrypto::block> string_msgs(CODEWORD_BITS);
  osuCrypto::BitVector string_choices(CODEWORD_BITS);

  for (int i = 0; i < CODEWORD_BITS; ++i) {
    string_msgs[i] = commit_seed_OTs[i];
    string_choices[i] = commit_seed_choices[i];
  }

  string_tmp_receiver.SetSeedOTs(string_msgs, string_choices);
  commit_receivers = string_tmp_receiver.GetCloneReceivers(exec_rnds.size(), rnd, exec_rnds);

  BYTEArrayVector recov_share(1, CODEWORD_BYTES);
  string_tmp_receiver.Commit(recov_share, rnd, chan);

  persistent_storage.PrepareFile(EVAL_RECOV_PREFIX, RECOV, recov_share.size());
  persistent_storage.WriteBuckets(EVAL_RECOV_PREFIX, RECOV, 0, 1, recov_share.data(), 0, recov_share.size(), 1);
}

void DuploEvaluator::PreprocessComponentType(std::string component_type, Circuit& circuit, uint32_t num_buckets, uint32_t num_parallel_execs, BucketType bucket_type) {

  if (string_to_circuit_map.find(component_type) != string_to_circuit_map.end()) {
    std::cout << component_type << " already preprocessed. Current implementation only supports one preprocess call per component_type." << std::endl;

    return;
  }

  string_to_circuit_map.emplace(component_type, circuit);
  num_buckets = PAD_TO_MULTIPLE(num_buckets, num_parallel_execs);

  //Compute parameters
  long double check_factor;
  uint32_t bucket_size;
  bool negate_check_factor;

  if (bucket_type == SINGLE) {
    PickBestSingleBucketParams(num_buckets, bucket_size, check_factor, negate_check_factor);
  } else if (bucket_type == MAJORITY) {
    PickBestMajorityBucketParams(num_buckets, bucket_size, check_factor, negate_check_factor);
  } else {
    throw std::runtime_error("Abort, bad PreprocessComponentType call!");
  }

#ifdef DUPLO_PRINT

  double cnc_check_prob;
  if (negate_check_factor) {
    cnc_check_prob = 1 - (1 / pow(2, check_factor));
  } else {
    cnc_check_prob = (1 / pow(2, check_factor));
  }

  std::cout << "bucket_size=" << bucket_size << ", " << "cnc_check_prob=" << cnc_check_prob << std::endl;
#endif

  auto component_commit_cnc_begin = GET_TIME();

  uint32_t num_eval_circuits = bucket_size * num_buckets;

  std::vector<std::future<void>> execs_finished(num_parallel_execs);

  std::vector<int> eval_circuits_from, eval_circuits_to, buckets_from, buckets_to;

  PartitionBufferFixedNum(buckets_from, buckets_to, num_parallel_execs, num_buckets);
  PartitionBufferFixedNum(eval_circuits_from, eval_circuits_to, num_parallel_execs, num_eval_circuits);

  std::vector<EvalGarbledCircuit> aux_garbled_circuits_data(num_eval_circuits, EvalGarbledCircuit(circuit, 0));

  BYTEArrayVector eval_hash(num_eval_circuits, osuCrypto::SHA1::HashSize);

  for (int exec_id = 0; exec_id < num_parallel_execs; ++exec_id) {

    //Starts the current execution
    execs_finished[exec_id] = thread_pool.push([this, &aux_garbled_circuits_data, &eval_hash, &eval_circuits_from, &eval_circuits_to, exec_id, &circuit, check_factor, negate_check_factor] (int id) {

      uint32_t exec_num_buckets = eval_circuits_to[exec_id] - eval_circuits_from[exec_id];

      float repl_factor;
      ComputeCheckFraction(check_factor, exec_num_buckets, repl_factor, negate_check_factor);

      uint32_t exec_num_total_circuits = ceil(repl_factor * exec_num_buckets);

      CommitReceiveAndCutAndChoose(exec_id, circuit, exec_num_total_circuits, check_factor, negate_check_factor, eval_circuits_from[exec_id], eval_circuits_to[exec_id], aux_garbled_circuits_data, eval_hash);
    });
  }

  for (std::future<void>& r : execs_finished) {
    r.wait();
  }

  circuit_info.emplace_back(std::make_tuple(component_type, circuit, num_buckets));

  auto component_commit_cnc_end = GET_TIME();

#ifdef DUPLO_PRINT
  PrintTimePerUnit(component_commit_cnc_begin, component_commit_cnc_end, num_buckets, "ComponentCommitCNC");
#endif

  uint64_t tables_bytes = GarbledCircuit::TotalTableSize(circuit) * num_eval_circuits;
  uint64_t solderings_bytes = EvalGarbledCircuit::SolderingsSize(circuit) * num_eval_circuits;
  uint64_t auxdata_bytes = EvalGarbledCircuit::AuxDataSize(circuit) * num_buckets;
  uint64_t recov_data_bytes = CSEC_BYTES * num_buckets;

  persistent_storage.PrepareFile(component_type, TABLES, tables_bytes);
  persistent_storage.PrepareFile(component_type, SOLDERINGS, solderings_bytes);
  persistent_storage.PrepareFile(component_type, AUXDATA, auxdata_bytes);
  persistent_storage.PrepareFile(component_type, RECOV_SOLD, recov_data_bytes);

  uint8_t bucket_seed[CSEC_BYTES];
  rnd.get<uint8_t>(bucket_seed, CSEC_BYTES);
  chan.asyncSendCopy(bucket_seed, CSEC_BYTES);

  std::vector<uint32_t> permuted_eval_ids(num_eval_circuits);
  std::vector<uint32_t> permuted_eval_ids_inv(num_eval_circuits);
  std::iota(std::begin(permuted_eval_ids), std::end(permuted_eval_ids), 0);
  PermuteArray(permuted_eval_ids.data(), num_eval_circuits, bucket_seed);
  for (int i = 0; i < num_eval_circuits; ++i) {
    permuted_eval_ids_inv[permuted_eval_ids[i]] = i;
  }

  auto component_bucketing_begin = GET_TIME();

  for (int exec_id = 0; exec_id < num_parallel_execs; ++exec_id) {

    //Starts the current execution
    execs_finished[exec_id] = thread_pool.push([this, &aux_garbled_circuits_data, &eval_hash, &buckets_from, &buckets_to, exec_id, &circuit, bucket_size, &permuted_eval_ids_inv, &component_type] (int id) {

      BucketAndReceiveEvalCircuits(component_type, exec_id, circuit, bucket_size, permuted_eval_ids_inv, buckets_from[exec_id], buckets_to[exec_id], aux_garbled_circuits_data, eval_hash);
    });
  }

  for (std::future<void>& r : execs_finished) {
    r.wait();
  }

  auto component_bucketing_end = GET_TIME();

#ifdef DUPLO_PRINT
  PrintTimePerUnit(component_bucketing_begin, component_bucketing_end, num_buckets, "ComponentBucketing");
#endif
}

void DuploEvaluator::PrepareComponents(uint64_t num_input_auth_buckets, uint32_t num_parallel_execs) {

  PrepareOutputWireAuthenticators(num_parallel_execs);
  PrepareInputWireAuthenticators(num_input_auth_buckets, num_parallel_execs);
}

void DuploEvaluator::Build(ComposedCircuit& composed_circuit, uint32_t num_parallel_execs) {

  auto composed_solder_begin = GET_TIME();

  string_to_composed_circuit_map.emplace(composed_circuit.composed_circuit_name, composed_circuit);

  std::vector<int> components_from, components_to, eval_inputs_from, eval_inputs_to, eval_outputs_from, eval_outputs_to;
  PartitionBufferFixedNum(components_from, components_to, num_parallel_execs, composed_circuit.circuits.size());
  PartitionBufferFixedNum(eval_inputs_from, eval_inputs_to, num_parallel_execs, composed_circuit.num_eval_inp_wires);
  PartitionBufferFixedNum(eval_outputs_from, eval_outputs_to, num_parallel_execs, composed_circuit.num_eval_out_wires);

  uint32_t num_total_solderings = 0;
  for (int l = 0; l < composed_circuit.circuits.size(); ++l) {

    std::string inp_wire_component_name(std::get<0>(composed_circuit.circuits[l]));
    Circuit& circuit = string_to_circuit_map[inp_wire_component_name];

    num_total_solderings += (circuit.num_inp_wires + composed_circuit.out_wire_holders[l].size());
  }

  persistent_storage.PrepareFile(EVAL_INP_BUCKET_PREFIX, INPUT_PERM_BITS, composed_circuit.num_eval_inp_wires); //Only need composed_circuit.num_eval_inp_wires, but then indices do not match when reading component_num

  //Prepare files for OT preprocessing
  persistent_storage.PrepareFile(EVAL_PREPROCESS_PREFIX, INPUT_MASKS_AUXDATA, composed_circuit.num_eval_inp_wires * (CODEWORD_BYTES + 1));
  persistent_storage.PrepareFile(EVAL_PREPROCESS_PREFIX, INPUT_MASKS_CORRECTIONS, composed_circuit.num_eval_inp_wires * CSEC_BYTES);

  persistent_storage.PrepareFile(EVAL_PREPROCESS_PREFIX, INPUT_MASKS_DELTA_AUXDATA, CODEWORD_BYTES);
  persistent_storage.PrepareFile(EVAL_PREPROCESS_PREFIX, INPUT_MASKS_DELTA_CORRECTION, CSEC_BYTES);

  persistent_storage.PrepareFile(EVAL_PREPROCESS_PREFIX, OUTPUT_MASKS_AUXDATA, composed_circuit.num_eval_out_wires * CODEWORD_BYTES);

  persistent_storage.PrepareFile(composed_circuit.composed_circuit_name, VERTICAL_SOLDERINGS, num_total_solderings * CSEC_BYTES);

  std::vector<std::future<void>> futures;
  for (int exec_id = 0; exec_id < num_parallel_execs; ++exec_id) {

    futures.emplace_back(thread_pool.push([this, &composed_circuit, &components_from, &components_to, &eval_inputs_from, &eval_inputs_to, &eval_outputs_from, &eval_outputs_to, exec_id] (int id) {

      ReceiveAndStoreComposedSolderings(exec_id, composed_circuit, components_from[exec_id], components_to[exec_id]);

      PreprocessEvalInputOTs(exec_id, eval_inputs_from[exec_id], eval_inputs_to[exec_id]);

      PreprocessBlindOutPermBits(exec_id, eval_outputs_from[exec_id], eval_outputs_to[exec_id]);

      DecommitInpPermBits(exec_id, composed_circuit.num_const_inp_wires, eval_inputs_from[exec_id], eval_inputs_to[exec_id]);
    }));
  }

  for (std::future<void>& f : futures) {
    f.wait();
  }

  auto composed_solder_end = GET_TIME();

#ifdef DUPLO_PRINT
  PrintTimePerUnit(composed_solder_begin, composed_solder_end, 1, "ComposedSolder");
#endif

}

void DuploEvaluator::Evaluate(ComposedCircuit& composed_circuit, osuCrypto::BitVector& inputs, BYTEArrayVector& output_keys, uint32_t num_parallel_execs) {

  std::vector<int> inp_circuit_from, inp_circuit_to, const_inp_from, const_inp_to, eval_inp_from, eval_inp_to;
  PartitionBufferFixedNum(inp_circuit_from, inp_circuit_to, num_parallel_execs, composed_circuit.circuits_in_layer[0].size());
  PartitionBufferFixedNum(const_inp_from, const_inp_to, num_parallel_execs, composed_circuit.num_const_inp_wires); //used only for frigate parsed circuits
  PartitionBufferFixedNum(eval_inp_from, eval_inp_to, num_parallel_execs, composed_circuit.num_eval_inp_wires); //used only for frigate parsed circuits

  BYTEArrayVector wire_keys(composed_circuit.max_wire_index, CSEC_BYTES);
  std::vector<std::future<void>> futures;

  if (num_parallel_execs == 1) {

    uint32_t num_const_inputs_from = GetTotalNumberOfConstInputWires(
                                       composed_circuit.circuits, 0, inp_circuit_from[0]);
    uint32_t num_const_inputs_to = GetTotalNumberOfConstInputWires(
                                     composed_circuit.circuits, 0, inp_circuit_to[0]);

    uint32_t num_eval_inputs_from = GetTotalNumberOfEvalInputWires(
                                      composed_circuit.circuits, 0, inp_circuit_from[0]);
    uint32_t num_eval_inputs_to = GetTotalNumberOfEvalInputWires(
                                    composed_circuit.circuits, 0, inp_circuit_to[0]);

    if ((num_const_inputs_from == 0) &&
        (num_const_inputs_to == 0)) { //we parsed a Frigate parsed circuit in which all input wires belong to the evaluator, we therefore need to split the input directly, not the circuits
      num_const_inputs_from = const_inp_from[0];
      num_const_inputs_to = const_inp_to[0];

      num_eval_inputs_from = eval_inp_from[0];
      num_eval_inputs_to = eval_inp_to[0];
    }

    //Call below on the number of inputs the current execution requires
    GetInputs(0, composed_circuit, inputs, num_const_inputs_from, num_const_inputs_to, num_eval_inputs_from, num_eval_inputs_to, wire_keys);

    for (int l = 0; l < composed_circuit.circuits_in_layer.size(); ++l) {

      EvaluateComponent(0, composed_circuit, 0, composed_circuit.circuits_in_layer[l].size(), l, wire_keys, true);
    }

  } else {

    for (int exec_id = 0; exec_id < num_parallel_execs; ++exec_id) {

      futures.emplace_back(thread_pool.push([this, &composed_circuit, &wire_keys, &inputs, &inp_circuit_from, &inp_circuit_to, &const_inp_from, &const_inp_to, &eval_inp_from, &eval_inp_to, exec_id] (int id) {

        uint32_t num_const_inputs_from = GetTotalNumberOfConstInputWires(
                                           composed_circuit.circuits, 0, inp_circuit_from[exec_id]);
        uint32_t num_const_inputs_to = GetTotalNumberOfConstInputWires(
                                         composed_circuit.circuits, 0, inp_circuit_to[exec_id]);

        uint32_t num_eval_inputs_from = GetTotalNumberOfEvalInputWires(
                                          composed_circuit.circuits, 0, inp_circuit_from[exec_id]);
        uint32_t num_eval_inputs_to = GetTotalNumberOfEvalInputWires(
                                        composed_circuit.circuits, 0, inp_circuit_to[exec_id]);

        if ((num_const_inputs_from == 0) &&
            (num_const_inputs_to == 0)) { //we parsed a Frigate parsed circuit in which all input wires belong to the evaluator, we therefore need to split the input directly, not the circuits
          num_const_inputs_from = const_inp_from[exec_id];
          num_const_inputs_to = const_inp_to[exec_id];

          num_eval_inputs_from = eval_inp_from[exec_id];
          num_eval_inputs_to = eval_inp_to[exec_id];
        }

        //Call below on the number of inputs the current execution requires
        GetInputs(exec_id, composed_circuit, inputs, num_const_inputs_from, num_const_inputs_to, num_eval_inputs_from, num_eval_inputs_to, wire_keys);
      }));
    }

    for (std::future<void>& f : futures) {
      f.wait();
    }

    for (int l = 0; l < composed_circuit.circuits_in_layer.size(); ++l) {

      uint32_t num_layer_circuits = composed_circuit.circuits_in_layer[l].size();
      std::vector<int> layer_circuit_from, layer_circuit_to;

      uint32_t num_layer_execs = std::min(num_parallel_execs, num_layer_circuits);
      PartitionBufferFixedNum(layer_circuit_from, layer_circuit_to, num_layer_execs, num_layer_circuits);

      std::vector<std::future<void>> futures;
      for (int exec_id = 0; exec_id < num_layer_execs; ++exec_id) {

        futures.emplace_back(thread_pool.push([this, &composed_circuit, &wire_keys, &inputs, &layer_circuit_from, &layer_circuit_to, num_parallel_execs, l, exec_id] (int id) {

          EvaluateComponent(exec_id, composed_circuit, layer_circuit_from[exec_id], layer_circuit_to[exec_id], l, wire_keys, (num_parallel_execs == 1));
        }));
      }

      //Cannot continue to next layer, until this one finishes completely
      for (std::future<void>& f : futures) {
        f.wait();
      }
    }
  }

  //Write all output keys to result array
  uint32_t write_pos = 0;
  for (int i = 0; i < composed_circuit.output_circuits.size(); ++i) {
    std::string circuit_name = composed_circuit.circuits[composed_circuit.output_circuits[i]].first;
    uint32_t function_num = composed_circuit.name_to_function_num[circuit_name];
    Circuit& circuit = composed_circuit.functions[function_num];

    uint32_t read_pos = composed_circuit.out_wire_holder_to_wire_idx[composed_circuit.output_circuits[i]];

    std::copy(wire_keys[read_pos], wire_keys[read_pos + circuit.num_out_wires], output_keys[write_pos]);
    write_pos += circuit.num_out_wires;
  }
}

void DuploEvaluator::DecodeKeys(ComposedCircuit & composed_circuit, std::vector<std::vector<uint32_t>>& const_outputs, std::vector<std::vector<uint32_t>>& eval_outputs, BYTEArrayVector & output_keys, std::vector<osuCrypto::BitVector>& outputs, bool non_interactive, uint32_t num_parallel_execs) {

  //Put all output circuits in out_components
  std::vector<std::pair<std::string, uint32_t>> out_components;
  for (int i = 0; i < composed_circuit.output_circuits.size(); ++i) {
    out_components.emplace_back(composed_circuit.circuits[composed_circuit.output_circuits[i]]);
  }

  std::vector<int> out_components_from, out_components_to;
  PartitionBufferFixedNum(out_components_from, out_components_to, num_parallel_execs, out_components.size());

  std::vector<std::future<void>> futures;
  for (int exec_id = 0; exec_id < num_parallel_execs; ++exec_id) {

    futures.emplace_back(thread_pool.push([this, &output_keys, &composed_circuit, &out_components, &const_outputs, &eval_outputs, &out_components_from, &out_components_to, &outputs, non_interactive, exec_id] (int id) {

      uint32_t curr_const_write_pos = 0;

      std::vector<BYTEArrayVector> eval_output_keys;
      uint32_t exec_num_const_output_wires = GetNumberOfOutputWires(const_outputs, out_components_from[exec_id], out_components_to[exec_id]);
      BYTEArrayVector exec_const_output_keys(exec_num_const_output_wires, CSEC_BYTES);

      uint32_t exec_num_components = out_components_to[exec_id] - out_components_from[exec_id];

      for (int l = 0; l < exec_num_components; ++l) {
        uint32_t curr_component = out_components_from[exec_id] + l;

        uint32_t read_pos = GetTotalNumberOfOutputWires(out_components, 0, curr_component);

        for (int i = 0; i < const_outputs[curr_component].size(); ++i) {
          //Copy const keys for sending
          std::copy(output_keys[read_pos + const_outputs[curr_component][i]],
                    output_keys[read_pos + const_outputs[curr_component][i] + 1],
                    exec_const_output_keys[curr_const_write_pos]);
          ++curr_const_write_pos;
        }

        eval_output_keys.emplace_back(BYTEArrayVector(eval_outputs[curr_component].size(), CSEC_BYTES));

        for (int i = 0; i < eval_outputs[curr_component].size(); ++i) {
          //Copy eval keys
          std::copy(output_keys[read_pos + eval_outputs[curr_component][i]],
                    output_keys[read_pos + eval_outputs[curr_component][i] + 1],
                    eval_output_keys[l][i]);
        }
      }

      exec_channels[exec_id].send(exec_const_output_keys.data(), exec_const_output_keys.size());

      std::vector<osuCrypto::BitVector> output_decodings;
      if (!DecommitOutPermBits(exec_id, out_components, eval_outputs, out_components_from[exec_id], out_components_to[exec_id], output_decodings, non_interactive)) {
        std::cout << "Abort, DecommitOutPermBits failed!" << std::endl;
        throw std::runtime_error("Abort, DecommitOutPermBits failed!");
      }

      for (int l = 0; l < exec_num_components; ++l) {
        uint32_t curr_component = out_components_from[exec_id] + l;

        outputs[curr_component] = osuCrypto::BitVector(eval_outputs[curr_component].size());

        DecodeGarbledOutput(eval_output_keys[l], output_decodings[l], outputs[curr_component], eval_outputs[curr_component].size());
      }
    }));
  }

  for (std::future<void>& f : futures) {
    f.wait();
  }
}

void DuploEvaluator::CommitReceiveAndCutAndChoose(uint32_t exec_id, Circuit & circuit, uint32_t exec_num_total_garbled, uint32_t check_factor, bool negate_check_factor, uint32_t exec_eval_circuits_from, uint32_t exec_eval_circuits_to, std::vector<EvalGarbledCircuit>& aux_garbled_circuits_data, BYTEArrayVector & eval_hash) {

  uint32_t num_inp_keys, num_out_keys, num_deltas, num_commit_keys, num_base_keys, input_keys_idx, output_keys_idx, deltas_idx;
  ComputeIndices(exec_num_total_garbled, circuit, num_inp_keys, num_out_keys, num_deltas, num_commit_keys, num_base_keys, input_keys_idx, output_keys_idx, deltas_idx);

  BYTEArrayVector commit_keys_share(num_commit_keys, CODEWORD_BYTES);

  //Commit to keys
  if (!commit_receivers[exec_id].Commit(commit_keys_share, exec_rnds[exec_id], exec_channels[exec_id], deltas_idx)) {
    std::cout << "Abort, key commit failed!" << std::endl;;
    throw std::runtime_error("Abort, key commit failed!");
  }

  BYTEArrayVector out_wire_commit_corrections(num_out_keys, CSEC_BYTES);
  exec_channels[exec_id].recv(out_wire_commit_corrections.data(), out_wire_commit_corrections.size());

  BYTEArrayVector garb_circuit_hashes(exec_num_total_garbled, osuCrypto::SHA1::HashSize);
  exec_channels[exec_id].recv(garb_circuit_hashes.data(), garb_circuit_hashes.size());

  //Sample and send challenge seed
  uint8_t cnc_seed[CSEC_BYTES];
  rnd.get<uint8_t>(cnc_seed, CSEC_BYTES);

  exec_channels[exec_id].asyncSendCopy(cnc_seed, CSEC_BYTES);

  //Select challenge circuits based on cnc_seed
  osuCrypto::BitVector cnc_check_circuits(exec_num_total_garbled);
  osuCrypto::PRNG cnc_rand;
  cnc_rand.SetSeed(load_block(cnc_seed));

  WeightedRandomString(cnc_check_circuits.data(), check_factor, cnc_check_circuits.sizeBytes(), cnc_rand, negate_check_factor);
  uint64_t num_checked_circuits = countSetBits(cnc_check_circuits.data(), 0, exec_num_total_garbled - 1);

  uint32_t cnc_num_inp_keys, cnc_num_out_keys, cnc_num_deltas, cnc_num_commit_keys, cnc_num_base_keys, cnc_inp_keys_idx, cnc_out_keys_idx, cnc_deltas_idx;
  ComputeIndices(num_checked_circuits, circuit, cnc_num_inp_keys, cnc_num_out_keys, cnc_num_deltas, cnc_num_commit_keys, cnc_num_base_keys, cnc_inp_keys_idx, cnc_out_keys_idx, cnc_deltas_idx);

  BYTEArrayVector cnc_keys_share(cnc_num_commit_keys, CODEWORD_BYTES);

  osuCrypto::BitVector perm_bits(cnc_num_commit_keys);
  exec_channels[exec_id].recv(perm_bits);

  uint32_t current_check_circuit_idx = 0;
  uint32_t current_eval_circuit_idx = exec_eval_circuits_from;
  for (int i = 0; i < exec_num_total_garbled; ++i) {
    if (cnc_check_circuits[i]) { //Checked circuit

      //Add delta
      std::copy(commit_keys_share[deltas_idx + i], commit_keys_share[deltas_idx + (i + 1)], cnc_keys_share[cnc_deltas_idx + current_check_circuit_idx]);

      //Add inputs
      std::copy(commit_keys_share[input_keys_idx + i * circuit.num_inp_wires], commit_keys_share[input_keys_idx + (i + 1) * circuit.num_inp_wires], cnc_keys_share[cnc_inp_keys_idx + current_check_circuit_idx * circuit.num_inp_wires]);
      for (int j = 0; j < circuit.num_inp_wires; ++j) {

        if (perm_bits[cnc_inp_keys_idx + current_check_circuit_idx * circuit.num_inp_wires + j]) {
          XOR_CodeWords(cnc_keys_share[cnc_inp_keys_idx + current_check_circuit_idx * circuit.num_inp_wires + j], cnc_keys_share[cnc_deltas_idx + current_check_circuit_idx]);
        }
      }

      //Add outputs
      std::copy(commit_keys_share[output_keys_idx + i * circuit.num_out_wires], commit_keys_share[output_keys_idx + (i + 1) * circuit.num_out_wires], cnc_keys_share[cnc_out_keys_idx + current_check_circuit_idx * circuit.num_out_wires]);
      for (int j = 0; j < circuit.num_out_wires; ++j) {

        if (perm_bits[cnc_out_keys_idx + current_check_circuit_idx * circuit.num_out_wires + j]) {
          XOR_CodeWords(cnc_keys_share[cnc_out_keys_idx + current_check_circuit_idx * circuit.num_out_wires + j], cnc_keys_share[cnc_deltas_idx + current_check_circuit_idx]);
        }
      }

      ++current_check_circuit_idx;
    }
  }

  //Receive postulated values
  BYTEArrayVector cnc_keys(cnc_num_commit_keys, CSEC_BYTES);
  exec_channels[exec_id].recv(cnc_keys.data(), cnc_keys.size());

  if (!commit_receivers[exec_id].BatchDecommit(cnc_keys_share, cnc_keys, exec_rnds[exec_id], exec_channels[exec_id], true)) {
    std::cout << "Abort, cut-and-choose decommit failed!" << std::endl;
    throw std::runtime_error("Abort, cut-and-choose decommit failed!");
  }

  GarblingHandler gh;
  EvalGarbledCircuit cnc_garbled_circuit(circuit);

  BYTEArrayVector cnc_garb_circuit_hashes(num_checked_circuits, osuCrypto::SHA1::HashSize);
  BYTEArrayVector output_keys(circuit.num_out_wires, CSEC_BYTES);
  BYTEArrayVector decommitted_output_keys(circuit.num_out_wires, CSEC_BYTES);

  current_check_circuit_idx = 0; //reset counter
  current_eval_circuit_idx = exec_eval_circuits_from; //reset counter
  bool completed_eval_copy = false;
  for (int i = 0; i < exec_num_total_garbled; ++i) {
    if (cnc_check_circuits[i]) {

      //Check that lsb(Delta) == 1
      if (!GetLSB(cnc_keys[cnc_deltas_idx + current_check_circuit_idx])) {
        std::cout << "Abort, lsb(delta) was incorrect!" << std::endl;
        throw std::runtime_error("Abort, lsb(delta) was incorrect!");
      }

      // //Check that lsb(base_inp_key)==0
      for (int j = 0; j < circuit.num_inp_wires; ++j) {

        XORLSB(cnc_keys[cnc_inp_keys_idx + current_check_circuit_idx * circuit.num_inp_wires + j], perm_bits[cnc_inp_keys_idx + current_check_circuit_idx * circuit.num_inp_wires + j]);

        if (perm_bits[cnc_inp_keys_idx + current_check_circuit_idx * circuit.num_inp_wires + j] != GetLSB(cnc_keys[cnc_inp_keys_idx + current_check_circuit_idx * circuit.num_inp_wires + j])) {
          std::cout << "Abort, lsb(base_inp_key) != 0" << std::endl;
          throw std::runtime_error("Abort, lsb(base_inp_key) != 0");
        }
      }

      //Garble the circuit and store output keys to output_keys
      gh.GarbleCircuit(cnc_keys[cnc_inp_keys_idx + current_check_circuit_idx * circuit.num_inp_wires], output_keys.data(), cnc_keys[cnc_deltas_idx + current_check_circuit_idx], cnc_garbled_circuit, cnc_garb_circuit_hashes[current_check_circuit_idx]);

      //Compute the decomitted output wires using out_wire_commit_corrections and the decomitted values in cnc_keys
      for (int j = 0; j < circuit.num_out_wires; ++j) {

        XORLSB(cnc_keys[cnc_out_keys_idx + current_check_circuit_idx * circuit.num_out_wires + j], perm_bits[cnc_out_keys_idx + current_check_circuit_idx * circuit.num_out_wires + j]);

        //Check that lsb(base_out_key)==0
        if (perm_bits[cnc_out_keys_idx + current_check_circuit_idx * circuit.num_out_wires + j] != GetLSB(cnc_keys[cnc_out_keys_idx + current_check_circuit_idx * circuit.num_out_wires + j])) {
          std::cout << "Abort, lsb(base_out_key) != 0" << std::endl;
          throw std::runtime_error("Abort, lsb(base_out_key) != 0");
        }

        //Unmask the decomitted output keys
        XOR_128(decommitted_output_keys[j], out_wire_commit_corrections[i * circuit.num_out_wires + j], cnc_keys[cnc_out_keys_idx + current_check_circuit_idx * circuit.num_out_wires + j]);
      }

      //Check that the computed output keys match the decomitted ones
      if (!std::equal(output_keys.data(), output_keys[circuit.num_out_wires - 1], decommitted_output_keys.data())) {
        std::cout << "Abort, garbled circuit wrong output commits!" << std::endl;
        throw std::runtime_error("Abort, garbled circuit wrong output commits!");
      }

      // Finally check that the comitted tables match the cnc constructed tables by comparing the hash
      if (!std::equal(cnc_garb_circuit_hashes[current_check_circuit_idx], cnc_garb_circuit_hashes[current_check_circuit_idx + 1], garb_circuit_hashes[i])) {
        std::cout << "Abort, garbled tables wrongly constructed. Hash doesn't match!" << std::endl;
        throw std::runtime_error("Abort, garbled tables wrongly constructed. Hash doesn't match!");
      }
      ++current_check_circuit_idx;
    } else if (current_eval_circuit_idx < exec_eval_circuits_to) {

      //Copy Delta
      std::copy(commit_keys_share[deltas_idx + i], commit_keys_share[deltas_idx + (i + 1)], aux_garbled_circuits_data[current_eval_circuit_idx].delta_share());

      //Copy inputs
      std::copy(commit_keys_share[input_keys_idx + i * circuit.num_inp_wires], commit_keys_share[input_keys_idx + (i + 1) * circuit.num_inp_wires], aux_garbled_circuits_data[current_eval_circuit_idx].inp_key_share());

      //Copy outputs
      std::copy(commit_keys_share[output_keys_idx + i * circuit.num_out_wires], commit_keys_share[output_keys_idx + (i + 1) * circuit.num_out_wires], aux_garbled_circuits_data[current_eval_circuit_idx].out_key_share());

      //Add output correction
      std::copy(out_wire_commit_corrections[i * circuit.num_out_wires], out_wire_commit_corrections[(i + 1) * circuit.num_out_wires], aux_garbled_circuits_data[current_eval_circuit_idx].out_soldering());

      std::copy(garb_circuit_hashes[i], garb_circuit_hashes[i + 1], eval_hash[current_eval_circuit_idx]);

      ++current_eval_circuit_idx;
    } else {
      completed_eval_copy = true;
    }
  }

  if (!completed_eval_copy) {
    std::cout << "Problem. Not enough eval circuits! Params should be set so this never occurs" << std::endl;
  }
}

void DuploEvaluator::PrepareOutputWireAuthenticators(uint32_t num_parallel_execs) {

  uint64_t num_circuit_buckets = 0; //only used for printing relative timings
  uint64_t num_circuit_auth_buckets = 0;

  for (std::tuple<std::string, Circuit, uint64_t>& c_info : circuit_info) {
    num_circuit_buckets += std::get<2>(c_info);
    num_circuit_auth_buckets += std::get<2>(c_info) * std::get<1>(c_info).num_out_wires;
  }

  //We need at least 5
  num_circuit_auth_buckets = std::max<uint64_t>(num_circuit_auth_buckets, 5);

  //Compute parameters
  long double circuit_check_factor;
  uint32_t circuit_auth_size;
  bool circuit_negate_check_factor;

  PickBestMajorityAuthParams(num_circuit_auth_buckets, circuit_auth_size, circuit_check_factor, circuit_negate_check_factor);

#ifdef DUPLO_PRINT

  double circuit_cnc_check_prob;
  if (circuit_negate_check_factor) {
    circuit_cnc_check_prob = 1 - (1 / pow(2, circuit_check_factor));
  } else {
    circuit_cnc_check_prob = (1 / pow(2, circuit_check_factor));
  }

  std::cout << "circuit_auth_size=" << circuit_auth_size << ", " << "circuit_cnc_check_prob=" << circuit_cnc_check_prob << std::endl;
#endif

  auto prepare_eval_commit_cnc_begin = GET_TIME();

  uint32_t num_circuit_eval_auths = num_circuit_auth_buckets * circuit_auth_size;

  std::vector<int> eval_circuit_auths_from, eval_circuit_auths_to;
  PartitionBufferFixedNum(eval_circuit_auths_from, eval_circuit_auths_to, num_parallel_execs, num_circuit_eval_auths);

  BYTEArrayVector circuit_aux_auth_data(num_circuit_eval_auths, CODEWORD_BYTES);
  std::vector<BYTEArrayVector> circuit_eval_auths(num_circuit_eval_auths, BYTEArrayVector(2, CSEC_BYTES));
  std::vector<uint32_t> circuit_eval_auths_ids(num_circuit_eval_auths);
  uint8_t circuit_aux_auth_delta_data[CODEWORD_BYTES];

  std::mutex delta_updated_mutex;
  std::condition_variable delta_updated_cond_val;
  std::atomic<bool> delta_updated(false);
  std::tuple<std::mutex&, std::condition_variable&, std::atomic<bool>&> delta_signal = make_tuple(std::ref(delta_updated_mutex), std::ref(delta_updated_cond_val), std::ref(delta_updated));

  std::vector<std::future<void>> execs_finished(num_parallel_execs);
  for (int exec_id = 0; exec_id < num_parallel_execs; ++exec_id) {

    //Starts the current execution
    execs_finished[exec_id] = thread_pool.push([this, &circuit_eval_auths, &circuit_aux_auth_data, &circuit_eval_auths_ids, &circuit_aux_auth_delta_data, &delta_signal, &eval_circuit_auths_from, &eval_circuit_auths_to, circuit_check_factor, circuit_negate_check_factor, exec_id] (int id) {

      uint32_t exec_num_circuit_auths = eval_circuit_auths_to[exec_id] - eval_circuit_auths_from[exec_id];

      float circuit_repl_factor;
      ComputeCheckFraction(circuit_check_factor, exec_num_circuit_auths, circuit_repl_factor, circuit_negate_check_factor);

      uint32_t exec_num_total_circuit_auths = ceil(circuit_repl_factor * exec_num_circuit_auths);

      CommitCircuitAuthAndCutAndChoose(exec_id, exec_num_total_circuit_auths, circuit_check_factor, circuit_negate_check_factor, eval_circuit_auths_from[exec_id], eval_circuit_auths_to[exec_id], circuit_eval_auths, circuit_aux_auth_data, circuit_eval_auths_ids, circuit_aux_auth_delta_data, delta_signal);

    });
  }

  for (std::future<void>& r : execs_finished) {
    r.wait();
  }

  auto prepare_eval_commit_cnc_end = GET_TIME();

#ifdef DUPLO_PRINT
  PrintTimePerUnit(prepare_eval_commit_cnc_begin, prepare_eval_commit_cnc_end, num_circuit_buckets, "PrepareOutputWireAuthsCNC");
#endif

  std::vector<std::vector<int>> session_circuit_buckets_from(num_parallel_execs);
  std::vector<std::vector<int>> session_circuit_buckets_to(num_parallel_execs);

  for (int i = 0; i < circuit_info.size(); ++i) {
    std::string& session_component_name(std::get<0>(circuit_info[i]));
    Circuit& session_circuits(std::get<1>(circuit_info[i]));
    uint64_t num_session_circuit_buckets = std::get<2>(circuit_info[i]);

    uint64_t num_session_auth_buckets = num_session_circuit_buckets * session_circuits.num_out_wires;
    uint64_t session_auth_bytes = 2 * CSEC_BYTES * num_session_auth_buckets * circuit_auth_size;
    uint64_t session_auth_soldeirngs_bytes = CSEC_BYTES * num_session_auth_buckets * circuit_auth_size;
    uint64_t session_auth_delta_solderings_bytes = CSEC_BYTES * num_session_circuit_buckets;
    uint64_t session_auth_ids_bytes = sizeof(uint32_t) * num_session_auth_buckets * circuit_auth_size;

    std::vector<int> tmp_from, tmp_to;
    PartitionBufferFixedNum(tmp_from, tmp_to, num_parallel_execs, num_session_circuit_buckets);
    for (int j = 0; j < num_parallel_execs; ++j) {
      session_circuit_buckets_from[j].push_back(tmp_from[j]);
      session_circuit_buckets_to[j].push_back(tmp_to[j]);
    }

    persistent_storage.PrepareFile(session_component_name, AUTHS, session_auth_bytes);
    persistent_storage.PrepareFile(session_component_name, AUTHS_SOLDERINGS, session_auth_soldeirngs_bytes);
    persistent_storage.PrepareFile(session_component_name, AUTHS_DELTA_SOLDERINGS, session_auth_delta_solderings_bytes);
    persistent_storage.PrepareFile(session_component_name, AUTHS_IDS, session_auth_ids_bytes);
  }

  uint8_t bucket_seed[CSEC_BYTES];
  rnd.get<uint8_t>(bucket_seed, CSEC_BYTES);
  chan.asyncSendCopy(bucket_seed, CSEC_BYTES);

  osuCrypto::PRNG bucket_rnd;
  bucket_rnd.SetSeed(load_block(bucket_seed));

  std::vector<uint32_t> circuit_permuted_eval_ids(num_circuit_eval_auths);
  std::vector<uint32_t> circuit_permuted_eval_ids_inv(num_circuit_eval_auths);
  std::iota(std::begin(circuit_permuted_eval_ids), std::end(circuit_permuted_eval_ids), 0);
  PermuteArray(circuit_permuted_eval_ids.data(), num_circuit_eval_auths, bucket_seed);
  for (int i = 0; i < num_circuit_eval_auths; ++i) {
    circuit_permuted_eval_ids_inv[circuit_permuted_eval_ids[i]] = i;
  }

  auto prepare_eval_bucketing_begin = GET_TIME();
  for (int exec_id = 0; exec_id < num_parallel_execs; ++exec_id) {

    //Starts the current execution
    execs_finished[exec_id] = thread_pool.push([this, &circuit_eval_auths, &circuit_aux_auth_data, &circuit_aux_auth_delta_data, &session_circuit_buckets_from, &session_circuit_buckets_to, &circuit_permuted_eval_ids_inv, &circuit_eval_auths_ids, circuit_auth_size, exec_id] (int id) {

      BucketAllCircuitAuths(exec_id, circuit_auth_size, circuit_permuted_eval_ids_inv, session_circuit_buckets_from[exec_id], session_circuit_buckets_to[exec_id], circuit_eval_auths, circuit_aux_auth_data, circuit_eval_auths_ids, circuit_aux_auth_delta_data);

    });
  }

  for (std::future<void>& r : execs_finished) {
    r.wait();
  }

  auto prepare_eval_bucketing_end = GET_TIME();

#ifdef DUPLO_PRINT
  PrintTimePerUnit(prepare_eval_bucketing_begin, prepare_eval_bucketing_end, num_circuit_buckets, "PrepareOutputWireAuthsBucketing");
#endif
}

void DuploEvaluator::PrepareInputWireAuthenticators(uint64_t num_input_auth_buckets, uint32_t num_parallel_execs) {

  uint64_t num_circuit_buckets = 0; //only used for printing relative timings

  for (std::tuple<std::string, Circuit, uint64_t>& c_info : circuit_info) {
    num_circuit_buckets += std::get<2>(c_info);
  }

  //We need at least 5
  num_input_auth_buckets = std::max<uint64_t>(num_input_auth_buckets, 5);

  //Compute parameters
  long double input_check_factor;
  uint32_t input_auth_size;
  bool input_negate_check_factor;
  PickBestMajorityBucketParams(num_input_auth_buckets, input_auth_size, input_check_factor, input_negate_check_factor);

#ifdef DUPLO_PRINT

  double input_cnc_check_prob;
  if (input_negate_check_factor) {
    input_cnc_check_prob = 1 - (1 / pow(2, input_check_factor));
  } else {
    input_cnc_check_prob = (1 / pow(2, input_check_factor));
  }

  std::cout << "input_auth_size=" << input_auth_size << ", " << "input_cnc_check_prob=" << input_cnc_check_prob << std::endl;
#endif

  auto prepare_eval_commit_cnc_begin = GET_TIME();

  uint32_t num_input_eval_auths = num_input_auth_buckets * input_auth_size;

  std::vector<int> eval_input_auths_from, eval_input_auths_to, input_buckets_from, input_buckets_to;
  PartitionBufferFixedNum(eval_input_auths_from, eval_input_auths_to, num_parallel_execs, num_input_eval_auths);
  PartitionBufferFixedNum(input_buckets_from, input_buckets_to, num_parallel_execs, num_input_auth_buckets);

  std::vector<EvalGarbledCircuit> input_aux_auth_data(num_input_eval_auths, EvalGarbledCircuit(inp_bucket_circuit)); //include space for the two wire auths

  std::vector<std::future<void>> execs_finished(num_parallel_execs);
  for (int exec_id = 0; exec_id < num_parallel_execs; ++exec_id) {

    //Starts the current execution
    execs_finished[exec_id] = thread_pool.push([this, &input_aux_auth_data, &eval_input_auths_from, &eval_input_auths_to, input_check_factor, input_negate_check_factor, exec_id] (int id) {

      uint32_t exec_num_input_auths = eval_input_auths_to[exec_id] - eval_input_auths_from[exec_id];

      float input_repl_factor;
      ComputeCheckFraction(input_check_factor, exec_num_input_auths, input_repl_factor, input_negate_check_factor);

      uint32_t exec_num_total_input_auths = ceil(input_repl_factor * exec_num_input_auths);

      CommitInputAuthAndCutAndChoose(exec_id, exec_num_total_input_auths, input_check_factor, input_negate_check_factor, eval_input_auths_from[exec_id], eval_input_auths_to[exec_id], input_aux_auth_data);
    });
  }

  for (std::future<void>& r : execs_finished) {
    r.wait();
  }

  auto prepare_eval_commit_cnc_end = GET_TIME();

#ifdef DUPLO_PRINT
  PrintTimePerUnit(prepare_eval_commit_cnc_begin, prepare_eval_commit_cnc_end, num_circuit_buckets, "PrepareInputWireAuthsCNC");
#endif

  uint8_t bucket_seed[CSEC_BYTES];
  rnd.get<uint8_t>(bucket_seed, CSEC_BYTES);
  chan.asyncSendCopy(bucket_seed, CSEC_BYTES);

  osuCrypto::PRNG bucket_rnd;
  bucket_rnd.SetSeed(load_block(bucket_seed));

  std::vector<uint32_t> input_permuted_eval_ids(num_input_eval_auths);
  std::vector<uint32_t> input_permuted_eval_ids_inv(num_input_eval_auths);
  std::iota(std::begin(input_permuted_eval_ids), std::end(input_permuted_eval_ids), 0);
  PermuteArray(input_permuted_eval_ids.data(), num_input_eval_auths, bucket_seed);
  for (int i = 0; i < num_input_eval_auths; ++i) {
    input_permuted_eval_ids_inv[input_permuted_eval_ids[i]] = i;
  }

  uint64_t tables_bytes = GarbledCircuit::TotalTableSize(inp_bucket_circuit) * num_input_eval_auths;
  uint64_t solderings_bytes = EvalGarbledCircuit::SolderingsSize(inp_bucket_circuit) * num_input_eval_auths;
  uint64_t auxdata_bytes = EvalGarbledCircuit::AuxDataSize(inp_bucket_circuit) * num_input_auth_buckets;
  uint64_t recov_data_bytes = CSEC_BYTES * num_input_auth_buckets;

  persistent_storage.PrepareFile(EVAL_INP_BUCKET_PREFIX, TABLES, tables_bytes);
  persistent_storage.PrepareFile(EVAL_INP_BUCKET_PREFIX, SOLDERINGS, solderings_bytes);
  persistent_storage.PrepareFile(EVAL_INP_BUCKET_PREFIX, AUXDATA, auxdata_bytes);
  persistent_storage.PrepareFile(EVAL_INP_BUCKET_PREFIX, RECOV_SOLD, recov_data_bytes);

  persistent_storage.PrepareFile(EVAL_INP_BUCKET_PREFIX, INPUT_MASKS_CORRECTIONS, num_input_eval_auths * CSEC_BYTES);


  auto prepare_eval_bucketing_begin = GET_TIME();
  for (int exec_id = 0; exec_id < num_parallel_execs; ++exec_id) {

    //Starts the current execution
    execs_finished[exec_id] = thread_pool.push([this, &input_aux_auth_data, &input_permuted_eval_ids_inv, &input_buckets_from, &input_buckets_to, input_auth_size, exec_id] (int id) {

      BucketAllInputAuths(exec_id, input_auth_size, input_permuted_eval_ids_inv, input_buckets_from[exec_id], input_buckets_to[exec_id], input_aux_auth_data);

    });
  }

  for (std::future<void>& r : execs_finished) {
    r.wait();
  }

  auto prepare_eval_bucketing_end = GET_TIME();

  string_to_circuit_map.emplace(EVAL_INP_BUCKET_PREFIX, inp_bucket_circuit);

#ifdef DUPLO_PRINT
  PrintTimePerUnit(prepare_eval_bucketing_begin, prepare_eval_bucketing_end, num_circuit_buckets, "PrepareInputWireAuthsBucketing");
#endif
}

void DuploEvaluator::BucketAndReceiveEvalCircuits(std::string component_type, uint32_t exec_id, Circuit & circuit, uint32_t bucket_size, std::vector<uint32_t>& permuted_eval_ids_inv, uint32_t exec_buckets_from, uint32_t exec_buckets_to, std::vector<EvalGarbledCircuit>& aux_garbled_circuits_data, BYTEArrayVector & eval_hash) {

  uint64_t exec_num_buckets = exec_buckets_to - exec_buckets_from;
  uint64_t exec_num_eval_circuits = exec_num_buckets * bucket_size;

  uint64_t garbled_table_size = GarbledCircuit::TotalTableSize(circuit);
  uint64_t solderings_size = EvalGarbledCircuit::SolderingsSize(circuit);
  uint64_t eval_aux_size = EvalGarbledCircuit::AuxDataSize(circuit);
  uint64_t total_eval_aux_size = EvalGarbledCircuit::TotalAuxDataSize(circuit);

  //Receive all garbled tables
  BYTEArrayVector exec_received_garbled_tables(exec_num_eval_circuits, garbled_table_size);
  exec_channels[exec_id].recv(exec_received_garbled_tables.data(), exec_received_garbled_tables.size());

  uint8_t hash_value[osuCrypto::SHA1::HashSize];
  PermutedIndex<EvalGarbledCircuit> exec_permuted_aux_info(aux_garbled_circuits_data, permuted_eval_ids_inv, exec_buckets_from * bucket_size, total_eval_aux_size);
  for (int i = 0; i < exec_num_eval_circuits; ++i) {
    uint32_t global_circuit_index = exec_buckets_from * bucket_size + i;

    HashGarbledCircuitTables(circuit, exec_received_garbled_tables[i], hash_value);
    if (!std::equal(hash_value, hash_value + osuCrypto::SHA1::HashSize, eval_hash[permuted_eval_ids_inv[global_circuit_index]])) {
      std::cout << "Abort, wrong eval garbled tables sent. Hash doesn't match!" << std::endl;
      throw std::runtime_error("Abort, wrong eval garbled tables sent. Hash doesn't match!");
    }
  }

  ReceiveAndStoreComponentSolderings(component_type, exec_id, circuit, bucket_size, exec_permuted_aux_info, exec_buckets_from, exec_buckets_to, exec_received_garbled_tables);
}

void DuploEvaluator::CommitCircuitAuthAndCutAndChoose(uint32_t exec_id, uint32_t exec_num_auths, uint32_t check_factor, bool negate_check_factor, uint32_t exec_eval_auths_from, uint32_t exec_eval_auths_to, std::vector<BYTEArrayVector>& eval_auths, BYTEArrayVector & aux_auth_data, std::vector<uint32_t>& eval_auths_ids, uint8_t aux_auth_delta_data[], std::tuple<std::mutex&, std::condition_variable&, std::atomic<bool>&>& delta_signal) {

  //If this is exec_id == 0 we produce one delta commitment.
  uint32_t num_commit_keys;
  if (exec_id == 0) {
    num_commit_keys = exec_num_auths + 1;
  } else {
    num_commit_keys = exec_num_auths;
  }

  BYTEArrayVector commit_keys_share(num_commit_keys, CODEWORD_BYTES);

  if (!commit_receivers[exec_id].Commit(commit_keys_share, exec_rnds[exec_id], exec_channels[exec_id], exec_num_auths)) {
    std::cout << "Abort, key commit failed!" << std::endl;;
    throw std::runtime_error("Abort, key commit failed!");
  }

  //If not execution 0, wait until execution 0 has put the delta commitment into aux_auth_delta_data
  std::condition_variable& delta_updated_cond_val = std::get<1>(delta_signal);
  std::atomic<bool>& delta_updated = std::get<2>(delta_signal);

  if (exec_id == 0) {
    std::copy(commit_keys_share[num_commit_keys - 1], commit_keys_share[num_commit_keys], aux_auth_delta_data);
    {
      std::lock_guard<std::mutex> lk(std::get<0>(delta_signal));
      delta_updated = true;
    }
    delta_updated_cond_val.notify_all();
  } else {

    std::mutex& delta_updated_mutex = std::get<0>(delta_signal);
    std::unique_lock<std::mutex> lock(delta_updated_mutex);
    
    while (!delta_updated) {
      delta_updated_cond_val.wait(lock);
    }
  }

  //Index offsets, only used for cut-and-choose. When written to disc wire authenticators are written 32 bytes at a time, so H_0 and H_1 are always next to each other.
  uint32_t H_0_idx = 0;
  uint32_t H_1_idx = exec_num_auths;

  BYTEArrayVector auths(2 * exec_num_auths, CSEC_BYTES);

  exec_channels[exec_id].recv(auths.data(), auths.size());

  //Sample and send challenge seed
  uint8_t cnc_seed[CSEC_BYTES];
  exec_rnds[exec_id].get<uint8_t>(cnc_seed, CSEC_BYTES);
  exec_channels[exec_id].asyncSendCopy(cnc_seed, CSEC_BYTES);

  //Select challenge auths based on cnc_seed
  osuCrypto::BitVector cnc_check_auths(exec_num_auths);
  osuCrypto::PRNG cnc_rand;
  cnc_rand.SetSeed(load_block(cnc_seed));

  WeightedRandomString(cnc_check_auths.data(), check_factor, cnc_check_auths.sizeBytes(), cnc_rand, negate_check_factor);
  uint64_t cnc_num_auths = countSetBits(cnc_check_auths.data(), 0, exec_num_auths - 1);

  osuCrypto::BitVector cnc_check_inputs(cnc_num_auths);
  cnc_rand.get<uint8_t>(cnc_check_inputs.data(), cnc_check_inputs.sizeBytes());

  //Array for holding the decommit shares of the decommited keys
  BYTEArrayVector cnc_keys_share(cnc_num_auths, CODEWORD_BYTES);

  uint32_t current_check_auth_idx = 0;
  uint32_t current_eval_auth_idx = exec_eval_auths_from;

  for (int i = 0; i < exec_num_auths; ++i) {
    if (cnc_check_auths[i]) { //Checked auths

      //Add key shares
      std::copy(commit_keys_share[i], commit_keys_share[(i + 1)], cnc_keys_share[current_check_auth_idx]);

      if (cnc_check_inputs[current_check_auth_idx]) {
        XOR_CodeWords(cnc_keys_share[current_check_auth_idx], aux_auth_delta_data);
      }

      ++current_check_auth_idx;
    }
  }

  BYTEArrayVector cnc_keys(cnc_num_auths, CSEC_BYTES);
  exec_channels[exec_id].recv(cnc_keys.data(), cnc_keys.size());
  if (!commit_receivers[exec_id].BatchDecommit(cnc_keys_share, cnc_keys, exec_rnds[exec_id], exec_channels[exec_id], true)) {
    std::cout << "Abort, auth cut-and-choose decommit failed!" << std::endl;
    throw std::runtime_error("Abort, auth cut-and-choose decommit failed!");
  }

  GarblingHandler gh;
  current_check_auth_idx = 0;
  uint32_t global_auth_idx;
  bool success = true;
  bool completed_eval_copy = false;
  for (int i = 0; i < exec_num_auths; ++i) {
    if (cnc_check_auths[i]) { //Checked auths
      global_auth_idx = exec_eval_auths_from + i;
      if (!VerifyAuth(cnc_keys[current_check_auth_idx], auths[H_0_idx + i], auths[H_1_idx + i], global_auth_idx, gh.key_schedule)) {
        success = false;
      }
      ++current_check_auth_idx;
    } else if (current_eval_auth_idx < exec_eval_auths_to) {

      //Store id info
      global_auth_idx = exec_eval_auths_from + i;
      eval_auths_ids[current_eval_auth_idx] = global_auth_idx;

      //Copy key info
      std::copy(commit_keys_share[i], commit_keys_share[(i + 1)], aux_auth_data[current_eval_auth_idx]);

      //Store eval authenticator
      std::copy(auths[H_0_idx + i], auths[H_0_idx + i + 1], eval_auths[current_eval_auth_idx][0]);
      std::copy(auths[H_1_idx + i], auths[H_1_idx + i + 1], eval_auths[current_eval_auth_idx][1]);

      ++current_eval_auth_idx;
    } else {
      completed_eval_copy = true;
    }
  }

  if (!success) {
    std::cout << "Abort, auth cut-and-choose eval failed!" << std::endl;
    throw std::runtime_error("Abort, auth cut-and-choose eval failed!");
  }

  if (!completed_eval_copy) {
    std::cout << "Problem. Not enough eval auths! Params should be set so this never occurs" << std::endl;
  }
}

void DuploEvaluator::CommitInputAuthAndCutAndChoose(uint32_t exec_id, uint32_t exec_num_total_garbled, uint32_t check_factor, bool negate_check_factor, uint32_t exec_eval_circuits_from, uint32_t exec_eval_circuits_to, std::vector<EvalGarbledCircuit>& input_aux_auth_data) {

  uint32_t num_inp_keys, num_out_keys, num_deltas, num_commit_keys, num_base_keys, input_keys_idx, output_keys_idx, deltas_idx;
  ComputeIndices(exec_num_total_garbled, inp_bucket_circuit, num_inp_keys, num_out_keys, num_deltas, num_commit_keys, num_base_keys, input_keys_idx, output_keys_idx, deltas_idx);

  BYTEArrayVector commit_keys_share(num_commit_keys, CODEWORD_BYTES);

  //Commit to keys
  if (!commit_receivers[exec_id].Commit(commit_keys_share, exec_rnds[exec_id], exec_channels[exec_id], deltas_idx)) {
    std::cout << "Abort, key commit failed!" << std::endl;;
    throw std::runtime_error("Abort, key commit failed!");
  }

  BYTEArrayVector inp_wire_commit_corrections(num_inp_keys, CSEC_BYTES);
  exec_channels[exec_id].recv(inp_wire_commit_corrections.data(), inp_wire_commit_corrections.size());

  BYTEArrayVector auths(2 * exec_num_total_garbled, CSEC_BYTES);
  exec_channels[exec_id].recv(auths.data(), auths.size());

  //Sample and send challenge seed
  uint8_t cnc_seed[CSEC_BYTES];
  exec_rnds[exec_id].get<uint8_t>(cnc_seed, CSEC_BYTES);
  exec_channels[exec_id].asyncSendCopy(cnc_seed, CSEC_BYTES);

  //Select challenge circuits based on cnc_seed
  osuCrypto::BitVector cnc_check_circuits(exec_num_total_garbled);
  osuCrypto::PRNG cnc_rand;
  cnc_rand.SetSeed(load_block(cnc_seed));

  WeightedRandomString(cnc_check_circuits.data(), check_factor, cnc_check_circuits.sizeBytes(), cnc_rand, negate_check_factor);
  uint64_t num_checked_circuits = countSetBits(cnc_check_circuits.data(), 0, exec_num_total_garbled - 1);

  uint32_t cnc_num_inp_keys, cnc_num_out_keys, cnc_num_deltas, cnc_num_commit_keys, cnc_num_base_keys, cnc_inp_keys_idx, cnc_out_keys_idx, cnc_deltas_idx;
  ComputeIndices(num_checked_circuits, inp_bucket_circuit, cnc_num_inp_keys, cnc_num_out_keys, cnc_num_deltas, cnc_num_commit_keys, cnc_num_base_keys, cnc_inp_keys_idx, cnc_out_keys_idx, cnc_deltas_idx);

  BYTEArrayVector cnc_keys_share(cnc_num_commit_keys, CODEWORD_BYTES);

  osuCrypto::BitVector perm_bits(cnc_num_commit_keys);
  exec_channels[exec_id].recv(perm_bits);

  //Receive postulated perm_bit values
  BYTEArrayVector cnc_keys(cnc_num_commit_keys, CSEC_BYTES);
  exec_channels[exec_id].recv(cnc_keys.data(), cnc_keys.size());

  uint32_t current_check_circuit_idx = 0;
  for (int i = 0; i < exec_num_total_garbled; ++i) {
    if (cnc_check_circuits[i]) { //Checked circuit

      //Add delta
      std::copy(commit_keys_share[deltas_idx + i], commit_keys_share[deltas_idx + (i + 1)], cnc_keys_share[cnc_deltas_idx + current_check_circuit_idx]);

      //Add inputs
      std::copy(commit_keys_share[input_keys_idx + i],
                commit_keys_share[input_keys_idx + (i + 1)],
                cnc_keys_share[cnc_inp_keys_idx + current_check_circuit_idx]);

      if (perm_bits[cnc_inp_keys_idx + current_check_circuit_idx]) {
        XOR_CodeWords(cnc_keys_share[cnc_inp_keys_idx + current_check_circuit_idx],
                      cnc_keys_share[cnc_deltas_idx + current_check_circuit_idx]);
      }

      ++current_check_circuit_idx;
    }
  }

  if (!commit_receivers[exec_id].BatchDecommit(cnc_keys_share, cnc_keys, exec_rnds[exec_id], exec_channels[exec_id], true)) {
    std::cout << "Abort, input auths cut-and-choose decommit failed!" << std::endl;
    throw std::runtime_error("Abort, input auths cut-and-choose decommit failed!");
  }

  GarblingHandler gh;

  uint8_t curr_hash_val[CSEC_BYTES];
  uint8_t curr_h_0[CSEC_BYTES];
  uint8_t curr_h_1[CSEC_BYTES];

  uint32_t H_0_idx = 0;
  uint32_t H_1_idx = exec_num_total_garbled;

  current_check_circuit_idx = 0; //reset counter
  uint32_t current_eval_circuit_idx = exec_eval_circuits_from;
  bool completed_eval_copy = false;
  for (int i = 0; i < exec_num_total_garbled; ++i) {
    if (cnc_check_circuits[i]) {

      //Check that lsb(Delta) == 1
      if (!GetLSB(cnc_keys[cnc_deltas_idx + current_check_circuit_idx])) {
        std::cout << "Abort, lsb(delta) was incorrect!" << std::endl;
        throw std::runtime_error("Abort, lsb(delta) was incorrect!");
      }

      if (perm_bits[cnc_inp_keys_idx + current_check_circuit_idx] && GetLSB(cnc_keys[cnc_inp_keys_idx + current_check_circuit_idx])) {
        std::cout << "Constructor lying about inp_perm_bits!" << std::endl;
        throw std::runtime_error("Constructor lying about inp_perm_bits!");
      }

      gh.GarbleInpBucket(curr_hash_val, curr_h_0, curr_h_1, cnc_keys[cnc_deltas_idx + current_check_circuit_idx]);

      XOR_128(curr_hash_val, cnc_keys[cnc_inp_keys_idx + current_check_circuit_idx]);

      XORLSB(curr_hash_val, perm_bits[cnc_inp_keys_idx + current_check_circuit_idx]);

      if (!std::equal(curr_hash_val, curr_hash_val + CSEC_BYTES, inp_wire_commit_corrections[i]) ||
          !std::equal(curr_hash_val, curr_hash_val + CSEC_BYTES, inp_wire_commit_corrections[i]) ||
          !std::equal(curr_h_0, curr_h_0 + CSEC_BYTES,
                      auths[H_0_idx + i]) ||
          !std::equal(curr_h_1, curr_h_1 + CSEC_BYTES,
                      auths[H_1_idx + i])) {
        std::cout << "Abort, garbled circuit wrong output commits!" << std::endl;
        throw std::runtime_error("Abort, garbled circuit wrong output commits!");
      }

      ++current_check_circuit_idx;
    } else if (current_eval_circuit_idx < exec_eval_circuits_to) {

      //Copy Delta
      std::copy(commit_keys_share[deltas_idx + i], commit_keys_share[deltas_idx + (i + 1)], input_aux_auth_data[current_eval_circuit_idx].delta_share());

      //Copy inputs
      std::copy(commit_keys_share[input_keys_idx + i],
                commit_keys_share[input_keys_idx + i + 1],
                input_aux_auth_data[current_eval_circuit_idx].inp_key_share());

      //Copy input commit corrections for writing to disc
      std::copy(inp_wire_commit_corrections[input_keys_idx + i], inp_wire_commit_corrections[input_keys_idx + i + 1], input_aux_auth_data[current_eval_circuit_idx].inp_soldering(0));

      //Store eval authenticator
      std::copy(auths[H_0_idx + i], auths[H_0_idx + i + 1], input_aux_auth_data[current_eval_circuit_idx].T_G(0)); //H_0
      std::copy(auths[H_1_idx + i], auths[H_1_idx + i + 1], input_aux_auth_data[current_eval_circuit_idx].T_E(0)); //H_1

      ++current_eval_circuit_idx;
    } else {
      completed_eval_copy = true;
    }
  }

  if (!completed_eval_copy) {
    std::cout << "Problem. Not enough eval auths! Params should be set so this never occurs" << std::endl;
  }
}

void DuploEvaluator::BucketAllCircuitAuths(uint32_t exec_id, uint32_t auth_size, std::vector<uint32_t>& permuted_eval_ids_inv, std::vector<int>& session_circuit_buckets_from, std::vector<int>& session_circuit_buckets_to, std::vector<BYTEArrayVector>& eval_auths, BYTEArrayVector & aux_auth_data, std::vector<uint32_t>& eval_auths_ids, uint8_t aux_auth_delta_data[]) {

  uint32_t num_sessions = session_circuit_buckets_from.size();

  uint32_t total_num_solderings = 0;
  for (int l = 0; l < num_sessions; ++l) {
    Circuit& session_circuit = std::get<1>(circuit_info[l]);
    uint32_t exec_session_num_circuit_buckets = session_circuit_buckets_to[l] - session_circuit_buckets_from[l];

    total_num_solderings += exec_session_num_circuit_buckets * (auth_size * session_circuit.num_out_wires + 1);
  }

  //For decomitting later on
  BYTEArrayVector solder_keys_share(total_num_solderings, CODEWORD_BYTES);

  uint32_t curr_solder_write_pos = 0;
  for (int l = 0; l < num_sessions; ++l) {

    std::string component_type = std::get<0>(circuit_info[l]);
    Circuit& session_circuit = std::get<1>(circuit_info[l]);

    uint32_t exec_session_num_circuit_buckets = session_circuit_buckets_to[l] - session_circuit_buckets_from[l];

    //Read all session circuit info
    BYTEArrayVector curr_session_aux_info;
    persistent_storage.ReadBuckets(component_type, AUXDATA, session_circuit_buckets_from[l], exec_session_num_circuit_buckets, curr_session_aux_info);

    //For each bucket in the session
    for (int i = 0; i < exec_session_num_circuit_buckets; ++i) {
      uint32_t curr_session_bucket_idx = i + session_circuit_buckets_from[l];

      //Copy Delta soldering
      std::copy(EvalGarbledCircuit::delta_share(session_circuit, curr_session_aux_info[i]),
                EvalGarbledCircuit::delta_share(session_circuit, curr_session_aux_info[i]) + CODEWORD_BYTES,
                solder_keys_share[curr_solder_write_pos]);
      XOR_CodeWords(solder_keys_share[curr_solder_write_pos], aux_auth_delta_data);

      ++curr_solder_write_pos;

      //Copy all bucket_size*session_circuit.num_out_wires solderings
      for (int j = 0; j < session_circuit.num_out_wires; ++j) {
        for (int a = 0; a < auth_size; ++a) {
          uint32_t perm_auth_idx = permuted_eval_ids_inv[(curr_session_bucket_idx * session_circuit.num_out_wires + j) * auth_size + a];

          std::copy(EvalGarbledCircuit::out_key_share(session_circuit, curr_session_aux_info[i], j),
                    EvalGarbledCircuit::out_key_share(session_circuit, curr_session_aux_info[i], j + 1),
                    solder_keys_share[curr_solder_write_pos]);

          XOR_CodeWords(solder_keys_share[curr_solder_write_pos], aux_auth_data[perm_auth_idx]);

          ++curr_solder_write_pos;
        }
      }
    }
  }

  BYTEArrayVector solder_keys(total_num_solderings, CSEC_BYTES);
  exec_channels[exec_id].recv(solder_keys.data(), solder_keys.size());
  if (!commit_receivers[exec_id].BatchDecommit(solder_keys_share, solder_keys, exec_rnds[exec_id], exec_channels[exec_id], true)) {
    std::cout << "Abort, auth soldering decommit failed!" << std::endl;
    throw std::runtime_error("Abort, auth soldering decommit failed!");
  }

  //Write solderings to disc
  uint32_t curr_solder_read_pos = 0;
  for (int l = 0; l < num_sessions; ++l) {

    std::string session_component_type = std::get<0>(circuit_info[l]);
    Circuit& session_circuit = std::get<1>(circuit_info[l]);

    uint32_t exec_session_num_circuit_buckets = session_circuit_buckets_to[l] - session_circuit_buckets_from[l];
    uint32_t exec_session_num_auth_buckets = exec_session_num_circuit_buckets * session_circuit.num_out_wires;
    uint32_t exec_session_auth_bucket_from = session_circuit_buckets_from[l] * session_circuit.num_out_wires;

    BYTEArrayVector exec_session_auths(exec_session_num_auth_buckets * auth_size, 2 * CSEC_BYTES);

    BYTEArrayVector exec_session_auths_solderings(exec_session_num_auth_buckets * auth_size, CSEC_BYTES);
    BYTEArrayVector exec_session_delta_solder(exec_session_num_circuit_buckets, CSEC_BYTES);
    BYTEArrayVector exec_session_global_ids(exec_session_num_auth_buckets * auth_size, sizeof(uint32_t));

    for (int i = 0; i < exec_session_num_circuit_buckets; ++i) {
      uint32_t curr_session_bucket_idx = i + session_circuit_buckets_from[l];

      std::copy(solder_keys[curr_solder_read_pos], solder_keys[curr_solder_read_pos + 1], exec_session_delta_solder[i]);

      ++curr_solder_read_pos;

      //Copy all bucket_size*session_circuit.num_out_wires solderings
      for (int j = 0; j < session_circuit.num_out_wires; ++j) {
        for (int a = 0; a < auth_size; ++a) {

          uint32_t perm_auth_idx = permuted_eval_ids_inv[(curr_session_bucket_idx * session_circuit.num_out_wires + j) * auth_size + a];

          //Copy auths
          std::copy(
            eval_auths[perm_auth_idx][0],
            eval_auths[perm_auth_idx][0] + CSEC_BYTES,
            exec_session_auths[(i * session_circuit.num_out_wires + j) * auth_size + a]);

          std::copy(
            eval_auths[perm_auth_idx][1],
            eval_auths[perm_auth_idx][1] + CSEC_BYTES,
            exec_session_auths[(i * session_circuit.num_out_wires + j) * auth_size + a] + CSEC_BYTES);

          //Copy auth solderings
          std::copy(solder_keys[curr_solder_read_pos],
                    solder_keys[curr_solder_read_pos + 1],
                    exec_session_auths_solderings[(i * session_circuit.num_out_wires + j) * auth_size + a]);

          //Copy global idx
          std::copy((uint8_t*) & (eval_auths_ids[perm_auth_idx]),
                    (uint8_t*) & (eval_auths_ids[perm_auth_idx]) + sizeof(uint32_t),
                    exec_session_global_ids[(i * session_circuit.num_out_wires + j) * auth_size + a]);

          ++curr_solder_read_pos;
        }
      }
    }

    uint64_t exec_auths_write_pos = session_circuit_buckets_from[l] * session_circuit.num_out_wires * auth_size * 2 * CSEC_BYTES;
    uint64_t exec_auths_solderings_write_pos = session_circuit_buckets_from[l] * session_circuit.num_out_wires * auth_size * CSEC_BYTES;
    uint64_t exec_auths_deltas_write_pos = session_circuit_buckets_from[l] * CSEC_BYTES;
    uint64_t exec_auths_ids_write_pos = session_circuit_buckets_from[l] * session_circuit.num_out_wires * auth_size * sizeof(uint32_t);

    //Write auths
    persistent_storage.WriteBuckets(session_component_type, AUTHS, exec_session_auth_bucket_from, exec_session_num_auth_buckets, exec_session_auths.data(), exec_auths_write_pos, exec_session_auths.size(), auth_size);

    //Write auth solderings
    persistent_storage.WriteBuckets(session_component_type, AUTHS_SOLDERINGS, exec_session_auth_bucket_from, exec_session_num_auth_buckets, exec_session_auths_solderings.data(), exec_auths_solderings_write_pos, exec_session_auths_solderings.size(), auth_size);

    //Write delta solderings
    persistent_storage.WriteBuckets(session_component_type, AUTHS_DELTA_SOLDERINGS, session_circuit_buckets_from[l], exec_session_num_circuit_buckets, exec_session_delta_solder.data(), exec_auths_deltas_write_pos, exec_session_delta_solder.size(), 1);

    //Write global auth ids
    persistent_storage.WriteBuckets(session_component_type, AUTHS_IDS, exec_session_auth_bucket_from, exec_session_num_auth_buckets, exec_session_global_ids.data(), exec_auths_ids_write_pos, exec_session_global_ids.size(), auth_size);
  }
}

void DuploEvaluator::BucketAllInputAuths(uint32_t exec_id, uint32_t input_auth_size, std::vector<uint32_t>& permuted_eval_ids_inv, uint32_t exec_input_buckets_from, uint32_t exec_input_buckets_to, std::vector<EvalGarbledCircuit>& input_aux_auth_data) {

  uint32_t exec_num_input_auths = exec_input_buckets_to - exec_input_buckets_from;

  uint64_t exec_num_eval_circuits = exec_num_input_auths * input_auth_size;

  uint64_t garbled_table_size = GarbledCircuit::TotalTableSize(inp_bucket_circuit);
  uint64_t solderings_size = EvalGarbledCircuit::SolderingsSize(inp_bucket_circuit);
  uint64_t eval_aux_size = EvalGarbledCircuit::AuxDataSize(inp_bucket_circuit);
  uint64_t total_eval_aux_size = EvalGarbledCircuit::TotalAuxDataSize(inp_bucket_circuit);
  PermutedIndex<EvalGarbledCircuit> exec_permuted_aux_info(input_aux_auth_data, permuted_eval_ids_inv, exec_input_buckets_from * input_auth_size, total_eval_aux_size);

  BYTEArrayVector exec_received_garbled_tables(exec_num_eval_circuits, garbled_table_size);

  BYTEArrayVector exec_write_inp_wire_commit_corrections(exec_num_eval_circuits, CSEC_BYTES);

  for (int i = 0; i < exec_num_eval_circuits; ++i) {
    uint32_t curr_index = exec_input_buckets_from * input_auth_size + i;
    //Need to use input_aux_auth_data here as we need to look up in permuted table array, not in permuted aux data which we use exec_permuted_aux_info for
    std::copy(input_aux_auth_data[permuted_eval_ids_inv[curr_index]].T_G(0),
              input_aux_auth_data[permuted_eval_ids_inv[curr_index]].T_G(1),
              exec_received_garbled_tables[i]);
    std::copy(input_aux_auth_data[permuted_eval_ids_inv[curr_index]].T_E(0),
              input_aux_auth_data[permuted_eval_ids_inv[curr_index]].T_E(1),
              exec_received_garbled_tables[i] + CSEC_BYTES);

    std::copy(input_aux_auth_data[permuted_eval_ids_inv[curr_index]].inp_soldering(0),
              input_aux_auth_data[permuted_eval_ids_inv[curr_index]].inp_soldering(1),
              exec_write_inp_wire_commit_corrections[i]);

    std::fill(input_aux_auth_data[permuted_eval_ids_inv[curr_index]].inp_soldering(0),
              input_aux_auth_data[permuted_eval_ids_inv[curr_index]].inp_soldering(1), 0); //reset inp_soldering
  }

  ReceiveAndStoreComponentSolderings(EVAL_INP_BUCKET_PREFIX, exec_id, inp_bucket_circuit, input_auth_size, exec_permuted_aux_info, exec_input_buckets_from, exec_input_buckets_to, exec_received_garbled_tables);

  uint64_t exec_masks_write_pos = exec_input_buckets_from * input_auth_size * CSEC_BYTES;
  persistent_storage.WriteBuckets(EVAL_INP_BUCKET_PREFIX, INPUT_MASKS_CORRECTIONS, exec_input_buckets_from, exec_num_input_auths, exec_write_inp_wire_commit_corrections.data(), exec_masks_write_pos, exec_write_inp_wire_commit_corrections.size(), input_auth_size);
}

void DuploEvaluator::ReceiveAndStoreComponentSolderings(std::string component_type, uint32_t exec_id, Circuit & circuit, uint32_t bucket_size, PermutedIndex<EvalGarbledCircuit>& exec_permuted_aux_info, uint32_t exec_buckets_from, uint32_t exec_buckets_to, BYTEArrayVector & exec_received_garbled_tables) {

  uint64_t exec_num_buckets = exec_buckets_to - exec_buckets_from;
  uint64_t exec_num_eval_circuits = exec_num_buckets * bucket_size;

  uint64_t garbled_table_size = GarbledCircuit::TotalTableSize(circuit);
  uint64_t solderings_size = EvalGarbledCircuit::SolderingsSize(circuit);
  uint64_t eval_aux_size = EvalGarbledCircuit::AuxDataSize(circuit);

  BYTEArrayVector exec_write_head_auxdata(exec_num_buckets, eval_aux_size); //Does not include space for solderings as this will be part of exec_write_solderings array
  BYTEArrayVector exec_write_solderings(exec_num_eval_circuits, solderings_size);
  ////////////////////////////Soldering/////////////////////////////////////

  uint32_t num_soldering_circuits = (bucket_size - 1) * exec_num_buckets;

  uint32_t solder_num_inp_keys, solder_num_out_keys, solder_num_deltas, solder_num_commit_keys, solder_num_base_keys, solder_inp_keys_idx, solder_out_keys_idx, solder_deltas_idx;
  ComputeIndices(num_soldering_circuits, circuit, solder_num_inp_keys, solder_num_out_keys, solder_num_deltas, solder_num_commit_keys, solder_num_base_keys, solder_inp_keys_idx, solder_out_keys_idx, solder_deltas_idx);

  BYTEArrayVector solder_keys_share(solder_num_commit_keys + exec_num_buckets, CODEWORD_BYTES);

  osuCrypto::BitVector perm_bits(solder_num_commit_keys);
  exec_channels[exec_id].recv(perm_bits);

  //Receive the postulated solderings and check correctness using batch decommit
  BYTEArrayVector solder_keys(solder_num_commit_keys + exec_num_buckets, CSEC_BYTES);
  exec_channels[exec_id].recv(solder_keys.data(), solder_keys.size());

  BYTEArrayVector recov_shares;
  persistent_storage.ReadBuckets(EVAL_RECOV_PREFIX, RECOV, 0, 1, recov_shares);

  int curr_head_circuit, curr_circuit, curr_solder_read_pos;
  for (int i = 0; i < exec_num_buckets; ++i) {
    curr_head_circuit = i * bucket_size;

    //Copy the head aux info for writing to disc
    std::copy(exec_permuted_aux_info[curr_head_circuit], exec_permuted_aux_info[curr_head_circuit] + eval_aux_size, exec_write_head_auxdata[i]); //Must be + eval_aux_size here!

    //Copy the head input soldering info for writing to disc
    std::copy(EvalGarbledCircuit::inp_soldering(circuit, exec_permuted_aux_info[curr_head_circuit]),
              EvalGarbledCircuit::inp_soldering(circuit, exec_permuted_aux_info[curr_head_circuit]) + solderings_size,
              exec_write_solderings[curr_head_circuit]);

    //Add Recov share
    XOR_CodeWords(solder_keys_share[solder_num_commit_keys + i],
                  EvalGarbledCircuit::delta_share(circuit, exec_permuted_aux_info[curr_head_circuit]),
                  recov_shares.data());

    for (int l = 1; l < bucket_size; ++l) {
      curr_circuit = curr_head_circuit + l;
      curr_solder_read_pos = curr_circuit - (i + 1);

      //Add delta decommits
      std::copy(EvalGarbledCircuit::delta_share(circuit, exec_permuted_aux_info[curr_circuit]),
                EvalGarbledCircuit::delta_share(circuit, exec_permuted_aux_info[curr_circuit]) + CODEWORD_BYTES,
                solder_keys_share[solder_deltas_idx + curr_solder_read_pos]);
      XOR_CodeWords(solder_keys_share[solder_deltas_idx + curr_solder_read_pos], EvalGarbledCircuit::delta_share(circuit, exec_permuted_aux_info[curr_head_circuit]));

      //Copy curr delta soldering
      std::copy(solder_keys[solder_deltas_idx + curr_solder_read_pos],
                solder_keys[solder_deltas_idx + curr_solder_read_pos + 1],
                EvalGarbledCircuit::delta_soldering(circuit, exec_permuted_aux_info[curr_circuit]));

      //Ensures that lsb(head_delta)=1 as we know at least one component i in this bucket has lsb(delta_i)=1 due to cnc and these solderings are head_delta \xor delta_l for all other bucket components l.
      if (GetLSB(EvalGarbledCircuit::delta_soldering(circuit, exec_permuted_aux_info[curr_circuit]))) {
        std::cout << "Abort, head circuit with lsb(delta)=0 detected!" << std::endl;
        throw std::runtime_error("Abort, head circuit with lsb(delta)=0 detected!");
      }

      for (int j = 0; j < circuit.num_inp_wires; ++j) {

        //Add input decommits
        std::copy(EvalGarbledCircuit::inp_key_share(circuit, exec_permuted_aux_info[curr_circuit], j),
                  EvalGarbledCircuit::inp_key_share(circuit, exec_permuted_aux_info[curr_circuit], j + 1),
                  solder_keys_share[solder_inp_keys_idx + curr_solder_read_pos * circuit.num_inp_wires + j]);
        XOR_CodeWords(solder_keys_share[solder_inp_keys_idx + curr_solder_read_pos * circuit.num_inp_wires + j], EvalGarbledCircuit::inp_key_share(circuit, exec_permuted_aux_info[curr_head_circuit], j));

        if (perm_bits[solder_inp_keys_idx + curr_solder_read_pos * circuit.num_inp_wires + j] && GetLSB(solder_keys[solder_inp_keys_idx + curr_solder_read_pos * circuit.num_inp_wires + j])) {
          std::cout << "Constructor lying about inp_perm_bits!" << std::endl;
          throw std::runtime_error("Constructor lying about inp_perm_bits!");
        }

        //Add input decommits
        if (perm_bits[solder_inp_keys_idx + curr_solder_read_pos * circuit.num_inp_wires + j]) {
          XOR_CodeWords(solder_keys_share[solder_inp_keys_idx + curr_solder_read_pos * circuit.num_inp_wires + j], EvalGarbledCircuit::delta_share(circuit, exec_permuted_aux_info[curr_circuit]));
        }

        //Add input solderings
        std::copy(solder_keys[solder_inp_keys_idx + curr_solder_read_pos * circuit.num_inp_wires + j],
                  solder_keys[solder_inp_keys_idx + curr_solder_read_pos * circuit.num_inp_wires + j + 1],
                  EvalGarbledCircuit::inp_soldering(circuit, exec_permuted_aux_info[curr_circuit], j));

        XORLSB(EvalGarbledCircuit::inp_soldering(circuit, exec_permuted_aux_info[curr_circuit], j), perm_bits[solder_inp_keys_idx + curr_solder_read_pos * circuit.num_inp_wires + j]);
      }

      for (int j = 0; j < circuit.num_out_wires; ++j) {

        //Add output decommits
        std::copy(EvalGarbledCircuit::out_key_share(circuit, exec_permuted_aux_info[curr_circuit], j),
                  EvalGarbledCircuit::out_key_share(circuit, exec_permuted_aux_info[curr_circuit], j + 1),
                  solder_keys_share[solder_out_keys_idx + curr_solder_read_pos * circuit.num_out_wires + j]);
        XOR_CodeWords(solder_keys_share[solder_out_keys_idx + curr_solder_read_pos * circuit.num_out_wires + j], EvalGarbledCircuit::out_key_share(circuit, exec_permuted_aux_info[curr_head_circuit], j));

        if (perm_bits[solder_out_keys_idx + curr_solder_read_pos * circuit.num_out_wires + j] && GetLSB(solder_keys[solder_out_keys_idx + curr_solder_read_pos * circuit.num_out_wires + j])) {
          std::cout << "Constructor lying about out_perm_bits!" << std::endl;
          throw std::runtime_error("Constructor lying about out_perm_bits!");
        }

        if (perm_bits[solder_out_keys_idx + curr_solder_read_pos * circuit.num_out_wires + j]) {
          XOR_CodeWords(solder_keys_share[solder_out_keys_idx + curr_solder_read_pos * circuit.num_out_wires + j], EvalGarbledCircuit::delta_share(circuit, exec_permuted_aux_info[curr_circuit]));
        }

        //Copy output solderings
        XOR_128(EvalGarbledCircuit::out_soldering(circuit, exec_permuted_aux_info[curr_circuit], j),
                solder_keys[solder_out_keys_idx + curr_solder_read_pos * circuit.num_out_wires + j]);

        XORLSB(EvalGarbledCircuit::out_soldering(circuit, exec_permuted_aux_info[curr_circuit], j), perm_bits[solder_out_keys_idx + curr_solder_read_pos * circuit.num_out_wires + j]);
      }

      //Copy curr soldering info for writing to disc
      std::copy(EvalGarbledCircuit::inp_soldering(circuit, exec_permuted_aux_info[curr_circuit]),
                EvalGarbledCircuit::inp_soldering(circuit, exec_permuted_aux_info[curr_circuit]) + solderings_size, exec_write_solderings[curr_circuit]);
    }
  }

  if (!commit_receivers[exec_id].BatchDecommit(solder_keys_share, solder_keys, exec_rnds[exec_id], exec_channels[exec_id], true)) {
    std::cout << "Abort, soldering decommit failed!" << std::endl;
    throw std::runtime_error("Abort, soldering decommit failed!");
  }

  //////////////////////////////Write to Disc/////////////////////////////////

  uint64_t exec_tables_write_pos = exec_buckets_from * bucket_size * garbled_table_size;
  persistent_storage.WriteBuckets(component_type, TABLES, exec_buckets_from, exec_num_buckets, exec_received_garbled_tables.data(), exec_tables_write_pos, exec_received_garbled_tables.size(), bucket_size);

  uint64_t exec_solderings_write_pos = exec_buckets_from * bucket_size * solderings_size;
  persistent_storage.WriteBuckets(component_type, SOLDERINGS, exec_buckets_from, exec_num_buckets, exec_write_solderings.data(), exec_solderings_write_pos, exec_write_solderings.size(), bucket_size);

  uint64_t exec_auxdata_write_pos = exec_buckets_from * eval_aux_size;
  persistent_storage.WriteBuckets(component_type, AUXDATA, exec_buckets_from, exec_num_buckets, exec_write_head_auxdata.data(), exec_auxdata_write_pos, exec_write_head_auxdata.size(), 1);

  uint64_t exec_recov_data_write_pos = exec_buckets_from * CSEC_BYTES;
  persistent_storage.WriteBuckets(component_type, RECOV_SOLD, exec_buckets_from, exec_num_buckets, solder_keys[solder_num_commit_keys], exec_recov_data_write_pos, exec_num_buckets * solder_keys.entry_size(), 1);
}

void DuploEvaluator::ReceiveAndStoreComposedSolderings(uint32_t exec_id, ComposedCircuit & composed_circuit, uint32_t inp_wire_components_from, uint32_t inp_wire_components_to) {

  uint32_t exec_num_inp_wire_components = inp_wire_components_to - inp_wire_components_from;

  if (exec_num_inp_wire_components == 0) {
    return;
  }

  std::vector<uint32_t> inp_pos;
  uint32_t exec_num_total_wire_solderings = 0;
  uint32_t exec_num_total_deltas = 0;
  for (int l = 0; l < exec_num_inp_wire_components; ++l) {
    uint32_t curr_component = inp_wire_components_from + l;

    inp_pos.emplace_back(exec_num_total_wire_solderings + exec_num_total_deltas);

    std::string inp_wire_component_name(std::get<0>(composed_circuit.circuits[curr_component]));
    Circuit& circuit = string_to_circuit_map[inp_wire_component_name];

    exec_num_total_wire_solderings += circuit.num_inp_wires;
    exec_num_total_deltas += composed_circuit.out_wire_holders[curr_component].size();
  }

  std::vector<uint32_t> global_inp_pos;
  uint32_t global_num_solderings_counter = 0;
  for (int l = 0; l < composed_circuit.circuits.size(); ++l) {

    global_inp_pos.emplace_back(global_num_solderings_counter);

    std::string inp_wire_component_name(std::get<0>(composed_circuit.circuits[l]));
    Circuit& circuit = string_to_circuit_map[inp_wire_component_name];
    global_num_solderings_counter += (circuit.num_inp_wires + composed_circuit.out_wire_holders[l].size());
  }

  uint32_t exec_num_total_solderings = exec_num_total_wire_solderings + exec_num_total_deltas;

  BYTEArrayVector solder_keys_share(exec_num_total_solderings, CODEWORD_BYTES);

  osuCrypto::BitVector perm_bits(exec_num_total_solderings);
  exec_channels[exec_id].recv(perm_bits);

  //Receive the postulated solderings and check correctness using batch decommit
  BYTEArrayVector solder_keys(exec_num_total_solderings, CSEC_BYTES);
  exec_channels[exec_id].recv(solder_keys.data(), solder_keys.size());

  std::vector<std::pair<uint32_t, bool>> revert_pair;

  for (int l = 0; l < exec_num_inp_wire_components; ++l) {
    uint32_t curr_component = inp_wire_components_from + l;

    std::string inp_wire_component_name(std::get<0>(composed_circuit.circuits[curr_component]));
    uint32_t inp_wire_component_num(std::get<1>(composed_circuit.circuits[curr_component]));
    Circuit& inp_circuit = string_to_circuit_map[inp_wire_component_name];

    BYTEArrayVector inp_wire_component_aux_info;
    persistent_storage.ReadBuckets(inp_wire_component_name, AUXDATA, inp_wire_component_num, 1, inp_wire_component_aux_info, true, exec_id);

    //Add all input component values to soldering
    for (int j = 0; j < inp_circuit.num_inp_wires; ++j) {
      std::copy(EvalGarbledCircuit::inp_key_share(inp_circuit, inp_wire_component_aux_info.data(), j),
                EvalGarbledCircuit::inp_key_share(inp_circuit, inp_wire_component_aux_info.data(), j + 1),
                solder_keys_share[inp_pos[l] + j]);
    }

    uint32_t curr_inp_wire = 0;
    for (int i = 0; i < composed_circuit.out_wire_holders[curr_component].size(); ++i) {

      //Add delta solderings from input wire component to all delta positions
      std::copy(EvalGarbledCircuit::delta_share(inp_circuit, inp_wire_component_aux_info.data()),
                EvalGarbledCircuit::delta_share(inp_circuit, inp_wire_component_aux_info.data()) + CODEWORD_BYTES,
                solder_keys_share[inp_pos[l] + inp_circuit.num_inp_wires + i]);

      //Get current circuit information and soldering indices
      std::pair<uint32_t, std::vector<uint32_t>>& out_wire_component_pair = composed_circuit.out_wire_holders[curr_component][i];
      std::pair<std::string, uint32_t> out_wire_component;
      if (std::get<0>(out_wire_component_pair) == std::numeric_limits<uint32_t>::max()) {
        out_wire_component.first = EVAL_INP_BUCKET_PREFIX;
        out_wire_component.second = std::get<1>(out_wire_component_pair)[0]; //input_bucket_index
      } else {
        out_wire_component = composed_circuit.circuits[std::get<0>(out_wire_component_pair)];
      }

      std::string out_wire_component_name = std::get<0>(out_wire_component);
      uint32_t out_wire_component_num = std::get<1>(out_wire_component);
      Circuit& out_circuit = string_to_circuit_map[out_wire_component_name];

      //Read current output circuit
      BYTEArrayVector out_wire_component_aux_info;
      persistent_storage.ReadBuckets(out_wire_component_name, AUXDATA, out_wire_component_num, 1, out_wire_component_aux_info, true, exec_id);

      //Add the delta soldering information
      XOR_CodeWords(solder_keys_share[inp_pos[l] + inp_circuit.num_inp_wires + i], EvalGarbledCircuit::delta_share(out_circuit, out_wire_component_aux_info.data()));

      //Run through the soldering pairs and apply the soldering
      std::vector<uint32_t>& out_wires = std::get<1>(out_wire_component_pair);


      for (int j = 0; j < out_wires.size(); ++j) {

        if (std::get<0>(out_wire_component_pair) == std::numeric_limits<uint32_t>::max()) {

          XOR_CodeWords(solder_keys_share[inp_pos[l] + curr_inp_wire], EvalGarbledCircuit::inp_key_share(out_circuit, out_wire_component_aux_info.data()));

        } else {
          XOR_CodeWords(solder_keys_share[inp_pos[l] + curr_inp_wire], EvalGarbledCircuit::out_key_share(out_circuit, out_wire_component_aux_info.data(), out_wires[j]));
        }

        if (perm_bits[inp_pos[l] + curr_inp_wire] && GetLSB(solder_keys[inp_pos[l] + curr_inp_wire])) {
          std::cout << "Constructor lying about inp_perm_bits!" << std::endl;
          throw std::runtime_error("Constructor lying about inp_perm_bits!");
        }

        XORLSB(solder_keys[inp_pos[l] + curr_inp_wire], perm_bits[inp_pos[l] + curr_inp_wire]);
        revert_pair.emplace_back(inp_pos[l] + curr_inp_wire, perm_bits[inp_pos[l] + curr_inp_wire]);

        if (perm_bits[inp_pos[l] + curr_inp_wire]) {

          XOR_CodeWords(solder_keys_share[inp_pos[l] + curr_inp_wire], EvalGarbledCircuit::delta_share(inp_circuit, inp_wire_component_aux_info.data()));
        }
        ++curr_inp_wire;
      }
    }

    persistent_storage.WriteBuckets(composed_circuit.composed_circuit_name, VERTICAL_SOLDERINGS, curr_component, 1, solder_keys[inp_pos[l]], global_inp_pos[curr_component] * CSEC_BYTES, (inp_circuit.num_inp_wires + composed_circuit.out_wire_holders[curr_component].size()) * CSEC_BYTES, 1);
  }

  for (int i = 0; i < revert_pair.size(); ++i) {
    XORLSB(solder_keys[revert_pair[i].first], revert_pair[i].second);
  }

  if (!commit_receivers[exec_id].BatchDecommit(solder_keys_share, solder_keys, exec_rnds[exec_id], exec_channels[exec_id], true)) {
    std::cout << "Abort, vertical soldering decommit failed!" << std::endl;
    throw std::runtime_error("Abort, vertical soldering decommit failed!");
  }
}

void DuploEvaluator::DecommitInpPermBits(uint32_t exec_id, uint32_t offset, uint32_t eval_inputs_from, uint32_t eval_inputs_to) {

  uint32_t exec_num_inputs = eval_inputs_to - eval_inputs_from;

  if (exec_num_inputs < 1) {
    return;
  }

  BYTEArrayVector commit_shares_lsb_blind(SSEC, CODEWORD_BYTES);
  if (!commit_receivers[exec_id].Commit(commit_shares_lsb_blind, exec_rnds[exec_id], exec_channels[exec_id], std::numeric_limits<uint32_t>::max(), ALL_RND_LSB_ZERO)) {
    std::cout << "Abort, inp blind commit failed!" << std::endl;
    throw std::runtime_error("Abort, inp blind commit failed!");
  }

  //Receive the postulated input eval decommit bits
  osuCrypto::BitVector decommit_lsb(exec_num_inputs);
  exec_channels[exec_id].recv(decommit_lsb);

  BYTEArrayVector decommit_lsb_share(exec_num_inputs, CODEWORD_BYTES);

  BYTEArrayVector inp_bucket_aux_data;
  persistent_storage.ReadBuckets(EVAL_INP_BUCKET_PREFIX, AUXDATA, (offset + eval_inputs_from), exec_num_inputs, inp_bucket_aux_data);

  std::vector<uint8_t> decommit_lsb_bytes(exec_num_inputs);

  for (int i = (offset + eval_inputs_from); i < (offset + eval_inputs_to); ++i) {
    uint32_t curr_local_idx = i - (offset + eval_inputs_from);

    std::copy(EvalGarbledCircuit::inp_key_share(inp_bucket_circuit, inp_bucket_aux_data[curr_local_idx], 0),
              EvalGarbledCircuit::inp_key_share(inp_bucket_circuit, inp_bucket_aux_data[curr_local_idx], 1),
              decommit_lsb_share[curr_local_idx]);
    decommit_lsb_bytes[curr_local_idx] = decommit_lsb[curr_local_idx];
  }

  if (!commit_receivers[exec_id].BatchDecommitLSB(decommit_lsb_share, decommit_lsb, commit_shares_lsb_blind, exec_rnds[exec_id], exec_channels[exec_id])) {
    std::cout << "Abort, inp blind lsb decommit failed!" << std::endl;
    throw std::runtime_error("Abort, inp blind lsb decommit failed!");
  }

  //Write eval decommit bits to disc. Notice we write them before actually verifying correctness. Doesn't matter for security though as we just abort if they are incorrect.
  persistent_storage.WriteBuckets(EVAL_INP_BUCKET_PREFIX, INPUT_PERM_BITS, eval_inputs_from, exec_num_inputs, decommit_lsb_bytes.data(), eval_inputs_from, decommit_lsb_bytes.size(), 1); //Notice written as bytes!
}

void DuploEvaluator::PreprocessEvalInputOTs(uint32_t exec_id, uint32_t eval_inputs_from, uint32_t eval_inputs_to) {

  uint32_t exec_num_eval_inputs = eval_inputs_to - eval_inputs_from;

  if (exec_num_eval_inputs == 0) {
    return;
  }

  uint32_t num_ots;
  if (exec_id == 0) {
    num_ots = exec_num_eval_inputs + SSEC;
  } else {
    num_ots = exec_num_eval_inputs;
  }

  BYTEArrayVector input_masks(num_ots, CSEC_BYTES);

  std::vector<osuCrypto::block> msgs(num_ots);
  osuCrypto::BitVector input_masks_choices(num_ots);
  input_masks_choices.randomize(exec_rnds[exec_id]);

  dot_receivers[exec_id]->receive(input_masks_choices, msgs, exec_rnds[exec_id], exec_channels[exec_id]);

  for (int i = 0; i < num_ots; ++i) {
    _mm_storeu_si128((__m128i*) input_masks[i], msgs[i]);
  }

  auto otx_end = GET_TIME();

  //Run Random Commit
  uint32_t num_commits = num_ots + 1;
  BYTEArrayVector commit_keys_share(num_commits, CODEWORD_BYTES);

  if (!commit_receivers[exec_id].Commit(commit_keys_share, exec_rnds[exec_id], exec_channels[exec_id])) {
    std::cout << "Abort, PreprocessEvalInputOTs random commit failed!" << std::endl;
    throw std::runtime_error("Abort, PreprocessEvalInputOTs random commit failed!");
  }

  //Run chosen commit
  BYTEArrayVector input_mask_corrections(num_commits, CSEC_BYTES);
  exec_channels[exec_id].recv(input_mask_corrections.data(), input_mask_corrections.size());


  //////////////////////////////////////CNC////////////////////////////////////
  if (exec_id == 0) {
    //Send own values to sender
    BYTEArrayVector cnc_ot_values(SSEC, CSEC_BYTES);
    osuCrypto::BitVector ot_delta_cnc_choices(SSEC);

    std::copy(input_masks[exec_num_eval_inputs], input_masks[num_ots], cnc_ot_values.data());

    for (int i = 0; i < SSEC; ++i) {
      ot_delta_cnc_choices[i] = input_masks_choices[exec_num_eval_inputs + i];
    }

    exec_channels[exec_id].send(cnc_ot_values.data(), cnc_ot_values.size());
    exec_channels[exec_id].send(ot_delta_cnc_choices.data(), ot_delta_cnc_choices.sizeBytes());

    //Compute decommit shares
    BYTEArrayVector chosen_decommit_shares(SSEC, CODEWORD_BYTES);
    for (int i = 0; i < SSEC; ++i) {
      std::copy(commit_keys_share[exec_num_eval_inputs + i], commit_keys_share[exec_num_eval_inputs + i + 1], chosen_decommit_shares[i]);

      if (ot_delta_cnc_choices[i]) {
        XOR_CodeWords(chosen_decommit_shares[i], commit_keys_share[num_ots]);
      }
    }

    //Receive decommits
    BYTEArrayVector decomitted_values(SSEC, CSEC_BYTES);
    if (!commit_receivers[exec_id].Decommit(chosen_decommit_shares, decomitted_values, exec_channels[exec_id])) {
      std::cout << "Sender decommit fail in OT CNC!" << std::endl;
      throw std::runtime_error("Sender decommit fail in OT CNC!");
    }

    //Apply the corrections
    uint8_t chosen_decommit_val[CSEC_BYTES];
    for (int i = 0; i < SSEC; ++i) {
      XOR_128(chosen_decommit_val, decomitted_values[i], input_mask_corrections[exec_num_eval_inputs + i]);
      if (input_masks_choices[exec_num_eval_inputs + i]) {
        XOR_128(chosen_decommit_val, input_mask_corrections[num_ots]);
      }

      //Check if they match known value
      if (!std::equal(input_masks[exec_num_eval_inputs + i], input_masks[exec_num_eval_inputs + i + 1], chosen_decommit_val)) {
        std::cout << "Sender cheating in OT CNC. Decomitted to wrong values. Did not commit to Delta!" << std::endl;
        throw std::runtime_error("Sender cheating in OT CNC. Decomitted to wrong values. Did not commit to Delta!");
      }
    }
  }
//////////////////////////////////////CNC////////////////////////////////////

  state_mutex.lock();
  curr_num_ready_inputs += exec_num_eval_inputs;
  state_mutex.unlock();

  BYTEArrayVector shares(exec_num_eval_inputs, CODEWORD_BYTES + 1);
  for (int i = 0; i < exec_num_eval_inputs; ++i) {
    std::copy(commit_keys_share[i], commit_keys_share[i + 1], shares[i]);
    if (input_masks_choices[i]) {
      *(shares[i] + CODEWORD_BYTES) = 1;
    } //else it's zero

    XOR_128(input_mask_corrections[i], input_masks[i]); // turns input_mask_corrections[i] into committed value
  }

  persistent_storage.WriteBuckets(EVAL_PREPROCESS_PREFIX, INPUT_MASKS_AUXDATA, eval_inputs_from, exec_num_eval_inputs, shares.data(), eval_inputs_from * (CODEWORD_BYTES + 1), exec_num_eval_inputs * (CODEWORD_BYTES + 1), 1);

  persistent_storage.WriteBuckets(EVAL_PREPROCESS_PREFIX, INPUT_MASKS_CORRECTIONS, eval_inputs_from, exec_num_eval_inputs, input_mask_corrections.data(), eval_inputs_from * CSEC_BYTES, exec_num_eval_inputs * CSEC_BYTES, 1);

  if (exec_id == 0) {
    persistent_storage.WriteBuckets(EVAL_PREPROCESS_PREFIX, INPUT_MASKS_DELTA_AUXDATA, 0, 1, commit_keys_share[num_ots], 0, CODEWORD_BYTES, 1);
    persistent_storage.WriteBuckets(EVAL_PREPROCESS_PREFIX, INPUT_MASKS_DELTA_CORRECTION, 0, 1, input_mask_corrections[num_ots], 0, CSEC_BYTES, 1);
  }
}

void DuploEvaluator::PreprocessBlindOutPermBits(uint32_t exec_id, uint32_t eval_outputs_from, uint32_t eval_outputs_to) {

  uint32_t exec_num_outputs = eval_outputs_to - eval_outputs_from;

  if (exec_num_outputs == 0) {
    return;
  }

  BYTEArrayVector commit_shares_lsb_blind(exec_num_outputs, CODEWORD_BYTES);
  if (!commit_receivers[exec_id].Commit(commit_shares_lsb_blind, exec_rnds[exec_id], exec_channels[exec_id], std::numeric_limits<uint32_t>::max(), ALL_RND_LSB_ZERO)) {
    std::cout << "Abort, out blind commit failed!" << std::endl;
    throw std::runtime_error("Abort, out blind commit failed!");
  }

  persistent_storage.WriteBuckets(EVAL_PREPROCESS_PREFIX, OUTPUT_MASKS_AUXDATA, eval_outputs_from, exec_num_outputs, commit_shares_lsb_blind.data(), eval_outputs_from * CODEWORD_BYTES, commit_shares_lsb_blind.size(), 1);
}

void DuploEvaluator::GetInputs(uint32_t exec_id, ComposedCircuit & composed_circuit, osuCrypto::BitVector& plain_inputs, uint32_t const_inputs_from, uint32_t const_inputs_to, uint32_t eval_inputs_from, uint32_t eval_inputs_to, BYTEArrayVector & wire_keys) {

  uint32_t exec_num_const_inputs = const_inputs_to - const_inputs_from;
  uint32_t exec_num_eval_inputs = eval_inputs_to - eval_inputs_from;

  //Read all input mask data from disk
  BYTEArrayVector input_masks_shares;
  BYTEArrayVector input_mask_corrections;
  BYTEArrayVector input_masks_delta_share;
  BYTEArrayVector input_mask_delta_correction;
  std::vector<uint8_t> choice_bits(exec_num_eval_inputs);

  persistent_storage.ReadBuckets(EVAL_PREPROCESS_PREFIX, INPUT_MASKS_AUXDATA, eval_inputs_from, exec_num_eval_inputs, input_masks_shares);
  persistent_storage.ReadBuckets(EVAL_PREPROCESS_PREFIX, INPUT_MASKS_CORRECTIONS, eval_inputs_from, exec_num_eval_inputs, input_mask_corrections);

  persistent_storage.ReadBuckets(EVAL_PREPROCESS_PREFIX, INPUT_MASKS_DELTA_AUXDATA, 0, 1, input_masks_delta_share);
  persistent_storage.ReadBuckets(EVAL_PREPROCESS_PREFIX, INPUT_MASKS_DELTA_CORRECTION, 0, 1, input_mask_delta_correction);

  for (int i = 0; i < exec_num_eval_inputs; ++i) {
    choice_bits[i] = (*(input_masks_shares[i] + CODEWORD_BYTES) == 1);
  }

  //Construct more aux info, construct the choice vector e_bytes and the decommit shares
  BYTEArrayVector eval_key_shares(2 * exec_num_eval_inputs, CODEWORD_BYTES);
  uint32_t eval_delta_offset = exec_num_eval_inputs;

  std::vector<uint8_t> e_bytes(exec_num_eval_inputs);
  std::vector<uint8_t> check_bytes(exec_num_eval_inputs);

  BYTEArrayVector input_perm_bits; //read as bytes!
  persistent_storage.ReadBuckets(EVAL_INP_BUCKET_PREFIX, INPUT_PERM_BITS, eval_inputs_from, exec_num_eval_inputs, input_perm_bits); //starts at 0, not at num_const_inp_wires

  BYTEArrayVector inp_bucket_aux_data;
  persistent_storage.ReadBuckets(EVAL_INP_BUCKET_PREFIX, AUXDATA, composed_circuit.num_const_inp_wires + eval_inputs_from, exec_num_eval_inputs, inp_bucket_aux_data); //Default prefetching

  for (int i = eval_inputs_from; i < eval_inputs_to; ++i) {
    uint32_t curr_local_idx = i - eval_inputs_from;

    //e = y
    e_bytes[curr_local_idx] = plain_inputs[i];
    //e = y \xor b
    e_bytes[curr_local_idx] ^= *(input_perm_bits.data() + curr_local_idx);

    //Store bits for later checking
    check_bytes[curr_local_idx] = e_bytes[curr_local_idx];

    //e = y \xor b \xor c
    e_bytes[curr_local_idx] ^= choice_bits[curr_local_idx];

    //First copy eval key share
    std::copy(EvalGarbledCircuit::inp_key_share(inp_bucket_circuit, inp_bucket_aux_data[curr_local_idx], 0),
              EvalGarbledCircuit::inp_key_share(inp_bucket_circuit, inp_bucket_aux_data[curr_local_idx], 1),
              eval_key_shares[curr_local_idx]);

    //XOR with mask key shares
    XOR_CodeWords(eval_key_shares[curr_local_idx], input_masks_shares[curr_local_idx]);

    //If e_j for this component is set XOR input_mask_delta onto the decommit
    if (e_bytes[curr_local_idx]) {
      XOR_CodeWords(eval_key_shares[curr_local_idx], input_masks_delta_share.data());
    }

    //Construct current component delta soldering share
    std::copy(EvalGarbledCircuit::delta_share(inp_bucket_circuit, inp_bucket_aux_data[curr_local_idx]),
              EvalGarbledCircuit::delta_share(inp_bucket_circuit, inp_bucket_aux_data[curr_local_idx]) + CODEWORD_BYTES,
              eval_key_shares[eval_delta_offset + curr_local_idx]);
    //XOR with mask keys
    XOR_CodeWords(eval_key_shares[eval_delta_offset + curr_local_idx], input_masks_delta_share.data());
  }

  osuCrypto::BitVector e(exec_num_eval_inputs);

  //Convert from bytes to bits
  for (int i = 0; i < exec_num_eval_inputs; ++i) {
    e[i] = e_bytes[i];
  }

  if (exec_num_eval_inputs != 0) {
    exec_channels[exec_id].send(e.data(), e.sizeBytes());
  }

  BYTEArrayVector const_keys(exec_num_const_inputs, CSEC_BYTES);
  if (exec_num_const_inputs != 0) {
    exec_channels[exec_id].recv(const_keys.data(), const_keys.size());
  }

  BYTEArrayVector decomitted_inp_values(2 * exec_num_eval_inputs, CSEC_BYTES);
  if (exec_num_eval_inputs != 0) {
    //Check the decommits
    if (!commit_receivers[exec_id].Decommit(eval_key_shares, decomitted_inp_values, exec_channels[exec_id])) {
      std::cout << "Sender fail inp decommit in inp delivery!" << std::endl;
      throw std::runtime_error("Abort, Sender fail inp decommit in inp delivery!");
    }
  }

  std::copy(const_keys.data(), const_keys.data() + const_keys.size(), wire_keys[const_inputs_from]);

  //Compute and store the resulting keys to input_keys
  for (int i = eval_inputs_from; i < eval_inputs_to; ++i) {
    uint32_t curr_local_idx = i - eval_inputs_from;
    std::copy(decomitted_inp_values[curr_local_idx],
              decomitted_inp_values[curr_local_idx + 1],
              wire_keys[composed_circuit.num_const_inp_wires + i]);
    XOR_128(wire_keys[composed_circuit.num_const_inp_wires + i], input_mask_corrections[curr_local_idx]);

    if (choice_bits[curr_local_idx]) { //choicebits == 1
      XOR_128(wire_keys[composed_circuit.num_const_inp_wires + i], input_mask_delta_correction.data());
    }

    if (choice_bits[curr_local_idx] ^
        e_bytes[curr_local_idx]) { //input_perm bits == 1
      XOR_128(wire_keys[composed_circuit.num_const_inp_wires + i], decomitted_inp_values[eval_delta_offset + curr_local_idx]);
    }

    XORLSB(wire_keys[composed_circuit.num_const_inp_wires + i], *(input_perm_bits.data() + curr_local_idx));

    //Ensure that sender did not flip the input key by comitting to r_0 \xor Delta_i. If the key isn't flipped, but he still didn't commit to r_0, the EvalBuckets will reject as they also check the key (if it is either 0 or 1).
    if (GetLSB(wire_keys[composed_circuit.num_const_inp_wires + i]) != check_bytes[curr_local_idx]) {
      std::cout << "Abort, sender cheated earlier in input commit, trying to flip input!" << std::endl;
      throw std::runtime_error("Abort, sender cheated earlier in input commit, trying to flip input!");
    }
  }

  AuthenticateInputKeys(exec_id, wire_keys, const_inputs_from, const_inputs_to);
  AuthenticateInputKeys(exec_id, wire_keys, composed_circuit.num_const_inp_wires + eval_inputs_from, composed_circuit.num_const_inp_wires + eval_inputs_to);
}

void DuploEvaluator::AuthenticateInputKeys(uint32_t exec_id, BYTEArrayVector & wire_keys, uint32_t inputs_from, uint32_t inputs_to) {

  uint32_t exec_num_inputs = inputs_to - inputs_from;

  BYTEArrayVector read_auths_input;
  BYTEArrayVector read_auths_input_solderings;
  BYTEArrayVector read_auths_input_corrections;

  persistent_storage.ReadBuckets(EVAL_INP_BUCKET_PREFIX, TABLES, inputs_from, exec_num_inputs, read_auths_input);
  uint32_t input_auth_size = read_auths_input.entry_size() / (2 * CSEC_BYTES);

  persistent_storage.ReadBuckets(EVAL_INP_BUCKET_PREFIX, SOLDERINGS, inputs_from, exec_num_inputs, read_auths_input_solderings);

  persistent_storage.ReadBuckets(EVAL_INP_BUCKET_PREFIX, INPUT_MASKS_CORRECTIONS, inputs_from, exec_num_inputs, read_auths_input_corrections);

  GarblingHandler gh;
  for (int i = inputs_from; i < inputs_to; ++i) {
    uint32_t curr_local_idx = i - inputs_from;
    uint8_t soldered_input[CSEC_BYTES];
    uint32_t num_accepts = 0;
    for (uint32_t a = 0; a < input_auth_size; ++a) {

      std::copy(wire_keys[i], wire_keys[i + 1], soldered_input);
      if (GetLSB(soldered_input)) {
        XOR_128(soldered_input, read_auths_input_solderings[curr_local_idx] + (2 * a + 1) * CSEC_BYTES);
      }

      XOR_128(soldered_input, read_auths_input_solderings[curr_local_idx] + 2 * a * CSEC_BYTES);

      XOR_128(soldered_input, read_auths_input_corrections[curr_local_idx] + a * CSEC_BYTES);

      if (gh.VerifyUsingInpBucket(read_auths_input[curr_local_idx] + 2 * a * CSEC_BYTES, read_auths_input[curr_local_idx] + (2 * a + 1) * CSEC_BYTES, soldered_input)) {
        ++num_accepts;
      }
    }

    if (num_accepts < input_auth_size / 2) {
      std::cout << "Abort, wrong input keys detected by input buckets!" << std::endl;
      throw std::runtime_error("Abort, wrong input keys detected by input buckets!");
    }
  }
}

void DuploEvaluator::EvaluateComponent(uint32_t exec_id, ComposedCircuit & composed_circuit, uint32_t components_from, uint32_t components_to, uint32_t exec_layer_num, BYTEArrayVector & wire_keys, bool single_eval) {

  uint32_t num_components = components_to - components_from;

  for (int l = 0; l < num_components; ++l) {
    uint32_t curr_component_idx = composed_circuit.circuits_in_layer[exec_layer_num][components_from + l];
    std::string component_name = std::get<0>(composed_circuit.circuits[curr_component_idx]);
    Circuit& circuit = string_to_circuit_map[component_name];

    BYTEArrayVector curr_inp_keys(circuit.num_inp_wires, CSEC_BYTES);
    BYTEArrayVector read_solderings;
    persistent_storage.ReadBuckets(composed_circuit.composed_circuit_name, VERTICAL_SOLDERINGS, curr_component_idx, 1, read_solderings, true, exec_id);

    uint32_t curr_inp_wire = 0;
    for (int i = 0; i < composed_circuit.out_wire_holders[curr_component_idx].size(); ++i) {

      std::pair<uint32_t, std::vector<uint32_t>>& curr_out_wire_holder_pair = composed_circuit.out_wire_holders[curr_component_idx][i];
      uint32_t circuit_idx = std::get<0>(curr_out_wire_holder_pair);
      std::vector<uint32_t> out_wires = std::get<1>(curr_out_wire_holder_pair);

      uint32_t out_start_pos = composed_circuit.out_wire_holder_to_wire_idx[circuit_idx];

      for (int j = 0; j < out_wires.size(); ++j) {
        std::copy(wire_keys[out_start_pos + out_wires[j]], wire_keys[out_start_pos + out_wires[j] + 1], curr_inp_keys[curr_inp_wire]);

        if (GetLSB(curr_inp_keys[curr_inp_wire])) {
          XOR_128(curr_inp_keys[curr_inp_wire], read_solderings.data() + (circuit.num_inp_wires + i) * CSEC_BYTES);
        }

        ++curr_inp_wire;
      }
    }

    //XOR onto current non-head circuit the input solderings of head circuit. Effectively applying vertical solderings
    XOR_UINT8_T(curr_inp_keys.data(), read_solderings.data(), circuit.num_inp_wires * CSEC_BYTES);

    //Where to write out_keys for this component
    uint32_t component_out_start_pos = composed_circuit.out_wire_holder_to_wire_idx[curr_component_idx];

    if (single_eval) { // we evaluate in serial
      EvaluateBucketParallel(exec_id, composed_circuit.circuits[curr_component_idx], curr_inp_keys.data(), wire_keys[component_out_start_pos]); //several threads together eval a bucket
    } else { // we evaluate in parallel
      EvaluateBucketSerial(exec_id, composed_circuit.circuits[curr_component_idx], curr_inp_keys.data(), wire_keys[component_out_start_pos]); //several threads eval different buckets
    }

  }
}

void DuploEvaluator::EvaluateBucketSerial(uint32_t exec_id, std::pair<std::string, uint32_t>& component, uint8_t input_keys[], uint8_t wire_keys[]) {

  std::string component_name = std::get<0>(component);
  uint32_t component_num = std::get<1>(component);
  Circuit& circuit = string_to_circuit_map[component_name];

  GarblingHandler gh;
  uint32_t garbled_table_size = GarbledCircuit::TotalTableSize(circuit);
  uint32_t solderings_size = EvalGarbledCircuit::SolderingsSize(circuit);

  BYTEArrayVector read_bucket;
  BYTEArrayVector read_solderings;

  persistent_storage.ReadBuckets(component_name, TABLES, component_num, 1, read_bucket);
  persistent_storage.ReadBuckets(component_name, SOLDERINGS, component_num, 1, read_solderings);

  uint32_t bucket_size = read_bucket.entry_size() / garbled_table_size;

  bool success = true;
  std::vector<BYTEArrayVector> cand_outputs(bucket_size, BYTEArrayVector(circuit.num_out_wires, CSEC_BYTES));
  for (uint32_t i = 0; i < bucket_size; ++i) {

    uint8_t* curr_garbled_tables = read_bucket.data() + i * garbled_table_size;
    uint8_t* curr_solderings = read_solderings.data() + i * solderings_size;

    gh.EvalGarbledCircuitSolderings(input_keys, circuit, curr_garbled_tables, curr_solderings, cand_outputs[i].data());

    if (i != 0) {
      if (!std::equal(cand_outputs[0][0], cand_outputs[0][circuit.num_out_wires], cand_outputs[i].data())) {
        success = false;
      }
    }
  }
  if (success) {
    std::copy(cand_outputs[0][0], cand_outputs[0][circuit.num_out_wires], wire_keys);
  } else {
    //Error handling, evaluate output authenticators, etc etc
    std::cout << "Problem! Bucket doesn't output the same!" << std::endl;
  }
}

void DuploEvaluator::EvaluateBucketParallel(uint32_t exec_id, std::pair<std::string, uint32_t>& component, uint8_t input_keys[], uint8_t wire_keys[]) {

  std::string component_name = std::get<0>(component);
  uint32_t component_num = std::get<1>(component);
  Circuit& circuit = string_to_circuit_map[component_name];

  GarblingHandler gh;
  uint32_t garbled_table_size = GarbledCircuit::TotalTableSize(circuit);
  uint32_t solderings_size = EvalGarbledCircuit::SolderingsSize(circuit);

  BYTEArrayVector read_bucket;
  BYTEArrayVector read_solderings;

  persistent_storage.ReadBuckets(component_name, TABLES, component_num, 1, read_bucket);
  persistent_storage.ReadBuckets(component_name, SOLDERINGS, component_num, 1, read_solderings);

  uint32_t bucket_size = read_bucket.entry_size() / garbled_table_size;

  std::vector<BYTEArrayVector> cand_outputs(bucket_size, BYTEArrayVector(circuit.num_out_wires, CSEC_BYTES));

  std::vector<std::future<void>> futures;
  for (uint32_t i = 0; i < bucket_size; ++i) {

    futures.emplace_back(std::async(std::launch::async, [this, &read_bucket, &read_solderings, &gh, &circuit, &cand_outputs, input_keys, i, garbled_table_size, solderings_size]() {

      uint8_t* curr_garbled_tables = read_bucket.data() + i * garbled_table_size;
      uint8_t* curr_solderings = read_solderings.data() + i * solderings_size;

      gh.EvalGarbledCircuitSolderings(input_keys, circuit, curr_garbled_tables, curr_solderings, cand_outputs[i].data());
    }));
  }

  for (std::future<void>& f : futures) {
    f.wait();
  }

  //Check all non-head circuits towards the head circuits' output
  bool success = true;
  for (int i = 1; i < bucket_size; ++i) { //We check towards head circuit
    if (!std::equal(cand_outputs[0][0], cand_outputs[0][circuit.num_out_wires], cand_outputs[i].data())) {
      success = false;
    }
  }

  if (success) {
    std::copy(cand_outputs[0][0], cand_outputs[0][circuit.num_out_wires], wire_keys);
  } else {
    //Error handling, evaluate output authenticators, etc etc
    std::cout << "Problem! Bucket doesn't output the same!" << std::endl;
  }
}

bool DuploEvaluator::DecommitOutPermBits(uint32_t exec_id, std::vector<std::pair<std::string, uint32_t>>& output_components, std::vector<std::vector<uint32_t>>& eval_outputs, uint32_t components_from, uint32_t components_to, std::vector<osuCrypto::BitVector>& output_decodings, bool non_interactive) {

  uint32_t exec_num_components = components_to - components_from;

  uint32_t num_eval_total_outputs = GetNumberOfOutputWires(eval_outputs, components_from, components_to);

  uint32_t eval_outputs_from = GetNumberOfOutputWires(eval_outputs, 0, components_from);

  BYTEArrayVector output_masks_shares;
  persistent_storage.ReadBuckets(EVAL_PREPROCESS_PREFIX, OUTPUT_MASKS_AUXDATA, eval_outputs_from, num_eval_total_outputs, output_masks_shares);

  BYTEArrayVector decommit_lsb_share(num_eval_total_outputs, CODEWORD_BYTES);

  uint32_t curr_bit_pos = 0;
  uint32_t curr_eval_out_idx = 0;
  for (int l = 0; l < exec_num_components; ++l) {
    uint32_t curr_component = components_from + l;
    std::string component_name = std::get<0>(output_components[curr_component]);
    uint32_t component_num = std::get<1>(output_components[curr_component]);
    Circuit& circuit = string_to_circuit_map[component_name];

    BYTEArrayVector component_aux_data;
    persistent_storage.ReadBuckets(component_name, AUXDATA, component_num, 1, component_aux_data, true, exec_id);

    for (int i = 0; i < eval_outputs[curr_component].size(); ++i) {

      std::copy(EvalGarbledCircuit::out_key_share(circuit, component_aux_data.data(), eval_outputs[curr_component][i]),
                EvalGarbledCircuit::out_key_share(circuit, component_aux_data.data(), eval_outputs[curr_component][i] + 1),
                decommit_lsb_share[curr_bit_pos]);

      XOR_CodeWords(decommit_lsb_share[curr_bit_pos], output_masks_shares[curr_bit_pos]);

      ++curr_bit_pos;
    }
  }

  BYTEArrayVector committed_values(num_eval_total_outputs, CSEC_BYTES);
  if (non_interactive) {
    if (!commit_receivers[exec_id].Decommit(decommit_lsb_share, committed_values, exec_channels[exec_id])) {
      return false;
    }
  } else {
    if (!commit_receivers[exec_id].BatchDecommit(decommit_lsb_share, committed_values, exec_rnds[exec_id], exec_channels[exec_id])) {
      return false;
    }
  }

  curr_bit_pos = 0;
  for (int l = 0; l < exec_num_components; ++l) {
    uint32_t curr_component = components_from + l;
    output_decodings.emplace_back(eval_outputs[curr_component].size());
    for (int i = 0; i < eval_outputs[curr_component].size(); ++i) {
      output_decodings[l][i] = GetLSB(committed_values[curr_bit_pos]);
      ++curr_bit_pos;
    }
  }

  return true;
}
