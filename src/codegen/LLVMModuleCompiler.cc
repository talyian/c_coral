#include "llvm-c/Core.h"
#include "../core/expr.hh"
#include "LLVMFunctionCompiler.hh"
#include "LLVMModuleCompiler.hh"

coral::codegen::LLVMModuleCompiler::LLVMModuleCompiler(ast::Module * m) {
  llvmContext = LLVMContextCreate();
  llvmModule = LLVMModuleCreateWithNameInContext("module", llvmContext);
  llvmBuilder = LLVMCreateBuilderInContext(llvmContext);
  m->accept(this);
}
