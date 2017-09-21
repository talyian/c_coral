#include <cstdio>
#include <map>
#include <algorithm>
#include <regex>
#include "llvm-c/Core.h"

#include "ast.hh"
#include "parser.hh"
#include "compiler.hh"

using std::string;
using std::cerr;
using std::endl;

LLVMContextRef context = LLVMContextCreate();

std::map<LLVMValueRef, std::map<string, LLVMValueRef>> names;
void define(LLVMValueRef scope, string name, LLVMValueRef value) { names[scope][name] = value; }
LLVMValueRef lookup(LLVMValueRef scope, string name) { return names[scope][name]; }

#define TSTR(x) LLVMPrintTypeToString(x)
#define ISPTR(x) (LLVMGetTypeKind(x) == LLVMPointerTypeKind)
#define ISARR(x) (LLVMGetTypeKind(x) == LLVMArrayTypeKind)
LLVMValueRef CastArgument(LLVMBuilderRef builder, LLVMValueRef arg, LLVMTypeRef paramtype) {
  auto argtype = LLVMTypeOf(arg);
  if (ISPTR(paramtype) && ISPTR(argtype) &&
      ISARR(LLVMGetElementType(argtype)) &&
      LLVMGetElementType(paramtype) == LLVMGetElementType(LLVMGetElementType(argtype))) {
    return LLVMBuildBitCast(builder, arg, paramtype, "");
  }
  if ( LLVMGetTypeKind(LLVMTypeOf(arg)) == LLVMIntegerTypeKind &&
       LLVMGetTypeKind(paramtype) == LLVMIntegerTypeKind) {
    return LLVMBuildTrunc(builder, arg, paramtype, "");    
  }
  return arg;
}

void GetLLVMType::visit(FuncType * f) {
  std::vector<LLVMTypeRef> llvm_params(f->args.size());
  std::transform(
    f->args.begin(), f->args.end(),
    llvm_params.begin(),
    [this] (Type * p) { return LTYPE(p); });
  out = LLVMFunctionType(LTYPE(f->ret), &llvm_params[0], f->args.size(), true);
}
void GetLLVMType::visit(VoidType * f) { out = LLVMVoidType(); }
void GetLLVMType::visit(PtrType * f) { out = LLVMPointerType(LTYPE(f->inner), 0); }
void GetLLVMType::visit(ArrType * f) { out = LLVMArrayType(LTYPE(f->inner), f->len); }
void GetLLVMType::visit(IntType * f) { out = LLVMIntTypeInContext(context, f->bits); }

void ExprValue::visit(String * s) { output = LLVMBuildGlobalString(builder, s->toString().c_str(), ""); }
void ExprValue::visit(Long * s) { output = LLVMConstInt(LLVMInt64TypeInContext(context), s->value, false); }
void ExprValue::visit(Double * s)  { output = LLVMConstReal(LLVMDoubleTypeInContext(context), s->value); }
void ExprValue::visit(Call * c)  {
    output = 0;
    auto bb = LLVMGetInsertBlock(builder);
    auto curfunc = LLVMGetBasicBlockParent(bb);
    auto module = LLVMGetGlobalParent(curfunc);
    LLVMValueRef func = lookup(0, c->callee);   // LLVMGetNamedGlobal(module, c->callee.c_str());
    if (func == 0) {
      cerr << bb << endl;
      cerr << curfunc << endl;
      cerr << module << endl;
      cerr << "not found: " << c->callee << endl;
      return;
    }
    LLVMTypeRef functype = LLVMTypeOf(func);
    functype = LLVMGetElementType(functype);
    if (!func) return;

    int nParam = LLVMCountParamTypes(functype);
    auto params = new LLVMTypeRef[nParam];
    LLVMGetParamTypes(functype, params);

    int nArgs = (int)c->arguments.size();
    auto args = new LLVMValueRef[nArgs];
    for(int i=0; i < (int)nArgs; i++) {
      Expr * expr = c->arguments[i];
      LLVMValueRef arg = VAL(expr);
      LLVMTypeRef paramtype = i < nParam ? params[i] : 0;
      if (paramtype)
      	args[i] = CastArgument(builder, arg, paramtype);
      else
	args[i] = arg;
    }
    output = LLVMBuildCall(builder, func, args, c->arguments.size(), "");
}

