#include "circuit.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <iostream>
#include <string.h>
#include <algorithm>
using namespace std;
//For Frigate
vector<Circuit> circuits;
ofstream fDuplo;
ofstream fBirstolDup;
ofstream fBirstol;
ofstream fSbox;
ofstream fFuncs;
std::string dir;
ofstream fRandom;
bool isAES = false;
bool isRandom = false;
bool isBristolDup, isBristol;

template <typename T1, typename T2>
	struct less_second {
		typedef pair<T1, T2> type;
		bool operator ()(type const& a, type const& b) const {
			return a.second < b.second;
		}
	};

//Parse the gate description given a char array of the description file.
Circuit duploParseCircuit(char raw_circuit[]) {
	Circuit circuit;	
	
	raw_circuit = strchr(raw_circuit, ' ') + 1; //skip FN
	circuit.idxCircuit = (uint32_t) atoi(raw_circuit);
//	circuit.name_function = "FN" + to_string(circuit.idxCircuit);
	
	raw_circuit = strchr(raw_circuit, ' ') + 1; //#number num_inp_wires
	circuit.num_inp_wires = (uint32_t) atoi(raw_circuit);	
	circuit.inp_wires_start = 0;	
	
	raw_circuit = strchr(raw_circuit, ' ') + 1; //#number out_inp_wires
	circuit.num_out_wires = (uint32_t) atoi(raw_circuit);	
	circuit.out_wires_start = circuit.num_inp_wires;	
	
	
	raw_circuit = strchr(raw_circuit, ' ') + 1; // #number total wires
	circuit.num_wires = (uint32_t) atoi(raw_circuit);
	
	raw_circuit = strchr(raw_circuit, ' ') + 1; // #name
	
	int r1 = 0; 
	while (raw_circuit[r1] != '\n')
	{
		circuit.circuit_name.append(1, raw_circuit[r1]);
		r1++;
	}
	char type[4];
	
	raw_circuit = strchr(raw_circuit, '\n') + 1; //Skip this line

	int curr_gate_num = 0;
	uint32_t num_inputs = 0, left_wire_idx, right_wire_idx, out_wire_idx, num_child_func = 0, child_wire, num_shift = 0;
	
	while (*raw_circuit != '-') {
		if (*raw_circuit == '\n') {
			raw_circuit = strchr(raw_circuit, '\n') + 1;
			continue;
		}
		if (*raw_circuit == '+')
		{
			raw_circuit = strchr(raw_circuit, ' ') + 1; //
			raw_circuit = strchr(raw_circuit, ' ') + 1; //FN
			num_child_func = (uint32_t) atoi(raw_circuit) - 1;
			uint32_t idx = num_child_func;
			std::unordered_map<uint32_t, uint32_t > global_inp_out_wires;
			raw_circuit = strchr(raw_circuit, '\n') + 1; //skip line
			if (*raw_circuit != '+')
			{
				fDuplo << "Error!\n";
				fDuplo << "Input Function: " << circuit.idxCircuit << "\n";
				fDuplo << "Syntax: " <<  "call function " << circuits[num_child_func].idxCircuit << " in  function " << circuit.idxCircuit;
				fDuplo.close();
				exit(1);
			}
			raw_circuit = strchr(raw_circuit, ' ') + 1; //skip ++ 

			circuit.num_non_free_gates += circuits[num_child_func].num_non_free_gates;
			
			for (uint32_t i = 0; i < circuits[num_child_func].num_inp_wires; i++)
			{
				child_wire = (uint32_t) atoi(raw_circuit);
				global_inp_out_wires.emplace(i, child_wire);
				raw_circuit = strchr(raw_circuit, ' ') + 1;
			}
			raw_circuit = strchr(raw_circuit, '\n') + 1;
			//cout << "num_child_func " << num_child_func << endl;

			if (*raw_circuit != '+')
			{
				fDuplo << "Error!\n";
				fDuplo << "Output Function: " << circuit.idxCircuit << "\n";
				fDuplo << "Syntax: " <<  "call function " << circuits[num_child_func].idxCircuit << " in  function " << circuit.idxCircuit;
				fDuplo.close();
				exit(1);
			}
			raw_circuit = strchr(raw_circuit, ' ') + 1;
			for (int i = 0; i < circuits[num_child_func].num_out_wires; i++)
			{
				child_wire = (uint32_t) atoi(raw_circuit);
				global_inp_out_wires.emplace(i + circuits[num_child_func].num_inp_wires, child_wire);
				raw_circuit = strchr(raw_circuit, ' ') + 1;
			}



			int num_inp_out_wires = circuits[num_child_func].num_inp_wires + circuits[num_child_func].num_out_wires;
			for (int j =0; j < circuits[num_child_func].gates.size(); j++) //skip zero/onegate =>j=2
			{
				circuit.gates.emplace_back(Gate());
				circuit.gates[curr_gate_num].type = circuits[num_child_func].gates[j].type;
				
				//left_wire
				if (circuits[num_child_func].gates[j].left_wire < num_inp_out_wires)
					circuit.gates[curr_gate_num].left_wire = global_inp_out_wires[circuits[num_child_func].gates[j].left_wire];
				else
					circuit.gates[curr_gate_num].left_wire = circuits[num_child_func].gates[j].left_wire + circuit.num_wires;

					//right_wire
				if (circuits[num_child_func].gates[j].right_wire < num_inp_out_wires)
					circuit.gates[curr_gate_num].right_wire = global_inp_out_wires[circuits[num_child_func].gates[j].right_wire];
				else
					circuit.gates[curr_gate_num].right_wire = circuits[num_child_func].gates[j].right_wire + circuit.num_wires;
				
				if (circuits[num_child_func].gates[j].out_wire < num_inp_out_wires)
					circuit.gates[curr_gate_num].out_wire = global_inp_out_wires[circuits[num_child_func].gates[j].out_wire];
				else
					circuit.gates[curr_gate_num].out_wire = circuits[num_child_func].gates[j].out_wire + circuit.num_wires;

				++curr_gate_num;
			}
			
			if (circuits[num_child_func].num_wires > num_shift)
				num_shift = circuits[num_child_func].num_wires;
		}
		else
		{
		
				left_wire_idx = (uint32_t) atoi(raw_circuit);
				raw_circuit = strchr(raw_circuit, ' ') + 1;
				right_wire_idx = (uint32_t) atoi(raw_circuit);
				raw_circuit = strchr(raw_circuit, ' ') + 1;
				out_wire_idx = (uint32_t) atoi(raw_circuit);

				raw_circuit = strchr(raw_circuit, ' ') + 1;

			//stupid code
			std::string type_string;
			for (int ii = 0; ii < 4; ++ii) {
				if (raw_circuit[ii] == '0')
					type_string.append("0");
				else
					type_string.append("1");
			}

			if (type_string.find("0000") != std::string::npos //(0)
				||type_string.find("0011") != std::string::npos //3 - 0011 (invert passthrough a)
				||type_string.find("0101") != std::string::npos // 5 - 0101 (invert passthorugh b)
				||type_string.find("0110") != std::string::npos // 6 - 0110 (xor)
				||type_string.find("1001") != std::string::npos // 9 - 1001 (reverse xor)
				||type_string.find("1010") != std::string::npos // 10 - 1010 (passthrough b)
				||type_string.find("1100") != std::string::npos // 12 - 1100 (passthrough a)
				||type_string.find("1111") != std::string::npos // // 15 - 1111 (1)
				) {
					circuit.gates.emplace_back(Gate());
					circuit.gates[curr_gate_num].type = type_string;
					circuit.gates[curr_gate_num].left_wire = left_wire_idx;
					circuit.gates[curr_gate_num].right_wire = right_wire_idx;
					circuit.gates[curr_gate_num].out_wire = out_wire_idx;
					++curr_gate_num;										
				}
				else {
					circuit.gates.emplace_back(Gate());
					circuit.gates[curr_gate_num].type = type_string;
					circuit.gates[curr_gate_num].left_wire = left_wire_idx;
					circuit.gates[curr_gate_num].right_wire = right_wire_idx;
					circuit.gates[curr_gate_num].out_wire = out_wire_idx;
					++curr_gate_num;
					++circuit.num_non_free_gates;
				}				
				raw_circuit = strchr(raw_circuit, '\n') + 1;			
		}
	}

	circuit.num_wires += num_shift;
	circuit.num_gates = curr_gate_num;
	  
	return circuit;
}


