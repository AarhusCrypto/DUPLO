add_executable(TestDUPLO
  run-all-tests.cpp
  test-circuit-parser.cpp
  test-garbled-circuit.cpp
  test-duplo-setup.cpp
  test-duplo-preprocess-bucketing.cpp
  test-duplo-preprocess-aux-data.cpp
  test-duplo-soldering.cpp)
target_link_libraries(TestDUPLO DUPLO gtest_main gtest)