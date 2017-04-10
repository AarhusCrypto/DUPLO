#include "duplo/duplo-constructor.h"

DuploConstructor::DuploConstructor(uint8_t seed[], uint32_t num_max_parallel_execs, bool ram_only) :
  Duplo(seed, num_max_parallel_execs, ram_only),
  commit_seed_OTs(NUM_COMMIT_SEED_OT),
  commit_senders(num_max_parallel_execs) {

}

DuploConstructor::~DuploConstructor() {

  chan.close();

  for (int e = 0; e < exec_channels.size(); ++e) {
    exec_channels[e].close();
  }

  end_point.stop();
}

void DuploConstructor::Connect(std::string ip_address, uint16_t port) {

  end_point.start(ios, ip_address, port, osuCrypto::EpMode::Server, "ep");

  chan = end_point.addChannel("chan", "chan");

  for (int e = 0; e < commit_senders.size(); ++e) {
    exec_channels.emplace_back(end_point.addChannel("exec_channel_" + std::to_string(e), "exec_channel_" + std::to_string(e)));
  }
}

void DuploConstructor::Setup() {

  //BaseOTs
  osuCrypto::u64 num_base_OTs = CSEC + SSEC;
  std::vector<osuCrypto::block> base_ots(num_base_OTs);
  osuCrypto::BitVector base_ot_choices(num_base_OTs);
  base_ot_choices.randomize(rnd);

  osuCrypto::NaorPinkas baseOTs;
  baseOTs.receive(base_ot_choices, base_ots, rnd, chan, 1);

  //Extended the base ots and set them for each dot_sender
  osuCrypto::KosDotExtSender temp_dot_sender;
  temp_dot_sender.setBaseOts(base_ots, base_ot_choices);

  for (int exec_id = 0; exec_id < commit_senders.size(); ++exec_id) {
    dot_senders.emplace_back(temp_dot_sender.split());
  }

  //Extended one last time to setup a kos sender
  std::vector<osuCrypto::block> currBaseSendOts(CSEC);
  osuCrypto::BitVector kos_ot_choices(CSEC);
  for (uint32_t i = 0; i < CSEC; ++i) {
    currBaseSendOts[i] = temp_dot_sender.mGens[i].get<osuCrypto::block>();
    kos_ot_choices[i] = base_ot_choices[i];
  }
  osuCrypto::KosOtExtSender kos_sender;
  kos_sender.setBaseOts(currBaseSendOts, kos_ot_choices);

  //Run kos OTX and store the resulting NUM_COMMIT_SEED_OT OTs appropriately
  kos_sender.send(commit_seed_OTs, rnd, chan);

  SplitCommitSender string_tmp_sender;
  string_tmp_sender.SetMsgBitSize(CSEC);

  std::vector<std::array<osuCrypto::block, 2>> string_msgs(CODEWORD_BITS);

  for (int i = 0; i < CODEWORD_BITS; ++i) {
    string_msgs[i][0] = commit_seed_OTs[i][0];
    string_msgs[i][1] = commit_seed_OTs[i][1];
  }

  string_tmp_sender.SetSeedOTs(string_msgs);
  string_tmp_sender.GetCloneSenders(commit_senders.size(), commit_senders);

  std::array<BYTEArrayVector, 2> recov_share = {
    BYTEArrayVector(1, CODEWORD_BYTES),
    BYTEArrayVector(1, CODEWORD_BYTES)
  };

  string_tmp_sender.Commit(recov_share, chan);

  persistent_storage.PrepareFile(CONST_RECOV_PREFIX, RECOV, 2 * recov_share[0].size());

  BYTEArrayVector recov_write_array(2, CODEWORD_BYTES);
  std::copy(recov_share[0].data(), recov_share[0].data() + recov_share[0].size(), recov_write_array.data());
  std::copy(recov_share[1].data(), recov_share[1].data() + recov_share[1].size(), recov_write_array[1]);

  persistent_storage.WriteBuckets(CONST_RECOV_PREFIX, RECOV, 0, 1, recov_write_array.data(), 0, recov_write_array.size(), 1);
}

