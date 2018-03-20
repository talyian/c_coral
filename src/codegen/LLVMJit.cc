#include "llvm-c/Core.h"
#include "llvm-c/ExecutionEngine.h"
#include "llvm-c/Support.h"
#include "llvm-c/BitWriter.h"
#include "llvm-c/TargetMachine.h"
#include "LLVMModuleCompiler.hh"
#include "LLVMJit.hh"

// extern "C" void pcre_compile(const char * pat, int len, char **err, int* offet, int a) {
//   printf("fake pcre stub\n\n");
// }


extern "C" void hello_world() {
  printf("HEllo World|\n");
}
namespace coral {
  namespace codegen {
	JIT::JIT(LLVMModuleRef module) {
	  LLVMInitializeNativeTarget();
	  LLVMInitializeNativeAsmPrinter();
	  LLVMLinkInMCJIT();
	  triple = LLVMGetDefaultTargetTriple();
	  if (LLVMGetTargetFromTriple(triple, &target, &err))
	  { std::cerr << err << "\n"; return; }
	  if (LLVMCreateExecutionEngineForModule(&engine, module, &err))
	  { std::cerr << err << "\n"; return; }
	  this->module = module;
	}

	void JIT::compileObjectFile(const char * path, const char * triple) {
	  char * err = 0;
	  machine = LLVMCreateTargetMachine(
		target, triple ? triple : this->triple, "", "",
		LLVMCodeGenLevelAggressive, LLVMRelocDefault, LLVMCodeModelDefault);
	  LLVMTargetMachineEmitToFile(
		machine, module, (char *)path, LLVMObjectFile, &err);
	}

	void JIT::runMain() {
      LLVMValueRef fn;
      if (0 == LLVMFindFunction(engine, "..init", &fn)) {
        std::cerr << "Running Init\n";
        GetFunctionPointer<void (*)()>("..init")();
      }
      if (0 == LLVMFindFunction(engine, "main", &fn)) {
        GetFunctionPointer<int (*)()>("main")();
      }
	}

	JIT::~JIT() {
	  LLVMDisposeMessage(triple);
	  LLVMDisposeExecutionEngine(engine);
	}

	void LLVMRunJIT(LLVMModuleRef module) {
	  JIT jit(module);
	  jit.runMain();
	};
  }
}
