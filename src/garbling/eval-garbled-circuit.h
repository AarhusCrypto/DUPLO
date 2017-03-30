#ifndef DUPLO_GARBLING_EVALGARBLEDCIRCUIT_H_
#define DUPLO_GARBLING_EVALGARBLEDCIRCUIT_H_

#include "garbling/garbled-circuit.h"

class EvalGarbledCircuit : public GarbledCircuit {
private:
  std::vector<uint8_t> aux_info; //Cannot be BYTEArrayVector as entires vary in size

public:
  EvalGarbledCircuit(Circuit& circuit);
  EvalGarbledCircuit(Circuit& circuit, bool tables);

  uint8_t* GetAuxArray();
  uint8_t* inp_key_share(uint32_t idx = 0);
  uint8_t* out_key_share(uint32_t idx = 0);
  uint8_t* delta_share();

  uint8_t* inp_bit_share(uint32_t idx = 0);
  uint8_t* out_bit_share(uint32_t idx = 0);

  uint8_t* inp_soldering(uint32_t idx = 0);
  uint8_t* out_soldering(uint32_t idx = 0);
  uint8_t* delta_soldering();

  static uint64_t SolderingsSize(Circuit& circuit) {
    return (circuit.num_inp_wires + circuit.num_out_wires + 1) * CSEC_BYTES;
  };
    
  static uint64_t AuxDataSize(Circuit& circuit) {
    return (circuit.num_inp_wires + circuit.num_out_wires + 1) * CODEWORD_BYTES;
  };

  static uint64_t TotalAuxDataSize(Circuit& circuit) {
    return AuxDataSize(circuit) + SolderingsSize(circuit);
  };

  static uint8_t* inp_key_share(Circuit& circuit, uint8_t* aux_info, uint32_t idx = 0) {
    return aux_info + idx * CODEWORD_BYTES;
  };

  static uint8_t* out_key_share(Circuit& circuit, uint8_t* aux_info, uint32_t idx = 0) {
    return inp_key_share(circuit, aux_info, circuit.num_inp_wires) + idx * CODEWORD_BYTES;
  };

  static uint8_t* delta_share(Circuit& circuit, uint8_t* aux_info) {
    return out_key_share(circuit, aux_info, circuit.num_out_wires);
  };

  static uint8_t* inp_soldering(Circuit& circuit, uint8_t* aux_info, uint32_t idx = 0) {
    return delta_share(circuit, aux_info) + CODEWORD_BYTES + idx * CSEC_BYTES;
  };

  static uint8_t* out_soldering(Circuit& circuit, uint8_t* aux_info, uint32_t idx = 0) {
    return inp_soldering(circuit, aux_info, circuit.num_inp_wires) + idx * CSEC_BYTES;
  };

  static uint8_t* delta_soldering(Circuit& circuit, uint8_t* aux_info) {
    return out_soldering(circuit, aux_info, circuit.num_out_wires);
  };

  static uint8_t* inp_soldering_no_aux(Circuit& circuit, uint8_t* soldering_info, uint32_t idx = 0) {
    return soldering_info + idx * CSEC_BYTES;
  };

  static uint8_t* out_soldering_no_aux(Circuit& circuit, uint8_t* soldering_info, uint32_t idx = 0) {
    return inp_soldering_no_aux(circuit, soldering_info, circuit.num_inp_wires) + idx * CSEC_BYTES;
  };

  static uint8_t* delta_soldering_no_aux(Circuit& circuit, uint8_t* soldering_info) {
    return out_soldering_no_aux(circuit, soldering_info, circuit.num_out_wires);
  };
};

#endif /* DUPLO_GARBLING_CONSEVALLEDCIRCUIT_H_ */