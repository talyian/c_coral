#include <iostream>
#include <llvm-c/Core.h>

#include "../parsing/lexer.hh"
#include "../core/treeprinter.hh"
#include "../codegen/moduleCompiler.hh"
#include "../codegen/jitEngine.hh"

using namespace coral;
using namespace std;

int main(int argc, char *argv[]) {
  if (argc < 2) {
	cerr << "Usage: " << argv[0] << " [filename.coral]\n";
	return 1;
  }

  auto filename = argv[1];
  CompilationUnit cc(std::vector<Module *> {
	parse_file("samples/prelude/extern.coral"),
    parse_file(filename),
  });
  cc.run_jit();
}
