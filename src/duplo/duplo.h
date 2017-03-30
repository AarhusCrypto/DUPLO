#ifndef DUPLO_DUPLO_H_
#define DUPLO_DUPLO_H_

#include "garbling/garbling-handler.h"
#include "garbling/const-garbled-circuit.h"
#include "circuit/circuit.h"
#include "duplo-util/typedefs.h"
#include "duplo-util/storage.h"

//For libOTe extension
#include "cryptoTools/Network/BtChannel.h"
#include "cryptoTools/Network/BtEndpoint.h"
#include "cryptoTools/Crypto/PRNG.h"
#include "libOTe/Base/naor-pinkas.h"

/*
A superclass to the two subclasses DuploConstructor and DuploEvaluator.
*/

enum BucketType {
  SINGLE = 0,
  MAJORITY = 1
};

enum OutType {
  CONST_OUT = 0,
  EVAL_OUT = 1,
  ALL_OUT = 2
};

class Duplo {

public:
  Duplo(uint8_t seed[], uint32_t num_max_parallel_execs = 1, bool ram_only = false);
  ~Duplo();

  virtual void Connect(std::string ip_address, uint16_t port) = 0;

  virtual void Setup() = 0;

  virtual void PreprocessComponentType(std::string component_type, Circuit& circuit, uint32_t num_circuits, uint32_t num_parallel_execs = 1, BucketType bucket_type = SINGLE) = 0;

  virtual void PrepareComponents(uint64_t num_input_auth_buckets, uint32_t num_parallel_execs = 1) = 0;

  virtual void Build(ComposedCircuit& composed_circuit, uint32_t num_parallel_execs = 1) = 0;

  osuCrypto::PRNG rnd;

  ctpl::thread_pool thread_pool;

  uint64_t curr_num_ready_inputs;
  uint64_t inputs_used;

  std::vector<osuCrypto::PRNG> exec_rnds;

  Storage persistent_storage;
  std::vector<std::tuple<std::string, Circuit, uint64_t>> circuit_info;
  std::unordered_map<std::string, Circuit> string_to_circuit_map;
  std::unordered_map<std::string, ComposedCircuit> string_to_composed_circuit_map;

  osuCrypto::BtIOService ios;
  osuCrypto::BtEndpoint end_point;
  osuCrypto::Channel* chan;
  std::vector<osuCrypto::Channel*> exec_channels;

  std::mutex state_mutex;

  Circuit inp_bucket_circuit;

  uint64_t GetTotalDataSent();

  uint32_t GetTotalNumberOfConstInputWires(std::vector<std::pair<std::string, uint32_t>>& input_components, uint32_t from = -1, uint32_t to = -1);
  uint32_t GetTotalNumberOfEvalInputWires(std::vector<std::pair<std::string, uint32_t>>& input_components, uint32_t from = -1, uint32_t to = -1);
  uint32_t GetTotalNumberOfInputWires(std::vector<std::pair<std::string, uint32_t>>& input_components, uint32_t from = -1, uint32_t to = -1);

  uint32_t GetTotalNumberOfConstOutputWires(std::vector<std::pair<std::string, uint32_t>>& output_components, uint32_t from = -1, uint32_t to = -1);
  uint32_t GetTotalNumberOfEvalOutputWires(std::vector<std::pair<std::string, uint32_t>>& output_components, uint32_t from = -1, uint32_t to = -1);
  uint32_t GetTotalNumberOfOutputWires(std::vector<std::pair<std::string, uint32_t>>& output_components, uint32_t from = -1, uint32_t to = -1);

  uint32_t GetNumberOfOutputWires(std::vector<std::vector<uint32_t>>& indices, uint32_t from, uint32_t to);
};

template <class GarbledCircuitType>
class PermutedIndex {
  std::vector<GarbledCircuitType>& aux_garbled_circuits_data;
  std::vector<uint32_t>& permuted_ids_inv;
  uint32_t exec_offset;

public:
  uint64_t aux_size;

  PermutedIndex(std::vector<GarbledCircuitType>& aux_garbled_circuits_data, std::vector<uint32_t>& permuted_ids_inv, uint32_t exec_offset, uint64_t aux_size) :
    aux_garbled_circuits_data(aux_garbled_circuits_data),
    permuted_ids_inv(permuted_ids_inv),
    exec_offset(exec_offset),
    aux_size(aux_size) {

  }

  uint8_t* operator[](uint32_t i) {
    return aux_garbled_circuits_data[permuted_ids_inv[exec_offset + i]].GetAuxArray();
  }
};

void WeightedRandomString(uint8_t res[], int weight, int res_length, osuCrypto::PRNG& rnd, bool negate_probability);

void FindBestSingleParams(uint32_t num_buckets, uint32_t& res_bucket, long double& check_val, bool& check_val_negate);

void MaxSingleProb(int b, uint32_t num_buckets, mpf_class& curr_p, mpf_class& max_prob);

void FindBestMajorityParams(uint32_t num_buckets, uint32_t& res_bucket, long double& check_val, bool& check_val_negate, uint32_t catch_reciproc_prob);
void MaxProbMajority(int b, uint32_t num_buckets, mpf_class& curr_p, mpf_class& max_prob, uint32_t catch_reciproc_prob);

void ComputeCheckFraction(int check_frac, int num_items, float& final_rep_frac, bool negate_check_probability);

void ComputeIndices(uint32_t num_circuits, Circuit& circuit, uint32_t& num_inp_keys, uint32_t& num_out_keys, uint32_t& num_deltas, uint32_t& num_commit_keys, uint32_t& num_base_keys, uint32_t& input_keys_idx, uint32_t& output_keys_idx, uint32_t& deltas_idx);

void PickBestSingleBucketParams(uint32_t num_buckets, uint32_t& bucket_size, long double& check_factor, bool& negate_check_factor);

void PickBestMajorityBucketParams(uint32_t num_buckets, uint32_t& bucket_size, long double& check_factor, bool& negate_check_factor);

void PickBestMajorityAuthParams(uint32_t num_buckets, uint32_t& auth_size, long double& check_factor, bool& negate_check_factor);

#endif /* DUPLO_DUPLO_H_ */