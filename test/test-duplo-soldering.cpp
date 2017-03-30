#include "test-duplo.h"

TEST_F(TestDuplo, Soldering) {

  std::string filename("test/data/p1_aes.wir.GC_duplo");

  ComposedCircuit composed_circuit = read_composed_circuit(filename.c_str());

  std::vector<osuCrypto::BitVector> const_outputs(composed_circuit.output_circuits.size());
  std::future<void> ret_const = std::async(std::launch::async,

  [this, &const_outputs, &filename]() {

    std::string const_composed_circuit_name("const_composed_test");
    ComposedCircuit const_composed_circuit = read_composed_circuit(filename.c_str(), const_composed_circuit_name);

    duplo_const.Connect(default_ip_address, default_port);
    duplo_const.Setup();
    for (int i = 0; i < const_composed_circuit.num_functions; ++i) {
      duplo_const.PreprocessComponentType(const_composed_circuit.functions[i].circuit_name, const_composed_circuit.functions[i], const_composed_circuit.num_circuit_copies[i], num_execs_components);
    }

    duplo_const.PrepareComponents(const_composed_circuit.num_inp_wires, num_execs_auths);

    duplo_const.Build(const_composed_circuit, num_execs_auths);

    osuCrypto::BitVector inputs(const_composed_circuit.num_const_inp_wires); //0 input

    duplo_const.Evaluate(const_composed_circuit, inputs, num_execs_online);

    std::vector<std::vector<uint32_t>> const_output_idices = const_composed_circuit.GetOutputIndices(true);
    std::vector<std::vector<uint32_t>> eval_output_indices = const_composed_circuit.GetOutputIndices(false);

    duplo_const.DecodeKeys(const_composed_circuit, const_output_idices, eval_output_indices, const_outputs, true, num_execs_online);
  });

  std::vector<osuCrypto::BitVector> eval_outputs(composed_circuit.output_circuits.size());
  std::future<void> ret_eval = std::async(std::launch::async,
  [this, &eval_outputs, &filename]() {

    std::string eval_composed_circuit_name("eval_composed_test");
    ComposedCircuit eval_composed_circuit = read_composed_circuit(filename.c_str(), eval_composed_circuit_name);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    duplo_eval.Connect(default_ip_address, default_port);
    duplo_eval.Setup();

    for (int i = 0; i < eval_composed_circuit.num_functions; ++i) {
      duplo_eval.PreprocessComponentType(eval_composed_circuit.functions[i].circuit_name, eval_composed_circuit.functions[i], eval_composed_circuit.num_circuit_copies[i], num_execs_components);
    }

    duplo_eval.PrepareComponents(eval_composed_circuit.num_inp_wires, num_execs_auths);

    duplo_eval.Build(eval_composed_circuit, num_execs_auths);

    osuCrypto::BitVector inputs(eval_composed_circuit.num_eval_inp_wires); // 0 input

    BYTEArrayVector output_keys(eval_composed_circuit.num_out_wires, CSEC_BYTES);
    duplo_eval.Evaluate(eval_composed_circuit, inputs, output_keys, num_execs_online);

    std::vector<std::vector<uint32_t>> const_output_idices = eval_composed_circuit.GetOutputIndices(true);
    std::vector<std::vector<uint32_t>> eval_output_indices = eval_composed_circuit.GetOutputIndices(false);

    duplo_eval.DecodeKeys(eval_composed_circuit, const_output_idices, eval_output_indices, output_keys, eval_outputs, true, num_execs_online);
  });

  ret_const.wait();
  ret_eval.wait();

  osuCrypto::BitVector composed_input(composed_circuit.num_inp_wires); //0 input
  osuCrypto::BitVector composed_res = eval_circuit(composed_circuit, composed_input);

  osuCrypto::BitVector all_outputs(composed_circuit.num_out_wires);
  uint32_t curr_output = 0;
  for (int i = 0; i < composed_circuit.output_circuits.size(); ++i) {
    std::string circuit_name = composed_circuit.circuits[composed_circuit.output_circuits[i]].first;
    uint32_t function_num = composed_circuit.name_to_function_num[circuit_name];
    Circuit& circuit = composed_circuit.functions[function_num];

    ASSERT_TRUE(std::equal(eval_outputs[i].data(), eval_outputs[i].data() + BITS_TO_BYTES(circuit.num_out_wires), const_outputs[i].data())); //same output

    //Move the outputs to a single array
    for (int j = 0; j < circuit.num_out_wires; ++j) {
      all_outputs[curr_output] = eval_outputs[i][j];
      ++curr_output;
    }
  }

  ASSERT_TRUE(std::equal(composed_res.data(), composed_res.data() + BITS_TO_BYTES(composed_circuit.num_out_wires), all_outputs.data())); //correct output
}

