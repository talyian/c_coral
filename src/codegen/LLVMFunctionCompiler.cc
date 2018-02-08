#include "llvm-c/Core.h"

#include "LLVMFunctionCompiler.hh"
#include "../core/expr.hh"

void coral::codegen::LLVMFunctionCompiler::visit(ast::Func * expr) {
  auto paramTypes = new LLVMTypeRef[expr->params.size()];
  for(ulong i=0; i<expr->params.size(); i++) {
    auto ppp = expr->params[i].get();
    // std::cout << ppp->name << " : "
    // 	      << (ppp->type ? ppp->type->name: "")
    // 	      << "\n";
    if (ppp->type && ppp->type->name == "Ptr")
      paramTypes[i] = LLVMPointerType(LLVMInt8Type(), 0);
    else
      paramTypes[i] = LLVMInt32Type();
  }
  auto ret_type = LLVMInt32TypeInContext(context);
  if (expr->type && expr->type->name == "Ptr")
    ret_type = LLVMPointerType(LLVMInt8Type(), 0);

  function = LLVMAddFunction(
	module,
	expr->name.c_str(),
	LLVMFunctionType(
 	  ret_type,
	  paramTypes,
	  expr->params.size(),
	  false));
  (*info)[expr] = function;

  for(ulong i=0; i<expr->params.size(); i++) {
	(*info)[expr->params[i].get()] = LLVMGetParam(function, i);
  }
  if (expr->body) {
    basic_block = LLVMAppendBasicBlock(function, "entry");
    LLVMPositionBuilderAtEnd(builder, basic_block);
    expr->body->accept(this);
  }
  delete [] paramTypes;
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::IfExpr * expr) {
  auto thenblock = LLVMAppendBasicBlock(function, "then");
  auto elseblock = LLVMAppendBasicBlock(function, "else");
  auto endblock = LLVMAppendBasicBlock(function, "end");
  out = LLVMBuildCondBr(
  	builder,
  	compile(expr->cond.get()),
  	thenblock,
  	elseblock);
  int branchreturns = 0;
  returns = 0;
  LLVMPositionBuilderAtEnd(builder, thenblock);
  expr->ifbody->accept(this);
  if (!returns) LLVMBuildBr(builder, endblock);
  branchreturns += returns > 0 ? 1 : 0;

  returns = 0;
  LLVMPositionBuilderAtEnd(builder, elseblock);
  if (expr->elsebody) expr->elsebody->accept(this);
  if (!returns) LLVMBuildBr(builder, endblock);
  branchreturns += returns > 0 ? 1 : 0;

  LLVMPositionBuilderAtEnd(builder, endblock);
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
	// std::cerr << "Not Found: " << var->name << "\n";
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
  case ast::ExprTypeKind::LetKind:
	if (this->rawPointer)
	  out = info->find(var->expr)->second;
	else
	  out = LLVMBuildLoad(builder, info->find(var->expr)->second, var->name.c_str()) ;
	return;
  default:
  	std::cerr << "unknown var kind : " << var->name << " :: " << ast::ExprNameVisitor::of(var->expr) << "\n";
  	break;
  }
  out = LLVMConstInt(LLVMInt32Type(), 0, false);
}
void coral::codegen::LLVMFunctionCompiler::visit(ast::BinOp * expr) {
  auto lhs = compile(expr->lhs.get());
  auto rhs = compile(expr->rhs.get());
  if (LLVMGetTypeKind(LLVMTypeOf(lhs))==LLVMPointerTypeKind) {
    if (expr->op == "+") {
      LLVMValueRef indices[1] = { rhs };
      out = LLVMBuildGEP(builder, lhs, &rhs, 1, "");
    } else {
      std::cerr << "Unknown Operator " << expr->op << " for Pointers\n";
      out = 0;
    }
    return;
  }
  if (expr->op == "-")
	out = LLVMBuildSub(builder, lhs, rhs, "");
  else if (expr->op == "+")
	out = LLVMBuildAdd(builder, lhs, rhs, "");
  else if (expr->op == "%")
	out = LLVMBuildSRem(builder, lhs, rhs, "");
  else if (expr->op == "*")
	out = LLVMBuildMul(builder, lhs, rhs, "");
  else if (expr->op == "/")
	out = LLVMBuildSDiv(builder, lhs, rhs, "");
  else if (expr->op == "=")
	out = LLVMBuildICmp(builder, LLVMIntEQ, lhs, rhs, "");
  else if (expr->op == "!=")
	out = LLVMBuildICmp(builder, LLVMIntNE, lhs, rhs, "");
  else if (expr->op == "<")
	out = LLVMBuildICmp(builder, LLVMIntSLT, lhs, rhs, "");
  else if (expr->op == "<=")
	out = LLVMBuildICmp(builder, LLVMIntSLE, lhs, rhs, "");
  else if (expr->op == ">")
	out = LLVMBuildICmp(builder, LLVMIntSGT, lhs, rhs, "");
  else if (expr->op == ">=")
	out = LLVMBuildICmp(builder, LLVMIntSGE, lhs, rhs, "");
}
void coral::codegen::LLVMFunctionCompiler::visit(ast::Return * expr) {
  returns++;
  out = LLVMBuildRet(builder, compile(expr->val.get()));
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::Call * expr) {
  expr->callee->accept(this);

  if (!out && ast::ExprTypeVisitor::of(expr->callee.get()) == ast::ExprTypeKind::VarKind) {
	auto var = (ast::Var *)expr->callee.get();
	if (var->name == "addrof") {
	  this->rawPointer = 1;
	  expr->arguments[0]->accept(this);
	  this->rawPointer = 0;
	  return;
	} else if (var->name == "derefi") {
	  expr->arguments[0]->accept(this);
	  auto intval = out;
	  auto ptrval = LLVMBuildIntToPtr(builder, intval, LLVMPointerType(LLVMInt32Type(), 0), "ptrcast");
	  out = LLVMBuildLoad(builder, ptrval, var->name.c_str());
	  return;
	}
	out = LLVMGetNamedFunction(module, var->name.c_str());
	if (!out)
	  out = LLVMAddFunction(
		module, var->name.c_str(),
		LLVMFunctionType(LLVMVoidType(), 0, 0, true));
  } else if (!out) {
	std::cerr << "missing var " << ast::ExprNameVisitor::of(expr->callee.get()) << "\n";
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

void coral::codegen::LLVMFunctionCompiler::visit(ast::Let * expr) {
  out = 0;
  expr->value->accept(this);
  auto llval = out;
  auto local = LLVMBuildAlloca(builder, LLVMTypeOf(llval), expr->var->name.c_str());
  LLVMBuildStore(builder, llval, local);
  (*info)[expr] = local;
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::Set * expr) {
  out = 0;
  expr->value->accept(this);
  auto llval = out;
  auto local = (*info)[expr->var->expr];
  if (ast::ExprTypeVisitor::of(expr->var->expr) == ast::ExprTypeKind::DefKind) {
	std::cerr << "Warning: writing to a parameter is not currently supported\n";
	out = 0;
	return;
  }
  LLVMBuildStore(builder, llval, local);
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::Comment * w) { }

void coral::codegen::LLVMFunctionCompiler::visit(ast::Member * w) {
  std::cerr << "Not implemented yet: Member Codegen\n";
}

void coral::codegen::LLVMFunctionCompiler::visit(ast::While * w) {

  auto whileblock = LLVMAppendBasicBlock(function, "while");
  auto body = LLVMAppendBasicBlock(function, "do");
  auto endblock = LLVMAppendBasicBlock(function, "end");
  LLVMBuildBr(builder, whileblock);

  LLVMPositionBuilderAtEnd(builder, whileblock);
  out = 0;
  w->cond->accept(this);
  LLVMBuildCondBr(builder, out, body, endblock);

  LLVMPositionBuilderAtEnd(builder, body);
  w->body->accept(this);
  LLVMBuildBr(builder, whileblock);

  LLVMPositionBuilderAtEnd(builder, endblock);
}
