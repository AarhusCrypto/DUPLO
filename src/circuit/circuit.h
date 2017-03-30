#ifndef DUPLO_CIRCUIT_CIRCUIT_H_
#define DUPLO_CIRCUIT_CIRCUIT_H_

#include "duplo-util/util.h"

enum CIRCUIT_TYPE {
	COMPOSED = 0,
	BRISTOL = 1
};

static std::array<std::string, 16> gate_tables = {
	"0000", //0 (0)
	"0001", //1 (NOR)
	"0010", //2
	"0011", //3 (NOT left-wire)
	"0100", //4
	"0101", //5 (NOT right-wire)
	"0110", //6 (XOR)
	"0111", //7 (NAND)
	"1000", //8 (AND)
	"1001", //9 (NXOR)
	"1010", //10 (right ID)
	"1011", //11
	"1100", //12 (left ID)
	"1101", //13
	"1110", //14 (OR)
	"1111"  //15 (1)
};

static std::unordered_map<std::string, uint8_t> gate_tables_idx = {
	{"0000", 0}, 	//0 (0)
	{"0001", 1}, 	//1 (NOR)
	{"0010", 2}, 	//2
	{"0011", 3}, 	//3 (NOT left-wire)
	{"0100", 4}, 	//4
	{"0101", 5}, 	//5 (NOT right-wire)
	{"0110", 6}, 	//6 (XOR)
	{"0111", 7}, 	//7 (NAND)
	{"1000", 8}, 	//8 (AND)
	{"1001", 9}, 	//9 (NXOR)
	{"1010", 10}, //10 (right ID)
	{"1011", 11}, //11
	{"1100", 12}, //12 (left ID)
	{"1101", 13}, //13
	{"1110", 14}, //14 (OR)
	{"1111", 15}	//15 (1)
};

static std::array<std::array<uint8_t, 3>, 16> gate_constants_array = {
	std::array<uint8_t, 3>{255, 255, 255},  //0 (0)
	std::array<uint8_t, 3>{1, 1, 0},        //1 (NOR)
	std::array<uint8_t, 3>{1, 0, 0},        //2
	std::array<uint8_t, 3>{255, 255, 255},  //3 (NOT left-wire)
	std::array<uint8_t, 3>{0, 1, 0},        //4
	std::array<uint8_t, 3>{255, 255, 255},  //5 (NOT right-wire)
	std::array<uint8_t, 3>{255, 255, 255},  //6 (XOR)
	std::array<uint8_t, 3>{0, 0, 1},        //7 (NAND)
	std::array<uint8_t, 3>{0, 0, 0},        //8 (AND)
	std::array<uint8_t, 3>{255, 255, 255},  //9 (NXOR)
	std::array<uint8_t, 3>{255, 255, 255},  //10 (right ID)
	std::array<uint8_t, 3>{0, 1, 1},        //11
	std::array<uint8_t, 3>{255, 255, 255},  //12 (left ID)
	std::array<uint8_t, 3>{1, 0, 1},        //13
	std::array<uint8_t, 3>{1, 1, 1},        //14 (OR)
	std::array<uint8_t, 3>{255, 255, 255}   //15 (1)
};

class Gate {
public:
	Gate(std::string type) : type(type), parity(false) {
		for (int i = 0; i < 4; ++i) {
			if (type[i] == '1') {
				parity = !parity;
			}
		}
	}

	uint32_t left_wire;
	uint32_t right_wire;
	uint32_t out_wire;
	std::string type;
	bool parity;
};

class Circuit {
public:
	std::vector<Gate> gates;
	uint32_t num_wires = 0;

	uint32_t num_const_inp_wires = 0;
	uint32_t num_eval_inp_wires = 0;
	uint32_t num_inp_wires = 0;

	uint32_t const_inp_wires_start = 0;
	uint32_t const_inp_wires_stop = 0;

	int32_t eval_inp_wires_start = 0;
	int32_t eval_inp_wires_stop = 0;

	uint32_t const_out_wires_start = 0;
	uint32_t const_out_wires_stop = 0;

	int32_t eval_out_wires_start = 0;
	int32_t eval_out_wires_stop = 0;

