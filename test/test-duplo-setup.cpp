#include "test-duplo.h"

TEST_F(TestDuplo, Setup) {
  

  std::future<void> ret_const = std::async(std::launch::async, [this]() {
    duplo_const.Connect(default_ip_address, default_port);
    duplo_const.Setup();
  });

  std::future<void> ret_eval = std::async(std::launch::async, [this]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    duplo_eval.Connect(default_ip_address, default_port);
    duplo_eval.Setup();
  });

  ret_const.wait();
  ret_eval.wait();
  

  //Check initial OT extension
  for (int i = 0; i < NUM_COMMIT_SEED_OT; ++i) {
    if (duplo_eval.commit_seed_choices[i]) {
      ASSERT_TRUE(compare128(duplo_eval.commit_seed_OTs[i], duplo_const.commit_seed_OTs[i][1]));
    } else {
      ASSERT_TRUE(compare128(duplo_eval.commit_seed_OTs[i], duplo_const.commit_seed_OTs[i][0]));
    }
  }
}