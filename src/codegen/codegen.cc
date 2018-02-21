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
	parser =  coralParseModule(path);
	module = (ast::Module *)_coralModule(parser);
	analyzers::NameResolver nresolver(module);
	analyzers::TypeResolver tresolver(module);
	analyzers::ReturnInserter returner(module);
	compiler = new codegen::LLVMModuleCompiler(module);
    llvmModule = compiler->llvmModule;

    std::cerr << COL_LIGHT_YELLOW;
    LLVMVerifyModule(llvmModule, LLVMPrintMessageAction, 0);
    std::cerr << COL_CLEAR << "\n";

    auto mpass = LLVMCreatePassManager();
    auto pmb = LLVMPassManagerBuilderCreate();
    LLVMPassManagerBuilderSetOptLevel(pmb, 1);
    LLVMPassManagerBuilderPopulateModulePassManager(pmb, mpass);
    LLVMRunPassManager(mpass, llvmModule);
    LLVMPassManagerBuilderDispose(pmb);
    LLVMDisposePassManager(mpass);
    // LLVMPassManagerBuilderPopulateFunctionPassManager(pmb, fpass);
    // auto fpass = LLVMCreateFunctionPassManagerForModule(llvmModule);
  }
  void CodeProcessingUnit::showSource() { PrettyPrinter::print(module); }

  void CodeProcessingUnit::showIR() {
    auto msg = LLVMPrintModuleToString(llvmModule);
    std::cout << msg;
    LLVMDisposeMessage(msg);

  }

  void CodeProcessingUnit::runJIT() {
    codegen::JIT jit(llvmModule);
    jit.runMain();
  }
}
