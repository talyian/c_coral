#include "llvm-c/Core.h"
#include "llvm-c/ExecutionEngine.h"
#include "llvm-c/Support.h"
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

	  LLVMLoadLibraryPermanently("pcre");
	  LLVMLoadLibraryPermanently(0);
	  LLVMLoadLibraryPermanently("asdf");

	  triple = LLVMGetDefaultTargetTriple();
	  if (LLVMGetTargetFromTriple(triple, &target, &err))
	  { std::cerr << err << "\n"; return; }
	  if (LLVMCreateExecutionEngineForModule(&engine, module, &err))
	  { std::cerr << err << "\n"; return; }
	}

	void JIT::runMain() {
	  GetFunctionPointer<int (*)()>("main")();
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