bool frigate_ParseComposedCircuit(char raw_circuit[]) {	
	

	uint32_t num_functions = (uint32_t) atoi(raw_circuit); //get number functions 
	circuits.resize(num_functions);

	raw_circuit = strchr(raw_circuit, ' ') + 1;
	uint32_t num_layer = (uint32_t) atoi(raw_circuit); //get number functions 

	raw_circuit = strchr(raw_circuit, ' ') + 1;
	uint32_t num_component = (uint32_t) atoi(raw_circuit); //get number functions 

	
	raw_circuit = strchr(raw_circuit, '\n') + 1; //skip line
	uint32_t num_const_inp_wires = (uint32_t) atoi(raw_circuit); //get number functions 
	
	raw_circuit = strchr(raw_circuit, ' ') + 1;
	uint32_t num_eval_inp_wires = (uint32_t) atoi(raw_circuit); //get number functions 
	
	raw_circuit = strchr(raw_circuit, ' ') + 1;
	uint32_t num_const_out_wires = (uint32_t) atoi(raw_circuit); //get number functions 

	raw_circuit = strchr(raw_circuit, ' ') + 1;
	uint32_t num_eval_out_wires = (uint32_t) atoi(raw_circuit); //get number functions 


	raw_circuit = strchr(raw_circuit, '\n') + 1; //skip line
	raw_circuit = strchr(raw_circuit, '\n') + 1; //skip line	

	circuits.resize(num_functions - 1);
	
	///////////////////////////
	//read each function!!!
	///////////////////////
	for (int i = 0; i < num_functions - 1; i++)
	{
		circuits[i] = duploParseCircuit(raw_circuit);
	
		if (isAES && i == 0)	
			circuits[0] = read_text_sBoxYale();

		if (isRandom && i == 0)	
			circuits[0] = generate_text_RandomCircuit();

		raw_circuit = strchr(raw_circuit, '-') + 1; //next function
		raw_circuit = strchr(raw_circuit, '\n') + 1; //skip line
		raw_circuit = strchr(raw_circuit, '\n') + 1; //skip line
	} //done with reading the component
		

//////////
///DUPLO format
/////////	
	
	std::vector<std::string> strFunction(num_functions - 1);

	for (int i = 0; i < num_functions - 1; i++)
	{	
		for (int j = 0; j < circuits[i].gates.size(); j++)
		{
		
				strFunction[i].append(to_string(circuits[i].gates[j].left_wire) + " "
						  + to_string(circuits[i].gates[j].right_wire) + " "
						   + to_string(circuits[i].gates[j].out_wire) + " "
					+ circuits[i].gates[j].type + "\n");
			
			
		}	
	}


	raw_circuit = strchr(raw_circuit, '\n') + 1;
	std::istringstream type_string(raw_circuit);
	string line;

	int pos;
	vector <std::tuple<int, string, string>> functions_duplo;
	std::map<uint32_t, uint32_t> real_functions;
	int num_function;
	string input_func, output_func;
	int idx_func=1;

	bool isFunctionWrap = true;
	while (getline(type_string, line) && !type_string.eof() && isFunctionWrap) {
		
		if (!line.empty()) //FN 2
		{
			if (line.at(0) == 'F')
			{
				pos = line.find("\n"); //out_wire
				line.erase(0, 3); // 'FN '
				num_function = std::stoi(line.substr(0, pos)); //2		
			
				getline(type_string, input_func);
				getline(type_string, output_func);	

				if (real_functions.find(num_function) == real_functions.end())
				{
					real_functions.emplace(num_function, idx_func);
					idx_func++;
				}
				functions_duplo.push_back(std::make_tuple(real_functions[num_function], input_func, output_func));	
			}
			else
			{
				isFunctionWrap = false;
				
			}
		}
	}

	
	
		fDuplo << real_functions.size()  << " " << functions_duplo.size() << "\n";//" // #numberfunction   #numberComponent\n";
		fDuplo << num_const_inp_wires << " " << num_eval_inp_wires << " " << num_const_out_wires << " " << num_const_out_wires << " " << num_eval_out_wires << "\n\n";// " //#input_eval #input_const #output_eval #output_const\n\n";	

		int id = 1;

			//sort by value
		vector<pair<uint32_t, uint32_t> > mapcopy(real_functions.begin(), real_functions.end());
		sort(mapcopy.begin(), mapcopy.end(), less_second<uint32_t, uint32_t>());
	
		for (auto it = mapcopy.begin(); it != mapcopy.end(); ++it)
		{
		
			//std::cout << " " << it->first << ":" << it->second;
			string head_func;

			head_func.append("FN " + to_string(it->second)  + " "  
						+ to_string(circuits[it->first - 1].num_inp_wires) + " " 
						+ to_string(circuits[it->first - 1].num_out_wires) + " " 
						+ to_string(circuits[it->first - 1].num_wires) + " " 
						+ to_string(circuits[it->first - 1].num_non_free_gates) + " " 
						+ to_string(circuits[it->first - 1].num_gates) + " " 
						+ "\n"); //# FN id num_inp_wires num_out_wires num_wires \n";
		
			fDuplo << head_func;
			fDuplo << strFunction[it->first - 1];
			fDuplo << "--end FN " << it->second << " -- \n\n";

			auto aa = to_string(it->second);		
			fFuncs.open(dir +"."+  circuits[it->first - 1].circuit_name);
			//fFuncs << circuits[it->first - 1].circuit_name << " " << circuits[it->first - 1].num_non_free_gates <<  endl;
			fFuncs << head_func;
			fFuncs << strFunction[it->first - 1];
			fFuncs.close();
			cout << circuits[it->first - 1].circuit_name << ".close()\n";
			id++;
		}

	if (isFunctionWrap)
	{
		fDuplo << "FN " << real_functions.size() + 1 << "\n\n"; 
	
		for (int i = 0; i <  functions_duplo.size(); i++)
		{
			fDuplo << "FN " << std::get<0>(functions_duplo[i]) << "\n";
			fDuplo << std::get<1>(functions_duplo[i]) << "\n";
			fDuplo << std::get<2>(functions_duplo[i]) << "\n\n";
		}
	
		fDuplo.close(); 
	}
	else
	{
		fDuplo.close();
		fDuplo.open(dir + "_duplo" ,std::ios::out | std::ios::trunc);

		fDuplo << "Error!\n";
		fDuplo << ".wir have a wrong format!\n";
		fDuplo << "fix: refactoring main function that contains ONLY function calls\n";
		cout << "fDuplo.close()\n";
		cout << "Error: .wir have a wrong format!\n";
		cout << "fix: refactoring main function that contains ONLY function calls\n";
		fDuplo.close();
		return false;
	}
	

//////////
///Bristol format => dont work for gate 2, 4...
/////////	

	if (isBristolDup || isBristol)
	{
			
		Circuit main_circuit;
		raw_circuit = strchr(raw_circuit, '\n') + 1; //FN main
		vector <std::pair<int, std::unordered_map<int, int>>> functions;

		uint32_t num_child_func = 0, child_wire, curr_gate_num, one_gate, zero_gate, num_shift = 0, num_non_free_gates = 0, max_wire = 0;
		curr_gate_num = 0;

		while (*raw_circuit != EOF) {
			if (*raw_circuit == '\n') {
				raw_circuit = strchr(raw_circuit, '\n') + 1;
				continue;
			}

			if (*raw_circuit == 'F')
			{
				raw_circuit = strchr(raw_circuit, ' ') + 1; //
				num_child_func = (uint32_t) atoi(raw_circuit) - 1;
				raw_circuit = strchr(raw_circuit, '\n') + 1; //skip line
				//cout << num_child_func << "\n";
				std::unordered_map<int, int> global_inp_out_wires;
				for (uint32_t i = 0; i < circuits[num_child_func].num_inp_wires; i++)
				{
					child_wire = (uint32_t) atoi(raw_circuit);
		//	cout << child_wire << " ";
					global_inp_out_wires.emplace(i, child_wire);
					if (child_wire > max_wire)
						max_wire = child_wire;
					raw_circuit = strchr(raw_circuit, ' ') + 1;
				}
		//	cout << "\n ";
				for (int i = 0; i < circuits[num_child_func].num_out_wires; i++)
				{
					child_wire = (uint32_t) atoi(raw_circuit);
					//cout << child_wire << " ";
					global_inp_out_wires.emplace(i + circuits[num_child_func].num_inp_wires, child_wire);
					if (child_wire > max_wire)
						max_wire = child_wire;
					raw_circuit = strchr(raw_circuit, ' ') + 1;
				}
				//cout << "\n ";
				functions.push_back(std::make_pair(num_child_func, global_inp_out_wires));
			}
		}

		main_circuit.num_wires = max_wire + 1;

		for (int i = 0; i < functions.size(); i++)
		{		

			int num_inp_out_wires = circuits[std::get<0>(functions[i])].num_inp_wires + circuits[std::get<0>(functions[i])].num_out_wires;
			for (int j = 0; j < circuits[std::get<0>(functions[i])].gates.size(); j++)
			{
				main_circuit.gates.emplace_back(Gate());

// 7 - 0111 (nand)
// 8 - 1000 (and)
// 14 - 1110 (or)
// 1 - 0001 (nor)
				
				if (circuits[std::get<0>(functions[i])].gates[j].type == "0111" || 
					circuits[std::get<0>(functions[i])].gates[j].type == "1000" ||
					circuits[std::get<0>(functions[i])].gates[j].type == "1110" ||
					circuits[std::get<0>(functions[i])].gates[j].type == "0001")
				{
					num_non_free_gates++;
				}
				
				main_circuit.gates[curr_gate_num].type = circuits[std::get<0>(functions[i])].gates[j].type;
				//left_wire
				if (circuits[std::get<0>(functions[i])].gates[j].left_wire < num_inp_out_wires)
					main_circuit.gates[curr_gate_num].left_wire = std::get<1>(functions[i])[circuits[std::get<0>(functions[i])].gates[j].left_wire];
				else
					main_circuit.gates[curr_gate_num].left_wire = circuits[std::get<0>(functions[i])].gates[j].left_wire + main_circuit.num_wires;
			
		//right_wire
				if (circuits[std::get<0>(functions[i])].gates[j].right_wire < num_inp_out_wires)
					main_circuit.gates[curr_gate_num].right_wire = std::get<1>(functions[i])[circuits[std::get<0>(functions[i])].gates[j].right_wire];
				else
					main_circuit.gates[curr_gate_num].right_wire = circuits[std::get<0>(functions[i])].gates[j].right_wire + main_circuit.num_wires;

				if (circuits[std::get<0>(functions[i])].gates[j].out_wire < num_inp_out_wires)
					main_circuit.gates[curr_gate_num].out_wire = std::get<1>(functions[i])[circuits[std::get<0>(functions[i])].gates[j].out_wire];
				else
					main_circuit.gates[curr_gate_num].out_wire = circuits[std::get<0>(functions[i])].gates[j].out_wire + main_circuit.num_wires;
				++curr_gate_num;
			}
			if (circuits[std::get<0>(functions[i])].num_wires > num_shift)
				num_shift = circuits[std::get<0>(functions[i])].num_wires;
						
		}
		main_circuit.num_wires = main_circuit.num_wires + num_shift;

		//////////
		///Bristol format for duplo test
		/////////	
		if (isBristolDup)
		{
			
			fBirstolDup.open(dir + "_bristol");

			fBirstolDup << main_circuit.gates.size() << " " << main_circuit.num_wires << " " 
												  << num_eval_inp_wires + num_const_inp_wires
												  << " //#gates, #wires, #out_wires_start // " 
												  << "# num_non_free_gates  = "  << num_non_free_gates << " " << " \n"; //#gate #wires
			fBirstolDup << num_const_inp_wires << " " << num_eval_inp_wires 
											<< " " << num_const_out_wires 
											<< " " << num_const_out_wires  
											 << " " << num_eval_out_wires 
											 << "  //#const_inputs #eval_inputs #total_outputs* #const_outputs #eval_outputs\n\n";	 //#input_eval #input_const #output_eval


	
			for (int j = 0; j < main_circuit.gates.size(); j++)
			{
				if (main_circuit.gates[j].type == "NOT")
				{
					fBirstolDup << "1 1 "
					<< main_circuit.gates[j].left_wire << " "
							   << main_circuit.gates[j].out_wire << " "
				<< main_circuit.gates[j].type << "\n";
				}
				else
				{
					fBirstolDup << "2 1 "
							<< main_circuit.gates[j].left_wire << " "
							   << main_circuit.gates[j].right_wire << " "
							   << main_circuit.gates[j].out_wire << " "
				<< main_circuit.gates[j].type << "\n";
				}
			
			}
			fBirstolDup.close();
			cout << "fBirstolDup.close()\n";

		 }
		 //////////
		///Bristol format for Lego+RR16
		/////////	
		if (isBristol)
		{			
		
			string strBristol = "";
			int num_gate_bristol = main_circuit.gates.size();
			int num_wire_bristol = main_circuit.num_wires;

			for (int j = 0; j < main_circuit.gates.size(); j++)
			{
				if (main_circuit.gates[j].type == "NOT")
				{
					strBristol += "1 1 "
					+ to_string(main_circuit.gates[j].left_wire) + " "
					+ to_string(main_circuit.gates[j].out_wire) + " INV\n";
				}
				else if (main_circuit.gates[j].type == "NXOR")
				{
					strBristol += "2 1 "
					+ to_string(main_circuit.gates[j].left_wire) + " "
					+ to_string(main_circuit.gates[j].right_wire) + " "
					+ to_string(num_wire_bristol) + " XOR\n";

					strBristol += "1 1 "
					+ to_string(num_wire_bristol) + " "
					+ to_string(main_circuit.gates[j].out_wire) + " INV\n";

					num_gate_bristol++;
					num_wire_bristol++;
				}
				else
				{
					strBristol += "2 1 "
							+ to_string(main_circuit.gates[j].left_wire) + " "
							 + to_string(main_circuit.gates[j].right_wire) + " "
							 + to_string(main_circuit.gates[j].out_wire) + " "
		+ main_circuit.gates[j].type + "\n";
				}
			
			}
			
			//add zero gate
			int zero_wire = num_wire_bristol;
				strBristol += "2 1 "
				+ to_string(0) + " "
				+ to_string(0) + " "
				+ to_string(zero_wire) + " XOR\n";
			
				num_gate_bristol++;
				num_wire_bristol++;
			
			cout << num_eval_out_wires << "\n";
			cout << num_const_out_wires << "\n";
			int start_out_wire = num_const_inp_wires + num_eval_inp_wires;
			cout << start_out_wire << "\n";
			for (int j = 0; j < num_const_out_wires; j++) //shift output to end
			{
				strBristol += "2 1 "
				+ to_string(start_out_wire + j) + " "
				+ to_string(zero_wire) + " "
				+ to_string(num_wire_bristol) + " XOR\n";
			
				num_gate_bristol++;
				num_wire_bristol++;
			}
			
			fBirstol.open(dir + "_bristolXORAND");

			fBirstol << num_gate_bristol << " " 
			<< num_wire_bristol << " \n"; 

			fBirstol << num_const_inp_wires << " " 
					<< num_eval_inp_wires << " " 
					  << num_const_out_wires << "\n\n";	 //#input_eval #input_const #output_eval
			fBirstol << strBristol;
			fBirstol.close();
			cout << "fBirstol.close()\n";

		}

	}
	return true;

}

