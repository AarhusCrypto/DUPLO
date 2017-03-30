#include "circuit/circuit.h"


//The input and output indices should be considered offsets to 0 and circuit.out_wires_start, respectively
void SetCircuitOffsetIndices(Circuit& circuit) {

  //Set Inputs
  circuit.num_inp_wires = circuit.num_const_inp_wires + circuit.num_eval_inp_wires;

  circuit.const_inp_wires_start = 0;
  circuit.const_inp_wires_stop = circuit.const_inp_wires_start + circuit.num_const_inp_wires;

  circuit.eval_inp_wires_start = circuit.const_inp_wires_stop;
  circuit.eval_inp_wires_stop = circuit.eval_inp_wires_start + circuit.num_eval_inp_wires;

  //Set Outputs
  circuit.const_out_wires_start = 0;
  circuit.const_out_wires_stop = circuit.const_out_wires_start + circuit.num_const_out_wires;

  circuit.eval_out_wires_start =  circuit.num_out_wires - circuit.num_eval_out_wires;
  circuit.eval_out_wires_stop =  circuit.eval_out_wires_start + circuit.num_eval_out_wires;
}

void SetComposedCircuitOffsetIndices(ComposedCircuit& composed_circuit) {

  composed_circuit.num_inp_wires = composed_circuit.num_eval_inp_wires + composed_circuit.num_const_inp_wires;
  composed_circuit.num_out_wires = std::max(composed_circuit.num_const_out_wires, composed_circuit.num_eval_out_wires);
}

void AddOutputIdentityGates(Circuit& circuit) {
  //Add identity AND-gates to all output wires. This is to simplify DUPLO evaluation as now it is easy to identify which output wire needs to be leaked for the evalutator to decode the output.
  uint32_t org_num_wires = circuit.num_wires;
  uint32_t org_out_start = circuit.out_wires_start;
  circuit.out_wires_start = org_num_wires;
  for (uint32_t i = 0; i < circuit.num_out_wires; ++i) {
    circuit.gates.emplace_back(Gate(gate_tables[8])); //1000
    circuit.gates[circuit.num_gates].left_wire = org_out_start + i;
    circuit.gates[circuit.num_gates].right_wire = org_out_start + i;
    circuit.gates[circuit.num_gates].out_wire = org_num_wires + i;

    ++circuit.num_gates;
    ++circuit.num_non_free_gates;
    ++circuit.num_wires;
  }
}

