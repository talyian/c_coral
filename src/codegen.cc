#include <cstdio>
#include <map>
#include <algorithm>
#include <regex>
#include "llvm-c/Core.h"

#include "ast.hh"
#include "parser.hh"
#include "codegen.hh"

using std::string;
using std::cerr;
using std::endl;

#define TSTR(x) LLVMPrintTypeToString(x)
#define ISPTR(x) (LLVMGetTypeKind(x) == LLVMPointerTypeKind)
#define ISARR(x) (LLVMGetTypeKind(x) == LLVMArrayTypeKind)

LLVMContextRef context = LLVMContextCreate();
LLVMValueRef FUNC_MULTI = (LLVMValueRef)-1;
std::map<LLVMValueRef, std::map<string, LLVMValueRef>> names;
// Define takes a scope, a name, and a LLVMValueRef that
// is a pointer to the the variable location!
void define_func(LLVMValueRef scope, string name, LLVMValueRef value) {
  names[scope][name] = value;
  names[scope][name + "=func"] = LLVMConstInt(LLVMInt32Type(), 0, false);
}
void define(LLVMValueRef scope, string name, LLVMValueRef value) {
  // cerr << "define: " << name << ": " << TSTR(LLVMTypeOf(value)) << endl;
  names[scope][name] = value;
  names[scope][name + "=func"] = 0;
}
#define lookup(scope, name) load(builder, scope, name)
LLVMValueRef load(LLVMBuilderRef builder, LLVMValueRef scope, string name) {
  auto loc = names[scope][name];
  if (names[scope][name + "=func"])
    return loc;
  // cerr << "lookup: " << name << " from " << TSTR(LLVMTypeOf(loc)) << endl;
  return LLVMBuildLoad(builder, loc, "");
}
LLVMTypeRef FixArgument(LLVMTypeRef t) {
  // pointer-to-arrays-of-T become pointers-to-T
  if (ISPTR(t) && ISARR(LLVMGetElementType(t))) {
    return LLVMPointerType(LLVMGetElementType(LLVMGetElementType(t)), 0);
  }
  // cerr << "+ " << TSTR(t) << endl;
  // cerr << "| " << ISARR(t) << endl;
  // cerr << "| " << ISPTR(t) << endl;  
  return t;
}
LLVMValueRef CastArgument(LLVMBuilderRef builder, LLVMValueRef arg, LLVMTypeRef paramtype) {
  auto argtype = LLVMTypeOf(arg);
  // cerr << "Casting " << TSTR(LLVMTypeOf(arg)) << " TO " << TSTR(paramtype) << endl;
  if (ISPTR(paramtype) && ISPTR(argtype) &&
      ISARR(LLVMGetElementType(argtype)) &&
      LLVMGetElementType(paramtype) == LLVMGetElementType(LLVMGetElementType(argtype))) {
    return LLVMBuildBitCast(builder, arg, paramtype, "");
  }
  if ( LLVMGetTypeKind(LLVMTypeOf(arg)) == LLVMIntegerTypeKind &&
       LLVMGetTypeKind(paramtype) == LLVMIntegerTypeKind) {
    if (LLVMGetIntTypeWidth(LLVMTypeOf(arg)) > LLVMGetIntTypeWidth(paramtype))
      return LLVMBuildTrunc(builder, arg, paramtype, "");
    else if (LLVMGetIntTypeWidth(LLVMTypeOf(arg)) < LLVMGetIntTypeWidth(paramtype))
      return LLVMBuildZExt(builder, arg, paramtype, "");
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
void GetLLVMType::visit(Type * f) { out = 0; }
void GetLLVMType::visit(VoidType * f) { out = LLVMVoidType(); }
void GetLLVMType::visit(PtrType * f) { out = LLVMPointerType(LTYPE(f->inner), 0); }
void GetLLVMType::visit(ArrType * f) { out = LLVMArrayType(LTYPE(f->inner), f->len); }
void GetLLVMType::visit(IntType * f) { out = LLVMIntTypeInContext(context, f->bits); }
void GetLLVMType::visit(FloatType * f) { out =
    f->bits == 32 ? LLVMFloatTypeInContext(context) : 
    f->bits == 64 ? LLVMDoubleTypeInContext(context) :
    0;
    if (!out) std::cerr << "Invalid Float Type " << f->toString() << std::endl;
}

void ExprValue::visit(String * s) {
  output =  LLVMBuildGlobalStringPtr(builder, s->toString().c_str(), "");
}
void ExprValue::visit(Long * s) { output = LLVMConstInt(LLVMInt64TypeInContext(context), s->value, false); }
void ExprValue::visit(Double * s)  { output = LLVMConstReal(LLVMDoubleTypeInContext(context), s->value); }
void ExprValue::visit(Cast * s)  {
  output = LLVMBuildTrunc(builder, VAL(s->expr), LTYPE(s->to_type), "");
}
void ExprValue::visit(Call * c)  {
    output = 0;
    auto bb = LLVMGetInsertBlock(builder);
    auto curfunc = LLVMGetBasicBlockParent(bb);
    auto module = LLVMGetGlobalParent(curfunc);

    // resolve arguments:
    int nArgs = (int)c->arguments.size();
    auto args = new LLVMValueRef[nArgs];
    for(int i=0; i < (int)nArgs; i++) {
      Expr * expr = c->arguments[i];
      LLVMValueRef arg = VAL(expr);
      args[i] = arg;
    }

    LLVMValueRef func = lookup(0, c->callee);
    if (func == (LLVMValueRef)-1) {
      auto callee2 = c->callee + "$" + LLVMPrintTypeToString(FixArgument(LLVMTypeOf(args[0])));
      func = lookup(0, callee2);
      if ((size_t)func < (size_t)1) {
	cerr << callee2 << " not found :(\n";
	return;
      }
    }
    else if (func == 0) {
      cerr << bb << endl;
      cerr << curfunc << endl;
      cerr << module << endl;
      cerr << "not found: " << c->callee << endl;
      return;
    }

    if (!func) return;
    LLVMTypeRef functype = LLVMTypeOf(func);
    int nParam = LLVMCountParamTypes(LLVMGetElementType(functype));
    if (nParam < 0) cerr << TSTR(functype) << " Number of Params invalid : " << nParam << endl;
    auto params = new LLVMTypeRef[nParam];
    LLVMGetParamTypes(LLVMGetElementType(functype), params);

    // cerr << "calling " << c->callee << "(" << TSTR(functype) << ")" << endl;
    // cerr << "nargs: " << nArgs << ", nparams: " << nParam << endl;
    for(int i=0; i < nArgs; i++) {
      Expr * expr = c->arguments[i];
      LLVMValueRef arg = VAL(expr);
      LLVMTypeRef paramtype = i < nParam ? params[i] : LLVMTypeOf(arg);
      // cerr << "arg " << i << " : " << LLVMPrintValueToString(arg) << endl;
      // cerr << TSTR(LLVMTypeOf(arg)) << " => " << (paramtype ? TSTR(paramtype) : "null?") << endl;
      if (paramtype)
      	args[i] = CastArgument(builder, arg, paramtype);
      else
	args[i] = arg;
    }
    output = LLVMBuildCall(builder, func, args, nArgs, "");
}

void ExprValue::visit(Var * s) {
  auto bb = LLVMGetInsertBlock(builder);
  auto fn = LLVMGetBasicBlockParent(bb);
  output = lookup(fn, s->toString());
}

void ExprValue::visit(AddrOf * s) {
  auto bb = LLVMGetInsertBlock(builder);
  auto fn = LLVMGetBasicBlockParent(bb);
  output = lookup(fn, s->var);
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


ModuleBuilder::ModuleBuilder(Module * m) {
    module = LLVMModuleCreateWithNameInContext(m->name.c_str(), context);
    builder = LLVMCreateBuilderInContext(context);
    for(auto i = m->lines.begin(), e=m->lines.end(); i != e; i++)
      (*i)->accept(this);
}

char * ModuleBuilder::finalize() {
  return LLVMPrintModuleToString(module);
}

void ModuleBuilder::visit(Return * c) {
  auto ret_type = LLVMGetReturnType(LLVMGetElementType(LLVMTypeOf(function)));
  auto cvalue = CastArgument(builder, VAL(c->value), ret_type);
  LLVMBuildRet(builder, cvalue);
}

void ModuleBuilder::visit(BlockExpr * c) {
  for(auto i = c->lines.begin(), e = c->lines.end(); i != e; i++) {
    (*i)->accept(this);
  }
}

void ModuleBuilder::visit(If * c) {
  auto bb = LLVMGetInsertBlock(builder);
  auto fn = LLVMGetBasicBlockParent(bb);
  auto ifbb = LLVMAppendBasicBlock(fn, "ifbb");
  auto elbb = LLVMAppendBasicBlock(fn, "elbb");
  LLVMBuildCondBr(builder, VAL(c->cond), ifbb, elbb);
  LLVMPositionBuilderAtEnd(builder, ifbb);
  c->ifbody->accept(this);
  LLVMPositionBuilderAtEnd(builder, elbb);
  c->elsebody->accept(this);
}

void ModuleBuilder::visit(Let * a) {
  LLVMValueRef func = function;
  auto val = VAL(a->value);
  auto valtype = LLVMTypeOf(val);
  auto dectype = LTYPE(a->var->type);
  if (dectype == 0) dectype = valtype;
  // cerr << "let-assign! " << TSTR(dectype) << " := " << TSTR(valtype) << endl;
  auto var = LLVMBuildAlloca(builder, dectype, "");
  define(func, a->var->name, var);
  LLVMBuildStore(builder, CastArgument(builder, val, dectype), var);
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
  if (c->multi) {
    auto name = c->name + "$" + LLVMPrintTypeToString(LTYPE(c->args[0]->type));
    define_func(0, name, func);
    define_func(0, c->name, FUNC_MULTI);
  } else define_func(0, c->name, func);

  for(auto i=0; i<(int)c->args.size(); i++) {
    define_func(func, c->args[i]->name, LLVMGetParam(func, i));
  }
  LLVMPositionBuilderAtEnd(builder, LLVMAppendBasicBlockInContext(context, func, "entry"));
  c->body->accept(this);

  // safety - all unterminated blocks get an implicit return
  // This only works for voidfuncs 
  for(auto bb = LLVMGetFirstBasicBlock(func); bb; bb = LLVMGetNextBasicBlock(bb)) {
    if (!LLVMGetBasicBlockTerminator(bb)) { 
      LLVMPositionBuilderAtEnd(builder, bb);
      LLVMBuildRetVoid(builder);
    }
  }
}

void ModuleBuilder::visit(Extern * c) {
  auto type = c->type ? c->type : new FuncType(new VoidType(), std::vector<Type *>(), true);
  LLVMValueRef func = LLVMAddFunction(module, c->name.c_str(), LTYPE(type));
  define_func(0, c->name, func);
}

void handle_module(Module & m) {
  ModuleBuilder moduleB(&m);
  printf("%s", moduleB.finalize());
}