void DuploConstructor::PreprocessComponentType(std::string component_type, Circuit& circuit, uint32_t num_buckets, uint32_t num_parallel_execs, BucketType bucket_type) {

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

  PartitionBufferFixedNum(eval_circuits_from, eval_circuits_to, num_parallel_execs, num_eval_circuits);
  PartitionBufferFixedNum(buckets_from, buckets_to, num_parallel_execs, num_buckets);

  std::vector<ConstGarbledCircuit> aux_garbled_circuits_data(num_eval_circuits, ConstGarbledCircuit(circuit, 0));

  for (int exec_id = 0; exec_id < num_parallel_execs; ++exec_id) {

    //Starts the current execution
    execs_finished[exec_id] = thread_pool.push([this, &aux_garbled_circuits_data, &eval_circuits_from, &eval_circuits_to, exec_id, &circuit, check_factor, negate_check_factor] (int id) {

      uint32_t exec_num_buckets = eval_circuits_to[exec_id] - eval_circuits_from[exec_id];

      float repl_factor;
      ComputeCheckFraction(check_factor, exec_num_buckets, repl_factor, negate_check_factor);

      uint32_t exec_num_total_circuits = ceil(repl_factor * exec_num_buckets);

      CommitGarbleAndCutAndChoose(exec_id, circuit, exec_num_total_circuits, check_factor, negate_check_factor, eval_circuits_from[exec_id], eval_circuits_to[exec_id], aux_garbled_circuits_data);
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

  uint64_t auxdata_bytes = ConstGarbledCircuit::AuxDataSize(circuit) * num_buckets;

  persistent_storage.PrepareFile(component_type, AUXDATA, auxdata_bytes);

  uint8_t bucket_seed[CSEC_BYTES];
  chan.recv(bucket_seed, CSEC_BYTES);

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
    execs_finished[exec_id] = thread_pool.push([this, &aux_garbled_circuits_data, &buckets_from, &buckets_to, exec_id, &circuit, bucket_size, &permuted_eval_ids_inv, &component_type] (int id) {

      BucketAndSendEvalCircuits(component_type, exec_id, circuit, bucket_size, permuted_eval_ids_inv, buckets_from[exec_id], buckets_to[exec_id], aux_garbled_circuits_data);
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

void DuploConstructor::PrepareComponents(uint64_t num_input_auth_buckets, uint32_t num_parallel_execs) {

  PrepareOutputWireAuthenticators(num_parallel_execs);
  PrepareInputWireAuthenticators(num_input_auth_buckets, num_parallel_execs);

}

void DuploConstructor::Build(ComposedCircuit& composed_circuit, uint32_t num_parallel_execs) {

  auto composed_solder_begin = GET_TIME();

  string_to_composed_circuit_map.emplace(composed_circuit.composed_circuit_name, composed_circuit);

  std::vector<int> components_from, components_to, eval_inputs_from, eval_inputs_to, eval_outputs_from, eval_outputs_to;
  PartitionBufferFixedNum(components_from, components_to, num_parallel_execs, composed_circuit.circuits.size());
  PartitionBufferFixedNum(eval_inputs_from, eval_inputs_to, num_parallel_execs, composed_circuit.num_eval_inp_wires);
  PartitionBufferFixedNum(eval_outputs_from, eval_outputs_to, num_parallel_execs, composed_circuit.num_eval_out_wires);

  persistent_storage.PrepareFile(CONST_PREPROCESS_PREFIX, INPUT_MASKS_AUXDATA, composed_circuit.num_eval_inp_wires * 2 * CODEWORD_BYTES);
  persistent_storage.PrepareFile(CONST_PREPROCESS_PREFIX, INPUT_MASKS_DELTA_AUXDATA, 2 * CODEWORD_BYTES);

  persistent_storage.PrepareFile(CONST_PREPROCESS_PREFIX, OUTPUT_MASKS_AUXDATA, composed_circuit.num_eval_out_wires * 2 * CODEWORD_BYTES);

  std::vector<std::future<void>> futures;
  for (int exec_id = 0; exec_id < num_parallel_execs; ++exec_id) {

    futures.emplace_back(thread_pool.push([this, &composed_circuit, &components_from, &components_to, &eval_inputs_from, &eval_inputs_to, &eval_outputs_from, &eval_outputs_to, exec_id] (int id) {

      ComputeAndSendComposedSolderings(exec_id, composed_circuit, components_from[exec_id], components_to[exec_id]);

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

void DuploConstructor::Evaluate(ComposedCircuit& composed_circuit, osuCrypto::BitVector& inputs, uint32_t num_parallel_execs) {

  std::vector<int> inp_circuit_from, inp_circuit_to, const_inp_from, const_inp_to, eval_inp_from, eval_inp_to;
  PartitionBufferFixedNum(inp_circuit_from, inp_circuit_to, num_parallel_execs, composed_circuit.circuits_in_layer[0].size());
  PartitionBufferFixedNum(const_inp_from, const_inp_to, num_parallel_execs, composed_circuit.num_const_inp_wires); //used only for frigate parsed circuits
  PartitionBufferFixedNum(eval_inp_from, eval_inp_to, num_parallel_execs, composed_circuit.num_eval_inp_wires); //used only for frigate parsed circuits

  std::vector<std::future<void>> futures;
  for (int exec_id = 0; exec_id < num_parallel_execs; ++exec_id) {

    futures.emplace_back(thread_pool.push([this, &composed_circuit, &inputs, &inp_circuit_from, &inp_circuit_to, &const_inp_from, &const_inp_to, &eval_inp_from, &eval_inp_to, exec_id] (int id) {

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
      GetInputs(exec_id, composed_circuit, inputs, num_const_inputs_from, num_const_inputs_to, num_eval_inputs_from, num_eval_inputs_to);
    }));
  }

  //Cannot continue to next layer, until this one finishes completely
  for (std::future<void>& f : futures) {
    f.wait();
  }
}

void DuploConstructor::DecodeKeys(ComposedCircuit& composed_circuit, std::vector<std::vector<uint32_t>>& const_outputs, std::vector<std::vector<uint32_t>>& eval_outputs, std::vector<osuCrypto::BitVector>& outputs, bool non_interactive, uint32_t num_parallel_execs) {

  //Put all output circuits in out_components
  std::vector<std::pair<std::string, uint32_t>> out_components;
  for (int i = 0; i < composed_circuit.output_circuits.size(); ++i) {
    out_components.emplace_back(composed_circuit.circuits[composed_circuit.output_circuits[i]]);
  }

  std::vector<int> out_components_from, out_components_to;
  PartitionBufferFixedNum(out_components_from, out_components_to, num_parallel_execs, out_components.size());

  std::vector<std::future<void>> futures;
  for (int exec_id = 0; exec_id < num_parallel_execs; ++exec_id) {

    futures.emplace_back(thread_pool.push([this, &composed_circuit, &out_components, &const_outputs, &eval_outputs, &out_components_from, &out_components_to, &outputs, non_interactive, exec_id] (int id) {

      uint32_t curr_const_read_pos = 0;

      std::vector<BYTEArrayVector> const_output_keys;
      uint32_t exec_num_const_output_wires = GetNumberOfOutputWires(const_outputs, out_components_from[exec_id], out_components_to[exec_id]);

      BYTEArrayVector exec_const_output_keys(exec_num_const_output_wires, CSEC_BYTES);
      exec_channels[exec_id].recv(exec_const_output_keys.data(), exec_const_output_keys.size());

      uint32_t exec_num_components = out_components_to[exec_id] - out_components_from[exec_id];

      for (int l = 0; l < exec_num_components; ++l) {
        uint32_t curr_component = out_components_from[exec_id] + l;

        const_output_keys.emplace_back(BYTEArrayVector(const_outputs[curr_component].size(), CSEC_BYTES));

        for (int i = 0; i < const_outputs[curr_component].size(); ++i) {
          //Copy const keys
          std::copy(exec_const_output_keys[curr_const_read_pos],
                    exec_const_output_keys[curr_const_read_pos + 1],
                    const_output_keys[l][i]);
          ++curr_const_read_pos;
        }
      }

      std::vector<osuCrypto::BitVector> output_decodings;
      DecommitOutPermBits(exec_id, out_components, const_outputs, eval_outputs, out_components_from[exec_id], out_components_to[exec_id], output_decodings, non_interactive);

      for (int l = 0; l < exec_num_components; ++l) {
        uint32_t curr_component = out_components_from[exec_id] + l;

        outputs[curr_component] = osuCrypto::BitVector(const_outputs[curr_component].size());

        DecodeGarbledOutput(const_output_keys[l], output_decodings[l], outputs[curr_component], const_outputs[curr_component].size());
      }
    }));
  }

  for (std::future<void>& f : futures) {
    f.wait();
  }
}

void DuploConstructor::CommitGarbleAndCutAndChoose(uint32_t exec_id, Circuit& circuit, uint32_t exec_num_total_garbled, uint32_t check_factor, bool negate_check_factor, uint32_t exec_eval_circuits_from, uint32_t exec_eval_circuits_to, std::vector<ConstGarbledCircuit>& aux_garbled_circuits_data) {

  uint32_t num_inp_keys, num_out_keys, num_deltas, num_commit_keys, num_base_keys, inp_keys_idx, out_keys_idx, deltas_idx;
  ComputeIndices(exec_num_total_garbled, circuit, num_inp_keys, num_out_keys, num_deltas, num_commit_keys, num_base_keys, inp_keys_idx, out_keys_idx, deltas_idx);

  std::array<BYTEArrayVector, 2> commit_keys_share = {
    BYTEArrayVector(num_commit_keys, CODEWORD_BYTES),
    BYTEArrayVector(num_commit_keys, CODEWORD_BYTES)
  };

  //Commit to the input/output wires and Deltas of all garbled circuits.
  commit_senders[exec_id].Commit(commit_keys_share, exec_channels[exec_id], deltas_idx);

  //Buffers
  BYTEArrayVector garbling_inp_keys(circuit.num_inp_wires, CSEC_BYTES);
  BYTEArrayVector garbling_out_keys(circuit.num_out_wires, CSEC_BYTES);
  uint8_t delta[CSEC_BYTES];

  GarblingHandler gh;
  ConstGarbledCircuit tmp_garbled_circuit(circuit);

  BYTEArrayVector garb_circuit_hashes(exec_num_total_garbled, osuCrypto::SHA1::HashSize);
  BYTEArrayVector out_wire_commit_corrections(num_out_keys, CSEC_BYTES);

  //Do all GARBLING and HASHING of circuits
  for (int i = 0; i < exec_num_total_garbled; ++i) {
    //Compute the delta used for this circuit
    XOR_128(delta, commit_keys_share[0][deltas_idx + i], commit_keys_share[1][deltas_idx + i]);

    //Compute the input used for this circuit, if lsb(input key_j) == 1 it means the committed key is the 1 key and we therefore flip the key using delta_i so that we garbled with the committed 0 keys.
    for (int j = 0; j < circuit.num_inp_wires; ++j) {
      XOR_128(garbling_inp_keys[j], commit_keys_share[0][inp_keys_idx + i * circuit.num_inp_wires + j], commit_keys_share[1][inp_keys_idx + i * circuit.num_inp_wires + j]);

      bool perm_bit = GetLSB(garbling_inp_keys[j]);
      SetLSB(garbling_inp_keys[j], 0);

      if (perm_bit) {
        XOR_128(garbling_inp_keys[j], delta);
      }
    }

    gh.GarbleCircuit(garbling_inp_keys.data(), garbling_out_keys.data(), delta, tmp_garbled_circuit, garb_circuit_hashes[i]);

    //Compute the output key corrections for this circuit. if lsb(output key_j) == 1 it means the committed key is supposed to be the 1 key and we therefore flip the key using delta so this becomes true.
    for (int j = 0; j < circuit.num_out_wires; ++j) {
      XOR_128(out_wire_commit_corrections[i * circuit.num_out_wires + j], commit_keys_share[0][out_keys_idx + i * circuit.num_out_wires + j], commit_keys_share[1][out_keys_idx + i * circuit.num_out_wires + j]);

      bool perm_bit = GetLSB(out_wire_commit_corrections[i * circuit.num_out_wires + j]);

      SetLSB(out_wire_commit_corrections[i * circuit.num_out_wires + j], 0);

      //Must be after above SetLSB!
      XOR_128(out_wire_commit_corrections[i * circuit.num_out_wires + j], garbling_out_keys[j]);

      if (perm_bit) {
        XOR_128(out_wire_commit_corrections[i * circuit.num_out_wires + j], delta);
      }
    }
  }

  //Send output keys commit corrections and hashes of tables. Thereby "commits" constructor to the tables
  exec_channels[exec_id].send(out_wire_commit_corrections.data(), out_wire_commit_corrections.size());
  exec_channels[exec_id].send(garb_circuit_hashes.data(), garb_circuit_hashes.size());

  ///////////////////////// CUT-AND-CHOOSE /////////////////////////

  //Receive challenge seed
  uint8_t cnc_seed[CSEC_BYTES];
  exec_channels[exec_id].recv(cnc_seed, CSEC_BYTES);

  //Select challenge circuits based on cnc_seed
  osuCrypto::BitVector cnc_check_circuits(exec_num_total_garbled);
  osuCrypto::PRNG cnc_rand;
  cnc_rand.SetSeed(load_block(cnc_seed));

  WeightedRandomString(cnc_check_circuits.data(), check_factor, cnc_check_circuits.sizeBytes(), cnc_rand, negate_check_factor);
  uint64_t num_checked_circuits = countSetBits(cnc_check_circuits.data(), 0, exec_num_total_garbled - 1);

  //Compute indices for convenient indexing
  uint32_t cnc_num_inp_keys, cnc_num_out_keys, cnc_num_deltas, cnc_num_commit_keys, cnc_num_base_keys, cnc_inp_keys_idx, cnc_out_keys_idx, cnc_deltas_idx;
  ComputeIndices(num_checked_circuits, circuit, cnc_num_inp_keys, cnc_num_out_keys, cnc_num_deltas, cnc_num_commit_keys, cnc_num_base_keys, cnc_inp_keys_idx, cnc_out_keys_idx, cnc_deltas_idx);

  //Arrays for holding the decommit shares of the decommited keys
  std::array<BYTEArrayVector, 2> cnc_keys_share = {
    BYTEArrayVector(cnc_num_commit_keys, CODEWORD_BYTES),
    BYTEArrayVector(cnc_num_commit_keys, CODEWORD_BYTES)
  };
  BYTEArrayVector cnc_keys(cnc_num_commit_keys, CSEC_BYTES);

  osuCrypto::BitVector perm_bits(cnc_num_commit_keys);

  uint32_t current_check_circuit_idx = 0;
  uint32_t current_eval_circuit_idx = exec_eval_circuits_from;
  bool completed_eval_copy = false;
  for (int i = 0; i < exec_num_total_garbled; ++i) {
    if (cnc_check_circuits[i]) { //Checked circuit

      //Add delta
      std::copy(commit_keys_share[0][deltas_idx + i], commit_keys_share[0][deltas_idx + (i + 1)], cnc_keys_share[0][cnc_deltas_idx + current_check_circuit_idx]);
      std::copy(commit_keys_share[1][deltas_idx + i], commit_keys_share[1][deltas_idx + (i + 1)], cnc_keys_share[1][cnc_deltas_idx + current_check_circuit_idx]);

      //Compute the delta value
      XOR_128(cnc_keys[cnc_deltas_idx + current_check_circuit_idx], cnc_keys_share[0][cnc_deltas_idx + current_check_circuit_idx], cnc_keys_share[1][cnc_deltas_idx + current_check_circuit_idx]);

      //Add all input keys
      std::copy(commit_keys_share[0][inp_keys_idx + i * circuit.num_inp_wires], commit_keys_share[0][inp_keys_idx + (i + 1) * circuit.num_inp_wires], cnc_keys_share[0][cnc_inp_keys_idx + current_check_circuit_idx * circuit.num_inp_wires]);
      std::copy(commit_keys_share[1][inp_keys_idx + i * circuit.num_inp_wires], commit_keys_share[1][inp_keys_idx + (i + 1) * circuit.num_inp_wires], cnc_keys_share[1][cnc_inp_keys_idx + current_check_circuit_idx * circuit.num_inp_wires]);

      //Compute the input values
      for (int j = 0; j < circuit.num_inp_wires; ++j) {

        //Compute the input keys
        XOR_128(cnc_keys[cnc_inp_keys_idx + current_check_circuit_idx * circuit.num_inp_wires + j], cnc_keys_share[0][cnc_inp_keys_idx + current_check_circuit_idx * circuit.num_inp_wires + j], cnc_keys_share[1][cnc_inp_keys_idx + current_check_circuit_idx * circuit.num_inp_wires + j]);

        //Use this template throughout Tomorrow!
        perm_bits[cnc_inp_keys_idx + current_check_circuit_idx * circuit.num_inp_wires + j] = GetLSB(cnc_keys[cnc_inp_keys_idx + current_check_circuit_idx * circuit.num_inp_wires + j]);

        //Flip the key and decommit values
        if (perm_bits[cnc_inp_keys_idx + current_check_circuit_idx * circuit.num_inp_wires + j]) {

          XOR_128(cnc_keys[cnc_inp_keys_idx + current_check_circuit_idx * circuit.num_inp_wires + j], cnc_keys_share[0][cnc_deltas_idx + current_check_circuit_idx]);
          XOR_128(cnc_keys[cnc_inp_keys_idx + current_check_circuit_idx * circuit.num_inp_wires + j], cnc_keys_share[1][cnc_deltas_idx + current_check_circuit_idx]);

          XOR_CodeWords(cnc_keys_share[0][cnc_inp_keys_idx + current_check_circuit_idx * circuit.num_inp_wires + j], cnc_keys_share[0][cnc_deltas_idx + current_check_circuit_idx]);
          XOR_CodeWords(cnc_keys_share[1][cnc_inp_keys_idx + current_check_circuit_idx * circuit.num_inp_wires + j], cnc_keys_share[1][cnc_deltas_idx + current_check_circuit_idx]);
        }
      }

      //Add all outputs
      std::copy(commit_keys_share[0][out_keys_idx + i * circuit.num_out_wires], commit_keys_share[0][out_keys_idx + (i + 1) * circuit.num_out_wires], cnc_keys_share[0][cnc_out_keys_idx + current_check_circuit_idx * circuit.num_out_wires]);
      std::copy(commit_keys_share[1][out_keys_idx + i * circuit.num_out_wires], commit_keys_share[1][out_keys_idx + (i + 1) * circuit.num_out_wires], cnc_keys_share[1][cnc_out_keys_idx + current_check_circuit_idx * circuit.num_out_wires]);

      //Compute the output values
      for (int j = 0; j < circuit.num_out_wires; ++j) {

        //Compute the output keys
        XOR_128(cnc_keys[cnc_out_keys_idx + current_check_circuit_idx * circuit.num_out_wires + j], cnc_keys_share[0][cnc_out_keys_idx + current_check_circuit_idx * circuit.num_out_wires + j], cnc_keys_share[1][cnc_out_keys_idx + current_check_circuit_idx * circuit.num_out_wires + j]);

        perm_bits[cnc_out_keys_idx + current_check_circuit_idx * circuit.num_out_wires + j] = GetLSB(cnc_keys[cnc_out_keys_idx + current_check_circuit_idx * circuit.num_out_wires + j]);

        //Flip the key and decommit values
        if (perm_bits[cnc_out_keys_idx + current_check_circuit_idx * circuit.num_out_wires + j]) {

          XOR_128(cnc_keys[cnc_out_keys_idx + current_check_circuit_idx * circuit.num_out_wires + j], cnc_keys_share[0][cnc_deltas_idx + current_check_circuit_idx]);
          XOR_128(cnc_keys[cnc_out_keys_idx + current_check_circuit_idx * circuit.num_out_wires + j], cnc_keys_share[1][cnc_deltas_idx + current_check_circuit_idx]);

          XOR_CodeWords(cnc_keys_share[0][cnc_out_keys_idx + current_check_circuit_idx * circuit.num_out_wires + j], cnc_keys_share[0][cnc_deltas_idx + current_check_circuit_idx]);
          XOR_CodeWords(cnc_keys_share[1][cnc_out_keys_idx + current_check_circuit_idx * circuit.num_out_wires + j], cnc_keys_share[1][cnc_deltas_idx + current_check_circuit_idx]);
        }
      }

      ++current_check_circuit_idx;
    } else if (current_eval_circuit_idx < exec_eval_circuits_to) {

      //Copy Delta
      std::copy(commit_keys_share[0][deltas_idx + i], commit_keys_share[0][deltas_idx + (i + 1)], aux_garbled_circuits_data[current_eval_circuit_idx].delta_commit0());
      std::copy(commit_keys_share[1][deltas_idx + i], commit_keys_share[1][deltas_idx + (i + 1)], aux_garbled_circuits_data[current_eval_circuit_idx].delta_commit1());

      //Copy inputs
      std::copy(commit_keys_share[0][inp_keys_idx + i * circuit.num_inp_wires], commit_keys_share[0][inp_keys_idx + (i + 1) * circuit.num_inp_wires], aux_garbled_circuits_data[current_eval_circuit_idx].inp_key_commit0());
      std::copy(commit_keys_share[1][inp_keys_idx + i * circuit.num_inp_wires], commit_keys_share[1][inp_keys_idx + (i + 1) * circuit.num_inp_wires], aux_garbled_circuits_data[current_eval_circuit_idx].inp_key_commit1());

      //Copy outputs
      std::copy(commit_keys_share[0][out_keys_idx + i * circuit.num_out_wires], commit_keys_share[0][out_keys_idx + (i + 1) * circuit.num_out_wires], aux_garbled_circuits_data[current_eval_circuit_idx].out_key_commit0());
      std::copy(commit_keys_share[1][out_keys_idx + i * circuit.num_out_wires], commit_keys_share[1][out_keys_idx + (i + 1) * circuit.num_out_wires], aux_garbled_circuits_data[current_eval_circuit_idx].out_key_commit1());

      ++current_eval_circuit_idx;
    } else {
      completed_eval_copy = true;
    }
  }

  if (!completed_eval_copy) {
    std::cout << "Problem. Not enough eval circuits! Params should be set so this never occurs" << std::endl;
  }

  exec_channels[exec_id].send(perm_bits);

  //BatchDecommit the cut-and-choose input and output keys. Needs to be done after BatchDecommit of perm bit values.
  exec_channels[exec_id].send(cnc_keys.data(), cnc_keys.size());
  commit_senders[exec_id].BatchDecommit(cnc_keys_share, exec_channels[exec_id], true);
}

void DuploConstructor::BucketAndSendEvalCircuits(std::string component_type, uint32_t exec_id, Circuit& circuit, uint32_t bucket_size, std::vector<uint32_t>& permuted_eval_ids_inv, uint32_t exec_buckets_from, uint32_t exec_buckets_to, std::vector<ConstGarbledCircuit>& aux_garbled_circuits_data) {

  uint64_t exec_num_buckets = exec_buckets_to - exec_buckets_from;
  uint64_t exec_num_eval_circuits = exec_num_buckets * bucket_size;

  uint64_t garbled_table_size = GarbledCircuit::TotalTableSize(circuit);
  uint64_t constr_aux_size = ConstGarbledCircuit::AuxDataSize(circuit);

  //Res buffers
  BYTEArrayVector garbled_tables(exec_num_eval_circuits, garbled_table_size);
  BYTEArrayVector exec_write_head_auxdata(exec_num_buckets, constr_aux_size);

  //Tmp buffers
  ConstGarbledCircuit garbled_circuit(circuit, 1);
  BYTEArrayVector input_keys(circuit.num_inp_wires, CSEC_BYTES);
  BYTEArrayVector out_buffer(circuit.num_out_wires, CSEC_BYTES);
  uint8_t delta[CSEC_BYTES];

  GarblingHandler gh;
  PermutedIndex<ConstGarbledCircuit> exec_permuted_aux_info(aux_garbled_circuits_data, permuted_eval_ids_inv, exec_buckets_from * bucket_size, constr_aux_size);
  for (int i = 0; i < exec_num_eval_circuits; ++i) {

    //Compute garbling keys
    XOR_128(delta, ConstGarbledCircuit::delta_commit0(circuit, exec_permuted_aux_info[i]),
            ConstGarbledCircuit::delta_commit1(circuit, exec_permuted_aux_info[i]));

    for (int j = 0; j < circuit.num_inp_wires; ++j) {
      XOR_128(input_keys[j], ConstGarbledCircuit::inp_key_commit0(circuit, exec_permuted_aux_info[i], j), ConstGarbledCircuit::inp_key_commit1(circuit, exec_permuted_aux_info[i], j));

      bool perm_bit = GetLSB(input_keys[j]);
      SetLSB(input_keys[j], 0);

      if (perm_bit) {
        XOR_128(input_keys[j], delta);
      }
    }

    //Garble circuit and write resulting tables to garbled_tables
    gh.GarbleCircuit(input_keys.data(), out_buffer.data(), delta, garbled_circuit);
    std::copy(garbled_circuit.GetTables(), garbled_circuit.GetTables() + garbled_circuit.size, garbled_tables[i]);
  }

  exec_channels[exec_id].send(garbled_tables.data(), garbled_tables.size());
  //garbled_tables could be deleted at this point to save space!

  ComputeAndSendComponentSolderings(component_type, exec_id, circuit, bucket_size, exec_permuted_aux_info, exec_buckets_from, exec_buckets_to);
}

void DuploConstructor::PrepareOutputWireAuthenticators(uint32_t num_parallel_execs) {

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

  std::vector<BYTEArrayVector> circuit_aux_auth_data(num_circuit_eval_auths, BYTEArrayVector(2, CODEWORD_BYTES));
  BYTEArrayVector circuit_aux_auth_delta_data(2, CODEWORD_BYTES);

  std::mutex delta_updated_mutex;
  std::condition_variable delta_updated_cond_val;
  std::atomic<bool> delta_updated(false);
  std::tuple<std::mutex&, std::condition_variable&, std::atomic<bool>&> delta_signal = make_tuple(std::ref(delta_updated_mutex), std::ref(delta_updated_cond_val), std::ref(delta_updated));

  std::vector<std::future<void>> execs_finished(num_parallel_execs);
  for (int exec_id = 0; exec_id < num_parallel_execs; ++exec_id) {

    //Starts the current execution
    execs_finished[exec_id] = thread_pool.push([this, &circuit_aux_auth_data, &circuit_aux_auth_delta_data, &delta_signal, &eval_circuit_auths_from, &eval_circuit_auths_to, circuit_check_factor, circuit_negate_check_factor, exec_id] (int id) {

      uint32_t exec_num_circuit_auths = eval_circuit_auths_to[exec_id] - eval_circuit_auths_from[exec_id];

      float circuit_repl_factor;
      ComputeCheckFraction(circuit_check_factor, exec_num_circuit_auths, circuit_repl_factor, circuit_negate_check_factor);

      uint32_t exec_num_total_circuit_auths = ceil(circuit_repl_factor * exec_num_circuit_auths);

      CommitCircuitAuthAndCutAndChoose(exec_id, exec_num_total_circuit_auths, circuit_check_factor, circuit_negate_check_factor, eval_circuit_auths_from[exec_id], eval_circuit_auths_to[exec_id], circuit_aux_auth_data, circuit_aux_auth_delta_data, delta_signal);

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
    Circuit& session_circuits(std::get<1>(circuit_info[i]));
    uint64_t num_session_circuit_buckets = std::get<2>(circuit_info[i]);

    std::vector<int> tmp_from, tmp_to;
    PartitionBufferFixedNum(tmp_from, tmp_to, num_parallel_execs, num_session_circuit_buckets);
    for (int j = 0; j < num_parallel_execs; ++j) {
      session_circuit_buckets_from[j].push_back(tmp_from[j]);
      session_circuit_buckets_to[j].push_back(tmp_to[j]);
    }
  }

  uint8_t bucket_seed[CSEC_BYTES];
  chan.recv(bucket_seed, CSEC_BYTES);
  osuCrypto::PRNG bucket_rnd;
  bucket_rnd.SetSeed(load_block(bucket_seed));

  //Circuit bucketing
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
    execs_finished[exec_id] = thread_pool.push([this, &circuit_aux_auth_data, &circuit_aux_auth_delta_data, &session_circuit_buckets_from, &session_circuit_buckets_to, &circuit_permuted_eval_ids_inv, circuit_auth_size, exec_id] (int id) {

      BucketAllCircuitAuths(exec_id, circuit_auth_size, circuit_permuted_eval_ids_inv, session_circuit_buckets_from[exec_id], session_circuit_buckets_to[exec_id], circuit_aux_auth_data, circuit_aux_auth_delta_data);

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

void DuploConstructor::PrepareInputWireAuthenticators(uint64_t num_input_auth_buckets, uint32_t num_parallel_execs) {

  uint64_t num_circuit_buckets = 0; //only used for printing relative timings
  for (std::tuple<std::string, Circuit, uint64_t>& c_info : circuit_info) {
    num_circuit_buckets += std::get<2>(c_info);
  }

  //We need at least 5
  num_input_auth_buckets = std::max<uint64_t>(num_input_auth_buckets, 5);

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

  std::vector<ConstGarbledCircuit> input_aux_auth_data(num_input_eval_auths, ConstGarbledCircuit(inp_bucket_circuit, 0));

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
  chan.recv(bucket_seed, CSEC_BYTES);
  osuCrypto::PRNG bucket_rnd;
  bucket_rnd.SetSeed(load_block(bucket_seed));

//Input bucketing
  std::vector<uint32_t> input_permuted_eval_ids(num_input_eval_auths);
  std::vector<uint32_t> input_permuted_eval_ids_inv(num_input_eval_auths);
  std::iota(std::begin(input_permuted_eval_ids), std::end(input_permuted_eval_ids), 0);
  PermuteArray(input_permuted_eval_ids.data(), num_input_eval_auths, bucket_seed);
  for (int i = 0; i < num_input_eval_auths; ++i) {
    input_permuted_eval_ids_inv[input_permuted_eval_ids[i]] = i;
  }

  uint64_t auxdata_bytes = ConstGarbledCircuit::AuxDataSize(inp_bucket_circuit) * num_input_auth_buckets;
  persistent_storage.PrepareFile(CONST_INP_BUCKET_PREFIX, AUXDATA, auxdata_bytes);

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

  string_to_circuit_map.emplace(CONST_INP_BUCKET_PREFIX, inp_bucket_circuit);

  auto prepare_eval_bucketing_end = GET_TIME();


#ifdef DUPLO_PRINT
  PrintTimePerUnit(prepare_eval_bucketing_begin, prepare_eval_bucketing_end, num_circuit_buckets, "PrepareInputWireAuthsBucketing");
#endif

}

void DuploConstructor::CommitCircuitAuthAndCutAndChoose(uint32_t exec_id, uint32_t exec_num_auths, uint32_t check_factor, bool negate_check_factor, uint32_t exec_eval_auths_from, uint32_t exec_eval_auths_to, std::vector<BYTEArrayVector>& aux_auth_data, BYTEArrayVector & aux_auth_delta_data, std::tuple<std::mutex&, std::condition_variable&, std::atomic<bool>&>& delta_signal) {

  //If this is exec_id == 0 we produce one delta commitment.
  uint32_t num_commit_keys;
  if (exec_id == 0) {
    num_commit_keys = exec_num_auths + 1;
  } else {
    num_commit_keys = exec_num_auths;
  }

  std::array<BYTEArrayVector, 2> commit_keys_share = {
    BYTEArrayVector(num_commit_keys, CODEWORD_BYTES),
    BYTEArrayVector(num_commit_keys, CODEWORD_BYTES)
  };

  //Commit to the input/output wires and Deltas of all garbled circuits.
  commit_senders[exec_id].Commit(commit_keys_share, exec_channels[exec_id], exec_num_auths);

  //If not execution 0, wait until execution 0 has put the delta commitment into aux_auth_delta_data
  std::condition_variable& delta_updated_cond_val = std::get<1>(delta_signal);
  std::atomic<bool>& delta_updated = std::get<2>(delta_signal);

  if (exec_id == 0) {
    std::copy(commit_keys_share[0][num_commit_keys - 1], commit_keys_share[0][num_commit_keys], aux_auth_delta_data[0]);
    std::copy(commit_keys_share[1][num_commit_keys - 1], commit_keys_share[1][num_commit_keys], aux_auth_delta_data[1]);
    
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

  uint32_t H_0_idx = 0;
  uint32_t H_1_idx = exec_num_auths;

  BYTEArrayVector auths(2 * exec_num_auths, CSEC_BYTES);

  uint8_t current_key[CSEC_BYTES];
  uint8_t delta[CSEC_BYTES];
  XOR_128(delta, aux_auth_delta_data[0], aux_auth_delta_data[1]);

  GarblingHandler gh;
  uint32_t global_auth_idx;
  for (int i = 0; i < exec_num_auths; ++i) {
    global_auth_idx = exec_eval_auths_from + i;

    XOR_128(current_key, commit_keys_share[0][i], commit_keys_share[1][i]);

    Auth(current_key, delta, global_auth_idx, auths[H_0_idx + i], auths[H_1_idx + i], gh.key_schedule);
  }

  //Send wire authenticators
  exec_channels[exec_id].send(auths.data(), auths.size());

  ///////////////////////// CUT-AND-CHOOSE /////////////////////////

  //Receive challenge seed
  uint8_t cnc_seed[CSEC_BYTES];
  exec_channels[exec_id].recv(cnc_seed, CSEC_BYTES);

  //Select challenge auths based on cnc_seed
  osuCrypto::BitVector cnc_check_auths(exec_num_auths);
  osuCrypto::PRNG cnc_rand;
  cnc_rand.SetSeed(load_block(cnc_seed));

  WeightedRandomString(cnc_check_auths.data(), check_factor, cnc_check_auths.sizeBytes(), cnc_rand, negate_check_factor);
  uint64_t cnc_num_auths = countSetBits(cnc_check_auths.data(), 0, exec_num_auths - 1);

  osuCrypto::BitVector cnc_check_inputs(cnc_num_auths);
  cnc_rand.get<uint8_t>(cnc_check_inputs.data(), cnc_check_inputs.sizeBytes());

  //Arrays for holding the decommit shares of the decommited keys
  std::array<BYTEArrayVector, 2> cnc_keys_share = {
    BYTEArrayVector(cnc_num_auths, CODEWORD_BYTES),
    BYTEArrayVector(cnc_num_auths, CODEWORD_BYTES)
  };
  BYTEArrayVector cnc_keys(cnc_num_auths, CSEC_BYTES);

  uint32_t current_check_auth_idx = 0;
  uint32_t current_eval_auth_idx = exec_eval_auths_from;
  bool completed_eval_copy = false;
  for (int i = 0; i < exec_num_auths; ++i) {
    if (cnc_check_auths[i]) { //Checked auths
      //Add key shares
      std::copy(commit_keys_share[0][i], commit_keys_share[0][(i + 1)], cnc_keys_share[0][current_check_auth_idx]);
      std::copy(commit_keys_share[1][i], commit_keys_share[1][(i + 1)], cnc_keys_share[1][current_check_auth_idx]);

      if (cnc_check_inputs[current_check_auth_idx]) {
        XOR_CodeWords(cnc_keys_share[0][current_check_auth_idx], aux_auth_delta_data[0]);
        XOR_CodeWords(cnc_keys_share[1][current_check_auth_idx], aux_auth_delta_data[1]);
      }

      //Add key
      XOR_128(cnc_keys[current_check_auth_idx], cnc_keys_share[0][current_check_auth_idx], cnc_keys_share[1][current_check_auth_idx]);

      ++current_check_auth_idx;
    } else if (current_eval_auth_idx < exec_eval_auths_to) {

      //Copy key info
      std::copy(commit_keys_share[0][i], commit_keys_share[0][(i + 1)], aux_auth_data[current_eval_auth_idx][0]);
      std::copy(commit_keys_share[1][i], commit_keys_share[1][(i + 1)], aux_auth_data[current_eval_auth_idx][1]);

      ++current_eval_auth_idx;
    } else {
      completed_eval_copy = true;
    }
  }

  if (!completed_eval_copy) {
    std::cout << "Problem. Not enough eval auths! Params should be set so this never occurs" << std::endl;
  }

  //BatchDecommit the cut-and-choose input and output keys.
  exec_channels[exec_id].send(cnc_keys.data(), cnc_keys.size());
  commit_senders[exec_id].BatchDecommit(cnc_keys_share, exec_channels[exec_id], true);
}

void DuploConstructor::CommitInputAuthAndCutAndChoose(uint32_t exec_id, uint32_t exec_num_total_garbled, uint32_t check_factor, bool negate_check_factor, uint32_t exec_eval_circuits_from, uint32_t exec_eval_circuits_to, std::vector<ConstGarbledCircuit>& input_aux_auth_data) {

  uint32_t num_inp_keys, num_out_keys, num_deltas, num_commit_keys, num_base_keys, inp_keys_idx, out_keys_idx, deltas_idx;
  ComputeIndices(exec_num_total_garbled, inp_bucket_circuit, num_inp_keys, num_out_keys, num_deltas, num_commit_keys, num_base_keys, inp_keys_idx, out_keys_idx, deltas_idx);

  std::array<BYTEArrayVector, 2> commit_keys_share = {
    BYTEArrayVector(num_commit_keys, CODEWORD_BYTES),
    BYTEArrayVector(num_commit_keys, CODEWORD_BYTES)
  };

  //Commit to the input/output wires and Deltas of all garbled circuits.
  commit_senders[exec_id].Commit(commit_keys_share, exec_channels[exec_id], deltas_idx);

  //Buffers
  uint8_t delta[CSEC_BYTES];

  GarblingHandler gh;
  std::vector<ConstGarbledCircuit> garbled_circuits(exec_num_total_garbled, ConstGarbledCircuit(inp_bucket_circuit));

  BYTEArrayVector inp_wire_commit_corrections(num_inp_keys, CSEC_BYTES);
  BYTEArrayVector auths(2 * exec_num_total_garbled, CSEC_BYTES);

  uint32_t H_0_idx = 0;
  uint32_t H_1_idx = exec_num_total_garbled;

  uint8_t blind_value_buffer[CSEC_BYTES];

  //Do all HASHING of keys
  for (int i = 0; i < exec_num_total_garbled; ++i) {
    //Compute the delta used for this circuit
    XOR_128(delta, commit_keys_share[0][deltas_idx + i], commit_keys_share[1][deltas_idx + i]);

    XOR_128(blind_value_buffer, commit_keys_share[0][inp_keys_idx + i], commit_keys_share[1][inp_keys_idx + i]);

    uint8_t perm_bit = GetLSB(blind_value_buffer);
    SetLSB(blind_value_buffer, 0);

    gh.GarbleInpBucket(inp_wire_commit_corrections[i], auths[H_0_idx + i], auths[H_1_idx + i], delta);

    XOR_128(inp_wire_commit_corrections[i], blind_value_buffer);

    //Needs to be GetLSB(xx) ^ GetLSB(xx)
    if (perm_bit) {
      XOR_128(inp_wire_commit_corrections[i], delta);
    }
  }

  //Send output keys commit corrections and hashes of tables. Thereby "commits" constructor to the tables
  exec_channels[exec_id].send(inp_wire_commit_corrections.data(), inp_wire_commit_corrections.size());

  //Send wire authenticators
  exec_channels[exec_id].send(auths.data(), auths.size());

  ///////////////////////// CUT-AND-CHOOSE /////////////////////////

  //Receive challenge seed
  uint8_t cnc_seed[CSEC_BYTES];
  exec_channels[exec_id].recv(cnc_seed, CSEC_BYTES);

  //Select challenge circuits based on cnc_seed
  osuCrypto::BitVector cnc_check_circuits(exec_num_total_garbled);
  osuCrypto::PRNG cnc_rand;
  cnc_rand.SetSeed(load_block(cnc_seed));

  WeightedRandomString(cnc_check_circuits.data(), check_factor, cnc_check_circuits.sizeBytes(), cnc_rand, negate_check_factor);
  uint64_t num_checked_circuits = countSetBits(cnc_check_circuits.data(), 0, exec_num_total_garbled - 1);

  //Compute indices for convenient indexing
  uint32_t cnc_num_inp_keys, cnc_num_out_keys, cnc_num_deltas, cnc_num_commit_keys, cnc_num_base_keys, cnc_inp_keys_idx, cnc_out_keys_idx, cnc_deltas_idx;
  ComputeIndices(num_checked_circuits, inp_bucket_circuit, cnc_num_inp_keys, cnc_num_out_keys, cnc_num_deltas, cnc_num_commit_keys, cnc_num_base_keys, cnc_inp_keys_idx, cnc_out_keys_idx, cnc_deltas_idx);

  //Arrays for holding the decommit shares of the decommited keys
  std::array<BYTEArrayVector, 2> cnc_keys_share = {
    BYTEArrayVector(cnc_num_commit_keys, CODEWORD_BYTES),
    BYTEArrayVector(cnc_num_commit_keys, CODEWORD_BYTES)
  };
  BYTEArrayVector cnc_keys(cnc_num_commit_keys, CSEC_BYTES);

  osuCrypto::BitVector perm_bits(cnc_num_commit_keys);

  uint32_t current_check_circuit_idx = 0;
  uint32_t current_eval_circuit_idx = exec_eval_circuits_from;
  bool completed_eval_copy = false;
  for (int i = 0; i < exec_num_total_garbled; ++i) {
    if (cnc_check_circuits[i]) { //Checked circuit

      //Add delta
      std::copy(commit_keys_share[0][deltas_idx + i], commit_keys_share[0][deltas_idx + (i + 1)], cnc_keys_share[0][cnc_deltas_idx + current_check_circuit_idx]);
      std::copy(commit_keys_share[1][deltas_idx + i], commit_keys_share[1][deltas_idx + (i + 1)], cnc_keys_share[1][cnc_deltas_idx + current_check_circuit_idx]);

      //Compute the delta value
      XOR_128(cnc_keys[cnc_deltas_idx + current_check_circuit_idx], cnc_keys_share[0][cnc_deltas_idx + current_check_circuit_idx], cnc_keys_share[1][cnc_deltas_idx + current_check_circuit_idx]);

      //Add inputs
      std::copy(commit_keys_share[0][inp_keys_idx + i],
                commit_keys_share[0][inp_keys_idx + (i + 1)],
                cnc_keys_share[0][cnc_inp_keys_idx + current_check_circuit_idx]);
      std::copy(commit_keys_share[1][inp_keys_idx + i],
                commit_keys_share[1][inp_keys_idx + (i + 1)],
                cnc_keys_share[1][cnc_inp_keys_idx + current_check_circuit_idx]);

      //Compute the input keys
      XOR_128(cnc_keys[cnc_inp_keys_idx + current_check_circuit_idx], cnc_keys_share[0][cnc_inp_keys_idx + current_check_circuit_idx], cnc_keys_share[1][cnc_inp_keys_idx + current_check_circuit_idx]);

      perm_bits[cnc_inp_keys_idx + current_check_circuit_idx] = GetLSB(cnc_keys[cnc_inp_keys_idx + current_check_circuit_idx]);

      //Flip the key and decommit values
      if (perm_bits[cnc_inp_keys_idx + current_check_circuit_idx]) {

        XOR_128(cnc_keys[cnc_inp_keys_idx + current_check_circuit_idx], cnc_keys_share[0][cnc_deltas_idx + current_check_circuit_idx]);
        XOR_128(cnc_keys[cnc_inp_keys_idx + current_check_circuit_idx], cnc_keys_share[1][cnc_deltas_idx + current_check_circuit_idx]);

        XOR_CodeWords(cnc_keys_share[0][cnc_inp_keys_idx + current_check_circuit_idx], cnc_keys_share[0][cnc_deltas_idx + current_check_circuit_idx]);
        XOR_CodeWords(cnc_keys_share[1][cnc_inp_keys_idx + current_check_circuit_idx], cnc_keys_share[1][cnc_deltas_idx + current_check_circuit_idx]);
      }

      ++current_check_circuit_idx;
    } else if (current_eval_circuit_idx < exec_eval_circuits_to) {

      //Copy Delta
      std::copy(commit_keys_share[0][deltas_idx + i], commit_keys_share[0][deltas_idx + (i + 1)], input_aux_auth_data[current_eval_circuit_idx].delta_commit0());
      std::copy(commit_keys_share[1][deltas_idx + i], commit_keys_share[1][deltas_idx + (i + 1)], input_aux_auth_data[current_eval_circuit_idx].delta_commit1());

      //Copy inputs
      std::copy(commit_keys_share[0][inp_keys_idx + i],
                commit_keys_share[0][inp_keys_idx + (i + 1)],
                input_aux_auth_data[current_eval_circuit_idx].inp_key_commit0());
      std::copy(commit_keys_share[1][inp_keys_idx + i],
                commit_keys_share[1][inp_keys_idx + (i + 1)],
                input_aux_auth_data[current_eval_circuit_idx].inp_key_commit1());

      ++current_eval_circuit_idx;
    } else {
      completed_eval_copy = true;
    }
  }

  if (!completed_eval_copy) {
    std::cout << "Problem. Not enough eval auths! Params should be set so this never occurs" << std::endl;
  }

  exec_channels[exec_id].send(perm_bits);

  //BatchDecommit the cut-and-choose input and output keys. Needs to be done after BatchDecommit of perm bit values.
  exec_channels[exec_id].send(cnc_keys.data(), cnc_keys.size());
  commit_senders[exec_id].BatchDecommit(cnc_keys_share, exec_channels[exec_id], true);
}

void DuploConstructor::BucketAllCircuitAuths(uint32_t exec_id, uint32_t auth_size, std::vector<uint32_t>& permuted_eval_ids_inv, std::vector<int>& session_circuit_buckets_from, std::vector<int>& session_circuit_buckets_to, std::vector<BYTEArrayVector>& aux_auth_data, BYTEArrayVector & aux_auth_delta_data) {

  uint32_t num_sessions = session_circuit_buckets_from.size();

  uint32_t total_num_solderings = 0;
  for (int l = 0; l < num_sessions; ++l) {
    Circuit& session_circuit = std::get<1>(circuit_info[l]);
    uint32_t exec_session_num_circuit_buckets = session_circuit_buckets_to[l] - session_circuit_buckets_from[l];

    total_num_solderings += exec_session_num_circuit_buckets * (auth_size * session_circuit.num_out_wires + 1);
  }

  //For decomitting later on
  std::array<BYTEArrayVector, 2> solder_keys_share = {
    BYTEArrayVector(total_num_solderings, CODEWORD_BYTES),
    BYTEArrayVector(total_num_solderings, CODEWORD_BYTES)
  };
  BYTEArrayVector solder_keys(total_num_solderings, CSEC_BYTES);

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
      XOR_128(solder_keys[curr_solder_write_pos],
              ConstGarbledCircuit::delta_commit0(session_circuit, curr_session_aux_info[i]),
              ConstGarbledCircuit::delta_commit1(session_circuit, curr_session_aux_info[i]));
      XOR_128(solder_keys[curr_solder_write_pos], aux_auth_delta_data[0]);
      XOR_128(solder_keys[curr_solder_write_pos], aux_auth_delta_data[1]);

      std::copy(ConstGarbledCircuit::delta_commit0(session_circuit, curr_session_aux_info[i]),
                ConstGarbledCircuit::delta_commit0(session_circuit, curr_session_aux_info[i]) + CODEWORD_BYTES,
                solder_keys_share[0][curr_solder_write_pos]);
      XOR_CodeWords(solder_keys_share[0][curr_solder_write_pos], aux_auth_delta_data[0]);

      std::copy(ConstGarbledCircuit::delta_commit1(session_circuit, curr_session_aux_info[i]),
                ConstGarbledCircuit::delta_commit1(session_circuit, curr_session_aux_info[i])  + CODEWORD_BYTES,
                solder_keys_share[1][curr_solder_write_pos]);
      XOR_CodeWords(solder_keys_share[1][curr_solder_write_pos], aux_auth_delta_data[1]);

      ++curr_solder_write_pos;

      //Copy all bucket_size*session_circuit.num_out_wires solderings
      for (int j = 0; j < session_circuit.num_out_wires; ++j) {
        for (int a = 0; a < auth_size; ++a) {
          uint32_t perm_auth_idx = permuted_eval_ids_inv[(curr_session_bucket_idx * session_circuit.num_out_wires + j) * auth_size + a];
          BYTEArrayVector& current_aux_auth_data = aux_auth_data[perm_auth_idx];

          XOR_128(solder_keys[curr_solder_write_pos],
                  ConstGarbledCircuit::out_key_commit0(session_circuit, curr_session_aux_info[i], j),
                  ConstGarbledCircuit::out_key_commit1(session_circuit, curr_session_aux_info[i], j));
          XOR_128(solder_keys[curr_solder_write_pos], current_aux_auth_data[0]);
          XOR_128(solder_keys[curr_solder_write_pos], current_aux_auth_data[1]);

          std::copy(ConstGarbledCircuit::out_key_commit0(session_circuit, curr_session_aux_info[i], j),
                    ConstGarbledCircuit::out_key_commit0(session_circuit, curr_session_aux_info[i], j + 1),
                    solder_keys_share[0][curr_solder_write_pos]);
          XOR_CodeWords(solder_keys_share[0][curr_solder_write_pos], current_aux_auth_data[0]);

          std::copy(ConstGarbledCircuit::out_key_commit1(session_circuit, curr_session_aux_info[i], j),
                    ConstGarbledCircuit::out_key_commit1(session_circuit, curr_session_aux_info[i], j + 1),
                    solder_keys_share[1][curr_solder_write_pos]);
          XOR_CodeWords(solder_keys_share[1][curr_solder_write_pos], current_aux_auth_data[1]);

          ++curr_solder_write_pos;
        }
      }
    }
  }

  //Send postulated solderings and prove correct using BatchDecommit
  exec_channels[exec_id].send(solder_keys.data(), solder_keys.size());
  commit_senders[exec_id].BatchDecommit(solder_keys_share, exec_channels[exec_id], true);

}

void DuploConstructor::BucketAllInputAuths(uint32_t exec_id, uint32_t input_auth_size, std::vector<uint32_t>& permuted_eval_ids_inv, uint32_t exec_input_buckets_from, uint32_t exec_input_buckets_to, std::vector<ConstGarbledCircuit>& input_aux_auth_data) {

  uint64_t constr_aux_size = ConstGarbledCircuit::AuxDataSize(inp_bucket_circuit);

  PermutedIndex<ConstGarbledCircuit> exec_permuted_aux_info(input_aux_auth_data, permuted_eval_ids_inv, exec_input_buckets_from * input_auth_size, constr_aux_size);

  ComputeAndSendComponentSolderings(CONST_INP_BUCKET_PREFIX, exec_id, inp_bucket_circuit, input_auth_size, exec_permuted_aux_info, exec_input_buckets_from, exec_input_buckets_to);
}

void DuploConstructor::ComputeAndSendComponentSolderings(std::string component_type, uint32_t exec_id, Circuit& circuit, uint32_t bucket_size, PermutedIndex<ConstGarbledCircuit>& exec_permuted_aux_info, uint32_t exec_buckets_from, uint32_t exec_buckets_to) {

  uint64_t exec_num_buckets = exec_buckets_to - exec_buckets_from;
  uint64_t constr_aux_size = ConstGarbledCircuit::AuxDataSize(circuit);

  BYTEArrayVector exec_write_head_auxdata(exec_num_buckets, constr_aux_size);

  ////////////////////////////Soldering/////////////////////////////////////

  uint32_t num_soldering_circuits = (bucket_size - 1) * exec_num_buckets;

  uint32_t solder_num_inp_keys, solder_num_out_keys, solder_num_deltas, solder_num_commit_keys, solder_num_base_keys, solder_inp_keys_idx, solder_out_keys_idx, solder_deltas_idx;
  ComputeIndices(num_soldering_circuits, circuit, solder_num_inp_keys, solder_num_out_keys, solder_num_deltas, solder_num_commit_keys, solder_num_base_keys, solder_inp_keys_idx, solder_out_keys_idx, solder_deltas_idx);

  uint32_t curr_head_circuit, curr_circuit, curr_solder_write_pos;

  std::array<BYTEArrayVector, 2> solder_keys_share = {
    BYTEArrayVector(solder_num_commit_keys + exec_num_buckets, CODEWORD_BYTES),
    BYTEArrayVector(solder_num_commit_keys + exec_num_buckets, CODEWORD_BYTES)
  };
  BYTEArrayVector solder_keys(solder_num_commit_keys + exec_num_buckets, CSEC_BYTES);

  osuCrypto::BitVector perm_bits(solder_num_commit_keys);

  BYTEArrayVector recov_shares;
  persistent_storage.ReadBuckets(CONST_RECOV_PREFIX, RECOV, 0, 1, recov_shares);

  BYTEArrayVector head_inp_keys(circuit.num_inp_wires, CSEC_BYTES);
  BYTEArrayVector head_out_keys(circuit.num_out_wires, CSEC_BYTES);

  uint8_t head_delta[CSEC_BYTES];
  uint8_t curr_circuit_delta[CSEC_BYTES];
  for (int i = 0; i < exec_num_buckets; ++i) {
    curr_head_circuit = i * bucket_size;
    XOR_128(head_delta,
            ConstGarbledCircuit::delta_commit0(circuit, exec_permuted_aux_info[curr_head_circuit]),
            ConstGarbledCircuit::delta_commit1(circuit, exec_permuted_aux_info[curr_head_circuit]));

    //Add Recov decommit data
    XOR_CodeWords(solder_keys_share[0][solder_num_commit_keys + i],
                  ConstGarbledCircuit::delta_commit0(circuit, exec_permuted_aux_info[curr_head_circuit]),
                  recov_shares.data());
    XOR_CodeWords(solder_keys_share[1][solder_num_commit_keys + i],
                  ConstGarbledCircuit::delta_commit1(circuit, exec_permuted_aux_info[curr_head_circuit]),
                  recov_shares.data() + CODEWORD_BYTES);
    XOR_128(solder_keys[solder_num_commit_keys + i],
            solder_keys_share[0][solder_num_commit_keys + i],
            solder_keys_share[1][solder_num_commit_keys + i]);

    for (int j = 0; j < circuit.num_inp_wires; ++j) {
      XOR_128(head_inp_keys[j],
              ConstGarbledCircuit::inp_key_commit0(circuit, exec_permuted_aux_info[curr_head_circuit], j),
              ConstGarbledCircuit::inp_key_commit1(circuit, exec_permuted_aux_info[curr_head_circuit], j));
    }

    for (int j = 0; j < circuit.num_out_wires; ++j) {
      XOR_128(head_out_keys[j],
              ConstGarbledCircuit::out_key_commit0(circuit, exec_permuted_aux_info[curr_head_circuit], j),
              ConstGarbledCircuit::out_key_commit1(circuit, exec_permuted_aux_info[curr_head_circuit], j));
    }

    //Store the current head circuit info for writing to disc
    std::copy(exec_permuted_aux_info[curr_head_circuit], exec_permuted_aux_info[curr_head_circuit] + exec_permuted_aux_info.aux_size, exec_write_head_auxdata[i]);

    //Do each circuit in the bucket
    for (int l = 1; l < bucket_size; ++l) {
      curr_circuit = curr_head_circuit + l;
      curr_solder_write_pos = curr_circuit - (i + 1);

      //Compute the delta soldering
      XOR_128(curr_circuit_delta, ConstGarbledCircuit::delta_commit0(circuit, exec_permuted_aux_info[curr_circuit]), ConstGarbledCircuit::delta_commit1(circuit, exec_permuted_aux_info[curr_circuit]));

      std::copy(curr_circuit_delta, curr_circuit_delta + CSEC_BYTES, solder_keys[solder_deltas_idx + curr_solder_write_pos]);
      XOR_128(solder_keys[solder_deltas_idx + curr_solder_write_pos], head_delta);

      //Add delta decommits
      std::copy(ConstGarbledCircuit::delta_commit0(circuit, exec_permuted_aux_info[curr_circuit]),
                ConstGarbledCircuit::delta_commit0(circuit, exec_permuted_aux_info[curr_circuit]) + CODEWORD_BYTES,
                solder_keys_share[0][solder_deltas_idx + curr_solder_write_pos]);
      XOR_CodeWords(solder_keys_share[0][solder_deltas_idx + curr_solder_write_pos], ConstGarbledCircuit::delta_commit0(circuit, exec_permuted_aux_info[curr_head_circuit]));

      std::copy(ConstGarbledCircuit::delta_commit1(circuit, exec_permuted_aux_info[curr_circuit]),
                ConstGarbledCircuit::delta_commit1(circuit, exec_permuted_aux_info[curr_circuit]) + CODEWORD_BYTES,
                solder_keys_share[1][solder_deltas_idx + curr_solder_write_pos]);
      XOR_CodeWords(solder_keys_share[1][solder_deltas_idx + curr_solder_write_pos], ConstGarbledCircuit::delta_commit1(circuit, exec_permuted_aux_info[curr_head_circuit]));

      for (int j = 0; j < circuit.num_inp_wires; ++j) {
        //Add input soldering
        XOR_128(solder_keys[solder_inp_keys_idx + curr_solder_write_pos * circuit.num_inp_wires + j], ConstGarbledCircuit::inp_key_commit0(circuit, exec_permuted_aux_info[curr_circuit], j), ConstGarbledCircuit::inp_key_commit1(circuit, exec_permuted_aux_info[curr_circuit], j));

        XOR_128(solder_keys[solder_inp_keys_idx + curr_solder_write_pos * circuit.num_inp_wires + j], head_inp_keys[j]);

        //Add input decommits
        std::copy(ConstGarbledCircuit::inp_key_commit0(circuit, exec_permuted_aux_info[curr_circuit], j),
                  ConstGarbledCircuit::inp_key_commit0(circuit, exec_permuted_aux_info[curr_circuit], j + 1),
                  solder_keys_share[0][solder_inp_keys_idx + curr_solder_write_pos * circuit.num_inp_wires + j]);
        XOR_CodeWords(solder_keys_share[0][solder_inp_keys_idx + curr_solder_write_pos * circuit.num_inp_wires + j], ConstGarbledCircuit::inp_key_commit0(circuit, exec_permuted_aux_info[curr_head_circuit], j));

        std::copy(ConstGarbledCircuit::inp_key_commit1(circuit, exec_permuted_aux_info[curr_circuit], j),
                  ConstGarbledCircuit::inp_key_commit1(circuit, exec_permuted_aux_info[curr_circuit], j + 1),
                  solder_keys_share[1][solder_inp_keys_idx + curr_solder_write_pos * circuit.num_inp_wires + j]);
        XOR_CodeWords(solder_keys_share[1][solder_inp_keys_idx + curr_solder_write_pos * circuit.num_inp_wires + j], ConstGarbledCircuit::inp_key_commit1(circuit, exec_permuted_aux_info[curr_head_circuit], j));

        perm_bits[solder_inp_keys_idx + curr_solder_write_pos * circuit.num_inp_wires + j] = GetLSB(solder_keys[solder_inp_keys_idx + curr_solder_write_pos * circuit.num_inp_wires + j]);

        //If xor of perm bits == 1 we XOR delta onto the soldering
        if (perm_bits[solder_inp_keys_idx + curr_solder_write_pos * circuit.num_inp_wires + j]) {

          XOR_128(solder_keys[solder_inp_keys_idx + curr_solder_write_pos * circuit.num_inp_wires + j], curr_circuit_delta);

          XOR_CodeWords(solder_keys_share[0][solder_inp_keys_idx + curr_solder_write_pos * circuit.num_inp_wires + j], ConstGarbledCircuit::delta_commit0(circuit, exec_permuted_aux_info[curr_circuit]));
          XOR_CodeWords(solder_keys_share[1][solder_inp_keys_idx + curr_solder_write_pos * circuit.num_inp_wires + j], ConstGarbledCircuit::delta_commit1(circuit, exec_permuted_aux_info[curr_circuit]));
        }
      }

      for (int j = 0; j < circuit.num_out_wires; ++j) {
        //Add output soldering
        XOR_128(solder_keys[solder_out_keys_idx + curr_solder_write_pos * circuit.num_out_wires + j], ConstGarbledCircuit::out_key_commit0(circuit, exec_permuted_aux_info[curr_circuit], j), ConstGarbledCircuit::out_key_commit1(circuit, exec_permuted_aux_info[curr_circuit], j));

        XOR_128(solder_keys[solder_out_keys_idx + curr_solder_write_pos * circuit.num_out_wires + j], head_out_keys[j]);

        //Add output decommits
        std::copy(ConstGarbledCircuit::out_key_commit0(circuit, exec_permuted_aux_info[curr_circuit], j),
                  ConstGarbledCircuit::out_key_commit0(circuit, exec_permuted_aux_info[curr_circuit], j + 1),
                  solder_keys_share[0][solder_out_keys_idx + curr_solder_write_pos * circuit.num_out_wires + j]);
        XOR_CodeWords(solder_keys_share[0][solder_out_keys_idx + curr_solder_write_pos * circuit.num_out_wires + j], ConstGarbledCircuit::out_key_commit0(circuit, exec_permuted_aux_info[curr_head_circuit], j));

        std::copy(ConstGarbledCircuit::out_key_commit1(circuit, exec_permuted_aux_info[curr_circuit], j),
                  ConstGarbledCircuit::out_key_commit1(circuit, exec_permuted_aux_info[curr_circuit], j + 1),
                  solder_keys_share[1][solder_out_keys_idx + curr_solder_write_pos * circuit.num_out_wires + j]);
        XOR_CodeWords(solder_keys_share[1][solder_out_keys_idx + curr_solder_write_pos * circuit.num_out_wires + j], ConstGarbledCircuit::out_key_commit1(circuit, exec_permuted_aux_info[curr_head_circuit], j));

        perm_bits[solder_out_keys_idx + curr_solder_write_pos * circuit.num_out_wires + j] = GetLSB(solder_keys[solder_out_keys_idx + curr_solder_write_pos * circuit.num_out_wires + j]);

        //If xor of perm bits == 1 we XOR delta onto the soldering
        if (perm_bits[solder_out_keys_idx + curr_solder_write_pos * circuit.num_out_wires + j]) {

          XOR_128(solder_keys[solder_out_keys_idx + curr_solder_write_pos * circuit.num_out_wires + j], curr_circuit_delta);

          XOR_CodeWords(solder_keys_share[0][solder_out_keys_idx + curr_solder_write_pos * circuit.num_out_wires + j], ConstGarbledCircuit::delta_commit0(circuit, exec_permuted_aux_info[curr_circuit]));
          XOR_CodeWords(solder_keys_share[1][solder_out_keys_idx + curr_solder_write_pos * circuit.num_out_wires + j], ConstGarbledCircuit::delta_commit1(circuit, exec_permuted_aux_info[curr_circuit]));
        }
      }
    }
  }

  exec_channels[exec_id].send(perm_bits);

  //Send postulated solderings and prove correct using BatchDecommit
  exec_channels[exec_id].send(solder_keys.data(), solder_keys.size());
  commit_senders[exec_id].BatchDecommit(solder_keys_share, exec_channels[exec_id], true);

  //////////////////////////////Write to Disc/////////////////////////////////

  uint64_t exec_auxdata_write_pos = exec_buckets_from * constr_aux_size;

  persistent_storage.WriteBuckets(component_type, AUXDATA, exec_buckets_from, exec_num_buckets, exec_write_head_auxdata.data(), exec_auxdata_write_pos, exec_write_head_auxdata.size(), bucket_size);
}

void DuploConstructor::ComputeAndSendComposedSolderings(uint32_t exec_id, ComposedCircuit& composed_circuit, uint32_t inp_wire_components_from, uint32_t inp_wire_components_to) {

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

  uint32_t exec_num_total_solderings = exec_num_total_wire_solderings + exec_num_total_deltas;

  std::array<BYTEArrayVector, 2> solder_keys_share = {
    BYTEArrayVector(exec_num_total_solderings, CODEWORD_BYTES),
    BYTEArrayVector(exec_num_total_solderings, CODEWORD_BYTES)
  };
  BYTEArrayVector solder_keys(exec_num_total_solderings, CSEC_BYTES);

  osuCrypto::BitVector perm_bits(exec_num_total_solderings);

  for (int l = 0; l < exec_num_inp_wire_components; ++l) {
    uint32_t curr_component = inp_wire_components_from + l;

    std::string inp_wire_component_name(std::get<0>(composed_circuit.circuits[curr_component]));
    uint32_t inp_wire_component_num(std::get<1>(composed_circuit.circuits[curr_component]));
    Circuit& inp_circuit = string_to_circuit_map[inp_wire_component_name];
    BYTEArrayVector inp_wire_component_aux_info;
    persistent_storage.ReadBuckets(inp_wire_component_name, AUXDATA, inp_wire_component_num, 1, inp_wire_component_aux_info, true, exec_id);

    //Add all input component values to soldering
    for (int j = 0; j < inp_circuit.num_inp_wires; ++j) {
      XOR_128(solder_keys[inp_pos[l] + j],
              ConstGarbledCircuit::inp_key_commit0(inp_circuit, inp_wire_component_aux_info.data(), j),
              ConstGarbledCircuit::inp_key_commit1(inp_circuit, inp_wire_component_aux_info.data(), j));

      std::copy(ConstGarbledCircuit::inp_key_commit0(inp_circuit, inp_wire_component_aux_info.data(), j),
                ConstGarbledCircuit::inp_key_commit0(inp_circuit, inp_wire_component_aux_info.data(), j + 1),
                solder_keys_share[0][inp_pos[l] + j]);

      std::copy(ConstGarbledCircuit::inp_key_commit1(inp_circuit, inp_wire_component_aux_info.data(), j),
                ConstGarbledCircuit::inp_key_commit1(inp_circuit, inp_wire_component_aux_info.data(), j + 1),
                solder_keys_share[1][inp_pos[l] + j]);
    }

    uint32_t curr_inp_wire = 0;
    for (int i = 0; i < composed_circuit.out_wire_holders[curr_component].size(); ++i) {

      //Add delta solderings from input wire component to all delta positions
      XOR_128(solder_keys[inp_pos[l] + inp_circuit.num_inp_wires + i],
              ConstGarbledCircuit::delta_commit0(inp_circuit, inp_wire_component_aux_info.data()),
              ConstGarbledCircuit::delta_commit1(inp_circuit, inp_wire_component_aux_info.data()));

      std::copy(ConstGarbledCircuit::delta_commit0(inp_circuit, inp_wire_component_aux_info.data()),
                ConstGarbledCircuit::delta_commit0(inp_circuit, inp_wire_component_aux_info.data()) + CODEWORD_BYTES,
                solder_keys_share[0][inp_pos[l] + inp_circuit.num_inp_wires + i]);

      std::copy(ConstGarbledCircuit::delta_commit1(inp_circuit, inp_wire_component_aux_info.data()),
                ConstGarbledCircuit::delta_commit1(inp_circuit, inp_wire_component_aux_info.data()) + CODEWORD_BYTES,
                solder_keys_share[1][inp_pos[l] + inp_circuit.num_inp_wires + i]);

      //Get current circuit information and soldering indices
      std::pair<uint32_t, std::vector<uint32_t>>& out_wire_component_pair = composed_circuit.out_wire_holders[curr_component][i];

      std::pair<std::string, uint32_t> out_wire_component;
      if (std::get<0>(out_wire_component_pair) == std::numeric_limits<uint32_t>::max()) {

        out_wire_component.first = CONST_INP_BUCKET_PREFIX;
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
      XOR_128(solder_keys[inp_pos[l] + inp_circuit.num_inp_wires + i],
              ConstGarbledCircuit::delta_commit0(out_circuit, out_wire_component_aux_info.data()));
      XOR_128(solder_keys[inp_pos[l] + inp_circuit.num_inp_wires + i],
              ConstGarbledCircuit::delta_commit1(out_circuit, out_wire_component_aux_info.data()));

      XOR_CodeWords(solder_keys_share[0][inp_pos[l] + inp_circuit.num_inp_wires + i], ConstGarbledCircuit::delta_commit0(out_circuit, out_wire_component_aux_info.data()));
      XOR_CodeWords(solder_keys_share[1][inp_pos[l] + inp_circuit.num_inp_wires + i], ConstGarbledCircuit::delta_commit1(out_circuit, out_wire_component_aux_info.data()));

      //Run through the soldering pairs and apply the soldering
      std::vector<uint32_t>& out_wires = std::get<1>(out_wire_component_pair);

      for (int j = 0; j < out_wires.size(); ++j) {

        if (std::get<0>(out_wire_component_pair) == std::numeric_limits<uint32_t>::max()) {

          XOR_128(solder_keys[inp_pos[l] + curr_inp_wire],
                  ConstGarbledCircuit::inp_key_commit0(out_circuit, out_wire_component_aux_info.data()));
          XOR_128(solder_keys[inp_pos[l] + curr_inp_wire],
                  ConstGarbledCircuit::inp_key_commit1(out_circuit, out_wire_component_aux_info.data()));

          XOR_CodeWords(solder_keys_share[0][inp_pos[l] + curr_inp_wire], ConstGarbledCircuit::inp_key_commit0(out_circuit, out_wire_component_aux_info.data()));

          XOR_CodeWords(solder_keys_share[1][inp_pos[l] + curr_inp_wire], ConstGarbledCircuit::inp_key_commit1(out_circuit, out_wire_component_aux_info.data()));

        } else {
          XOR_128(solder_keys[inp_pos[l] + curr_inp_wire],
                  ConstGarbledCircuit::out_key_commit0(out_circuit, out_wire_component_aux_info.data(), out_wires[j]));
          XOR_128(solder_keys[inp_pos[l] + curr_inp_wire],
                  ConstGarbledCircuit::out_key_commit1(out_circuit, out_wire_component_aux_info.data(), out_wires[j]));

          XOR_CodeWords(solder_keys_share[0][inp_pos[l] + curr_inp_wire], ConstGarbledCircuit::out_key_commit0(out_circuit, out_wire_component_aux_info.data(), out_wires[j]));

          XOR_CodeWords(solder_keys_share[1][inp_pos[l] + curr_inp_wire], ConstGarbledCircuit::out_key_commit1(out_circuit, out_wire_component_aux_info.data(), out_wires[j]));
        }

        perm_bits[inp_pos[l] + curr_inp_wire] = GetLSB(solder_keys[inp_pos[l] + curr_inp_wire]);

        if (perm_bits[inp_pos[l] + curr_inp_wire]) {

          XOR_128(solder_keys[inp_pos[l] + curr_inp_wire],
                  ConstGarbledCircuit::delta_commit0(inp_circuit, inp_wire_component_aux_info.data()));
          XOR_128(solder_keys[inp_pos[l] + curr_inp_wire],
                  ConstGarbledCircuit::delta_commit1(inp_circuit, inp_wire_component_aux_info.data()));
          XOR_CodeWords(solder_keys_share[0][inp_pos[l] + curr_inp_wire], ConstGarbledCircuit::delta_commit0(inp_circuit, inp_wire_component_aux_info.data()));
          XOR_CodeWords(solder_keys_share[1][inp_pos[l] + curr_inp_wire], ConstGarbledCircuit::delta_commit1(inp_circuit, inp_wire_component_aux_info.data()));

        }
        ++curr_inp_wire;
      }
    }
  }

  exec_channels[exec_id].send(perm_bits);

  //Send postulated solderings and prove correct using BatchDecommit
  exec_channels[exec_id].send(solder_keys.data(), solder_keys.size());

  commit_senders[exec_id].BatchDecommit(solder_keys_share, exec_channels[exec_id], true);
}

void DuploConstructor::DecommitInpPermBits(uint32_t exec_id, uint32_t offset, uint32_t eval_inputs_from, uint32_t eval_inputs_to) {

  uint32_t exec_num_inputs = eval_inputs_to - eval_inputs_from;

  if (exec_num_inputs < 1) {
    return;
  }

  std::array<BYTEArrayVector, 2> commit_shares_lsb_blind = {
    BYTEArrayVector(SSEC, CODEWORD_BYTES),
    BYTEArrayVector(SSEC, CODEWORD_BYTES)
  };
  commit_senders[exec_id].Commit(commit_shares_lsb_blind, exec_channels[exec_id], std::numeric_limits<uint32_t>::max(), ALL_RND_LSB_ZERO);

  std::array<BYTEArrayVector, 2> decommit_lsb_share = {
    BYTEArrayVector(exec_num_inputs, CODEWORD_BYTES),
    BYTEArrayVector(exec_num_inputs, CODEWORD_BYTES)
  };
  osuCrypto::BitVector decommit_lsb(exec_num_inputs);

  BYTEArrayVector inp_bucket_aux_data;
  persistent_storage.ReadBuckets(CONST_INP_BUCKET_PREFIX, AUXDATA, (offset + eval_inputs_from), exec_num_inputs, inp_bucket_aux_data);

  for (int i = (offset + eval_inputs_from); i < (offset + eval_inputs_to); ++i) {
    uint32_t curr_local_idx = i - (offset + eval_inputs_from);

    std::copy(ConstGarbledCircuit::inp_key_commit0(inp_bucket_circuit, inp_bucket_aux_data[curr_local_idx], 0),
              ConstGarbledCircuit::inp_key_commit0(inp_bucket_circuit, inp_bucket_aux_data[curr_local_idx], 1),
              decommit_lsb_share[0][curr_local_idx]);

    std::copy(ConstGarbledCircuit::inp_key_commit1(inp_bucket_circuit, inp_bucket_aux_data[curr_local_idx], 0),
              ConstGarbledCircuit::inp_key_commit1(inp_bucket_circuit, inp_bucket_aux_data[curr_local_idx], 1),
              decommit_lsb_share[1][curr_local_idx]);

    decommit_lsb[curr_local_idx] = GetLSB(decommit_lsb_share[0][curr_local_idx]) ^ GetLSB(decommit_lsb_share[1][curr_local_idx]);
  }

  //Send postulated perm bit values and prove correct using BatchDecommit
  exec_channels[exec_id].send(decommit_lsb);
  commit_senders[exec_id].BatchDecommitLSB(decommit_lsb_share, commit_shares_lsb_blind, exec_channels[exec_id]);
}

void DuploConstructor::PreprocessEvalInputOTs(uint32_t exec_id, uint32_t eval_inputs_from, uint32_t eval_inputs_to) {

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

  //Run OTX
  uint8_t delta[CSEC_BYTES] = {0};
  BYTEArrayVector input_masks(num_ots, CSEC_BYTES);
  std::vector<std::array<osuCrypto::block, 2>> msgs(num_ots);

  dot_senders[exec_id]->send(msgs, exec_rnds[exec_id], exec_channels[exec_id]);

  osuCrypto::block block_delta = msgs[0][0] ^ msgs[0][1];
  _mm_storeu_si128((__m128i*) delta, block_delta);

  for (int i = 0; i < num_ots; ++i) {
    _mm_storeu_si128((__m128i*) input_masks[i], msgs[i][0]);
  }

  //Run Random Commit
  uint32_t num_commits = num_ots + 1;

  std::array<BYTEArrayVector, 2> commit_keys_share = {
    BYTEArrayVector(num_commits, CODEWORD_BYTES),
    BYTEArrayVector(num_commits, CODEWORD_BYTES)
  };

  commit_senders[exec_id].Commit(commit_keys_share, exec_channels[exec_id]);

  //Run chosen commit
  BYTEArrayVector input_mask_corrections(num_commits, CSEC_BYTES);
  for (int i = 0; i < num_ots; ++i) {
    XOR_128(input_mask_corrections[i], commit_keys_share[0][i], commit_keys_share[1][i]);
    XOR_128(input_mask_corrections[i], input_masks[i]);
  }
  XOR_128(input_mask_corrections[num_ots], commit_keys_share[0][num_ots], commit_keys_share[1][num_ots]);
  XOR_128(input_mask_corrections[num_ots], delta);

  exec_channels[exec_id].send(input_mask_corrections.data(), input_mask_corrections.size());

  //////////////////////////////////////CNC////////////////////////////////////
  if (exec_id == 0) {

    //Receive values from receiver and check that they are valid OTs. In the same loop we also build the decommit information.
    BYTEArrayVector cnc_ot_values(SSEC, CSEC_BYTES);
    osuCrypto::BitVector ot_delta_cnc_choices(SSEC);
    exec_channels[exec_id].recv(cnc_ot_values.data(), cnc_ot_values.size());
    exec_channels[exec_id].recv(ot_delta_cnc_choices.data(), ot_delta_cnc_choices.sizeBytes());

    uint8_t correct_ot_value[CSEC_BYTES];
    std::array<BYTEArrayVector, 2> chosen_decommit_shares = {
      BYTEArrayVector(SSEC, CODEWORD_BYTES),
      BYTEArrayVector(SSEC, CODEWORD_BYTES)
    };

    for (int i = 0; i < SSEC; ++i) {
      std::copy(commit_keys_share[0][exec_num_eval_inputs + i], commit_keys_share[0][exec_num_eval_inputs + i + 1], chosen_decommit_shares[0][i]);
      std::copy(commit_keys_share[1][exec_num_eval_inputs + i], commit_keys_share[1][exec_num_eval_inputs + i + 1], chosen_decommit_shares[1][i]);
      std::copy(input_masks[exec_num_eval_inputs + i], input_masks[exec_num_eval_inputs + i + 1], correct_ot_value);

      if (ot_delta_cnc_choices[i]) {

        XOR_CodeWords(chosen_decommit_shares[0][i], commit_keys_share[0][num_ots]);
        XOR_CodeWords(chosen_decommit_shares[1][i], commit_keys_share[1][num_ots]);
        XOR_128(correct_ot_value, delta);
      }

      if (!std::equal(correct_ot_value, correct_ot_value + CSEC_BYTES,  cnc_ot_values[i])) {
        std::cout << "Receiver cheating. Trying to make us open to wrong OT!" << std::endl;
        throw std::runtime_error("Receiver cheating. Trying to make us open to wrong OT!");
      }
    }

    //As receiver sent correct input masks, we now decommit to the same values. Will prove that sender indeed comitted to Delta
    commit_senders[exec_id].Decommit(chosen_decommit_shares, exec_channels[exec_id]);
  }

  //Update global input wire counter
  state_mutex.lock();
  curr_num_ready_inputs = exec_num_eval_inputs;
  state_mutex.unlock();

  //Write data to disc
  BYTEArrayVector shares(exec_num_eval_inputs, 2 * CODEWORD_BYTES);
  BYTEArrayVector delta_shares(2, CODEWORD_BYTES);

  for (int i = 0; i < exec_num_eval_inputs; ++i) {
    std::copy(commit_keys_share[0][i], commit_keys_share[0][i + 1], shares[i]);
    std::copy(commit_keys_share[1][i], commit_keys_share[1][i + 1], shares[i] + CODEWORD_BYTES);
  }
  std::copy(commit_keys_share[0][num_ots], commit_keys_share[0][num_ots + 1], delta_shares[0]);
  std::copy(commit_keys_share[1][num_ots], commit_keys_share[1][num_ots + 1], delta_shares[1]);


  persistent_storage.WriteBuckets(CONST_PREPROCESS_PREFIX, INPUT_MASKS_AUXDATA, eval_inputs_from, exec_num_eval_inputs, shares.data(), eval_inputs_from * 2 * CODEWORD_BYTES, shares.size(), 1);

  if (exec_id == 0) {
    persistent_storage.WriteBuckets(CONST_PREPROCESS_PREFIX, INPUT_MASKS_DELTA_AUXDATA, 0, 2, delta_shares.data(), 0, delta_shares.size(), 1);
  }
}

void DuploConstructor::PreprocessBlindOutPermBits(uint32_t exec_id, uint32_t eval_outputs_from, uint32_t eval_outputs_to) {

  uint32_t exec_num_outputs = eval_outputs_to - eval_outputs_from;

  std::array<BYTEArrayVector, 2> commit_shares_lsb_blind = {
    BYTEArrayVector(exec_num_outputs, CODEWORD_BYTES),
    BYTEArrayVector(exec_num_outputs, CODEWORD_BYTES)
  };
  commit_senders[exec_id].Commit(commit_shares_lsb_blind, exec_channels[exec_id], std::numeric_limits<uint32_t>::max(), ALL_RND_LSB_ZERO);

  //Write data to disc
  BYTEArrayVector shares(exec_num_outputs, 2 * CODEWORD_BYTES);

  for (int i = 0; i < exec_num_outputs; ++i) {
    std::copy(commit_shares_lsb_blind[0][i], commit_shares_lsb_blind[0][i + 1], shares[i]);
    std::copy(commit_shares_lsb_blind[1][i], commit_shares_lsb_blind[1][i + 1], shares[i] + CODEWORD_BYTES);
  }

  persistent_storage.WriteBuckets(CONST_PREPROCESS_PREFIX, OUTPUT_MASKS_AUXDATA, eval_outputs_from, exec_num_outputs, shares.data(), eval_outputs_from * 2 * CODEWORD_BYTES, shares.size(), 1);
}

void DuploConstructor::GetInputs(uint32_t exec_id, ComposedCircuit& composed_circuit, osuCrypto::BitVector& plain_inputs, uint32_t const_inputs_from, uint32_t const_inputs_to, uint32_t eval_inputs_from, uint32_t eval_inputs_to) {

  uint32_t exec_num_const_inputs = const_inputs_to - const_inputs_from;
  uint32_t exec_num_eval_inputs = eval_inputs_to - eval_inputs_from;

  osuCrypto::BitVector e(exec_num_eval_inputs);
  std::vector<uint8_t> e_bytes(exec_num_eval_inputs);

  //Read all required dot input masks and their global delta
  BYTEArrayVector input_masks_shares;
  BYTEArrayVector input_masks_delta_shares;
  BYTEArrayVector eval_inp_bucket_aux_data;

  std::array<BYTEArrayVector, 2> eval_decommits = {
    BYTEArrayVector(2 * exec_num_eval_inputs, CODEWORD_BYTES),
    BYTEArrayVector(2 * exec_num_eval_inputs, CODEWORD_BYTES)
  };
  uint32_t eval_delta_offset = exec_num_eval_inputs;

  if (exec_num_eval_inputs != 0) {
    exec_channels[exec_id].recv(e);
  }

  //Convert from bits to bytes
  for (int i = 0; i < exec_num_eval_inputs; ++i) {
    e_bytes[i] = e[i];
  }

  persistent_storage.ReadBuckets(CONST_PREPROCESS_PREFIX, INPUT_MASKS_AUXDATA, eval_inputs_from, exec_num_eval_inputs, input_masks_shares);
  persistent_storage.ReadBuckets(CONST_PREPROCESS_PREFIX, INPUT_MASKS_DELTA_AUXDATA, 0, 2, input_masks_delta_shares);
  persistent_storage.ReadBuckets(CONST_INP_BUCKET_PREFIX, AUXDATA, composed_circuit.num_const_inp_wires + eval_inputs_from, exec_num_eval_inputs, eval_inp_bucket_aux_data);


  BYTEArrayVector const_keys(exec_num_const_inputs, CSEC_BYTES);

  BYTEArrayVector const_inp_bucket_aux_data;
  persistent_storage.ReadBuckets(CONST_INP_BUCKET_PREFIX, AUXDATA, const_inputs_from, exec_num_const_inputs, const_inp_bucket_aux_data);
  //Send input keys
  for (int i = const_inputs_from; i < const_inputs_to; ++i) {
    uint32_t curr_local_idx = i - const_inputs_from;

    XOR_128(const_keys[curr_local_idx],
            ConstGarbledCircuit::inp_key_commit0(inp_bucket_circuit, const_inp_bucket_aux_data[curr_local_idx]),
            ConstGarbledCircuit::inp_key_commit1(inp_bucket_circuit, const_inp_bucket_aux_data[curr_local_idx]));

    uint8_t perm_bit = GetLSB(const_keys[curr_local_idx]);
    SetLSB(const_keys[curr_local_idx], 0);

    if (perm_bit ^ plain_inputs[i]) {

      XOR_128(const_keys[curr_local_idx],
              ConstGarbledCircuit::delta_commit0(inp_bucket_circuit, const_inp_bucket_aux_data[curr_local_idx]));
      XOR_128(const_keys[curr_local_idx],
              ConstGarbledCircuit::delta_commit1(inp_bucket_circuit, const_inp_bucket_aux_data[curr_local_idx]));
    }
  }

  //Construct eval input keys
  for (int i = eval_inputs_from; i < eval_inputs_to; ++i) {
    uint32_t curr_local_idx = i - eval_inputs_from;

    //First copy eval keys
    std::copy(ConstGarbledCircuit::inp_key_commit0(inp_bucket_circuit, eval_inp_bucket_aux_data[curr_local_idx], 0),
              ConstGarbledCircuit::inp_key_commit0(inp_bucket_circuit, eval_inp_bucket_aux_data[curr_local_idx], 1),
              eval_decommits[0][curr_local_idx]);

    std::copy(ConstGarbledCircuit::inp_key_commit1(inp_bucket_circuit, eval_inp_bucket_aux_data[curr_local_idx], 0),
              ConstGarbledCircuit::inp_key_commit1(inp_bucket_circuit, eval_inp_bucket_aux_data[curr_local_idx], 1),
              eval_decommits[1][curr_local_idx]);

    //XOR with mask keys
    XOR_CodeWords(eval_decommits[0][curr_local_idx], input_masks_shares[curr_local_idx]);
    XOR_CodeWords(eval_decommits[1][curr_local_idx], input_masks_shares[curr_local_idx] + CODEWORD_BYTES);

    //If e_j for this component is set XOR input_mask_delta onto the decommit
    if (e_bytes[curr_local_idx]) {
      XOR_CodeWords(eval_decommits[0][curr_local_idx],
                    input_masks_delta_shares[0]);
      XOR_CodeWords(eval_decommits[1][curr_local_idx],
                    input_masks_delta_shares[1]);
    }

    //Construct current component delta soldering
    std::copy(ConstGarbledCircuit::delta_commit0(inp_bucket_circuit, eval_inp_bucket_aux_data[curr_local_idx]),
              ConstGarbledCircuit::delta_commit0(inp_bucket_circuit, eval_inp_bucket_aux_data[curr_local_idx]) + CODEWORD_BYTES,
              eval_decommits[0][eval_delta_offset + curr_local_idx]);

    std::copy(ConstGarbledCircuit::delta_commit1(inp_bucket_circuit, eval_inp_bucket_aux_data[curr_local_idx]),
              ConstGarbledCircuit::delta_commit1(inp_bucket_circuit, eval_inp_bucket_aux_data[curr_local_idx]) + CODEWORD_BYTES,
              eval_decommits[1][eval_delta_offset + curr_local_idx]);

    //XOR with mask keys
    XOR_CodeWords(eval_decommits[0][eval_delta_offset + curr_local_idx], input_masks_delta_shares[0]);
    XOR_CodeWords(eval_decommits[1][eval_delta_offset + curr_local_idx], input_masks_delta_shares[1]);
  }

  if (exec_num_const_inputs != 0) {
    exec_channels[exec_id].send(const_keys.data(), const_keys.size());
  }

  if (exec_num_eval_inputs != 0) {
    commit_senders[exec_id].Decommit(eval_decommits, exec_channels[exec_id]);
  }
}

void DuploConstructor::DecommitOutPermBits(uint32_t exec_id, std::vector<std::pair<std::string, uint32_t>>& output_components, std::vector<std::vector<uint32_t>>& const_outputs, std::vector<std::vector<uint32_t>>& eval_outputs, uint32_t components_from, uint32_t components_to, std::vector<osuCrypto::BitVector>& output_decodings, bool non_interactive) {

  uint32_t exec_num_components = components_to - components_from;

  uint32_t num_eval_total_outputs = GetNumberOfOutputWires(eval_outputs, components_from, components_to);

  uint32_t eval_outputs_from = GetNumberOfOutputWires(eval_outputs, 0, components_from);

  BYTEArrayVector output_lsb_masks_shares;
  persistent_storage.ReadBuckets(CONST_PREPROCESS_PREFIX, OUTPUT_MASKS_AUXDATA, eval_outputs_from, num_eval_total_outputs, output_lsb_masks_shares);

  std::array<BYTEArrayVector, 2> decommit_lsb_share = {
    BYTEArrayVector(num_eval_total_outputs, CODEWORD_BYTES),
    BYTEArrayVector(num_eval_total_outputs, CODEWORD_BYTES)
  };

  uint32_t curr_bit_pos = 0;
  uint32_t curr_eval_out_idx = 0;
  for (int l = 0; l < exec_num_components; ++l) {
    uint32_t curr_component = components_from + l;
    std::string component_name = std::get<0>(output_components[curr_component]);
    uint32_t component_num = std::get<1>(output_components[curr_component]);
    Circuit& circuit = string_to_circuit_map[component_name];

    BYTEArrayVector component_aux_data;
    persistent_storage.ReadBuckets(component_name, AUXDATA, component_num, 1, component_aux_data, true, exec_id);
    output_decodings.emplace_back(const_outputs[curr_component].size());

    for (int i = 0; i < circuit.num_out_wires; ++i) {

      std::copy(ConstGarbledCircuit::out_key_commit0(circuit, component_aux_data.data(), i),
                ConstGarbledCircuit::out_key_commit0(circuit, component_aux_data.data(), i + 1),
                decommit_lsb_share[0][curr_bit_pos]);
      std::copy(ConstGarbledCircuit::out_key_commit1(circuit, component_aux_data.data(), i),
                ConstGarbledCircuit::out_key_commit1(circuit, component_aux_data.data(), i + 1),
                decommit_lsb_share[1][curr_bit_pos]);

      if ((circuit.const_out_wires_start <= i) &&
          (i < circuit.const_out_wires_stop)) {
        output_decodings[l][i] = GetLSB(decommit_lsb_share[0][curr_bit_pos]) ^ GetLSB(decommit_lsb_share[1][curr_bit_pos]);
      }

      if ((circuit.eval_out_wires_start <= i) &&
          (i < circuit.eval_out_wires_stop)) {
        XOR_CodeWords(decommit_lsb_share[0][curr_bit_pos], output_lsb_masks_shares[curr_eval_out_idx]);
        XOR_CodeWords(decommit_lsb_share[1][curr_bit_pos], output_lsb_masks_shares[curr_eval_out_idx] + CODEWORD_BYTES);
        ++curr_eval_out_idx;
      }

      ++curr_bit_pos;
    }
  }

  if (non_interactive) {
    commit_senders[exec_id].Decommit(decommit_lsb_share, exec_channels[exec_id]);  
  } else {
    commit_senders[exec_id].BatchDecommit(decommit_lsb_share, exec_channels[exec_id]);
  }
}