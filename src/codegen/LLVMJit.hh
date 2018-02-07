#pragma once
#include "llvm-c/Core.h"
#include "llvm-c/ExecutionEngine.h"

namespace coral {
  namespace codegen {
	class JIT {
	public:
	  char * triple = 0;
	  LLVMTargetRef target = 0;
	  LLVMExecutionEngineRef engine = 0;
	  char * err = 0;

	  JIT(LLVMModuleRef module);
	  ~JIT();

	  void runMain();

	  template <typename T>
	  T GetFunctionPointer(const char * name) { return (T)LLVMGetFunctionAddress(engine, name); }
	};
	void LLVMRunJIT(LLVMModuleRef module);
  }
}
