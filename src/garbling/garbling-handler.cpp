#include "garbling/garbling-handler.h"

GarblingHandler::GarblingHandler() {
  aes128_load_key(global_aes_key, key_schedule);
}

void GarblingHandler::GarbleCircuit(uint8_t input_zero_keys[], uint8_t output_zero_keys[], uint8_t delta[], GarbledCircuit& garbled_circuit, uint8_t garbled_hash[]) {

  Circuit& plain_circuit = garbled_circuit.circuit;
  GarbleCircuit(input_zero_keys, output_zero_keys, delta, garbled_circuit);
  HashGarbledCircuitTables(plain_circuit, garbled_circuit.GetTables(), garbled_hash);
}

void GarblingHandler::GarbleCircuit(uint8_t input_zero_keys[], uint8_t output_zero_keys[], uint8_t delta[], GarbledCircuit& garbled_circuit) {

  Circuit& plain_circuit = garbled_circuit.circuit;
  std::vector<__m128i> intrin_values(plain_circuit.num_wires);

  __m128i intrin_delta = _mm_lddqu_si128((__m128i *) delta);

  //Load input 0-keys into intrin_values
  for (int i = 0; i < plain_circuit.num_inp_wires; ++i) {
    intrin_values[i] = _mm_lddqu_si128((__m128i *) (input_zero_keys + i * CSEC_BYTES));
  }

  //Garble circuit
  uint32_t curr_garbling_idx = 0;
  uint32_t curr_non_free_gate = 0;
  for (int i = 0; i < plain_circuit.num_gates; ++i) {
    Gate& g = plain_circuit.gates[i];
    if (g.parity) {
      IntrinGarbleGate(garbled_circuit.T_G(curr_non_free_gate), garbled_circuit.T_E(curr_non_free_gate), intrin_values[g.left_wire], intrin_values[g.right_wire], intrin_values[g.out_wire], intrin_delta, curr_garbling_idx, key_schedule, gate_tables_idx[g.type]);
      ++curr_non_free_gate;
      curr_garbling_idx += 2;
    } else {
      if (g.type == gate_tables[3]) {
        intrin_values[g.out_wire] = _mm_xor_si128(intrin_values[g.left_wire], intrin_delta);
      } else if (g.type == gate_tables[5]) {
        intrin_values[g.out_wire] = _mm_xor_si128(intrin_values[g.right_wire], intrin_delta);
      } else if (g.type == gate_tables[6]) {
        intrin_values[g.out_wire] = _mm_xor_si128(intrin_values[g.left_wire], intrin_values[g.right_wire]);
      } else if (g.type == gate_tables[9]) {
        intrin_values[g.out_wire] = _mm_xor_si128(intrin_values[g.left_wire], intrin_values[g.right_wire]);
        intrin_values[g.out_wire] = _mm_xor_si128(intrin_values[g.out_wire], intrin_delta);
      } else if (g.type == gate_tables[10]) {
        intrin_values[g.out_wire] = intrin_values[g.right_wire];
      } else if (g.type == gate_tables[12]) {
        intrin_values[g.out_wire] = intrin_values[g.left_wire];
      }
    }
  }

  //Store output keys to output_zero_keys
  for (int i = 0; i < plain_circuit.num_out_wires; ++i) {
    _mm_storeu_si128((__m128i*) (output_zero_keys + i * CSEC_BYTES), intrin_values[plain_circuit.out_wires_start + i]);
  }
}

void GarblingHandler::GarbleInpBucket(uint8_t hash_delta[], uint8_t h_0[], uint8_t h_1[], uint8_t delta[]) {

  uint32_t id = 0;
  __m128i delta_128 = _mm_lddqu_si128((__m128i *) delta);

  __m128i id_128 = (__m128i) _mm_load_ss((float*) &id);

  __m128i hash_delta_128;
  AESHash(delta_128, hash_delta_128, id_128, key_schedule);
  _mm_storeu_si128((__m128i*) hash_delta, hash_delta_128);

  __m128i auth0_128, auth1_128;
  AESHash(hash_delta_128, auth0_128, id_128, key_schedule);
  hash_delta_128 = _mm_xor_si128(hash_delta_128, delta_128);

  AESHash(hash_delta_128, auth1_128, id_128, key_schedule);

  _mm_storeu_si128((__m128i*) h_0, auth0_128);
  _mm_storeu_si128((__m128i*) h_1, auth1_128);

  int res = std::memcmp(h_0, h_1, CSEC_BYTES);
  if (res > 0) {
    //Do nothing
  } else if (res < 0) {
    //Swap the order of the authenticators
    uint8_t tmp[CSEC_BYTES];
    std::copy(h_0, h_0 + CSEC_BYTES, tmp);
    std::copy(h_1, h_1 + CSEC_BYTES, h_0);
    std::copy(tmp, tmp + CSEC_BYTES, h_1);
  } else {
    std::cout << "Congrats, this only happens with prob. 2^-128! It must be your lucky day!" << std::endl;
  }
}

bool GarblingHandler::VerifyUsingInpBucket(uint8_t h_0[], uint8_t h_1[], uint8_t hash_delta[]) {

  uint32_t id = 0;
  __m128i hash_delta_128 = _mm_lddqu_si128((__m128i *) hash_delta);

  __m128i id_128 = (__m128i) _mm_load_ss((float*) &id);

  __m128i hash_hash_delta_128;
  AESHash(hash_delta_128, hash_hash_delta_128, id_128, key_schedule);

  __m128i h_0_128 = _mm_lddqu_si128((__m128i *) h_0);
  __m128i h_1_128 = _mm_lddqu_si128((__m128i *) h_1);

  if (compare128(h_0_128, hash_hash_delta_128) ||
      compare128(h_1_128, hash_hash_delta_128)) {
    return true;
  } else return false;
}

