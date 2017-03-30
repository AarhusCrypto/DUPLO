#include "test.h"

#include "garbling/garbling-handler.h"

void TestCircuit(Circuit& circuit, osuCrypto::BitVector& input) {

  uint32_t num_circuits = 1;
  osuCrypto::PRNG rnd;
  rnd.SetSeed(load_block(duplo_constant_seeds[0]));
  std::vector<GarbledCircuit> garbled_circuits(num_circuits, GarbledCircuit(circuit));

  BYTEArrayVector deltas(num_circuits, CSEC_BYTES);
  rnd.get<uint8_t>(deltas.data(), deltas.size());

  std::vector<BYTEArrayVector> input_zero_keys(num_circuits, BYTEArrayVector(circuit.num_inp_wires, CSEC_BYTES));
  for (int i = 0; i < num_circuits; ++i) {
    rnd.get<uint8_t>(input_zero_keys[i].data(), input_zero_keys[i].size());
  }
  
  std::vector<BYTEArrayVector> output_zero_keys(num_circuits, BYTEArrayVector(circuit.num_inp_wires, CSEC_BYTES));

  BYTEArrayVector input_keys(circuit.num_inp_wires, CSEC_BYTES);
  BYTEArrayVector res_output_keys(circuit.num_out_wires, CSEC_BYTES);
  std::vector<osuCrypto::BitVector> output_decryptions;
  std::vector<osuCrypto::BitVector> output_results;

  GarblingHandler gh;
  for (int i = 0; i < num_circuits; ++i) {
    //Init garbled circuit and delta
    SetLSB(deltas[i], 1);

    //Garble Circuit
    gh.GarbleCircuit(input_zero_keys[i].data(), output_zero_keys[i].data(), deltas[i], garbled_circuits[i]);

    gh.EncodeInput(input_zero_keys[i], deltas[i], input, input_keys);

    //Extract output decoding bits
    output_decryptions.emplace_back(circuit.num_out_wires);
    for (int j = 0; j < circuit.num_out_wires; ++j) {
      output_decryptions[i][j] = GetLSB(output_zero_keys[i][j]);
    }

    //Init output_results
    output_results.emplace_back(circuit.num_out_wires);

    //Eval garbled circuit
    gh.EvalGarbledCircuit(input_keys.data(), circuit, garbled_circuits[i].GetTables(), res_output_keys.data());

    //Decode garbled circuit
    DecodeGarbledOutput(res_output_keys, output_decryptions[i], output_results[i], garbled_circuits[i].circuit.num_out_wires);

    //Check if expected_output matches actual output
    osuCrypto::BitVector expected_output = eval_circuit(circuit, input);
    for (int j = 0; j < circuit.num_out_wires; ++j) {

      ASSERT_TRUE(expected_output[j] == output_results[i][j]);
    }
  }
}

TEST(GarbledCircuit, AES) {

  Circuit circuit = read_bristol_circuit("test/data/AES-non-expanded.txt");
  osuCrypto::BitVector input = GetBitsReversed(fopen("test/data/aes_input_0.bin", "rb"), circuit.num_inp_wires);
  TestCircuit(circuit, input);
}

TEST(GarbledCircuit, SHA1) {

  Circuit circuit = read_bristol_circuit("test/data/sha-1.txt");
  osuCrypto::BitVector input = GetBitsReversed(fopen("test/data/sha1_input_0.bin", "rb"), circuit.num_inp_wires);
  TestCircuit(circuit, input);
}

TEST(GarbledCircuit, SHA256) {
  Circuit circuit = read_bristol_circuit("test/data/sha-256.txt");
  osuCrypto::BitVector input = GetBitsReversed(fopen("test/data/sha256_input_0.bin", "rb"), circuit.num_inp_wires);
  TestCircuit(circuit, input);
}