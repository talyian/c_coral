cmake_minimum_required(VERSION 3.5)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

add_executable(typegraph_test
  constraints.cc
  solution.cc
  typegraph.cc
  tests/test_graph.cc)
target_compile_features(typegraph_test PUBLIC cxx_defaulted_move_initializers)

add_custom_target(typegraph_check ${CMAKE_BINARY_DIR}/typegraph/typegraph_test DEPENDS typegraph_test)
