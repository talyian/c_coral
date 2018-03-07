cmake_minimum_required (VERSION 3.5)
project (coral)
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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Source Files
add_library(core OBJECT
  core/prettyprinter.cc core/expr.cc utils/opts.cc)

find_package(BISON)
bison_target(bisonParser parser/bisonParser.yy ${CMAKE_SOURCE_DIR}/parser/bisonParser.tab.cc)
find_package(FLEX)
flex_target(flexLexer parser/flexLexer.l ${CMAKE_SOURCE_DIR}/parser/flexLexer.cc)
add_library(parser OBJECT
  parser/lexer.cc parser/parser.cc ${FLEX_flexLexer_OUTPUTS} ${BISON_bisonParser_OUTPUT_SOURCE})

add_library(analyzers OBJECT
  analyzers/TypeResolver.cc analyzers/NameResolver.cc analyzers/ReturnInserter.cc
  analyzers/ImportResolver.cc
  analyzers/typegraph/constraint.cc analyzers/typegraph/TypeGraph.cc
  analyzers/typegraph/TypeGraph.cc
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
  codegen/main_jit.cc ) #
target_link_libraries(coral-jit ${llvm_libs})

add_executable (coral-test
  $<TARGET_OBJECTS:analyzers>
  $<TARGET_OBJECTS:parser>
  $<TARGET_OBJECTS:core>
  $<TARGET_OBJECTS:codegen>
  tests/runner/codegen.cc
  tests/runner/parser.cc
  tests/runner/main_test.cc)
target_link_libraries(coral-test ${llvm_libs})
