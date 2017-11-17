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
	void visit(UnknownType * m) {
	  std::cerr << "Unknown Type\n";
	  out = LLVMVoidType();
	}
	void visit(UserType * m) {
	  if (m->name == "FdReader") {
		std::cerr << "TODO: special-casing FdReader\n";
		LLVMTypeRef fields[] = { LLVMInt32Type() };
		out = LLVMStructTypeInContext(context, fields, 1, true);
		return;
	  }
	  std::cerr << "Translating User Type to Pointer!!\n";
	  out = LLVMPointerType(LLVMInt8Type(), 0);
	}
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
	std::map<Expr *, LLVMValueRef> savedValues;

	ModuleCompiler(coral::Module * m);
	ModuleCompiler(coral::Module * m, std::map<Expr *, LLVMValueRef> savedValues);
	void visit(FuncDef * m);
	void visit(Extern * m);
	void visit(Let * m);
	void visit(Struct * m);
	std::string getIR();
  };


  class ExpressionCompiler : public Visitor {
  public:
	LLVMContextRef context;
	LLVMModuleRef llvmModule;
	LLVMValueRef llvmFunction = 0;
	LLVMBasicBlockRef llvmBB = 0;
	LLVMBuilderRef llvmBuilder = 0;
	LLVMValueRef out = 0;
	std::map<Expr *, LLVMValueRef> savedValues;
	ExpressionCompiler(coral::ModuleCompiler * m) : Visitor("exprcompiler ") {
	  context = m->context;
	  llvmModule = m->llvmModule;
	  llvmFunction = m->llvmFunction;
	  llvmBuilder = m->llvmBuilder;
	  savedValues = m->savedValues;
	}

	void visit(Member * e) {
	  e->base->accept(this);
	  // TODO: e->base needs a type
	  // which means that
	  // e->base
	  auto outBase = out;
	  std::cerr << "TODO member-base : " << e->base << "\n";
	}

	void visit(Set * e) {
	  e->value->accept(this);
	  auto llvmValue = out;
	  e->var->accept(this);
	  auto llvmAddr = out;
	  out = LLVMBuildStore(llvmBuilder, llvmValue, llvmAddr);
	}

	void visit(VoidExpr * e) { /* pass */ }

	void visit(BinOp * e) {
	  e->lhs->accept(this);
	  auto lval = out;
	  e->rhs->accept(this);
	  auto rval = out;
	  auto builder = llvmBuilder;
	  if (e->op == "+") { out = LLVMBuildAdd(builder, lval, rval, ""); }
	  else if (e->op == "-") { out = LLVMBuildSub(builder, lval, rval, ""); }
	  else if (e->op == "*") { out = LLVMBuildMul(builder, lval, rval, ""); }
	  else if (e->op == "/") { out = LLVMBuildSDiv(builder, lval, rval, ""); }
	  else if (e->op == "%") { out = LLVMBuildURem(builder, lval, rval, ""); }
	  else if (e->op == "<") { out = LLVMBuildICmp(builder, LLVMIntSLT, lval, rval, ""); }
	  else if (e->op == ">") { out = LLVMBuildICmp(builder, LLVMIntSGT, lval, rval, ""); }
	  else if (e->op == "=") { out = LLVMBuildICmp(builder, LLVMIntEQ, lval, rval, ""); }
	  else if (e->op == "!=") { out = LLVMBuildICmp(builder, LLVMIntNE, lval, rval, ""); }
	  else std::cerr << "unknown operation " << e->op << std::endl;
	}

	void visit(Return * r) {
	  out = 0;
	  if (r->value) r->value->accept(this);
	  if (out && LLVMGetTypeKind(LLVMTypeOf(out)) != LLVMVoidTypeKind)
		out = LLVMBuildRet(llvmBuilder, out);
	  else
		out = LLVMBuildRetVoid(llvmBuilder);
	}
	void visit(BlockExpr * e) { for (auto &line: e->lines) line->accept(this); }
	void visit(Let * e)	{
	  auto allocaInstr = LLVMBuildAlloca(llvmBuilder, TOLLVMTYPE(e->var->type), e->var->name.c_str());
	  e->value->accept(this);
	  auto value = out;
	  out = LLVMBuildStore(llvmBuilder, value, allocaInstr);
	  savedValues[e] = LLVMBuildLoad(llvmBuilder, allocaInstr, "");
	}
	void visit(If * e) {
	  auto bb = LLVMGetInsertBlock(llvmBuilder);
	  auto fn = LLVMGetBasicBlockParent(bb);
	  auto ifbb = LLVMAppendBasicBlock(fn, "ifbb");
	  auto elbb = LLVMAppendBasicBlock(fn, "elbb");
	  auto nextbb = LLVMAppendBasicBlock(fn, "nxb");

	  e->cond->accept(this);
	  LLVMBuildCondBr(llvmBuilder, out, ifbb, elbb);
	  LLVMPositionBuilderAtEnd(llvmBuilder, ifbb);
	  e->ifbody->accept(this);
	  if (!e->ifterminated)
		LLVMBuildBr(llvmBuilder, nextbb);
	  LLVMPositionBuilderAtEnd(llvmBuilder, elbb);
	  e->elsebody->accept(this);
	  if (!e->elseterminated)
		LLVMBuildBr(llvmBuilder, nextbb);
	  LLVMPositionBuilderAtEnd(llvmBuilder, nextbb);
	  out = 0;
	}
	void visit(Long * e) {
	  out = LLVMConstInt(LLVMInt64Type(), e->value, false);
	}
	void visit(String * e) {
	  out = LLVMBuildGlobalStringPtr(llvmBuilder, e->toString().c_str(), "");
	}
	void visit(Var * e) {
	  // std::cerr << "compiling var " << e << "|" << e->name << " :: " << e->ref << "\n";
	  if (!e->ref) {
		std::cerr << "Undefined reference: "<< e->name << "\n";
		out = 0;
	  } else {
		// std::cerr << "ref!\n";
		out = savedValues[e->ref];
		if (!out) std::cerr << "Missing reference info: "<< e->name << "\n";
	  }
	}
	void visit(Call * e) {
	  std::vector<LLVMValueRef> args(e->arguments.size());
	  for(std::vector<int>::size_type i=0; i<e->arguments.size(); i++) {
		out = 0;
		e->arguments[i]->accept(this);
		args[i] = out;
		// std::cerr << "arg " << i << ": " << LLVMPrintValueToString(out) << "\n";
	  }
	  out = 0;
	  e->callee->accept(this);
	  auto callee = out;
	  if (callee) {
		out = LLVMBuildCall(llvmBuilder, callee, &args[0], args.size(), "");
	  }
	  else
		std::cerr << "no callee!" << e->callee->toString() << "\n";

	}
  };
}
