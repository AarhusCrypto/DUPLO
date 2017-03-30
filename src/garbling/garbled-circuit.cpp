#include "garbling/garbled-circuit.h"

GarbledCircuit::GarbledCircuit(Circuit& circuit) :
  garbled_tables(NumTables(circuit), CSEC_BYTES),
  circuit(circuit),
  size(garbled_tables.size()) {
}

GarbledCircuit::GarbledCircuit(Circuit& circuit, bool tables) :
  circuit(circuit),
  garbled_tables(tables ? BYTEArrayVector(NumTables(circuit), CSEC_BYTES) : BYTEArrayVector()),
  size(tables ? TotalTableSize(circuit): 0) {
}

uint8_t* GarbledCircuit::T_G(uint32_t idx) {
  return T_G(garbled_tables.data(), circuit, idx);
}

uint8_t* GarbledCircuit::T_E(uint32_t idx) {
  return T_E(garbled_tables.data(), circuit, idx);
}

uint8_t* GarbledCircuit::GetTables() {
  return garbled_tables.data();
}