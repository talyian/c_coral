#include <iostream>
#include <llvm-c/Core.h>

#include "../parsing/lexer.hh"
#include "../core/treeprinter.hh"
#include "../codegen/moduleCompiler.hh"
#include "../codegen/jitEngine.hh"

// #include "../passes/InferTypesPass.cc"
// #include "../passes/ReturnInsertPass.cc"
// #include "../passes/UnclassifyPass.cc"
// #include "../passes/MainFuncPass.cc"

using namespace coral;
using namespace std;

int main(int argc, char *argv[]) {
  if (argc < 2) {
	cerr << "Usage: " << argv[0] << " [filename.coral]\n";
	return 1;
  }
  auto filename = argv[1];
  cout << ir_modules(std::vector<Module *> {
	parse_file("samples/prelude/extern.coral"),
    parse_file(filename),
  });
}