#include <stdio.h>
void frigate_read_text_circuit(const char* circuit_file, bool isBDup, bool isB, bool is_AES, bool is_random)
{
	isBristol = isB;
	isBristolDup = isBDup;

	FILE* file;
	size_t file_size;
	file = fopen(circuit_file, "r");

	std::string str(circuit_file);
	dir = str;

	isAES = is_AES;
	isRandom = is_random;

	fDuplo.open(str + "_duplo");
	
	if (file == NULL) {
		printf("ERROR: Could not open text circuit: %s\n", circuit_file);
		exit(EXIT_FAILURE);
	}
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	rewind(file);

	std::unique_ptr<char[]> data(new char[file_size + 1]);
	size_t size = fread(data.get(), 1, file_size, file);
	if (size != file_size) {
		printf("ERROR while loading file from frigate: %s\n", circuit_file);
		exit(EXIT_FAILURE);
	}
	data[file_size] = EOF;
	if (ferror(file)) {
		printf("ERROR: fread() error\n");
		exit(EXIT_FAILURE);
	}
	fclose(file);
	bool isDuploFormat=frigate_ParseComposedCircuit(data.get());
	//if (!isDuploFormat)
	remove(circuit_file);
	
}

void print_wires(std::unordered_map<string, uint32_t> wires)
{
	for (const auto& i : wires) {
		std::cout << "Key:[" << i.first << "] Value:[" << i.second << "]\n";
	}
}
//Sbox
Circuit sBoxYale_parse(char raw_circuit[]) { 
	Circuit sBox;
	std::unordered_map<string, uint32_t> wires;
	string line, left_wire, right_wire, out_wire, gate;
	int pos, pos_wire, curr_wire_num = 0, curr_gate_num = 0;
	std::string delimiter = "\n";

	sBox.num_inp_wires = 8;
	sBox.num_out_wires = 8;
	sBox.inp_wires_start = 0;
	sBox.out_wires_start = 8;

	for (int i = 0; i < sBox.num_inp_wires; i++)
	{
		wires.emplace("x" + to_string(i), 7-i); //input wires
		wires.emplace("s" + to_string(i), 7-i + 8); //output wires
		curr_wire_num += 2;
	}

	//print_wires(wires);
	
	raw_circuit = strchr(raw_circuit, '\n') + 1; // Jan 18 +  09
	raw_circuit = strchr(raw_circuit, '\n') + 1; // Straight-line program for AES sbox 
	raw_circuit = strchr(raw_circuit, '\n') + 1; // Joan Boyar and Rene Peralta
	raw_circuit = strchr(raw_circuit, '\n') + 1;
	raw_circuit = strchr(raw_circuit, '\n') + 1; // input is X0 + ..,X7  
	raw_circuit = strchr(raw_circuit, '\n') + 1; //output is S0 + ...,S7
	raw_circuit = strchr(raw_circuit, '\n') + 1;// arithmetic is over GF2
	raw_circuit = strchr(raw_circuit, '\n') + 1;
	raw_circuit = strchr(raw_circuit, '\n') + 1;// begin top linear transformation 
	
	std::istringstream type_string(raw_circuit);
	int cnt_gate = 0;
	while (curr_gate_num != 115 && !type_string.eof() && getline(type_string, line)) {
		if (!line.empty() && line.at(0) == ' ')
		{
			//cout << "\n" << line << "\n";//y14 = x3 + x5;
			//line.erase(0,  2); // remove " " at the first line
			line.erase(0, line.find_first_not_of(' '));  
			if (line.at(0) != '/')
			{
				pos_wire = line.find(" "); //out_wire
				out_wire = line.substr(0, pos_wire); //y14
				//cout << out_wire << " ";
				if (wires.find(out_wire) == wires.end()) //check whether wires exits
				{
					wires.emplace(out_wire, curr_wire_num); //sBox.num_wires start from 1
					++curr_wire_num;
				}
			
				line.erase(0, pos_wire + 3); // ' = '
			
				pos_wire = line.find(" "); //right_wire
				right_wire = line.substr(0, pos_wire); //x3
				//cout << right_wire << " ";

				if (wires.find(right_wire) == wires.end()) //check whether wires exits
				{
					wires.emplace(right_wire, curr_wire_num); //sBox.num_wires start from 1
					++curr_wire_num;
				}
				line.erase(0, pos_wire + 1);

				pos_wire = line.find(" "); //gate
				gate = line.substr(0, pos_wire); //+
				//cout << gate << " ";

				if (gate == "+") 
					gate = "0110"; // 6 - 0110 (xor)
				else if (gate == "X")
				{
					gate = "1000";// 8 - 1000 (and)
					sBox.num_non_free_gates++;
				}
				else if (gate == "XNOR") // 9 - 1001 (reverse xor)
					gate = "1001";
				else
					exit(1);

				line.erase(0, pos_wire + 1);

				pos_wire = line.find(";"); //left_wire
				left_wire = line.substr(0, pos_wire); //x5
				//cout << left_wire << " ";

				if (wires.find(left_wire) == wires.end()) //check whether wires exits
				{
					wires.emplace(left_wire, curr_wire_num); //sBox.num_wires start from 1
					++curr_wire_num;
				}

				sBox.gates.emplace_back(Gate());
				sBox.gates[curr_gate_num].type = gate;
				sBox.gates[curr_gate_num].left_wire =  wires[left_wire];
				sBox.gates[curr_gate_num].right_wire =  wires[right_wire];
				sBox.gates[curr_gate_num].out_wire = wires[out_wire];
				++curr_gate_num;
			}
		}	
	}
	sBox.num_wires = curr_wire_num;
	
	fSbox << "FN " <<  sBox.gates.size() << " " << sBox.num_wires << " //#gate #wires \n";
	for (int i = 0; i < sBox.gates.size(); i++)
	{
		fSbox << sBox.gates[i].left_wire  << " " << sBox.gates[i].right_wire  << " "
						 << sBox.gates[i].out_wire << " " << sBox.gates[i].type << "\n"; 
	}
	return sBox;
}

