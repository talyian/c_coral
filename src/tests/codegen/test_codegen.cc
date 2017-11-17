#include "parsing/lexer.hh"
#include "codegen/jitEngine.hh"
#include <iostream>
using namespace std;

int passed = 0, total = 0;

void checkFile(const char * filename) {
  coral::CompilationUnit cc(std::vector<Module *> {
	  parse_file("samples/prelude/extern.coral"),
	  parse_file(filename),
  });

  cc.generate_llvm_modules();
  cout << "----------------------------------------\n";
  cout << cc.get_source();
  cout << "----------------------------------------\n";
  cout << cc.get_ir();
  // cc.run_jit();
  printf("%30s\tOK\n", filename);
}

void runCodegenTests() {
  cout << "----------[ Codegen Tests ]----------\n";
  // checkFile("tests/core/hello_world.coral");
  // checkFile("tests/core/fizzbuzz.coral");
  // checkFile("tests/core/functions.coral");
  checkFile("tests/codegen/wip.coral");
  cout << "----------[ "<< passed <<" / "<< total <<" ]----------\n";
}

int main() {
  runCodegenTests();
}