void ParseGates(Circuit& circuit, char raw_circuit[], CIRCUIT_TYPE circuit_type) {
  uint32_t left_wire_idx, right_wire_idx, out_wire_idx;

  char terminate_char;
  if (circuit_type == COMPOSED) {
    terminate_char = '-';
  } else if (circuit_type == BRISTOL) {
    terminate_char = EOF;
  }

  while (*raw_circuit != terminate_char) {
    if (*raw_circuit == '\n') {
      raw_circuit = strchr(raw_circuit, '\n') + 1;
      continue;
    }
    if (circuit_type == COMPOSED) {
      left_wire_idx = (uint32_t) atoi(raw_circuit);

      raw_circuit = strchr(raw_circuit,  ' ') + 1;
      right_wire_idx = (uint32_t) atoi(raw_circuit);

      raw_circuit = strchr(raw_circuit,  ' ') + 1;
      out_wire_idx = (uint32_t) atoi(raw_circuit);

      raw_circuit = strchr(raw_circuit,  ' ') + 1;
      circuit.gates.emplace_back(Gate(std::string(raw_circuit, 4)));
      circuit.gates[circuit.num_gates].left_wire = left_wire_idx;
      circuit.gates[circuit.num_gates].right_wire = right_wire_idx;
      circuit.gates[circuit.num_gates].out_wire = out_wire_idx;

      if (circuit.gates[circuit.num_gates].parity) {
        ++circuit.num_non_free_gates;
      }
      ++circuit.num_gates;

      raw_circuit = strchr(raw_circuit,  '\n') + 1;
    } else if (circuit_type == BRISTOL) {
      
      char type[4];

      uint32_t num_inputs = (uint32_t) atoi(raw_circuit);
      raw_circuit = strchr(raw_circuit,  ' ') + 1;
      raw_circuit = strchr(raw_circuit,  ' ') + 1; //We skip num_output wires as they all have 1.

      if (num_inputs == 1) {
        left_wire_idx = (uint32_t) atoi(raw_circuit);
        raw_circuit = strchr(raw_circuit,  ' ') + 1;
        out_wire_idx = (uint32_t) atoi(raw_circuit);
        raw_circuit = strchr(raw_circuit,  ' ') + 1;
        raw_circuit = strchr(raw_circuit,  '\n') + 1;
        circuit.gates.emplace_back(Gate(gate_tables[3]));
        circuit.gates[circuit.num_gates].left_wire = left_wire_idx;
        circuit.gates[circuit.num_gates].right_wire = 0;
        circuit.gates[circuit.num_gates].out_wire = out_wire_idx;

        ++circuit.num_gates;
      } else {
        left_wire_idx = (uint32_t) atoi(raw_circuit);
        raw_circuit = strchr(raw_circuit,  ' ') + 1;
        right_wire_idx = (uint32_t) atoi(raw_circuit);
        raw_circuit = strchr(raw_circuit,  ' ') + 1;
        out_wire_idx = (uint32_t) atoi(raw_circuit);

        raw_circuit = strchr(raw_circuit,  ' ') + 1;

        memcpy(type, raw_circuit, 4 * sizeof(char));
        std::string type_string(type);
        raw_circuit = strchr(raw_circuit,  '\n') + 1;

        //Do not change the order of these if statements, they matter as eg. AND is substring of NAND
        if (type_string.find("NAND") != std::string::npos) {
          circuit.gates.emplace_back(Gate(gate_tables[7]));
          circuit.gates[circuit.num_gates].left_wire = left_wire_idx;
          circuit.gates[circuit.num_gates].right_wire = right_wire_idx;
          circuit.gates[circuit.num_gates].out_wire = out_wire_idx;

          ++circuit.num_gates;
          ++circuit.num_non_free_gates;
        } else if (type_string.find("AND") != std::string::npos) {
          circuit.gates.emplace_back(gate_tables[8]);
          circuit.gates[circuit.num_gates].left_wire = left_wire_idx;
          circuit.gates[circuit.num_gates].right_wire = right_wire_idx;
          circuit.gates[circuit.num_gates].out_wire = out_wire_idx;

          ++circuit.num_gates;
          ++circuit.num_non_free_gates;
        } else if ((type_string.find("NXOR") != std::string::npos) ||
                   (type_string.find("XNOR") != std::string::npos)) {
          circuit.gates.emplace_back(Gate(gate_tables[9]));
          circuit.gates[circuit.num_gates].left_wire = left_wire_idx;
          circuit.gates[circuit.num_gates].right_wire = right_wire_idx;
          circuit.gates[circuit.num_gates].out_wire = out_wire_idx;

          ++circuit.num_gates;
        } else if (type_string.find("XOR") != std::string::npos) {
          circuit.gates.emplace_back(Gate(gate_tables[6]));
          circuit.gates[circuit.num_gates].left_wire = left_wire_idx;
          circuit.gates[circuit.num_gates].right_wire = right_wire_idx;
          circuit.gates[circuit.num_gates].out_wire = out_wire_idx;

          ++circuit.num_gates;
        } else if (type_string.find("NOR") != std::string::npos) {
          circuit.gates.emplace_back(Gate(gate_tables[1]));
          circuit.gates[circuit.num_gates].left_wire = left_wire_idx;
          circuit.gates[circuit.num_gates].right_wire = right_wire_idx;
          circuit.gates[circuit.num_gates].out_wire = out_wire_idx;

          ++circuit.num_gates;
          ++circuit.num_non_free_gates;
        } else if (type_string.find("OR") != std::string::npos) {
          circuit.gates.emplace_back(Gate(gate_tables[14]));
          circuit.gates[circuit.num_gates].left_wire = left_wire_idx;
          circuit.gates[circuit.num_gates].right_wire = right_wire_idx;
          circuit.gates[circuit.num_gates].out_wire = out_wire_idx;

          ++circuit.num_gates;
          ++circuit.num_non_free_gates;
        } else {
          std::cout << "cannot parse gate of type: " << type_string << std::endl;
          exit(EXIT_FAILURE);
        }
      }
    }
  }
}

