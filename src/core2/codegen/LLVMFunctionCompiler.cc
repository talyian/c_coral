#include "llvm-c/Core.h"

#include "LLVMFunctionCompiler.hh"
#include "../core/expr.hh"

void coral::codegen::LLVMFunctionCompiler::visit(ast::Func * expr) {
  auto paramTypes = new LLVMTypeRef[expr->params.size()];
  for(ulong i=0; i<expr->params.size(); i++) {
	paramTypes[i] = LLVMInt32Type();
  }
  function = LLVMAddFunction(
	module,
	expr->name.c_str(),
	LLVMFunctionType(
	  LLVMInt32TypeInContext(context),
	  paramTypes,
	  expr->params.size(),
	  false));
  (*info)[expr] = function;
  for(ulong i=0; i<expr->params.size(); i++) {
	(*info)[expr->params[i].get()] = LLVMGetParam(function, i);
  }
  basic_block = LLVMAppendBasicBlock(function, "entry");
  LLVMPositionBuilderAtEnd(builder, basic_block);
  expr->body->accept(this);
  delete [] paramTypes;
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
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::IntLiteral * expr) {
  out = LLVMConstInt(LLVMInt32Type(), std::stol(expr->value), false);
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::StringLiteral * expr) {
  auto stringval = expr->getString();
  auto global = LLVMAddGlobal(module, LLVMArrayType(LLVMInt8Type(), stringval.size()), "");
  auto llval = LLVMConstStringInContext(context, stringval.c_str(), stringval.size(), false);
  LLVMSetInitializer(global, llval);
  out = global;
  // out = LLVMConstInt(LLVMInt32Type(), std::stol(expr->value), false);
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::Var * var) {
  out = 0;
  if (info->find(var->expr) == info->end()) {
	std::cerr << "Not Found: " << var->name << "\n";
	return;
  }
  switch (ast::ExprTypeVisitor::of(var->expr)) {
  case ast::ExprTypeKind::DefKind:
	out = info->find(var->expr)->second;
  	// std::cout << "codegening: var def " << ((ast::Def *)var->expr)->name << "\n";
	// std::cout << "saved: " << LLVMPrintValueToString(out) << "\n";
	return;
  case ast::ExprTypeKind::FuncKind:
  	// std::cout << "codegening: var fun " << ((ast::Func *)var->expr)->name << "\n";
	out = info->find(var->expr)->second;
	return;
  default:
  	std::cout << "var : " << var->name << " :: " << ast::ExprNameVisitor::of(var->expr) << "\n";
  	break;
  }
  out = LLVMConstInt(LLVMInt32Type(), 0, false);
}
void coral::codegen::LLVMFunctionCompiler::visit(ast::BinOp * expr) {
  if (expr->op == "-")
	out = LLVMBuildSub(builder, compile(expr->lhs.get()), compile(expr->rhs.get()), "");
  else if (expr->op == "+")
	out = LLVMBuildAdd(builder, compile(expr->lhs.get()), compile(expr->rhs.get()), "");
  else if (expr->op == "%")
	out = LLVMBuildSRem(builder, compile(expr->lhs.get()), compile(expr->rhs.get()), "");
  else if (expr->op == "*")
	out = LLVMBuildMul(builder, compile(expr->lhs.get()), compile(expr->rhs.get()), "");
  else if (expr->op == "/")
	out = LLVMBuildSDiv(builder, compile(expr->lhs.get()), compile(expr->rhs.get()), "");
  else if (expr->op == "=")
	out = LLVMBuildICmp(builder, LLVMIntEQ, compile(expr->lhs.get()), compile(expr->rhs.get()), "");
  else if (expr->op == "!=")
	out = LLVMBuildICmp(builder, LLVMIntNE, compile(expr->lhs.get()), compile(expr->rhs.get()), "");
  else if (expr->op == "<")
	out = LLVMBuildICmp(builder, LLVMIntSLT, compile(expr->lhs.get()), compile(expr->rhs.get()), "");
  else if (expr->op == "<=")
	out = LLVMBuildICmp(builder, LLVMIntSLE, compile(expr->lhs.get()), compile(expr->rhs.get()), "");
  else if (expr->op == ">")
	out = LLVMBuildICmp(builder, LLVMIntSGT, compile(expr->lhs.get()), compile(expr->rhs.get()), "");
  else if (expr->op == ">=")
	out = LLVMBuildICmp(builder, LLVMIntSGE, compile(expr->lhs.get()), compile(expr->rhs.get()), "");
}
void coral::codegen::LLVMFunctionCompiler::visit(ast::Return * expr) {
  out = LLVMBuildRet(builder, compile(expr->val.get()));
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::Call * expr) {
  expr->callee->accept(this);

  if (!out && ast::ExprTypeVisitor::of(expr->callee.get()) == ast::ExprTypeKind::VarKind) {
	auto var = (ast::Var *)expr->callee.get();
	out = LLVMGetNamedFunction(module, var->name.c_str());
	if (!out)
	  out = LLVMAddFunction(
		module, var->name.c_str(),
		LLVMFunctionType(LLVMVoidType(), 0, 0, true));
  } else if (!out) {
	std::cout << "missing var " << ast::ExprNameVisitor::of(expr->callee.get()) << "\n";
  }
  auto llvmVarRef = out;
  auto llvmArgs = new LLVMValueRef[expr->arguments.size()];
  // std::cerr << "Function " << expr->callee.get() << "\n";
  // std::cerr << LLVMPrintValueToString(llvmVarRef) << "\n";
  for(ulong i=0; i<expr->arguments.size(); i++) {
	// std::cerr << "  arg[" << i << "]"<< expr->arguments[i].get() << "\n";
	expr->arguments[i]->accept(this);
	llvmArgs[i] = out;
  }
  out = LLVMBuildCall(
	builder, llvmVarRef,
	llvmArgs, expr->arguments.size(), "");
  delete [] llvmArgs;
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::Block * expr) {
  for(auto && line : expr->lines) if (line) line->accept(this);
}
