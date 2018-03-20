#include "llvm-c/Core.h"
#include "../core/expr.hh"
#include "LLVMFunctionCompiler.hh"
#include "LLVMModuleCompiler.hh"
#include "LLVMTypeMap.hh"

#include <algorithm>

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

void coral::codegen::LLVMModuleCompiler::visit(ast::Union * unionDef) {
  LLVMFunctionCompiler functionCompiler(llvmContext, llvmModule, llvmBuilder, &info, 0);
  auto llvmType = LLVMStructCreateNamed(llvmContext, unionDef->name.c_str());
  size_t byte_length = 0;
  int i = 0;
  for(auto &union_case: unionDef->cases) {
    byte_length = std::max(byte_length, llvm::SizeOfType(union_case->type.get()));
    i++;
  }

  // std::cerr << i << " Union Byte Length " << byte_length << "\n";

  auto fields = new LLVMTypeRef [2];
  fields[0] = LLVMInt16TypeInContext(llvmContext);
  fields[1] = LLVMArrayType(LLVMInt8TypeInContext(llvmContext), byte_length);
  LLVMStructSetBody(llvmType, fields, 2, false);

  for(size_t field_index = 0; field_index < unionDef->cases.size(); field_index++) {
    auto union_case = unionDef->cases[field_index].get();
    auto constructor_name = unionDef->name + "." + union_case->name;
    auto case_type = functionCompiler.LLVMTypeFromCoral(union_case->type.get());

    auto func = LLVMAddFunction(
      llvmModule,
      constructor_name.c_str(),
      LLVMFunctionType(llvmType, &case_type, 1, false));

    info[union_case] = func;

    auto basic_block = LLVMAppendBasicBlock(func, "entry");
    LLVMPositionBuilderAtEnd(llvmBuilder, basic_block);
    auto new_inst = LLVMBuildAlloca(llvmBuilder, llvmType, "newInst");

    LLVMValueRef indices[2] = {
      LLVMConstInt(LLVMInt32TypeInContext(llvmContext), 0, false),
      LLVMConstInt(LLVMInt32TypeInContext(llvmContext), 0, false),
    };

    // std::cerr << "Creating constructor for field " << field_index << "\n";
    LLVMBuildStore(
      llvmBuilder,
      LLVMConstInt(LLVMInt16TypeInContext(llvmContext), field_index, false),
      LLVMBuildGEP(llvmBuilder, new_inst, indices, 2, ""));

    indices[1] = LLVMConstInt(LLVMInt32TypeInContext(llvmContext), 1, false);
    auto value = LLVMGetParam(func, 0);
    auto addr = LLVMBuildGEP(llvmBuilder, new_inst, indices, 2, "");
    auto typed_addr = LLVMBuildBitCast(
      llvmBuilder, addr,
      LLVMPointerType(LLVMTypeOf(value), 0), "");
    LLVMBuildStore(llvmBuilder, value, typed_addr);
    LLVMBuildRet(llvmBuilder, LLVMBuildLoad(llvmBuilder, new_inst, ""));
  }
  LLVMAddGlobal(llvmModule, llvmType, unionDef->name.c_str());
}
