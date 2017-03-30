#include "test.h"
#include "AES_Encrypt.h"

#include "util/util.h"

TEST(TestParser, AESBristolCorrectness) {
  Circuit circuit = read_bristol_circuit("test/data/AES-non-expanded.txt");

  osuCrypto::BitVector input = GetBitsReversed(fopen("test/data/aes_input_0.bin", "rb"), circuit.num_inp_wires);
  osuCrypto::BitVector expected_output = GetBitsReversed(fopen("test/data/aes_expected_0.bin", "rb"), circuit.num_out_wires);

  osuCrypto::BitVector res = eval_circuit(circuit, input);
  ASSERT_TRUE(std::equal(res.data(), res.data() + BITS_TO_BYTES(circuit.num_out_wires), expected_output.data()));
}

TEST(TestParser, OurComposedAESEqualBristol) {
 ComposedCircuit composed_circuit = read_composed_circuit("test/data/p1_aes.wir.GC_duplo");

 osuCrypto::BitVector input = GetBits(fopen("test/data/aes_input_0.bin", "rb"), composed_circuit.num_inp_wires);
 osuCrypto::BitVector expected_output = GetBits(fopen("test/data/aes_expected_0.bin", "rb"), composed_circuit.num_out_wires);
 
 osuCrypto::BitVector output = eval_circuit(composed_circuit, input);

 ASSERT_TRUE(std::equal(output.data(), output.data() + BITS_TO_BYTES(composed_circuit.num_out_wires), expected_output.data()));
}


TEST(TestParser, OurComposedAESCorrect) {
  ComposedCircuit composed_circuit = read_composed_circuit("test/data/p1_aes.wir.GC_duplo");
  Circuit circuit_bristol = read_bristol_circuit("test/data/AES-non-expanded.txt");

  std::vector<uint8_t> input(
  { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, //text
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f //key
  });

  std::vector<uint8_t> plaintext, key;
  for (int i = 0; i < 16; i++) {
    plaintext.push_back(input[i]);
    key.push_back(input[16 + i]);
  }

  std::vector<uint8_t> raw_output = AESEncrypt(plaintext, key);
  osuCrypto::BitVector our_input(composed_circuit.num_inp_wires);
  for (int i = 0; i < composed_circuit.num_inp_wires; i++) {
    our_input[i] = GetBit(i, input.data());
  }

  osuCrypto::BitVector our_output = eval_circuit(composed_circuit, our_input);

  //Reverse input/output evaluation for Bristol circuit
  osuCrypto::BitVector bristol_output = eval_circuit(circuit_bristol, our_input, true);

  ASSERT_TRUE(std::equal(our_output.data(), our_output.data() + BITS_TO_BYTES(composed_circuit.num_out_wires), raw_output.data()));
  ASSERT_TRUE(std::equal(our_output.data(), our_output.data() + BITS_TO_BYTES(composed_circuit.num_out_wires), bristol_output.data()));
}

TEST(TestParser, OurAmortizedComposedAESCorrect) {
  ComposedCircuit circuit = read_composed_circuit("test/data/p1_aes.wir.GC_duplo");
  ComposedCircuit composed_circuit = read_composed_circuit("test/data/p32_aes.wir.GC_duplo");

  std::vector<uint8_t> input(
  { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, //text
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f //key
  });

  osuCrypto::BitVector our_input(input.data(), circuit.num_inp_wires);

  uint32_t num_iter = 32;
  osuCrypto::BitVector p_inputs(num_iter * circuit.num_inp_wires);

  uint32_t j = 0;
  for (uint32_t p = 0; p < num_iter; p++) { // first num_iter*128 bits is texts
    for (uint32_t i = 0; i < circuit.num_inp_wires / 2; i++) {
      p_inputs[j++] = our_input[i];
    }
  }
  for (uint32_t p = 0; p < num_iter; p++) { // last num_iter*128 bits is keys
    for (uint32_t i = circuit.num_inp_wires / 2; i < circuit.num_inp_wires; i++) {
      p_inputs[j++] = our_input[i];
    }
  }


  osuCrypto::BitVector our_output = eval_circuit(circuit, our_input);
  osuCrypto::BitVector composed_output = eval_circuit(composed_circuit, p_inputs);


  for (uint32_t p = 0; p < num_iter; p++) {
    ASSERT_TRUE(std::equal(our_output.data(),
                           our_output.data() + BITS_TO_BYTES(circuit.num_out_wires),
                           composed_output.data() + p * BITS_TO_BYTES(circuit.num_out_wires)));
  }
}
