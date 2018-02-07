#include "llvm-c/Core.h"
#include "llvm-c/ExecutionEngine.h"
#include "LLVMModuleCompiler.hh"
#include "LLVMJit.hh"

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
