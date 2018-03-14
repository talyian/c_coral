#pragma once
#include "llvm-c/Core.h"
#include "../core/expr.hh"
#include <iostream>
#include <map>
#include <string>
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
	  std::map<ast::BaseExpr *, LLVMValueRef> * info;

	  int rawPointer = 0; // a hack to implement the derefi intrinsic
	  int returns = 0; // a hack to track if a block has a terminator
	  LLVMValueRef compile(ast::BaseExpr * e) {
		out = 0;
		e->accept(this);
		if (!out) std::cerr << "Failed to compile " << ast::ExprNameVisitor::of(e) << std::endl;
		return out; }

	  LLVMFunctionCompiler(
		LLVMContextRef context,
		LLVMModuleRef module,
		LLVMBuilderRef builder,
		std::map<ast::BaseExpr *, LLVMValueRef> * info,
		ast::Func *)
		: context(context), module(module), builder(builder), info(info) { }

	  virtual std::string visitorName() { return "FunctionCompiler"; }
      LLVMTypeRef LLVMTypeFromCoral(coral::type::Type * t);

      void visit(ast::Func * expr);
	  void visit(ast::IfExpr * expr);
	  void visit(ast::Call * expr);
	  void visit(ast::IntLiteral * expr);
	  void visit(ast::FloatLiteral * expr);
	  void visit(ast::StringLiteral * expr);
	  void visit(ast::Var * expr);
	  void visit(ast::Return * expr);
	  void visit(ast::BinOp * expr);
	  void visit(ast::Block * expr);
	  void visit(ast::Let * expr);
	  void visit(ast::While * expr);
	  void visit(ast::Set * expr);
	  void visit(ast::Member * expr);
	  void visit(ast::Comment * w);
	  void visit(ast::TupleLiteral * w);
	};
  }
}
