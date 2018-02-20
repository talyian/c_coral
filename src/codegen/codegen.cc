#include "core/expr.hh"
#include "utils/ansicolor.hh"

#include "codegen.hh"
#include "LLVMJit.hh"
#include "llvm-c/Transforms/PassManagerBuilder.h"
#include "llvm-c/Analysis.h"

#include <cstdio>

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

    llvmModule = compiler->llvmModule;
    // auto fpass = LLVMCreateFunctionPassManagerForModule(llvmModule);
    auto mpass = LLVMCreatePassManager();
    auto pmb = LLVMPassManagerBuilderCreate();
    LLVMPassManagerBuilderSetOptLevel(pmb, 1);
    // LLVMPassManagerBuilderPopulateFunctionPassManager(pmb, fpass);
    LLVMPassManagerBuilderPopulateModulePassManager(pmb, mpass);
    LLVMRunPassManager(mpass, llvmModule);
  }
  void CodeProcessingUnit::showSource() { PrettyPrinter::print(module); }

  void CodeProcessingUnit::showIR() {
    std::cout << LLVMPrintModuleToString(llvmModule);
  }

  void CodeProcessingUnit::runJIT() {
    std::cerr << COL_LIGHT_YELLOW;
    LLVMVerifyModule(llvmModule, LLVMPrintMessageAction, 0);
    std::cerr << COL_CLEAR << "\n";
    codegen::JIT jit(llvmModule);
    jit.runMain();
  }
}
