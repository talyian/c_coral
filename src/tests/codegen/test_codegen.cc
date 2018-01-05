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
  cout << "--[Source]-----------------------------------\n";
  cout << cc.get_source();
  cout << "--[LLVM IR]----------------------------------\n";
  cout << cc.get_ir();
  cout << "--[Output]------------------------------------\n";
  cc.run_jit();
  cout << "\n----------------------------------------------\n";
  printf("%30s\tOK\n", filename);
}

void runCodegenTests() {
  cout << "\n\e[1;32m" << "==[ Codegen Tests ]==========================\e[0m\n";
  // checkFile("tests/core/hello_world.coral");
  // checkFile("tests/core/fizzbuzz.coral");
  // checkFile("tests/core/functions.coral");
  checkFile("tests/codegen/wip.coral");
  cout << "----------[ "<< passed <<" / "<< total <<" ]----------\n";
}

int main() {
  runCodegenTests();
}
