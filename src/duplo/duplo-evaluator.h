#ifndef DUPLO_DUPLOEVAL_H_
#define DUPLO_DUPLOEVAL_H_

#include "duplo/duplo.h"
#include "garbling/eval-garbled-circuit.h"
#include "libOTe/TwoChooseOne/KosOtExtReceiver.h"
#include "libOTe/TwoChooseOne/KosDotExtReceiver.h"
#include "split-commit/split-commit-rec.h"

class DuploEvaluator : public Duplo {
public:
  DuploEvaluator(uint8_t seed[], uint32_t num_max_parallel_execs = 1, bool ram_only = false);
  ~DuploEvaluator();

  void Connect(std::string ip_address, uint16_t port);
  
  void Setup();

  void PreprocessComponentType(std::string component_type, Circuit& circuit, uint32_t num_buckets, uint32_t num_parallel_execs = 1, BucketType bucket_type = SINGLE);

  void PrepareComponents(uint64_t num_input_auth_buckets, uint32_t num_parallel_execs = 1);

  void Build(ComposedCircuit& composed_circuit, uint32_t num_parallel_execs = 1);

  void Evaluate(ComposedCircuit& composed_circuit, osuCrypto::BitVector& inputs, BYTEArrayVector& output_keys, uint32_t num_parallel_execs = 1);

  void DecodeKeys(ComposedCircuit& composed_circuit, std::vector<std::vector<uint32_t>>& const_outputs, std::vector<std::vector<uint32_t>>& eval_outputs, BYTEArrayVector& output_keys, std::vector<osuCrypto::BitVector>& outputs, bool non_interactive, uint32_t num_parallel_execs = 1);

  std::vector<osuCrypto::block> commit_seed_OTs;
  osuCrypto::BitVector commit_seed_choices;

private:
  std::vector<std::unique_ptr<osuCrypto::OtExtReceiver>> dot_receivers;
  std::vector<SplitCommitReceiver> commit_receivers;

  void CommitReceiveAndCutAndChoose(uint32_t exec_id, Circuit& circuit, uint32_t exec_num_total_garbled, uint32_t check_factor, bool negate_check_factor, uint32_t exec_eval_circuits_from, uint32_t exec_eval_circuits_to, std::vector<EvalGarbledCircuit>& aux_garbled_circuits_data, BYTEArrayVector& eval_hash);

  void BucketAndReceiveEvalCircuits(std::string component_type, uint32_t exec_id, Circuit& circuit, uint32_t bucket_size, std::vector<uint32_t>& permuted_eval_ids_inv, uint32_t exec_buckets_from, uint32_t exec_buckets_to, std::vector<EvalGarbledCircuit>& aux_garbled_circuits_data, BYTEArrayVector& eval_hash);

  void PrepareOutputWireAuthenticators(uint32_t num_parallel_execs);

  void PrepareInputWireAuthenticators(uint64_t num_input_auth_buckets, uint32_t num_parallel_execs);

  void CommitCircuitAuthAndCutAndChoose(uint32_t exec_id, uint32_t exec_num_auths, uint32_t check_factor, bool negate_check_factor, uint32_t exec_eval_auths_from, uint32_t exec_eval_auths_to, std::vector<BYTEArrayVector>& eval_auths, BYTEArrayVector& aux_auth_data, std::vector<uint32_t>& aux_auth_ids, uint8_t aux_auth_delta_data[], std::tuple<std::mutex&, std::condition_variable&, std::atomic<bool>&>& delta_signal);

  void CommitInputAuthAndCutAndChoose(uint32_t exec_id, uint32_t exec_num_total_garbled, uint32_t check_factor, bool negate_check_factor, uint32_t exec_eval_circuits_from, uint32_t exec_eval_circuits_to, std::vector<EvalGarbledCircuit>& aux_garbled_circuits_data);

  void BucketAllCircuitAuths(uint32_t exec_id, uint32_t auth_size, std::vector<uint32_t>& permuted_eval_ids_inv, std::vector<int>& session_circuit_buckets_from, std::vector<int>& session_circuit_buckets_to, std::vector<BYTEArrayVector>& eval_auths, BYTEArrayVector& aux_auth_data, std::vector<uint32_t>& aux_auth_ids, uint8_t aux_auth_delta_data[]);

  void BucketAllInputAuths(uint32_t exec_id, uint32_t input_auth_size, std::vector<uint32_t>& permuted_eval_ids_inv, uint32_t exec_input_buckets_from, uint32_t exec_input_buckets_to, std::vector<EvalGarbledCircuit>& input_aux_auth_data);

  void ReceiveAndStoreComponentSolderings(std::string component_type, uint32_t exec_id, Circuit& circuit, uint32_t bucket_size, PermutedIndex<EvalGarbledCircuit>& exec_permuted_aux_info, uint32_t exec_buckets_from, uint32_t exec_buckets_to, BYTEArrayVector& exec_received_garbled_tables);

  void ReceiveAndStoreComposedSolderings(uint32_t exec_id, ComposedCircuit& composed_circuit, uint32_t inp_wire_components_from, uint32_t inp_wire_components_to);

  void DecommitInpPermBits(uint32_t exec_id, uint32_t offset, uint32_t eval_inputs_from, uint32_t eval_inputs_to);

  void PreprocessEvalInputOTs(uint32_t exec_id, uint32_t eval_inputs_from, uint32_t eval_inputs_to);

  void PreprocessBlindOutPermBits(uint32_t exec_id, uint32_t eval_outputs_from, uint32_t eval_outputs_to);

  void GetInputs(uint32_t exec_id, ComposedCircuit& composed_circuit, osuCrypto::BitVector& plain_inputs, uint32_t const_inputs_from, uint32_t const_inputs_to, uint32_t eval_inputs_from, uint32_t eval_inputs_to, BYTEArrayVector& wire_keys);

  void AuthenticateInputKeys(uint32_t exec_id, BYTEArrayVector& wire_keys, uint32_t inputs_from, uint32_t inputs_to);

  void EvaluateComponent(uint32_t exec_id, ComposedCircuit& composed_circuit, uint32_t components_from, uint32_t components_to, uint32_t exec_layer_num, BYTEArrayVector& output_keys, bool single_eval);

  void EvaluateBucketParallel(uint32_t exec_id, std::pair<std::string, uint32_t>& component, uint8_t input_keys[], uint8_t output_keys[]);

  void EvaluateBucketSerial(uint32_t exec_id, std::pair<std::string, uint32_t>& component, uint8_t input_keys[], uint8_t output_keys[]);

  bool DecommitOutPermBits(uint32_t exec_id, std::vector<std::pair<std::string, uint32_t>>& output_components, std::vector<std::vector<uint32_t>>& eval_outputs, uint32_t components_from, uint32_t components_to, std::vector<osuCrypto::BitVector>& output_decodings, bool non_interactive);
};

#endif /* DUPLO_DUPLOEVAL_H_ */