#include "llvm-c/Core.h"
#include "core/type.hh"

namespace coral{
  namespace llvm {
    LLVMTypeRef GetLLVMType(LLVMModuleRef module, LLVMContextRef context, type::Type * t, bool &success);
    size_t SizeOfType(coral::type::Type * t);
  }
}
