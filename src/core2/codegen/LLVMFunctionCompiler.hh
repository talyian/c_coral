#include "llvm-c/Core.h"

#include "../core/expr.hh"
#include <iostream>
namespace coral {
  namespace codegen {
	class LLVMFunctionCompiler : public ast::ExprVisitor {
	public:
	  LLVMContextRef context;
	  LLVMModuleRef module;
	  LLVMBuilderRef builder;

	  LLVMValueRef function = 0;
	  LLVMBasicBlockRef basic_block = 0;

	  LLVMValueRef out = 0;
	  LLVMValueRef compile(ast::BaseExpr * e) {
		out = 0;
		e->accept(this);
		if (!out) std::cerr << "Failed to compile " << ast::ExprNameVisitor::of(e) << std::endl;
		return out; }

	  LLVMFunctionCompiler(
		LLVMContextRef context,
		LLVMModuleRef module,
		LLVMBuilderRef builder,
		ast::Func * func)
		: context(context), module(module), builder(builder) { }

	  void visit(ast::Func * expr);
	  void visit(ast::IfExpr * expr);
	  void visit(ast::Call * expr);
	  void visit(ast::IntLiteral * expr);
	  void visit(ast::Var * expr);
	  void visit(ast::Return * expr);
	  void visit(ast::BinOp * expr);
	  void visit(ast::Block * expr);
	};
  }
}