//Parse the gate description given a char array of the description file.
Circuit ParseCircuit(char raw_circuit[], CIRCUIT_TYPE circuit_type, std::string prefix) {
  Circuit circuit;

  if (circuit_type == COMPOSED) {
    raw_circuit = strchr(raw_circuit, ' ') + 1; //skip FN
    circuit.composed_index = (uint32_t) atoi(raw_circuit) - 1;

    circuit.circuit_name = prefix + "_" + std::to_string(circuit.composed_index);

    raw_circuit = strchr(raw_circuit, ' ') + 1; //#number num_inp_wires
    circuit.num_eval_inp_wires = (uint32_t) atoi(raw_circuit);
    circuit.num_const_inp_wires = 0;
    //Will make circuit.num_inp_wires values have the same value
    //All intermediate circuits are interpreted as eval party owns all input wires.

    circuit.out_wires_start = circuit.num_eval_inp_wires;

    raw_circuit = strchr(raw_circuit, ' ') + 1; //#number num_out_wires
    circuit.num_out_wires = (uint32_t) atoi(raw_circuit);
    circuit.num_eval_out_wires = circuit.num_out_wires;
    circuit.num_const_out_wires = circuit.num_out_wires;


    raw_circuit = strchr(raw_circuit, ' ') + 1; // #number total wires
    circuit.num_wires = (uint32_t) atoi(raw_circuit);

    raw_circuit = strchr(raw_circuit, '\n') + 1; //Skip this line

  } else if (circuit_type == BRISTOL) {
    raw_circuit = strchr(raw_circuit, ' ') + 1; //we dont need num_gates
    circuit.num_wires = (uint32_t) atoi(raw_circuit);
    raw_circuit = strchr(raw_circuit, '\n') + 1; //Skip to next line
    circuit.num_const_inp_wires = (uint32_t) atoi(raw_circuit);

    raw_circuit = strchr(raw_circuit, ' ') + 1;
    circuit.num_eval_inp_wires = (uint32_t)atoi(raw_circuit);

    raw_circuit = strchr(raw_circuit, ' ') + 1;
    circuit.num_out_wires = (uint32_t)atoi(raw_circuit);

    circuit.out_wires_start = circuit.num_wires - circuit.num_out_wires;
    circuit.num_const_out_wires = circuit.num_out_wires;
    circuit.num_eval_out_wires = circuit.num_out_wires;

    raw_circuit = strchr(raw_circuit,  '\n') + 1; //Skip to next line
    raw_circuit = strchr(raw_circuit,  '\n') + 1; //Skip to next line
  }

  ParseGates(circuit, raw_circuit, circuit_type);

  SetCircuitOffsetIndices(circuit);
  AddOutputIdentityGates(circuit);

  return circuit;
}

ComposedCircuit::ComposedCircuit() {

}

