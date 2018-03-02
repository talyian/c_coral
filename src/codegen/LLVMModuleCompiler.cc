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

void coral::codegen::LLVMModuleCompiler::visit(ast::Func * f) {
  LLVMFunctionCompiler fcc(
    llvmContext,
    llvmModule,
    llvmBuilder,
    &info,
    f);
  fcc.visit(f);
}

void coral::codegen::LLVMModuleCompiler::visit(ast::Extern *e) {
  auto name = e->name;
  auto type = e->type.get();

  LLVMFunctionCompiler functionCompiler(llvmContext, llvmModule, llvmBuilder, &info, 0);
  auto llvmType = functionCompiler.LLVMTypeFromCoral(type);
  if (type->name == "Func") {
    info[e] = out = LLVMAddFunction(llvmModule, name.c_str(), llvmType);
  } else {
    // TODO: extern global var?
  }
}

void coral::codegen::LLVMModuleCompiler::visit(ast::Tuple * tuple) {
  LLVMFunctionCompiler functionCompiler(llvmContext, llvmModule, llvmBuilder, &info, 0);
  auto type = LLVMStructCreateNamed(llvmContext, tuple->name.c_str());
  std::vector<LLVMTypeRef> fields;
  for(auto &f: tuple->fields)
    fields.push_back(functionCompiler.LLVMTypeFromCoral(f->type.get()));
  LLVMStructSetBody(type, &fields[0], fields.size(), true);
}