//Sbox
Circuit sBoxYale113_parse(char raw_circuit[]) { 
	Circuit sBox;
	std::unordered_map<string, uint32_t> wires;
	string line, left_wire, right_wire, out_wire, gate;
	int pos, pos_wire, curr_wire_num = 0, curr_gate_num = 0;
	std::string delimiter = "\n";

	sBox.num_inp_wires = 8;
	sBox.num_out_wires = 8;
	sBox.inp_wires_start = 0;
	sBox.out_wires_start = 8;
	sBox.circuit_name = "fAES";

	for (int i = 0; i < sBox.num_inp_wires; i++)
	{
		wires.emplace("U" + to_string(i), 7 - i); //input wires
		wires.emplace("S" + to_string(i), 7 - i + 8); //output wires
		curr_wire_num += 2;
	}

	//print_wires(wires);
	
	raw_circuit = strchr(raw_circuit, '\n') + 1; // 113 gates
	raw_circuit = strchr(raw_circuit, '\n') + 1; // 8 inputs 
	raw_circuit = strchr(raw_circuit, '\n') + 1; // U0 U1 U2 U3 U4 U5 U6 U7 
	raw_circuit = strchr(raw_circuit, '\n') + 1; //8 outputs
	raw_circuit = strchr(raw_circuit, '\n') + 1; // S3 S7 S0 S6 S4 S1 S2 S5 
	raw_circuit = strchr(raw_circuit, '\n') + 1; //begin
	
	std::istringstream type_string(raw_circuit);
	int cnt_gate = 0;
	while (curr_gate_num != 113 && !type_string.eof() && getline(type_string, line)) {
		if (!line.empty())
		{
			if (line.at(0) != '/')
			{
				pos_wire = line.find(" "); //out_wire
				out_wire = line.substr(0, pos_wire); //y14
				//cout << out_wire << " ";
				if (wires.find(out_wire) == wires.end()) //check whether wires exits
				{
					wires.emplace(out_wire, curr_wire_num); //sBox.num_wires start from 1
					++curr_wire_num;
				}
			
				line.erase(0, pos_wire + 3); // ' = '
			
				pos_wire = line.find(" "); //right_wire
				right_wire = line.substr(0, pos_wire); //x3
				//cout << right_wire << " ";

				if (wires.find(right_wire) == wires.end()) //check whether wires exits
				{
					wires.emplace(right_wire, curr_wire_num); //sBox.num_wires start from 1
					++curr_wire_num;
				}
				line.erase(0, pos_wire + 1);

				pos_wire = line.find(" "); //gate
				gate = line.substr(0, pos_wire); //+
				//cout << gate << " ";

				if (gate == "+") // 6 - 0110 (xor)
					gate = "0110";
				else if (gate == "x")// 8 - 1000 (and)
				{
					gate = "1000";
					sBox.num_non_free_gates++;
				}
				else if (gate == "#") // 9 - 1001 (reverse xor)
					gate = "1001";
				else
					exit(1);

				line.erase(0, pos_wire + 1);

				pos_wire = line.find(";"); //left_wire
				left_wire = line.substr(0, pos_wire); //x5
				//cout << left_wire << " ";

				if (wires.find(left_wire) == wires.end()) //check whether wires exits
				{
					wires.emplace(left_wire, curr_wire_num); //sBox.num_wires start from 1
					++curr_wire_num;
				}

				sBox.gates.emplace_back(Gate());
				sBox.gates[curr_gate_num].type = gate;
				sBox.gates[curr_gate_num].left_wire =  wires[left_wire];
				sBox.gates[curr_gate_num].right_wire =  wires[right_wire];
				sBox.gates[curr_gate_num].out_wire = wires[out_wire];
				++curr_gate_num;
			}
		}	
	}
	sBox.num_wires = curr_wire_num;
	
	fSbox << "FN " <<  sBox.gates.size() << " " << sBox.num_wires << " //#gate #wires \n";
	for (int i = 0; i < sBox.gates.size(); i++)
	{
		fSbox <<  sBox.gates[i].left_wire  << " " << sBox.gates[i].right_wire  << " "
						 << sBox.gates[i].out_wire << " " << sBox.gates[i].type << "\n"; 
	}
	return sBox;
}