ComposedCircuit::ComposedCircuit(Circuit circuit, uint32_t num_parallel_copies, uint32_t num_inputs_used) {
  composed_circuit_name = circuit.circuit_name + "_" + std::to_string(num_parallel_copies);

  num_functions = 1;
  num_layers = 1;
  num_components = num_parallel_copies;

  num_const_inp_wires = circuit.num_const_inp_wires * num_parallel_copies;
  num_eval_inp_wires = circuit.num_eval_inp_wires * num_parallel_copies;
  num_const_out_wires = circuit.num_const_out_wires * num_parallel_copies;
  num_eval_out_wires = circuit.num_eval_out_wires * num_parallel_copies;

  if (num_const_out_wires != num_eval_out_wires) {
    std::cout << "Currently, only supported that parties have the same output wires!" << std::endl;
    throw std::runtime_error("Currently, only supported that parties have the same output!");
  }

  SetComposedCircuitOffsetIndices(*this);
  max_wire_index = num_inp_wires + num_out_wires;

  functions.emplace_back(circuit);
  name_to_function_num.emplace(circuit.circuit_name, 0);

  circuits_in_layer.resize(1);

  num_circuit_copies.resize(1);
  num_circuit_copies[0] = num_parallel_copies;
  for (int i = 0; i < num_parallel_copies; ++i) {
    circuits.emplace_back(
      std::make_pair(circuit.circuit_name, i));
  }

  out_wire_holders.resize(num_parallel_copies);

  uint32_t curr_const_inp_idx = num_inputs_used;
  uint32_t curr_eval_inp_idx = num_inputs_used + num_const_inp_wires;
  for (int i = 0; i < num_parallel_copies; ++i) {
    circuits_in_layer[0].emplace_back(i);
    output_circuits.emplace_back(i);

    for (int j = 0; j < circuit.num_const_inp_wires; ++j) {
      out_wire_holders[i].emplace_back(std::make_pair(std::numeric_limits<uint32_t>::max(), std::vector<uint32_t>(1, curr_const_inp_idx)));
      ++curr_const_inp_idx;
    }

    for (int j = 0; j < circuit.num_eval_inp_wires; ++j) {
      out_wire_holders[i].emplace_back(std::make_pair(std::numeric_limits<uint32_t>::max(), std::vector<uint32_t>(1, curr_eval_inp_idx)));
      ++curr_eval_inp_idx;
    }

    out_wire_holder_to_wire_idx.emplace(i, num_inp_wires + i * circuit.num_out_wires);
  }
}

std::vector<std::vector<uint32_t>> ComposedCircuit::GetOutputIndices(bool is_const) {

  std::vector<std::vector<uint32_t>> res;

  for (int i = 0; i < output_circuits.size(); ++i) {
    std::string circuit_name = circuits[output_circuits[i]].first;
    uint32_t function_num = name_to_function_num[circuit_name];
    Circuit& circuit = functions[function_num];

    res.emplace_back(std::vector<uint32_t>());
    if (is_const) {
      for (int j = circuit.const_out_wires_start; j < circuit.const_out_wires_stop; ++j) {
        res.back().emplace_back(j);
      }
    } else {
      for (int j = circuit.eval_out_wires_start; j < circuit.eval_out_wires_stop; ++j) {
        res.back().emplace_back(j);
      }
    }
  }

  return res;
}

