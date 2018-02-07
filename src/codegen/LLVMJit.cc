#include "llvm-c/Core.h"
#include "llvm-c/ExecutionEngine.h"
#include "LLVMModuleCompiler.hh"

namespace coral {
  namespace codegen {
	void LLVMRunJIT(LLVMModuleRef module) {
	  LLVMInitializeNativeTarget();
	  LLVMInitializeNativeAsmPrinter();
	  LLVMLinkInMCJIT();

	  char * triple = LLVMGetDefaultTargetTriple();
	  LLVMTargetRef target = 0;
	  LLVMExecutionEngineRef engine = 0;
	  char * err = 0;

	  if (LLVMGetTargetFromTriple(triple, &target, &err))
	  { std::cerr << err << "\n"; return; }
	  if (LLVMCreateExecutionEngineForModule(&engine, module, &err))
	  { std::cerr << err << "\n"; return; }

	  // LLVMValueRef func = 0;
	  // LLVMFindFunction(engine, "main", &func);
	  // LLVMRunFunction(engine, func, 0, 0);
	  // LLVMDeleteFunction(func);
	  // LLVMFreeMachineCodeForFunction(engine, func);

	  auto main = (int (*)())LLVMGetFunctionAddress(engine, "main");
	  main();
	  LLVMDisposeMessage(triple);
	  LLVMDisposeExecutionEngine(engine);
	};
  }
}