Circuit read_text_sBoxYale()
{
	FILE* file;
	size_t file_size;
	const char* circuit_file = "tests/duplo/SLP_AES_113.txt";
	//const char* circuit_file = "tests/dp/AES_Sbox.txt";
	file = fopen(circuit_file, "r");

	std::string str(circuit_file);

	
	fSbox.open(str + "GC");
	if (file == NULL) {
		printf("ERROR: Could not open text circuit: %s\n", circuit_file);
		exit(EXIT_FAILURE);
	}
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	rewind(file);

	std::unique_ptr<char[]> data(new char[file_size + 1]);
	size_t size = fread(data.get(), 1, file_size, file);
	if (size != file_size) {
		printf("ERROR while loading file from frigate: %s\n", circuit_file);
		exit(EXIT_FAILURE);
	}
	data[file_size] = EOF;
	if (ferror(file)) {
		printf("ERROR: fread() error\n");
		exit(EXIT_FAILURE);
	}
	fclose(file);
	Circuit sBox = sBoxYale113_parse(data.get());
	fSbox.close();
	return sBox;
}

Circuit generate_text_RandomCircuit()
{

	fRandom.open("tests/dp/random.GC");
	Circuit fcircuit = generate_random_circuit(128);
	fRandom.close();
	return fcircuit;
}


