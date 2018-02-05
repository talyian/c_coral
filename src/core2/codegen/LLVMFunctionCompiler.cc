#include "llvm-c/Core.h"

#include "LLVMFunctionCompiler.hh"
#include "../core/expr.hh"

void coral::codegen::LLVMFunctionCompiler::visit(ast::Func * expr) {
  function = LLVMAddFunction(
	module,
	expr->name.c_str(),
	LLVMFunctionType(
	  LLVMVoidTypeInContext(context),
	  0,
	  0,
	  false));
  basic_block = LLVMAppendBasicBlock(function, "entry");
  LLVMPositionBuilderAtEnd(builder, basic_block);
  expr->body->accept(this);
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::IfExpr * expr) {
  auto thenblock = LLVMAppendBasicBlock(function, "b");
  auto elseblock = LLVMAppendBasicBlock(function, "a");
  out = LLVMBuildCondBr(
  	builder,
  	compile(expr->cond.get()),
  	thenblock,
  	elseblock);
  LLVMPositionBuilderAtEnd(builder, thenblock);
  expr->ifbody->accept(this);
  LLVMPositionBuilderAtEnd(builder, elseblock);
  expr->elsebody->accept(this);
  // LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, false));
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::IntLiteral * expr) {
  out = LLVMConstInt(LLVMInt32Type(), std::stol(expr->value), false);
}
void coral::codegen::LLVMFunctionCompiler::visit(ast::Var * expr) {
  out = LLVMConstInt(LLVMInt32Type(), 0, false);
}
void coral::codegen::LLVMFunctionCompiler::visit(ast::BinOp * expr) {
  out = LLVMBuildICmp(builder, LLVMIntSLE, compile(expr->lhs.get()), compile(expr->rhs.get()), "");
}
void coral::codegen::LLVMFunctionCompiler::visit(ast::Return * expr) {
  out = LLVMBuildRet(builder, compile(expr->val.get()));
}
void coral::codegen::LLVMFunctionCompiler::visit(ast::Call * expr) {
  std::cout << "TODO: call\n";
}
void coral::codegen::LLVMFunctionCompiler::visit(ast::Block * expr) {
  for(auto && line : expr->lines) if (line) line->accept(this);
}
