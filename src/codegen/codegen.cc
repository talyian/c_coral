#include "../core/expr.hh"
#include <cstdio>

#include "codegen.hh"
#include "LLVMJit.hh"

extern "C" const char * itoa(int i) {
  return (new std::string(std::to_string(i)))->c_str();
}

namespace coral {

  void Run(const char * path) {
    CodeProcessingUnit cc(path);
    cc.showSource();
    cc.showIR();
    std::cout << "Running: " << path << "\n";
    cc.runJIT();
  }

  CodeProcessingUnit::CodeProcessingUnit(const char * path) {
	FILE * f = fopen(path, "r");
	if (!f) { std::cout << "Couldn't Open " << path << "\n"; return; }
	fclose(f);
	auto parser =  coralParseModule(path);
	module = (ast::Module *)_coralModule(parser);
	analyzers::NameResolver nresolver(module);
	analyzers::TypeResolver tresolver(module);
	analyzers::ReturnInserter returner(module);
	compiler = new codegen::LLVMModuleCompiler(module);
  }
  void CodeProcessingUnit::showSource() { PrettyPrinter::print(module); }

  void CodeProcessingUnit::showIR() {
    std::cout << LLVMPrintModuleToString(compiler->llvmModule);
  }

  void CodeProcessingUnit::runJIT() {
    codegen::JIT jit(compiler->llvmModule);
    jit.runMain();
  }
}
