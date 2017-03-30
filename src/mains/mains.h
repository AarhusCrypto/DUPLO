#include "ezOptionParser/ezOptionParser.hpp"

using namespace ez;

//Hardcoded default values
static std::string default_num_iters("10");
static std::string default_circuit_name("aes");
static std::string default_execs("1,1,1");
static std::string default_ip_address("localhost");
static std::string default_port("28001");
static std::string default_ram_only("0");
static std::string default_circuit_file("");

void Usage(ezOptionParser& opt) {
  std::string usage;
  opt.getUsage(usage);
  std::cout << usage;
};