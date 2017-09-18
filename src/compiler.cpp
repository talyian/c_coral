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
  LLVMTypeRef printftype = LLVMFunctionType(Tvoid, NULL, 0, true);
  return LLVMAddFunction(module, name, printftype);
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
      [this] (Expr * e) { return LLVMBuildGlobalString(builder, e->toString().c_str(), ""); });
    LLVMBuildCall(builder, func, &args[0], args.size(), "");
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
