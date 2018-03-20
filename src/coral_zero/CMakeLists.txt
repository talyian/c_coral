cmake_minimum_required (VERSION 3.5)
project (coral_zero)

set(CMAKE_CXX_COMPILER "clang++-5.0")
# We're compiling with -g -rdynamic to get rich backtrace symbols
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

add_executable (coral-zero
  main.cc)

add_custom_target (zero ${CMAKE_BINARY_DIR}/coral_zero/coral-zero /home/jimmy/coral/src//coral_zero/main.c0.json
  DEPENDS coral-zero
  WORKING_DIRECTORY ${CMAKE_SRC_DIR})
