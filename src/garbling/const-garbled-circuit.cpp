
#include "garbling/const-garbled-circuit.h"

ConstGarbledCircuit::ConstGarbledCircuit(Circuit& circuit) :
  GarbledCircuit(circuit),
  aux_info(AuxDataSize(circuit)) {
}

ConstGarbledCircuit::ConstGarbledCircuit(Circuit& circuit, bool tables) :
  GarbledCircuit(circuit, tables),
  aux_info(!tables ? std::vector<uint8_t>(AuxDataSize(circuit)) : std::vector<uint8_t>()) {
}

uint8_t* ConstGarbledCircuit::GetAuxArray() {
  return aux_info.data();
}

uint8_t* ConstGarbledCircuit::inp_key_commit0(uint32_t idx) {
  return inp_key_commit0(circuit, aux_info.data(), idx);
}

uint8_t* ConstGarbledCircuit::out_key_commit0(uint32_t idx) {
  return out_key_commit0(circuit, aux_info.data(), idx);
}

uint8_t* ConstGarbledCircuit::delta_commit0() {
  return delta_commit0(circuit, aux_info.data());
}

uint8_t* ConstGarbledCircuit::inp_key_commit1(uint32_t idx) {
  return inp_key_commit1(circuit, aux_info.data(), idx); 
}

uint8_t* ConstGarbledCircuit::out_key_commit1(uint32_t idx) {
  return out_key_commit1(circuit, aux_info.data(), idx); 
}

uint8_t* ConstGarbledCircuit::delta_commit1() {
  return delta_commit1(circuit, aux_info.data()); 
}