#ifndef DUPLO_GARBLING_GARBLEDCIRCUIT_H_
#define DUPLO_GARBLING_GARBLEDCIRCUIT_H_

#include "circuit/circuit.h"

class GarbledCircuit {
private:
  BYTEArrayVector garbled_tables;
  std::vector<uint8_t> aux_info;

public:

  Circuit& circuit;

  GarbledCircuit(Circuit& circuit);
  GarbledCircuit(Circuit& circuit, bool tables);

  uint64_t size;
  uint8_t* T_G(uint32_t idx);
  uint8_t* T_E(uint32_t idx);
  uint8_t* GetTables();

  uint8_t* inp_key_commit0(uint32_t idx);
  uint8_t* out_key_commit0(uint32_t idx);
  uint8_t* delta_commit0();
  
  uint8_t* inp_key_commit1(uint32_t idx);
  uint8_t* out_key_commit1(uint32_t idx);
  uint8_t* delta_commit1();

  static uint64_t NumTables(Circuit& circuit) {
    return 2 * circuit.num_non_free_gates;
  };

  static uint64_t TotalTableSize(Circuit& circuit) {
    return NumTables(circuit) * CSEC_BYTES;
  };

  static uint8_t* T_G(uint8_t garbled_tables[], Circuit& circuit, uint32_t idx) {
    return garbled_tables + idx * CSEC_BYTES;
  }

  static uint8_t* T_E(uint8_t garbled_tables[], Circuit& circuit, uint32_t idx) {
    return garbled_tables + (circuit.num_non_free_gates + idx) * CSEC_BYTES;
  }

};

#endif /* DUPLO_GARBLING_GARBLEDCIRCUIT_H_ */