	uint32_t num_const_out_wires = 0;
	uint32_t num_eval_out_wires = 0;
	uint32_t num_out_wires = 0;
	uint32_t out_wires_start = 0;

	uint32_t num_non_free_gates = 0;
	uint32_t num_gates = 0;

	uint32_t composed_index = 0;
	std::string circuit_name = "";
};

class ComposedCircuit {
public:

	ComposedCircuit();
	ComposedCircuit(Circuit circuit, uint32_t num_parallel_copies, uint32_t num_inputs_used = 0);
	std::vector<std::vector<uint32_t>> GetOutputIndices(bool is_const);

	std::vector<std::vector<uint32_t>> circuits_in_layer;

	std::vector<std::pair<std::string, uint32_t>> circuits; //holds unique identifier for all circuits in the ComposedCircuits, name and number. To get the actual circuit lookup name using name_to_function_num and lookup this in functions

	std::vector<Circuit> functions; //holds the circuits
	std::vector<uint32_t> num_circuit_copies; //Specifies how many of each entry in functions there are of each in total

	std::vector<uint32_t> output_circuits; //The circuits which are to be treated as output circuits, ie the output wires of these circuit will the the resulting keys of evaluation

	std::vector<std::vector<std::pair<uint32_t, std::vector<uint32_t>>>> out_wire_holders;

	std::unordered_map<uint32_t, uint32_t> out_wire_holder_to_wire_idx;
	std::unordered_map<std::string, uint32_t> name_to_function_num;

	uint32_t num_inp_wires = 0;
	uint32_t num_out_wires = 0;
	uint32_t num_layers = 0;
	uint32_t num_functions = 0;
	uint32_t num_components = 0;

	uint32_t max_wire_index = 0;

	uint32_t num_eval_inp_wires = 0;
	uint32_t num_eval_out_wires = 0;

	uint32_t num_const_inp_wires = 0;
	uint32_t num_const_out_wires = 0;

	std::string composed_circuit_name = "";

};

//read circuit from frigate format
//The wires are ordered so that the first n1 wires correspond to the first input value, the next n2 wires correspond to the second input value. The next n3 wires correspond to the output of the circuit.
//each circuit contains each function of the program
void SetCircuitOffsetIndices(Circuit& circuit);
void SetComposedCircuitOffsetIndices(ComposedCircuit& composed_circuit);
void AddOutputIdentityGates(Circuit& circuit);
void AddInputIdentityGates(ComposedCircuit& composed_circuit,
                           std::unordered_map<uint32_t, std::vector<uint32_t>>& holder_to_global_inp_wires,
                           std::unordered_map<uint32_t, std::vector<uint32_t>>& holder_to_global_out_wires,
                           std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>>& global_out_wire_to_holder,
                           std::unordered_map<uint32_t, uint32_t>& unique_non_out_wires, std::string circuits_prefix);

void ParseGates(Circuit& circuit, char raw_circuit[], CIRCUIT_TYPE circuit_type);
Circuit ParseCircuit(char raw_circuit[], CIRCUIT_TYPE circuit_type, std::string prefix = "");

void RelayerComposedCircuit(ComposedCircuit& composed_circuit);

ComposedCircuit ParseComposedCircuit(char* data, std::string circuits_prefix = "", uint32_t num_inputs_used = 0);

ComposedCircuit read_composed_circuit(const char* circuit_file, std::string circuits_prefix = "");
Circuit read_bristol_circuit(const char* circuit_file);

osuCrypto::BitVector eval_circuit(Circuit& circuit, osuCrypto::BitVector& input, bool reverse = false);
osuCrypto::BitVector eval_circuit(ComposedCircuit& composed_circuit, osuCrypto::BitVector& input);

osuCrypto::BitVector ParseBits(FILE* input_file, uint32_t num_bits_start, uint32_t num_bits, bool read_reversed);

osuCrypto::BitVector GetBitsReversed(FILE* input_file, uint32_t num_bits, uint32_t num_bits_start = 0);

osuCrypto::BitVector GetBits(FILE* input_file, uint32_t num_bits, uint32_t num_bits_start = 0);

#endif /* DUPLO_CIRCUIT_CIRCUIT_H_ */