ComposedCircuit ParseComposedCircuit(char raw_circuit[], std::string circuits_prefix, uint32_t num_inputs_used) {

  ComposedCircuit composed_circuit;
  composed_circuit.composed_circuit_name = circuits_prefix;

  composed_circuit.num_functions = (uint32_t) atoi(raw_circuit); //get number functions

  raw_circuit = strchr(raw_circuit, ' ') + 1;
  composed_circuit.num_components = (uint32_t) atoi(raw_circuit);
  composed_circuit.num_layers = composed_circuit.num_components;

  raw_circuit = strchr(raw_circuit, '\n') + 1; //next line
  composed_circuit.num_const_inp_wires = (uint32_t) atoi(raw_circuit);

  raw_circuit = strchr(raw_circuit, ' ') + 1;
  composed_circuit.num_eval_inp_wires = (uint32_t) atoi(raw_circuit);

  raw_circuit = strchr(raw_circuit, ' ') + 1;
  composed_circuit.num_const_out_wires = (uint32_t) atoi(raw_circuit);

  raw_circuit = strchr(raw_circuit, ' ') + 1;
  composed_circuit.num_eval_out_wires = (uint32_t) atoi(raw_circuit);

  if (composed_circuit.num_const_out_wires != composed_circuit.num_eval_out_wires) {
    std::cout << "Currently, only supported that parties have the same output wires!" << std::endl;
    throw std::runtime_error("Currently, only supported that parties have the same output!");
  }

  raw_circuit = strchr(raw_circuit, '\n') + 1; //skip rest of line
  raw_circuit = strchr(raw_circuit, '\n') + 1; //skip entire empty line

  SetComposedCircuitOffsetIndices(composed_circuit);

  /////////////////////////read each function///////////////////////////
  for (int i = 0; i < composed_circuit.num_functions; i++) { //starts at 1, we do not count main function
    composed_circuit.functions.emplace_back(ParseCircuit(raw_circuit, COMPOSED, circuits_prefix));
    composed_circuit.name_to_function_num.emplace(composed_circuit.functions.back().circuit_name, composed_circuit.functions.back().composed_index);
    raw_circuit = strchr(raw_circuit, '-') + 1; //next function
    raw_circuit = strchr(raw_circuit, '\n') + 1; //skip line
    raw_circuit = strchr(raw_circuit, '\n') + 1; //skip line
  }
  raw_circuit = strchr(raw_circuit, '\n') + 1; //skip line FN main

  //We put exactly 1 circuit in each layer
  composed_circuit.circuits_in_layer.resize(composed_circuit.num_layers);
  for (int i = 0; i < composed_circuit.num_layers; ++i) {
    composed_circuit.circuits_in_layer[i].emplace_back(i);
  }

  //Parse function calls and populate maps in the process
  composed_circuit.out_wire_holders.resize(composed_circuit.num_components);
  composed_circuit.num_circuit_copies.resize(composed_circuit.num_functions);

  std::unordered_map<uint32_t, std::vector<uint32_t>> holder_to_global_inp_wires, holder_to_global_out_wires;
  std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> global_out_wire_to_holder;

  //Add input wires to unique_wires_map
  for (int i = 0; i < composed_circuit.num_inp_wires; ++i) {
    global_out_wire_to_holder.emplace(i, std::make_pair(std::numeric_limits<uint32_t>::max(), i));
  }

  uint32_t org_out_wires_end = composed_circuit.num_inp_wires + composed_circuit.num_out_wires;

  uint32_t curr_function = 0; //starts here due to identity gates
  uint32_t curr_circ_inp_wires = 0;
  while (*raw_circuit != EOF) {
    if (*raw_circuit == '\n') {
      raw_circuit = strchr(raw_circuit, '\n') + 1;
      continue;
    }

    //Add function call to circuit and increment num_circuit_copies
    raw_circuit = strchr(raw_circuit, ' ') + 1; //FN i
    uint32_t function_id = (uint32_t) atoi(raw_circuit) - 1; //FN starts at 1
    composed_circuit.circuits.emplace_back(
      std::make_pair(composed_circuit.functions[function_id].circuit_name,
                     composed_circuit.num_circuit_copies[function_id]));
    ++composed_circuit.num_circuit_copies[function_id];

    raw_circuit = strchr(raw_circuit, '\n') + 1; //input wire list

    std::vector<uint32_t> inputs;
    while (*raw_circuit != '\n') { //run through input list

      uint32_t global_in_wire = (uint32_t) atoi(raw_circuit);
      inputs.emplace_back(global_in_wire);
      raw_circuit = strchr(raw_circuit, ' ') + 1;
    }

    holder_to_global_inp_wires.emplace(curr_function, inputs);

    std::unordered_map<uint32_t, std::vector<uint32_t>> out_holder_info;
    std::vector<std::vector<uint32_t>> insert_order;
    for (int i = 0; i < inputs.size(); ++i) {
      uint32_t out_holder_num = std::get<0>(global_out_wire_to_holder[inputs[i]]);
      uint32_t out_holder_wire_num = std::get<1>(global_out_wire_to_holder[inputs[i]]);

      if (out_holder_info.find(out_holder_num) == out_holder_info.end()) {
        out_holder_info.emplace(out_holder_num, std::vector<uint32_t>());
        insert_order.emplace_back(std::vector<uint32_t>());
      }
      out_holder_info[out_holder_num].emplace_back(out_holder_wire_num);
      insert_order[insert_order.size() - 1].emplace_back(out_holder_num);
    }

    for (int i = 0; i < out_holder_info.size(); ++i) {
      composed_circuit.out_wire_holders[curr_function].emplace_back(std::make_pair(insert_order[i][0], out_holder_info[insert_order[i][0]]));
    }


    raw_circuit = strchr(raw_circuit, '\n') + 1; //output wire list
    std::vector<uint32_t> outputs;
    bool holds_out_wire = false;

    while (*raw_circuit != '\n') { //run through output list

      uint32_t global_out_wire = (uint32_t) atoi(raw_circuit);

      if (global_out_wire < org_out_wires_end && !holds_out_wire) {
        composed_circuit.output_circuits.emplace_back(curr_function);
        holds_out_wire = true;
      }

      composed_circuit.max_wire_index = std::max(composed_circuit.max_wire_index, global_out_wire);

      outputs.emplace_back(global_out_wire);

      raw_circuit = strchr(raw_circuit, ' ') + 1;
    }
    holder_to_global_out_wires.emplace(curr_function, outputs);

    for (int i = 0; i < outputs.size(); ++i) {
      //overwrites if necessary, which is needed if wires are reused
      global_out_wire_to_holder[outputs[i]] = std::make_pair(curr_function, i);
    }

    composed_circuit.out_wire_holder_to_wire_idx.emplace(curr_function, outputs[0]);

    ++curr_function;

    raw_circuit = strchr(raw_circuit, '\n') + 1;
    raw_circuit = strchr(raw_circuit, '\n') + 1;
  }

  //Post processing for inputs
  for (int i = 0; i < composed_circuit.out_wire_holders.size(); ++i) {

    std::vector<std::pair<uint32_t, std::vector<uint32_t>>> inp_pair;

    for (int j = 0; j < composed_circuit.out_wire_holders[i].size(); ++j) {
      if (composed_circuit.out_wire_holders[i][j].first == std::numeric_limits<uint32_t>::max()) {

        for (int m = 0; m < composed_circuit.out_wire_holders[i][j].second.size(); ++m) {
          inp_pair.emplace_back(std::numeric_limits<uint32_t>::max(), std::vector<uint32_t>(1, composed_circuit.out_wire_holders[i][j].second[m]));
        }
      } else {
        inp_pair.emplace_back(composed_circuit.out_wire_holders[i][j]);
      }
    }
    composed_circuit.out_wire_holders[i] = inp_pair;
  }

  //Need to add one to be able to index the biggest element
  ++composed_circuit.max_wire_index;

  //Fix the layering
  RelayerComposedCircuit(composed_circuit);

  return composed_circuit;
}

