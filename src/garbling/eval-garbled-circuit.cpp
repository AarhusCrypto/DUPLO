
#include "garbling/eval-garbled-circuit.h"

EvalGarbledCircuit::EvalGarbledCircuit(Circuit& circuit) :
  GarbledCircuit(circuit),
  aux_info(TotalAuxDataSize(circuit)) {
}

EvalGarbledCircuit::EvalGarbledCircuit(Circuit& circuit, bool tables) :
  GarbledCircuit(circuit, tables),
  aux_info(!tables ? std::vector<uint8_t>(TotalAuxDataSize(circuit)) : std::vector<uint8_t>()) {
}

uint8_t* EvalGarbledCircuit::GetAuxArray() {
  return aux_info.data();
}

uint8_t* EvalGarbledCircuit::inp_key_share(uint32_t idx) {  
  return inp_key_share(circuit, aux_info.data(), idx);
}

uint8_t* EvalGarbledCircuit::out_key_share(uint32_t idx) {
  return out_key_share(circuit, aux_info.data(), idx);
}

uint8_t* EvalGarbledCircuit::delta_share() {
  return delta_share(circuit, aux_info.data());
}

uint8_t* EvalGarbledCircuit::inp_soldering(uint32_t idx) {
  return inp_soldering(circuit, aux_info.data(), idx);
}

uint8_t* EvalGarbledCircuit::out_soldering(uint32_t idx) {
  return out_soldering(circuit, aux_info.data(), idx);
}

uint8_t* EvalGarbledCircuit::delta_soldering() {
  return delta_soldering(circuit, aux_info.data());
}