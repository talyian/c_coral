#include <cstdio>
#include <map>
#include <algorithm>
#include <regex>
#include "llvm-c/Core.h"

#include "../src/ast.h"
#include "../obj/parser.hh"

LLVMContextRef context = LLVMContextCreate();
LLVMTypeRef Tvoid = LLVMVoidTypeInContext(context);
LLVMTypeRef TInt32 = LLVMInt32TypeInContext(context);

LLVMValueRef add_func(LLVMModuleRef module, const char * name) {
  LLVMValueRef existing = LLVMGetNamedFunction(module, name);
  if (existing) return existing;
  LLVMTypeRef printftype = LLVMFunctionType(Tvoid, NULL, 0, true);
  return LLVMAddFunction(module, name, printftype);
}


class BinaryValue : public Visitor {
public:
  LLVMBuilderRef builder;
  LLVMValueRef output;
  BinOp * op;
  BinaryValue(LLVMBuilderRef b, BinOp * oper) : builder(b), op(oper) {
    op->lhs->accept(this);
  }
  virtual void visit(Long * s);
  virtual void visit(Double * s);
  LLVMValueRef lhval();
  LLVMValueRef rhval();
};
  
class ArgumentValue : public Visitor {
public:
  LLVMBuilderRef builder;
  LLVMValueRef output;
  ArgumentValue(LLVMBuilderRef b, Expr * e) : builder(b) {
    e->accept(this);
  }
  virtual void visit(String * s) {
    output = LLVMBuildGlobalString(builder, s->toString().c_str(), "");
  }
  virtual void visit(Long * s) {
    output = LLVMConstInt(LLVMInt64TypeInContext(context), s->value, false);
  }
  virtual void visit(Double * s) {
    output = LLVMConstReal(LLVMDoubleTypeInContext(context), s->value);    
  }
  virtual void visit(BinOp * s) {
    output = BinaryValue(builder, s).output;
  }
};

LLVMValueRef BinaryValue::lhval() { return ArgumentValue(builder, op->lhs).output; }
LLVMValueRef BinaryValue::rhval() { return ArgumentValue(builder, op->rhs).output; }

void BinaryValue::visit(Long * s) {
  if (op->op == "+") { output = LLVMBuildAdd(builder, lhval(), rhval(), ""); }
  else if (op->op == "-") { output = LLVMBuildSub(builder, lhval(), rhval(), ""); }
  else if (op->op == "*") { output = LLVMBuildMul(builder, lhval(), rhval(), ""); }
  else if (op->op == "/") { output = LLVMBuildSDiv(builder, lhval(), rhval(), ""); } 
}

void BinaryValue::visit(Double * s) {
  if (op->op == "+") { output = LLVMBuildFAdd(builder, lhval(), rhval(), ""); }
  else if (op->op == "-") { output = LLVMBuildFSub(builder, lhval(), rhval(), ""); }
  else if (op->op == "*") { output = LLVMBuildFMul(builder, lhval(), rhval(), ""); }
  else if (op->op == "/") { output = LLVMBuildFDiv(builder, lhval(), rhval(), ""); } 
}

class CodeGen : public Visitor {
public:
  LLVMBuilderRef builder;
  LLVMModuleRef module;
  LLVMValueRef main_func;
  LLVMBasicBlockRef bb;
  
  CodeGen(Module & m) { 
    builder = LLVMCreateBuilderInContext(context);
    module = LLVMModuleCreateWithNameInContext(m.name.c_str(), context);
    main_func = LLVMAddFunction(
      module, "main",
      LLVMFunctionType(TInt32, NULL, 0, false));
    bb = LLVMAppendBasicBlockInContext(context, main_func, "entry");
    LLVMPositionBuilderAtEnd(builder, bb);
  }

  std::map<std::string, LLVMValueRef> names;
  void enlist(std::string name, LLVMValueRef val) { names[name] = val; }
  LLVMValueRef deref(std::string name) { return names[name]; }
 
  virtual void visit(Expr * c) {
    std::cout << "expr\n";
  }

  virtual void visit(Call * c) {
    LLVMValueRef func = deref(c->callee);
    if (!func) return;
    std::vector<LLVMValueRef> args(c->arguments.size());
    std::transform(
      c->arguments.begin(),
      c->arguments.end(),
      args.begin(),
      [this] (Expr * e) { return ArgumentValue(builder, e).output; });
    LLVMBuildCall(builder, func, &args[0], args.size(), "");
  }

  virtual void visit(FuncDef * c) {
    LLVMValueRef existing = LLVMGetNamedFunction(module, c->name.c_str());
    if (existing) {
      LLVMPositionBuilderAtEnd(builder, LLVMGetLastBasicBlock(existing));
    } else { 
      LLVMTypeRef ftype = LLVMFunctionType(Tvoid, NULL, 0, true);
      LLVMValueRef func = LLVMAddFunction(module, c->name.c_str(), ftype);
      enlist(c->name, func);
      LLVMBasicBlockRef newbb = LLVMAppendBasicBlockInContext(context, func, "entry");
      LLVMPositionBuilderAtEnd(builder, newbb);
      LLVMValueRef ret = LLVMBuildRetVoid(builder);
      LLVMPositionBuilderBefore(builder, ret);      
    }
    c->body->accept(this);
  }

  virtual void visit(Extern * c) {
    LLVMValueRef func = add_func(module, c->Name.c_str());
    enlist(c->Name, func);
  }

  void finalize() {
    LLVMBuildRet(builder, LLVMConstInt(TInt32, 0, false));
    char * ir = LLVMPrintModuleToString(module);
    printf("%s\n", ir);
  }
};

void handle_module(Module & m) {
  CodeGen cg(m);
  for(auto iter = m.lines.begin(); iter != m.lines.end(); iter++) {
    (*iter)->accept(&cg);
  }
  cg.finalize();
}