void RelayerComposedCircuit(ComposedCircuit& composed_circuit) {

  std::unordered_map<uint32_t, uint32_t> wire_to_layer;
  std::vector<uint32_t> new_layer(composed_circuit.circuits.size());

  //Input wires from layer -1
  for (int i = 0; i < composed_circuit.num_inp_wires; ++i) {
    wire_to_layer.emplace(i, 0);
  }


  for (int l = 0; l < composed_circuit.num_layers; ++l) {
    for (int c = 0; c < composed_circuit.circuits_in_layer[l].size(); ++c) {

      uint32_t curr_out_circuit_idx = composed_circuit.circuits_in_layer[l][c];

      std::string circuit_name = composed_circuit.circuits[curr_out_circuit_idx].first;
      uint32_t function_num = composed_circuit.name_to_function_num[circuit_name];
      Circuit& circuit = composed_circuit.functions[function_num];

      uint32_t curr_layer = 0;
      for (int i = 0; i < composed_circuit.out_wire_holders[curr_out_circuit_idx].size(); ++i) {

        std::pair<uint32_t, std::vector<uint32_t>>& curr_out_wire_holder_pair = composed_circuit.out_wire_holders[curr_out_circuit_idx][i];
        uint32_t inp_wire_circuit_idx = std::get<0>(curr_out_wire_holder_pair);
        std::vector<uint32_t> out_wires = std::get<1>(curr_out_wire_holder_pair);

        uint32_t out_start_pos = 0;
        if (inp_wire_circuit_idx != std::numeric_limits<uint32_t>::max()) {
          out_start_pos = composed_circuit.out_wire_holder_to_wire_idx[inp_wire_circuit_idx];
        }


        for (int j = 0; j < out_wires.size(); ++j) {
          curr_layer = std::max(curr_layer, wire_to_layer[out_start_pos + out_wires[j]]);
        }
      }

      new_layer[curr_out_circuit_idx] = curr_layer;

      uint32_t out_start_pos = composed_circuit.out_wire_holder_to_wire_idx[curr_out_circuit_idx];

      for (int j = 0; j < circuit.num_out_wires; ++j) {
        wire_to_layer.emplace(out_start_pos + j, curr_layer + 1);
      }
    }
  }

  //Reset old layers
  composed_circuit.num_layers = *std::max_element(new_layer.begin(), new_layer.end()) + 1;
  composed_circuit.circuits_in_layer = std::vector<std::vector<uint32_t>>(composed_circuit.num_layers);

  for (int c = 0; c < composed_circuit.circuits.size(); ++c) {

    uint32_t curr_layer = new_layer[c];
    composed_circuit.circuits_in_layer[curr_layer].emplace_back(c);
  }
}

