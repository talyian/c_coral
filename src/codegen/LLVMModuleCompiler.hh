#pragma once

#include "llvm-c/Core.h"
#include "../core/expr.hh"
#include "LLVMFunctionCompiler.hh"

namespace coral {
  namespace codegen {

	class LLVMModuleCompiler : public ast::ExprVisitor {
	public :
	  LLVMContextRef llvmContext;
	  LLVMModuleRef llvmModule;
	  LLVMBuilderRef llvmBuilder;
	  ast::Module * astModule;

	  std::map<ast::BaseExpr *, LLVMValueRef> info;
	  // output values
	  LLVMValueRef out = 0;
	  LLVMBasicBlockRef outBlock = 0;

	  LLVMModuleCompiler(ast::Module * m);
      std::string visitorName() { return "ModuleCompiler"; }
	  std::string GetIR() {
	    return std::string(LLVMPrintModuleToString(llvmModule));
	  }

	  virtual ~LLVMModuleCompiler() {
		LLVMDisposeBuilder(llvmBuilder);
		LLVMContextDispose(llvmContext);
	  }

	  void visit(ast::Module * m) {
		for (auto && line : m->body->lines) if (line) line->accept(this);
	  }
      void visit(ast::Let * e) {
        e->value->accept(this);
        info[e] = out;
      }
      void visit(ast::IntLiteral * e) {
        out = LLVMConstInt(LLVMIntType(32), std::stol(e->value), false);
        info[e] = out;
      }
	  void visit(ast::Comment *) { }
	  void visit(ast::Func * f);
      void visit(ast::Extern *e);
      void visit(ast::Tuple *e);
      void visit(ast::Union *e);
	};

  }
}