void GarblingHandler::EncodeInput(BYTEArrayVector& input_zero_keys, uint8_t delta[], osuCrypto::BitVector& input, BYTEArrayVector& input_keys) {

  uint32_t num_inputs = input_keys.num_entries();

  for (int i = 0; i < num_inputs; ++i) {
    std::copy(input_zero_keys[i], input_zero_keys[i + 1], input_keys[i]);
    if (input[i]) {
      XOR_128(input_keys[i], delta);
    }
  }
}

void GarblingHandler::EvalGarbledCircuit(uint8_t input_keys[], Circuit& circuit, uint8_t garbled_tables[], uint8_t output_keys[]) {

  std::vector<__m128i> intrin_values(circuit.num_wires);

  //Load input keys into intrin_values
  for (int i = 0; i < circuit.num_inp_wires; ++i) {
    intrin_values[i] = _mm_lddqu_si128((__m128i *) (input_keys + i * CSEC_BYTES));
  }

  //Eval circuit
  uint32_t curr_non_free_gate = 0;
  uint32_t curr_garbling_idx = 0;
  for (int i = 0; i < circuit.num_gates; ++i) {
    Gate& g = circuit.gates[i];
    if (g.parity) {
      IntrinEvalGate(GarbledCircuit::T_G(garbled_tables, circuit, curr_non_free_gate), GarbledCircuit::T_E(garbled_tables, circuit, curr_non_free_gate), intrin_values[g.left_wire], intrin_values[g.right_wire], intrin_values[g.out_wire], curr_garbling_idx, key_schedule);

      ++curr_non_free_gate;
      curr_garbling_idx += 2;
    } else {
      if (g.type == gate_tables[3] ||
          g.type == gate_tables[12]) {
        intrin_values[g.out_wire] = intrin_values[g.left_wire];
      } else if (g.type == gate_tables[5] ||
                 g.type == gate_tables[10]) {
        intrin_values[g.out_wire] = intrin_values[g.right_wire];
      } else if (g.type == gate_tables[6] ||
                 g.type == gate_tables[9]) {
        intrin_values[g.out_wire] = _mm_xor_si128(intrin_values[g.left_wire], intrin_values[g.right_wire]);
      }
    }
  }

  for (int i = 0; i < circuit.num_out_wires; ++i) {
    __m128i curr_out_key = _mm_lddqu_si128((__m128i *) (output_keys + i * CSEC_BYTES));
    curr_out_key = _mm_xor_si128(curr_out_key, intrin_values[circuit.out_wires_start + i]);
    _mm_storeu_si128((__m128i*) (output_keys + i * CSEC_BYTES), curr_out_key);
  }
}

void GarblingHandler::EvalGarbledCircuitSolderings(uint8_t input_keys[], Circuit& circuit, uint8_t garbled_tables[], uint8_t solderings[], uint8_t output_keys[]) {

  std::vector<__m128i> intrin_values(circuit.num_wires);

  __m128i delta_soldering = _mm_lddqu_si128((__m128i *) EvalGarbledCircuit::delta_soldering_no_aux(circuit, solderings));
  //Load input keys into intrin_values
  __m128i current_soldering;
  for (int i = 0; i < circuit.num_inp_wires; ++i) {
    intrin_values[i] = _mm_lddqu_si128((__m128i *) (input_keys + i * CSEC_BYTES));

    if (GetLSB(intrin_values[i])) {
      intrin_values[i] = _mm_xor_si128(intrin_values[i], delta_soldering);
    }

    current_soldering = _mm_lddqu_si128((__m128i *) EvalGarbledCircuit::inp_soldering_no_aux(circuit, solderings, i));
    intrin_values[i] = _mm_xor_si128(intrin_values[i], current_soldering);
  }

  //Eval circuit
  uint32_t curr_non_free_gate = 0;
  uint32_t curr_garbling_idx = 0;
  for (int i = 0; i < circuit.num_gates; ++i) {
    Gate& g = circuit.gates[i];
    if (g.parity) {
      IntrinEvalGate(GarbledCircuit::T_G(garbled_tables, circuit, curr_non_free_gate), GarbledCircuit::T_E(garbled_tables, circuit, curr_non_free_gate), intrin_values[g.left_wire], intrin_values[g.right_wire], intrin_values[g.out_wire], curr_garbling_idx, key_schedule);

      ++curr_non_free_gate;
      curr_garbling_idx += 2;
    } else {
      if (g.type == gate_tables[3] ||
          g.type == gate_tables[12]) {
        intrin_values[g.out_wire] = intrin_values[g.left_wire];
      } else if (g.type == gate_tables[5] ||
                 g.type == gate_tables[10]) {
        intrin_values[g.out_wire] = intrin_values[g.right_wire];
      } else if (g.type == gate_tables[6] ||
                 g.type == gate_tables[9]) {
        intrin_values[g.out_wire] = _mm_xor_si128(intrin_values[g.left_wire], intrin_values[g.right_wire]);
      }
    }
  }

  for (int i = 0; i < circuit.num_out_wires; ++i) {
    current_soldering = _mm_lddqu_si128((__m128i *) EvalGarbledCircuit::out_soldering_no_aux(circuit, solderings, i));
    intrin_values[circuit.out_wires_start + i] = _mm_xor_si128(intrin_values[circuit.out_wires_start + i], current_soldering);

    if (GetLSB((intrin_values[circuit.out_wires_start + i]))) {
      intrin_values[circuit.out_wires_start + i] = _mm_xor_si128(intrin_values[circuit.out_wires_start + i], delta_soldering);
    }

    _mm_storeu_si128((__m128i*) (output_keys + i * CSEC_BYTES), intrin_values[circuit.out_wires_start + i]);
  }
}
