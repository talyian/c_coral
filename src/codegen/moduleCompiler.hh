#pragma once

#include "../core/expr.hh"
#include "../core/type.hh"

#include <llvm-c/Core.h>

#define TOLLVMTYPE(t) ToLLVMType(t, context).out
namespace coral {
  class ToLLVMType : public coral::TypeVisitor {
  public:
	LLVMContextRef context;
	LLVMTypeRef out = 0;
	ToLLVMType(coral::Type * t, LLVMContextRef context) : context(context) { t->accept(this); }
	void visit(FuncType * m);
	void visit(VoidType * m);
	void visit(IntType * m);
	void visit(PtrType * m);
  };

  /*
	Module Compiler handles funcdefs,
	global variables,
	and externs. It is expected that any other expressions have been cleaned up
	by a preceding MainFuncPass.
  */
  class ModuleCompiler : public Visitor {
  public:
	LLVMContextRef context;
	LLVMModuleRef llvmModule;
	LLVMValueRef llvmFunction = 0;
	LLVMBasicBlockRef llvmBB = 0;
	LLVMBuilderRef llvmBuilder = 0;
	ModuleCompiler(coral::Module * m);
	void visit(FuncDef * m);
	void visit(Extern * m);
	void visit(Let * m);
	std::string getIR();
  };


  class ExpressionCompiler : public Visitor {
  public:
	LLVMContextRef context;
	LLVMModuleRef llvmModule;
	LLVMValueRef llvmFunction = 0;
	LLVMBasicBlockRef llvmBB = 0;
	LLVMBuilderRef llvmBuilder = 0;
	ExpressionCompiler(coral::ModuleCompiler * m) : Visitor("exprc ") {
	  context = m->context;
	  llvmModule = m->llvmModule;
	  llvmFunction = m->llvmFunction;
	  llvmBuilder = m->llvmBuilder;
	}
	void visit(BlockExpr * e) { for (auto &line: e->lines) line->accept(this); }
  };


}
