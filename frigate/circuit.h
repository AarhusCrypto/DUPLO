
#include <stdio.h>
#include "ast.h"
#include "types.h"
#include "variable.h"
#include "wire.h"

#include <iostream>
#include <fstream>

using namespace std;




class Gate {
public:
  uint32_t left_wire;
  uint32_t right_wire;
  uint32_t out_wire;
  string type;
};

class Circuit {
public:
  std::vector<Gate> gates;
  uint32_t num_wires=0;
  uint32_t num_inp_wires=0;
  uint32_t num_out_wires=0;
	uint32_t inp_wires_start=0;
	uint32_t out_wires_start;
	uint32_t idxCircuit;
	uint32_t num_non_free_gates = 0;
	uint32_t num_gates = 0;
	string circuit_name;
//	Circuit & operator=(const Circuit &cc)
//	{
//		num_wires = cc.num_wires;
//		num_inp_wires =  cc.num_inp_wires;
//		num_out_wires =  cc.num_out_wires;
//		inp_wires_start =  cc.inp_wires_start;
//		out_wires_start = cc.out_wires_start;
//		idxCircuit = cc.idxCircuit;
//		gates = cc.gates;
//		return *this;
//	}
};


//read circuit from frigate format
//The wires are ordered so that the first n1 wires correspond to the first input value, the next n2 wires correspond to the second input value. The next n3 wires correspond to the output of the circuit.
Circuit duploParseCircuit(char raw_circuit[]);
bool frigate_ParseComposedCircuit(char* data);

void frigate_read_text_circuit(const char* circuit_file, bool isBristolDup, bool isBristol,bool isAES, bool is_random); //each circuit contains each function of the program

Circuit sBoxYale_parse(char raw_circuit[]);
Circuit read_text_sBoxYale();

Circuit generate_random_circuit(int num_nonXOR_gate);
Circuit generate_text_RandomCircuit();
