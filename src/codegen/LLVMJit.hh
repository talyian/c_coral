#pragma once
#include "llvm-c/Core.h"

namespace coral {
  namespace codegen {
	void LLVMRunJIT(LLVMModuleRef module);
  }
}
