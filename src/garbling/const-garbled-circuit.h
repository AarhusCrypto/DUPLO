#ifndef DUPLO_GARBLING_CONSTGARBLEDCIRCUIT_H_
#define DUPLO_GARBLING_CONSTGARBLEDCIRCUIT_H_

#include "garbling/garbled-circuit.h"

class ConstGarbledCircuit : public GarbledCircuit {
private:
  std::vector<uint8_t> aux_info;

public:
  ConstGarbledCircuit(Circuit& circuit);
  ConstGarbledCircuit(Circuit& circuit, bool tables);
  uint8_t* GetAuxArray();
  uint8_t* inp_key_commit0(uint32_t idx = 0);
  uint8_t* out_key_commit0(uint32_t idx = 0);
  uint8_t* delta_commit0();

  uint8_t* inp_key_commit1(uint32_t idx = 0);
  uint8_t* out_key_commit1(uint32_t idx = 0);
  uint8_t* delta_commit1();
  
  static uint64_t AuxDataSize(Circuit& circuit) {
    return (circuit.num_inp_wires + circuit.num_out_wires + 1) * CODEWORD_BYTES * 2;
  };

  static uint8_t* inp_key_commit0(Circuit& circuit, uint8_t* aux_info, uint32_t idx = 0) {
    return aux_info + idx * CODEWORD_BYTES;
  };

  static uint8_t* out_key_commit0(Circuit& circuit, uint8_t* aux_info, uint32_t idx = 0) {
    return inp_key_commit0(circuit, aux_info, circuit.num_inp_wires) + idx * CODEWORD_BYTES;
  };

  static uint8_t* delta_commit0(Circuit& circuit, uint8_t* aux_info) {
    return out_key_commit0(circuit, aux_info, circuit.num_out_wires);
  };

  static uint8_t* inp_key_commit1(Circuit& circuit, uint8_t* aux_info, uint32_t idx = 0) {
    return delta_commit0(circuit, aux_info) + (1 + idx) * CODEWORD_BYTES;
  };

  static uint8_t* out_key_commit1(Circuit& circuit, uint8_t* aux_info, uint32_t idx = 0) {
    return inp_key_commit1(circuit, aux_info, circuit.num_inp_wires) + idx * CODEWORD_BYTES;
  };

  static uint8_t* delta_commit1(Circuit& circuit, uint8_t* aux_info) {
    return out_key_commit1(circuit, aux_info, circuit.num_out_wires);
  };
};

#endif /* DUPLO_GARBLING_CONSTGARBLEDCIRCUIT_H_ */