Circuit generate_random_circuit(int num_nonXOR_gate)
{

	Circuit cRandom;
	string left_wire, right_wire, out_wire, gate;
	int pos, pos_wire, curr_wire_num = 0, curr_gate_num = 0;
	std::string delimiter = "\n";

	std::vector<int> wires;

	cRandom.num_inp_wires = 256;
	cRandom.num_out_wires = 128;
	cRandom.inp_wires_start = 0;
	cRandom.out_wires_start = 256;
	cRandom.circuit_name = "fRandom";
	int rand_out_wire, idx_rnd_inp_wire1, idx_rnd_inp_wire2, rand_type_gate;
	int max_wire = 256;
	cRandom.num_wires = max_wire + 256 + 128;
	int num_input_wires1 = cRandom.num_inp_wires / 2;
	curr_wire_num = 256 + 128;

	for (int i = 0; i < num_input_wires1; i++) //input
	{
		
		rand_type_gate = rand() % 2;
		cRandom.gates.emplace_back(Gate());
		
		if (rand_type_gate == 0)
			cRandom.gates[curr_gate_num].type = "0110";// 6 - 0110 (xor)
		else
		{
			cRandom.gates[curr_gate_num].type = "1000";// 8 - 1000 (and)
			cRandom.num_non_free_gates++;
		}
		cRandom.gates[curr_gate_num].left_wire =  i;
		cRandom.gates[curr_gate_num].right_wire =  num_input_wires1 + i;
		cRandom.gates[curr_gate_num].out_wire = curr_wire_num;
		
		wires.push_back(curr_wire_num);
		wires.push_back(i);
		wires.push_back(num_input_wires1 + i);

		++curr_gate_num;
		curr_wire_num++;
	}

	while (cRandom.num_non_free_gates != num_nonXOR_gate) {
		idx_rnd_inp_wire1 = rand() % wires.size();
		idx_rnd_inp_wire2 = rand() % wires.size();
		rand_out_wire = rand() % max_wire + 256 + 128;

		if (rand_out_wire > curr_wire_num)
		{
			rand_out_wire = curr_wire_num;
			curr_wire_num++;
		}
		rand_type_gate = rand() % 2;
		cRandom.gates.emplace_back(Gate());

		if (rand_type_gate == 0)
			cRandom.gates[curr_gate_num].type = "0110";// 6 - 0110 (xor)
		else
		{
			cRandom.gates[curr_gate_num].type = "1000";// 8 - 1000 (and)
			cRandom.num_non_free_gates++;
		}		
		
		cRandom.gates[curr_gate_num].left_wire =  wires[idx_rnd_inp_wire1];
		cRandom.gates[curr_gate_num].right_wire = wires[idx_rnd_inp_wire2];
		cRandom.gates[curr_gate_num].out_wire = rand_out_wire;
		++curr_gate_num;
		wires.push_back(rand_out_wire);
	}

	for (int i = 0; i < cRandom.num_out_wires; i++) //output
	{
		idx_rnd_inp_wire1 = rand() % wires.size();
		idx_rnd_inp_wire2 = rand() % wires.size();
		
		cRandom.gates.emplace_back(Gate());
		cRandom.gates[curr_gate_num].type = "0110";// 6 - 0110 (xor)
		
		cRandom.gates[curr_gate_num].left_wire =  wires[idx_rnd_inp_wire1];
		cRandom.gates[curr_gate_num].right_wire = wires[idx_rnd_inp_wire2];
		cRandom.gates[curr_gate_num].out_wire = cRandom.out_wires_start + i;
		++curr_gate_num;
	}
cRandom.num_gates = curr_gate_num; 
	fRandom << "FN " <<  cRandom.gates.size() << " " << cRandom.num_wires << " //#gate #wires \n";
	for (int i = 0; i < cRandom.gates.size(); i++)
	{
		fRandom << cRandom.gates[i].left_wire  << " " << cRandom.gates[i].right_wire  << " "
						 << cRandom.gates[i].out_wire << " " << cRandom.gates[i].type << "\n"; 
	}
	return cRandom;
}