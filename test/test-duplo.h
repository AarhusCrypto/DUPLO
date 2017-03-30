#include "test.h"

#include "duplo/duplo-constructor.h"
#include "duplo/duplo-evaluator.h"

#include "duplo-util/storage.h"

static uint32_t num_max_parallel_execs = 4;

class TestDuplo : public ::testing::Test {

protected:
  DuploConstructor duplo_const;
  DuploEvaluator duplo_eval;

  uint32_t num_execs_components = 1;
  uint32_t num_execs_auths = 1;
  uint32_t num_execs_online = 1;

  TestDuplo() :
    duplo_const(duplo_constant_seeds[0], num_max_parallel_execs),
    duplo_eval(duplo_constant_seeds[1], num_max_parallel_execs) {
  };
};