cmake_minimum_required (VERSION 3.5)
project (coral)
add_subdirectory(typegraph)

include_directories(.)
# LLVM settings
find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs support core irreader
  # Analysis
  # BitReader
  # Core
  # ExecutionEngine
  # InstCombine
  # Interpreter
  # MC
  # MCDisassembler
  MCJIT
  # Object
  # OrcJIT
  Passes
  # RuntimeDyld
  # ScalarOpts
  # Support
  # Target
  # TransformUtils
  # native
  nativecodegen
  )

set(CMAKE_CXX_COMPILER "clang++-5.0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Source Files
add_library(core OBJECT
  core/prettyprinter.cc core/expr.cc utils/opts.cc core/exprVisitor.cc)

find_package(BISON)
bison_target(bisonParser parser/bisonParser.yy ${CMAKE_SOURCE_DIR}/parser/bisonParser.tab.cc)
find_package(FLEX)
flex_target(flexLexer parser/flexLexer.l ${CMAKE_SOURCE_DIR}/parser/flexLexer.cc)
add_library(parser OBJECT
  parser/lexer.cc parser/parser.cc ${FLEX_flexLexer_OUTPUTS} ${BISON_bisonParser_OUTPUT_SOURCE})

add_library(analyzers OBJECT
  analyzers/TypeResolver.cc analyzers/NameResolver.cc analyzers/ReturnInserter.cc
  analyzers/ImportResolver.cc
  typegraph/typegraph.cc typegraph/constraints.cc typegraph/solution.cc
)
add_library(codegen OBJECT
  codegen/codegen.cc
  codegen/LLVMFunctionCompiler.cc
  codegen/LLVMModuleCompiler.cc
  codegen/LLVMJit.cc
)

add_executable (coral-parse
  $<TARGET_OBJECTS:analyzers>
  $<TARGET_OBJECTS:parser>
  $<TARGET_OBJECTS:core>
  parser/main_parse.cc )

add_executable (coral-jit
  $<TARGET_OBJECTS:analyzers>
  $<TARGET_OBJECTS:parser>
  $<TARGET_OBJECTS:core>
  $<TARGET_OBJECTS:codegen>
  codegen/main_jit.cc)
target_link_libraries(coral-jit ${llvm_libs} pcre2-8)

add_executable (coral-test
  $<TARGET_OBJECTS:analyzers>
  $<TARGET_OBJECTS:parser>
  $<TARGET_OBJECTS:core>
  $<TARGET_OBJECTS:codegen>
  tests/runner/codegen.cc
  tests/runner/parser.cc
  tests/runner/main_test.cc)
target_link_libraries(coral-test ${llvm_libs})

add_custom_target(run-test ${CMAKE_BINARY_DIR}/bin/coral-test
  DEPENDS coral-test
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_custom_target(wip ${CMAKE_BINARY_DIR}/bin/coral-jit tests/cases/wip.coral
  DEPENDS coral-jit
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