//Reads circuit in textual format. Writes byte length of text file to file_size.
Circuit read_bristol_circuit(const char* circuit_file) {
  FILE* file;
  size_t file_size;
  file = fopen(circuit_file, "r");
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
    printf("ERROR while loading file: %s\n", circuit_file);
    exit(EXIT_FAILURE);
  }
  data[file_size] = EOF;
  if (ferror(file)) {
    printf("ERROR: fread() error\n");
    exit(EXIT_FAILURE);
  }
  fclose(file);
  Circuit circuit = ParseCircuit(data.get(), BRISTOL);

  return circuit;

}


ComposedCircuit read_composed_circuit(const char* circuit_file, std::string circuits_prefix) {
  FILE* file;
  size_t file_size;
  file = fopen(circuit_file, "r");
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
  ComposedCircuit circuit = ParseComposedCircuit(data.get(), circuits_prefix);


  return circuit;
}

osuCrypto::BitVector eval_circuit(Circuit & circuit, osuCrypto::BitVector& input, bool reversed) {

  osuCrypto::BitVector evals(circuit.num_wires);
  for (int i = 0; i < circuit.num_inp_wires; ++i) {
    if (reversed) {
      evals[i] = GetBitReversed(i, input.data()); //load inputs into evals vector
    } else {
      evals[i] = input[i]; //load inputs into evals vector
    }
  }

  for (int i = 0; i < circuit.num_gates; ++i) {
    Gate& g = circuit.gates[i];
    if (g.type[3 - (2 * evals[g.left_wire] + evals[g.right_wire])] == '1') {
      evals[g.out_wire] = 1;
    } else {
      evals[g.out_wire] = 0;
    }
  }

  osuCrypto::BitVector eval_res_bits(circuit.num_out_wires);
  for (int i = 0; i < circuit.num_out_wires; ++i) {
    if (reversed) {
      SetBitReversed(i, evals[circuit.out_wires_start + i], eval_res_bits.data());
    } else {
      eval_res_bits[i] = evals[circuit.out_wires_start + i];
    }
  }

  return eval_res_bits;
}

