#include "duplo-mains/mains.h"
#include "duplo/duplo-constructor.h"

int main(int argc, const char* argv[]) {
  ezOptionParser opt;

  opt.overview = "DuploConstructor Passing Parameters Guide.";
  opt.syntax = "Duploconst first second third forth fifth sixth";
  opt.example = "Duploconst -n 4 -c aes -e 8,2,1 -ip 10.11.100.216 -p 28001\n\n";
  opt.footer = "ezOptionParser 0.1.4  Copyright (C) 2011 Remik Ziemlinski\nThis program is free and without warranty.\n";

  opt.add(
    "", // Default.
    0, // Required?
    0, // Number of args expected.
    0, // Delimiter if expecting multiple args.
    "Display usage instructions.", // Help description.
    "-h",     // Flag token.
    "-help",  // Flag token.
    "--help", // Flag token.
    "--usage" // Flag token.
  );

  opt.add(
    default_num_iters.c_str(), // Default.
    0, // Required?
    1, // Number of args expected.
    0, // Delimiter if expecting multiple args.
    "Number of circuits to produce and evaluate.", // Help description.
    "-n"
  );

  opt.add(
    default_circuit_name.c_str(), // Default.
    0, // Required?
    1, // Number of args expected.
    0, // Delimiter if expecting multiple args.
    "Circuit name. Can be either aes, sha-1, sha-256 or cbc.", // Help description.
    "-c" // Flag token.
  );

  opt.add(
    default_execs.c_str(), // Default.
    0, // Required?
    3, // Number of args expected.
    ',', // Delimiter if expecting multiple args.
    "Number of parallel executions for each phase. Preprocessing, Offline and Online.", // Help description.
    "-e"
  );

  opt.add(
    default_ip_address.c_str(), // Default.
    0, // Required?
    1, // Number of args expected.
    0, // Delimiter if expecting multiple args.
    "IP Address of Machine running TinyConst", // Help description.
    "-ip"
  );

  opt.add(
    default_port.c_str(), // Default.
    0, // Required?
    1, // Number of args expected.
    0, // Delimiter if expecting multiple args.
    "Port to listen on/connect to", // Help description.
    "-p"
  );

  opt.add(
    default_ram_only.c_str(), // Default.
    0, // Required?
    1, // Number of args expected.
    0, // Delimiter if expecting multiple args.
    "Use ram", // Help description.
    "-d"
  );

  opt.add(
    default_circuit_file.c_str(), // Default.
    0, // Required?
    1, // Number of args expected.
    0, // Delimiter if expecting multiple args.
    "circuit file", // Help description.
    "-f"
  );

  //Attempt to parse input
  opt.parse(argc, argv);

  //Check if help was requested and do some basic validation
  if (opt.isSet("-h")) {
    Usage(opt);
    return 1;
  }
  std::vector<std::string> badOptions;
  if (!opt.gotExpected(badOptions)) {
    for (int i = 0; i < badOptions.size(); ++i)
      std::cerr << "ERROR: Got unexpected number of arguments for option " << badOptions[i] << ".\n\n";
    Usage(opt);
    return 1;
  }

  //Copy inputs into the right variables
  int num_iters, num_execs_components, num_execs_auths, num_execs_online, port, ram_only;
  std::vector<int> num_execs;
  std::string circuit_name, ip_address, exec_name, circuit_file;

  std::string prefix("const_");
  opt.get("-n")->getInt(num_iters);
  opt.get("-c")->getString(circuit_name);
  opt.get("-d")->getInt(ram_only);
  opt.get("-f")->getString(circuit_file);
  circuit_name = prefix + circuit_name;

  opt.get("-e")->getInts(num_execs);
  num_execs_components = num_execs[0];
  num_execs_auths = num_execs[1];
  num_execs_online = num_execs[2];

  opt.get("-ip")->getString(ip_address);
  opt.get("-p")->getInt(port);

  //Set the circuit variables according to circuit_name
  ComposedCircuit composed_circuit;

  if (!circuit_file.empty()) {
    exec_name = prefix + std::to_string(num_iters) + circuit_file;
    composed_circuit = read_composed_circuit(circuit_file.c_str(), circuit_name);
  } else {

    if (circuit_name.find("p_aes") != std::string::npos) {
      exec_name = prefix + std::to_string(num_iters) + "xAES";
      Circuit tmp = read_bristol_circuit("test/data/AES-non-expanded.txt");
      tmp.circuit_name = exec_name;
      ComposedCircuit composed_tmp = ComposedCircuit(tmp, num_iters);
      composed_circuit = composed_tmp;
    } else if (circuit_name.find("p_sha-256") != std::string::npos) {
      exec_name = prefix + std::to_string(num_iters) + "xSHA-256";
      Circuit tmp = read_bristol_circuit("test/data/sha-256.txt");
      tmp.circuit_name = exec_name;
      ComposedCircuit composed_tmp = ComposedCircuit(tmp, num_iters);
      composed_circuit = composed_tmp;
    } else if (circuit_name.find("p_sha-1") != std::string::npos) {
      exec_name = prefix + std::to_string(num_iters) + "xSHA-1";
      Circuit tmp = read_bristol_circuit("test/data/sha-1.txt");
      tmp.circuit_name = exec_name;
      ComposedCircuit composed_tmp = ComposedCircuit(tmp, num_iters);
      composed_circuit = composed_tmp;
    } else if (circuit_name.find("p_cbc") != std::string::npos) {
      exec_name = prefix + std::to_string(num_iters) + "xAES-CBC-MAC";
      Circuit tmp = read_bristol_circuit("test/data/aescbcmac16.txt");
      tmp.circuit_name = exec_name;
      ComposedCircuit composed_tmp = ComposedCircuit(tmp, num_iters);
      composed_circuit = composed_tmp;
    } else if (circuit_name.find("p_and") != std::string::npos) {
      exec_name = prefix + std::to_string(num_iters) + "xAND";
      Circuit tmp = read_bristol_circuit("test/data/and.txt");
      tmp.circuit_name = exec_name;
      ComposedCircuit composed_tmp = ComposedCircuit(tmp, num_iters);
      composed_circuit = composed_tmp;
    } else if (circuit_name.find("p_add32") != std::string::npos) {
      exec_name = prefix + std::to_string(num_iters) + "xADD32";
      Circuit tmp = read_bristol_circuit("test/data/adder_32bit.txt");
      tmp.circuit_name = exec_name;
      ComposedCircuit composed_tmp = ComposedCircuit(tmp, num_iters);
      composed_circuit = composed_tmp;
    } else if (circuit_name.find("p_add64") != std::string::npos) {
      exec_name = prefix + std::to_string(num_iters) + "xADD64";
      Circuit tmp = read_bristol_circuit("test/data/adder_64bit.txt");
      tmp.circuit_name = exec_name;
      ComposedCircuit composed_tmp = ComposedCircuit(tmp, num_iters);
      composed_circuit = composed_tmp;
    } else if (circuit_name.find("p_mul32") != std::string::npos) {
      exec_name = prefix + std::to_string(num_iters) + "xMUL32";
      Circuit tmp = read_bristol_circuit("test/data/mult_32x32.txt");
      tmp.circuit_name = exec_name;
      ComposedCircuit composed_tmp = ComposedCircuit(tmp, num_iters);
      composed_circuit = composed_tmp;
    } else {
      std::cout << "No such circuit" << std::endl;
      return 1;
    }
  }

  //Compute the required number of common_tools that are to be created. We create one main param and one for each sub-thread that will be spawned later on. Need to know this at this point to setup context properly

  int max_num_parallel_execs = max_element(num_execs.begin(), num_execs.end())[0];

  DuploConstructor duplo_const(duplo_constant_seeds[0], max_num_parallel_execs, ram_only);

  duplo_const.Connect(ip_address, (uint16_t) port);

  std::cout << "====== " << num_iters << " x " << exec_name << " ======" << std::endl;
  if (!circuit_file.empty()) {
    std::cout << "====== " << circuit_file << " ======" << std::endl;  
  }
  

  //Values used for network syncing after each phase
  uint8_t rcv;
  uint8_t snd;

  //Run initial Setup (BaseOT) phase
  auto setup_begin = GET_TIME();
  duplo_const.Setup();
  auto setup_end = GET_TIME();

  uint64_t setup_data_sent = duplo_const.GetTotalDataSent();

  //Run Preprocessing phase
  auto preprocess_begin = GET_TIME();
  for (int i = 0; i < composed_circuit.num_functions; ++i) {
    duplo_const.PreprocessComponentType(composed_circuit.functions[i].circuit_name, composed_circuit.functions[i], composed_circuit.num_circuit_copies[i], num_execs_components);
  }
  auto preprocess_end = GET_TIME();

  //Sync with Evaluator
  duplo_const.chan->recv(&rcv, 1);
  duplo_const.chan->send(&snd, 1);

  uint64_t preprocess_data_sent = duplo_const.GetTotalDataSent() - setup_data_sent;

  auto prepare_eval_begin = GET_TIME();
  duplo_const.PrepareComponents(composed_circuit.num_inp_wires, num_execs_auths);
  auto prepare_eval_end = GET_TIME();

  //Sync with Evaluator
  duplo_const.chan->recv(&rcv, 1);
  duplo_const.chan->send(&snd, 1);

  uint64_t prepare_data_sent = duplo_const.GetTotalDataSent() - setup_data_sent - preprocess_data_sent;

  auto build_begin = GET_TIME();
  duplo_const.Build(composed_circuit, num_execs_auths);
  auto build_end = GET_TIME();

  //Sync with Evaluator
  duplo_const.chan->recv(&rcv, 1);
  duplo_const.chan->send(&snd, 1);

  uint64_t build_data_sent = duplo_const.GetTotalDataSent() - setup_data_sent - preprocess_data_sent - prepare_data_sent;

  osuCrypto::BitVector inputs(composed_circuit.num_const_inp_wires); //0 input

  auto eval_circuits_begin = GET_TIME();
  duplo_const.Evaluate(composed_circuit, inputs, num_execs_online);
  auto eval_circuits_end = GET_TIME();

  //Sync with Evaluator
  duplo_const.chan->recv(&rcv, 1);
  duplo_const.chan->send(&snd, 1);

  uint64_t eval_data_sent = duplo_const.GetTotalDataSent() - setup_data_sent - preprocess_data_sent - prepare_data_sent - build_data_sent;

  std::vector<osuCrypto::BitVector> outputs(composed_circuit.output_circuits.size());
  std::vector<std::vector<uint32_t>> const_output_idices = composed_circuit.GetOutputIndices(true);
  std::vector<std::vector<uint32_t>> eval_output_indices = composed_circuit.GetOutputIndices(false);

  auto decode_keys_begin = GET_TIME();
  duplo_const.DecodeKeys(composed_circuit, const_output_idices, eval_output_indices, outputs, num_execs_online);
  auto decode_keys_end = GET_TIME();

  //Sync with Evaluator
  duplo_const.chan->recv(&rcv, 1);
  duplo_const.chan->send(&snd, 1);

  uint64_t decode_data_sent = duplo_const.GetTotalDataSent() - setup_data_sent - preprocess_data_sent - prepare_data_sent - build_data_sent - eval_data_sent;

  // Average out the timings of each phase and print results
  uint64_t setup_time_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(setup_end - setup_begin).count();
  uint64_t preprocess_time_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(preprocess_end - preprocess_begin).count();

  uint64_t build_time_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(build_end - build_begin).count();
  uint64_t prepare_eval_time_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(prepare_eval_end - prepare_eval_begin).count();

  uint64_t eval_circuits_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(eval_circuits_end - eval_circuits_begin).count();
  uint64_t decode_keys_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(decode_keys_end - decode_keys_begin).count();

  std::cout << "Setup ms: " << (double) setup_time_nano / num_iters / 1000000 << ", data sent: " << (double) setup_data_sent / num_iters / 1000 << " kB" << std::endl;
  std::cout << "Circuit Preprocess ms: " << (double) preprocess_time_nano / num_iters / 1000000 << ", data sent: " << (double) preprocess_data_sent / num_iters / 1000 << " kB" << std::endl;
  std::cout << "Auth Preprocess ms: " << (double) prepare_eval_time_nano / num_iters / 1000000 << ", data sent: " << (double) prepare_data_sent / num_iters / 1000 << " kB" << std::endl;
  std::cout << "Build ms: " << (double) build_time_nano / num_iters / 1000000 << ", data sent: " << (double) build_data_sent / num_iters / 1000 << " kB" << std::endl;
  std::cout << "Eval circuits ms: " << (double) eval_circuits_nano / num_iters / 1000000 << ", data sent: " << (double) eval_data_sent / num_iters / 1000 << " kB" << std::endl;
  std::cout << "Decode keys ms: " << (double) decode_keys_nano / num_iters / 1000000 << ", data sent: " << (double) decode_data_sent / num_iters / 1000 << " kB" << std::endl;

  std::cout << "=============================" << std::endl;

  std::cout << "Ind. Preprocess ms: "
            << (double)(preprocess_time_nano + prepare_eval_time_nano) / num_iters / 1000000
            << ", data sent: " << (double)(preprocess_data_sent + prepare_data_sent) / num_iters / 1000 << " kB" << std::endl;
  std::cout << "D. Preprocess ms: " << (double) build_time_nano / num_iters / 1000000 << ", data sent: " << (double) build_data_sent / num_iters / 1000 << " kB" << std::endl;

  std::cout << "Online ms: " << (double)(eval_circuits_nano + decode_keys_nano) / num_iters / 1000000
            << ", data sent: " << (double)(eval_data_sent + decode_data_sent) / num_iters / 1000 << " kB" << std::endl;

  std::cout << "( " << num_iters
            << ", " << (double)(preprocess_data_sent + prepare_data_sent) / num_iters / 1000
            << ", " << (double)build_data_sent / num_iters / 1000
            << ", " << (double)(eval_data_sent + decode_data_sent) / num_iters / 1000 << ")" << std::endl;
  std::cout << "( " << num_iters
            << ", " << (double)(preprocess_data_sent + prepare_data_sent + build_data_sent + eval_data_sent + decode_data_sent) / num_iters / 1000 << ")" << std::endl;

  std::cout << "=============================" << std::endl;
}