void ExprValue::visit(Var * s) {
  auto bb = LLVMGetInsertBlock(builder);
  auto fn = LLVMGetBasicBlockParent(bb);
  output = lookup(fn, s->toString());
}

void ExprValue::visit(BinOp * c) { output = BinaryValue(builder, c).output; }

void BinaryValue::visitLong()  {
  auto lval = VAL(op->lhs);
  auto rval = VAL(op->rhs);
    if (op->op == "+") { output = LLVMBuildAdd(builder, lval, rval, ""); }
  else if (op->op == "-") { output = LLVMBuildSub(builder, lval, rval, ""); }
  else if (op->op == "*") { output = LLVMBuildMul(builder, lval, rval, ""); }
  else if (op->op == "/") { output = LLVMBuildSDiv(builder, lval, rval, ""); }
  else if (op->op == "%") { output = LLVMBuildURem(builder, lval, rval, ""); }    
  else if (op->op == "<") { output = LLVMBuildICmp(builder, LLVMIntSLT, lval, rval, ""); }
  else if (op->op == ">") { output = LLVMBuildICmp(builder, LLVMIntSGT, lval, rval, ""); }
  else if (op->op == "=") { output = LLVMBuildICmp(builder, LLVMIntEQ, lval, rval, ""); }
  else if (op->op == "!=") { output = LLVMBuildICmp(builder, LLVMIntNE, lval, rval, ""); }
  else cerr << "unknown operation " << op->op << endl;
}

void BinaryValue::visitDouble() {
  auto lval = VAL(op->lhs);
  auto rval = VAL(op->rhs);
  if (op->op == "+") { output = LLVMBuildFAdd(builder, lval, rval, ""); }
  else if (op->op == "-") { output = LLVMBuildFSub(builder, lval, rval, ""); }
  else if (op->op == "*") { output = LLVMBuildFMul(builder, lval, rval, ""); }
  else cerr << "unknown operation " << op->op << endl;
}

void ModuleBuilder::visit(Return * c) {
  auto ret_type = LLVMGetReturnType(LLVMGetElementType(LLVMTypeOf(function)));
  auto cvalue = CastArgument(builder, VAL(c->value), ret_type);
  LLVMBuildRet(builder, cvalue);
}

void ModuleBuilder::visit(BlockExpr * c) {
  for(auto i = c->lines.begin(), e = c->lines.end(); i != e; i++)
    (*i)->accept(this);
}

void ModuleBuilder::visit(If * c) {
  auto bb = LLVMGetInsertBlock(builder);
  auto fn = LLVMGetBasicBlockParent(bb);
  auto ifbb = LLVMAppendBasicBlock(fn, "ifbb");
  auto elbb = LLVMAppendBasicBlock(fn, "elbb");
  cerr << c->cond->toString() << endl;
  LLVMBuildCondBr(builder, VAL(c->cond), ifbb, elbb);
  LLVMPositionBuilderAtEnd(builder, ifbb);
  c->ifbody->accept(this);
  LLVMPositionBuilderAtEnd(builder, elbb);
  c->elsebody->accept(this);
}

void ModuleBuilder::visit(FuncDef * c) {
  std::vector<LLVMTypeRef> llvm_params(c->args.size());
  std::transform(
    c->args.begin(), c->args.end(),
    llvm_params.begin(),
    [this] (Def * p) { return LTYPE(p->type); });
  auto type = LLVMFunctionType(LTYPE(c->rettype), &llvm_params[0], c->args.size(), true);
  LLVMValueRef func = LLVMAddFunction(module, c->name.c_str(), type);
  function = func;
  define(0, c->name, func);
  for(auto i=0; i<(int)c->args.size(); i++) {
    define(func, c->args[i]->name, LLVMGetParam(func, i));
  }
  auto bb = LLVMAppendBasicBlockInContext(context, func, "entry");
  LLVMPositionBuilderAtEnd(builder, bb);
  c->body->accept(this);
}

void ModuleBuilder::visit(Extern * c) {
  auto type = c->type ? c->type : new FuncType(new VoidType(), std::vector<Type *>(), true);
  LLVMValueRef func = LLVMAddFunction(module, c->name.c_str(), LTYPE(type));
  define(0, c->name, func);
}

void handle_module(Module & m) {
  ModuleBuilder moduleB(&m);
  moduleB.finalize();
}