TEST_F(TestDuplo, ParallelSoldering) {

  std::string filename("test/data/AES-non-expanded.txt");
  uint32_t num_iters = 13;
  Circuit circuit = read_bristol_circuit(filename.c_str());
  ComposedCircuit composed_circuit(circuit, num_iters);

  std::vector<osuCrypto::BitVector> const_outputs(composed_circuit.output_circuits.size());
  std::future<void> ret_const = std::async(std::launch::async,

  [this, &const_outputs, &filename, num_iters]() {

    Circuit circuit = read_bristol_circuit(filename.c_str());
    circuit.circuit_name = "const_composed_test";
    ComposedCircuit const_composed_circuit(circuit, num_iters);

    duplo_const.Connect(default_ip_address, default_port);

    duplo_const.Setup();

    for (int i = 0; i < const_composed_circuit.num_functions; ++i) {
      duplo_const.PreprocessComponentType(const_composed_circuit.functions[i].circuit_name, const_composed_circuit.functions[i], const_composed_circuit.num_circuit_copies[i], num_execs_components);
    }

    duplo_const.PrepareComponents(const_composed_circuit.num_inp_wires, num_execs_auths);

    duplo_const.Build(const_composed_circuit, num_execs_auths);

    osuCrypto::BitVector inputs(const_composed_circuit.num_const_inp_wires); //0 input

    duplo_const.Evaluate(const_composed_circuit, inputs, num_execs_online);

    std::vector<std::vector<uint32_t>> const_output_idices = const_composed_circuit.GetOutputIndices(true);
    std::vector<std::vector<uint32_t>> eval_output_indices = const_composed_circuit.GetOutputIndices(false);

    duplo_const.DecodeKeys(const_composed_circuit, const_output_idices, eval_output_indices, const_outputs, false, num_execs_online);
  });

  std::vector<osuCrypto::BitVector> eval_outputs(composed_circuit.output_circuits.size());
  std::future<void> ret_eval = std::async(std::launch::async,
  [this, &eval_outputs, &filename, num_iters]() {

    Circuit circuit = read_bristol_circuit(filename.c_str());
    circuit.circuit_name = "eval_composed_test";
    ComposedCircuit eval_composed_circuit(circuit, num_iters);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    duplo_eval.Connect(default_ip_address, default_port);
    duplo_eval.Setup();

    for (int i = 0; i < eval_composed_circuit.num_functions; ++i) {
      duplo_eval.PreprocessComponentType(eval_composed_circuit.functions[i].circuit_name, eval_composed_circuit.functions[i], eval_composed_circuit.num_circuit_copies[i], num_execs_components);
    }

    duplo_eval.PrepareComponents(eval_composed_circuit.num_inp_wires, num_execs_auths);

    duplo_eval.Build(eval_composed_circuit, num_execs_auths);

    osuCrypto::BitVector inputs(eval_composed_circuit.num_eval_inp_wires); // 0 input

    BYTEArrayVector output_keys(eval_composed_circuit.num_out_wires, CSEC_BYTES);
    duplo_eval.Evaluate(eval_composed_circuit, inputs, output_keys, num_execs_online);

    std::vector<std::vector<uint32_t>> const_output_idices = eval_composed_circuit.GetOutputIndices(true);
    std::vector<std::vector<uint32_t>> eval_output_indices = eval_composed_circuit.GetOutputIndices(false);

    duplo_eval.DecodeKeys(eval_composed_circuit, const_output_idices, eval_output_indices, output_keys, eval_outputs, false, num_execs_online);
  });

  ret_const.wait();
  ret_eval.wait();

  osuCrypto::BitVector composed_input(composed_circuit.num_inp_wires); //0 input
  osuCrypto::BitVector composed_res = eval_circuit(composed_circuit, composed_input);

  for (int i = 0; i < num_iters; ++i) {
    ASSERT_TRUE(std::equal(eval_outputs[i].data(), eval_outputs[i].data() + BITS_TO_BYTES(circuit.num_out_wires), const_outputs[i].data())); //same output
    ASSERT_TRUE(std::equal(composed_res.data() + i * BITS_TO_BYTES(circuit.num_out_wires), composed_res.data() + (i + 1) * BITS_TO_BYTES(circuit.num_out_wires), const_outputs[i].data())); //correct output
  }
}