osuCrypto::BitVector eval_circuit(ComposedCircuit& composed_circuit, osuCrypto::BitVector& input) {

  osuCrypto::BitVector evals(composed_circuit.max_wire_index);
  for (int i = 0; i < composed_circuit.num_inp_wires; ++i) {
    evals[i] = input[i]; //load inputs into evals vector
  }

  //Evaluate circuits
  uint32_t curr_out_circuit_idx = 0;
  for (int l = 0; l < composed_circuit.num_layers; ++l) {
    for (int c = 0; c < composed_circuit.circuits_in_layer[l].size(); ++c) {

      std::string circuit_name = composed_circuit.circuits[curr_out_circuit_idx].first;
      uint32_t function_num = composed_circuit.name_to_function_num[circuit_name];
      Circuit& circuit = composed_circuit.functions[function_num];

      osuCrypto::BitVector current_input(circuit.num_inp_wires);

      uint32_t curr_inp_wire = 0;
      for (int i = 0; i < composed_circuit.out_wire_holders[curr_out_circuit_idx].size(); ++i) {

        std::pair<uint32_t, std::vector<uint32_t>>& curr_out_wire_holder_pair = composed_circuit.out_wire_holders[curr_out_circuit_idx][i];
        uint32_t inp_wire_circuit_idx = std::get<0>(curr_out_wire_holder_pair);
        std::vector<uint32_t> out_wires = std::get<1>(curr_out_wire_holder_pair);

        uint32_t out_start_pos = composed_circuit.out_wire_holder_to_wire_idx[inp_wire_circuit_idx];

        for (int j = 0; j < out_wires.size(); ++j) {
          current_input[curr_inp_wire] = evals[out_start_pos + out_wires[j]];
          ++curr_inp_wire;
        }
      }
      osuCrypto::BitVector current_output = eval_circuit(circuit, current_input);

      uint32_t out_wire_idx = composed_circuit.out_wire_holder_to_wire_idx[curr_out_circuit_idx];

      for (int j = 0; j < circuit.num_out_wires; ++j) {
        evals[out_wire_idx + j] = current_output[j];
      }

      ++curr_out_circuit_idx;
    }
  }

  //Go from bytes to bits
  osuCrypto::BitVector output(composed_circuit.num_out_wires);
  uint32_t curr_global_output_idx = 0;

  for (int i = 0; i < composed_circuit.output_circuits.size(); ++i) {
    std::string circuit_name = composed_circuit.circuits[composed_circuit.output_circuits[i]].first;
    uint32_t function_num = composed_circuit.name_to_function_num[circuit_name];
    Circuit& circuit = composed_circuit.functions[function_num];

    uint32_t read_pos = composed_circuit.out_wire_holder_to_wire_idx[composed_circuit.output_circuits[i]];

    for (int j = 0; j < circuit.num_out_wires; ++j) {
      output[curr_global_output_idx] = evals[read_pos + j];
      ++curr_global_output_idx;
    }
  }

  return output;
}

osuCrypto::BitVector ParseBits(FILE* input_file, uint32_t num_bits_start, uint32_t num_bits, bool read_reversed) {
  osuCrypto::BitVector input(num_bits - num_bits_start);

  uint8_t* buffer;
  long filelen;
  fseek(input_file, 0, SEEK_END);          // Jump to the end of the file
  filelen = ftell(input_file);             // Get the current byte offset in the file
  rewind(input_file);                      // Jump back to the beginning of the file
  buffer = new uint8_t[(filelen + 1)]; // Enough memory for file + \0
  fread(buffer, filelen, 1, input_file); // Read in the entire file

  //Read input the right way!
  for (int i = num_bits_start; i < num_bits; ++i) {
    if (read_reversed) {
      input[i] = GetBitReversed(i, buffer);
    } else {
      input[i] = GetBit(i, buffer);
    }
  }

  return input;
}

osuCrypto::BitVector GetBitsReversed(FILE* input_file, uint32_t num_bits, uint32_t num_bits_start) {
  return ParseBits(input_file, num_bits_start, num_bits, true);
}

osuCrypto::BitVector GetBits(FILE* input_file, uint32_t num_bits, uint32_t num_bits_start) {
  return ParseBits(input_file, num_bits_start, num_bits, false);
}