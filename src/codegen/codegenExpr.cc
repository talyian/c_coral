#include "codegen.hh"
#include <algorithm>

using namespace std;
using namespace coral;

void ExprValue::visit(String * s) {
  output =  LLVMBuildGlobalStringPtr(mb->builder, s->toString().c_str(), "");
}

void ExprValue::visit(Long * s) { output = LLVMConstInt(LLVMInt64TypeInContext(mb->context), s->value, false); }

void ExprValue::visit(Double * s)  { output = LLVMConstReal(LLVMDoubleTypeInContext(mb->context), s->value); }

void ExprValue::visit(Cast * s)  {
  output = LLVMBuildTrunc(mb->builder, VAL(s->expr), LTYPE(s->to_type), "");
}

void ExprValue::visit(Call * c)  {
  output = 0;
  // auto bb = LLVMGetInsertBlock(mb->builder);
  // auto curfunc = LLVMGetBasicBlockParent(bb);
  // auto module = LLVMGetGlobalParent(curfunc);

  // LLVMValueRef func = VAL(c->callee);
  LLVMValueRef func = 0;

  if (LLVMGetTypeKind(LLVMTypeOf(func)) == LLVMPointerTypeKind &&
      LLVMGetTypeKind(LLVMGetElementType(LLVMTypeOf(func))) == LLVMFunctionTypeKind) {
    vector<LLVMValueRef> args(c->arguments.size());
    // std::transform(
    //   c->arguments.begin(), c->arguments.end(), args.begin(),
    //   [this] (Expr * arg) { return VAL(arg); });
    output = LLVMBuildCall(mb->builder, func, &args[0], args.size(), "");
  } else {
    cerr << "no func " << LLVMPrintTypeToString(LLVMTypeOf(func)) << endl;
  }
  return;
  // // resolve arguments:
  // int nArgs = (int)c->arguments.size();
  // auto args = new LLVMValueRef[nArgs];
  // for(int i=0; i < (int)nArgs; i++) {
  //   Expr * expr = c->arguments[i];
  //   LLVMValueRef arg = VAL(expr);
  //   args[i] = arg;
  // }

  // LLVMValueRef func = 0;
  // do {
  //   // cerr << "lookup " << c->callee << endl;
  //   func = lookup(0, c->callee);
  //   // cerr << "FOUND " << (size_t)func << endl;
  //   if (func != 0 && func != FUNC_MULTI) break;

  //   auto callee2 = c->callee + "$" + LLVMPrintTypeToString(FixArgument(LLVMTypeOf(args[0])));
  //   // cerr << "trying " << callee2 << endl;
  //   func = lookup(0, callee2);
  //   if (func != 0 && func != FUNC_MULTI) break;

  //   cerr << bb << endl;
  //   cerr << curfunc << endl;
  //   cerr << module << endl;
  //   cerr << "not found: " << c->callee << endl;
  //   return;
  // } while(0);

  // if (!func) return;
  // LLVMTypeRef functype = LLVMTypeOf(func);
  // // cerr << c->callee << " functype " << LLVMPrintTypeToString(functype) << endl;
  // int nParam = LLVMCountParamTypes(LLVMGetElementType(functype));
  // if (nParam < 0) cerr << TSTR(functype) << " Number of Params invalid : " << nParam << endl;
  // auto params = new LLVMTypeRef[nParam];
  // LLVMGetParamTypes(LLVMGetElementType(functype), params);

  // // cerr << "calling " << c->callee << "(" << TSTR(functype) << ")" << endl;
  // // cerr << "nargs: " << nArgs << ", nparams: " << nParam << endl;
  // for(int i=0; i < nArgs; i++) {
  //   Expr * expr = c->arguments[i];
  //   LLVMValueRef arg = VAL(expr);
  //   LLVMTypeRef paramtype = i < nParam ? params[i] : LLVMTypeOf(arg);
  //   // cerr << " call " << c->callee << " arg " << i << " : " << LLVMPrintValueToString(arg) << endl;
  //   // cerr << "\t\t"  << i << " " << nParam << " " << LLVMPrintTypeToString(paramtype) << endl;
  //   // cerr << TSTR(LLVMTypeOf(arg)) << " => " << (paramtype ? TSTR(paramtype) : "null?") << endl;

  //   if (paramtype)
  //     args[i] = CastArgument(mb->builder, arg, paramtype);
  //   else
  //     args[i] = arg;
  // }
  // delete [] args;
  // delete [] params;
}

void ExprValue::visit(Var * s) {
  auto bb = LLVMGetInsertBlock(mb->builder);
  auto fn = LLVMGetBasicBlockParent(bb);
  output = lookup(fn, s->toString());
}

void ExprValue::visit(AddrOf * s) {
  auto bb = LLVMGetInsertBlock(mb->builder);
  auto fn = LLVMGetBasicBlockParent(bb);
  auto name = s->var;
  auto scope = fn;
  auto loc = mb->names[scope][name];
  if (!loc && scope) {
    loc = mb->names[0][name];
    if (loc) scope = 0;
  }
  output = loc;
  // cerr << "var : " << s->var << endl;
  // cerr << "output: " << LLVMPrintValueToString(output) << endl;
}

void ExprValue::visit(BinOp * c) {
  auto lval = VAL(c->lhs);
  auto rval = VAL(c->rhs);

  if (c->op == "+") { output = LLVMBuildAdd(mb->builder, lval, rval, ""); }
  else if (c->op == "-") { output = LLVMBuildSub(mb->builder, lval, rval, ""); }
  else if (c->op == "*") { output = LLVMBuildMul(mb->builder, lval, rval, ""); }
  else if (c->op == "/") { output = LLVMBuildSDiv(mb->builder, lval, rval, ""); }
  else if (c->op == "%") { output = LLVMBuildURem(mb->builder, lval, rval, ""); }
  else if (c->op == "<") { output = LLVMBuildICmp(mb->builder, LLVMIntSLT, lval, rval, ""); }
  else if (c->op == ">") { output = LLVMBuildICmp(mb->builder, LLVMIntSGT, lval, rval, ""); }
  else if (c->op == "=") { output = LLVMBuildICmp(mb->builder, LLVMIntEQ, lval, rval, ""); }
  else if (c->op == "!=") { output = LLVMBuildICmp(mb->builder, LLVMIntNE, lval, rval, ""); }
  else cerr << "unknown operation " << c->op << endl;
}

void ExprValue::visit(Index * index) {
  auto base = index->base;
  auto indices = index->indices;
  // TODO: lookup type of expression
}

void ExprValue::visit(Tuple * t) {

}

void BinaryValue::visitLong()  {
}

void BinaryValue::visitDouble() {
  auto lval = VAL(op->lhs);
  auto rval = VAL(op->rhs);
  if (op->op == "+") { output = LLVMBuildFAdd(mb->builder, lval, rval, ""); }
  else if (op->op == "-") { output = LLVMBuildFSub(mb->builder, lval, rval, ""); }
  else if (op->op == "*") { output = LLVMBuildFMul(mb->builder, lval, rval, ""); }
  else cerr << "unknown operation " << op->op << endl;
}
