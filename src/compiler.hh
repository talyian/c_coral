#include <map>

#define LTYPE(t) GetLLVMType(builder, t).out
class GetLLVMType : public TypeVisitor {
public:
  LLVMTypeRef out;
  LLVMBuilderRef builder;
  GetLLVMType(LLVMBuilderRef b, Type * t) : builder(b) { t->accept(this); }
  virtual void visit(VoidType * f);    
  virtual void visit(FuncType * f);  
  virtual void visit(PtrType * f);
  virtual void visit(ArrType * f);
  virtual void visit(IntType * f);
};

// VAL(e): Convert an Expr * e to LLVMValueRef
#define VAL(e) ExprValue(builder, e).output
class ExprValue : public Visitor {
public:
  LLVMBuilderRef builder;
  LLVMValueRef output;
  ExprValue(LLVMBuilderRef b, Expr * e) : builder(b) { e->accept(this); }
  virtual void visit(String * s);
  virtual void visit(Long * s);
  virtual void visit(Double * s);
  virtual void visit(BinOp * s);
  virtual void visit(Var * s);
  virtual void visit(Call * s);
};

// Type-specialize over different operand types
class BinaryValue : public Visitor {
public:
  LLVMBuilderRef builder;
  LLVMValueRef output;
  BinOp * op;
  BinaryValue(LLVMBuilderRef b, BinOp * e) : builder(b), op(e) { op->lhs->accept(this); }
  void visitLong();
  void visitDouble();
  virtual void visit(Long * s) { visitLong(); }
  virtual void visit(Double * s) { visitDouble(); }
  virtual void visit(Var * s) { visitLong(); }
};

extern LLVMContextRef context;

// Compile a coral file
class ModuleBuilder : public Visitor {
public:
  LLVMModuleRef module;
  LLVMBuilderRef builder;
  LLVMValueRef function;
  LLVMBasicBlockRef bb;

  std::map<std::tuple<LLVMValueRef, std::string>, LLVMValueRef> names;

  ModuleBuilder(Module * m) {
    module = LLVMModuleCreateWithNameInContext(m->name.c_str(), context);
    builder = LLVMCreateBuilderInContext(context);
    for(auto i = m->lines.begin(), e=m->lines.end(); i != e; i++)
      (*i)->accept(this);
  }
  void finalize() {
    char * ir = LLVMPrintModuleToString(module);
    printf("%s\n", ir);
  }

  virtual void visit(BlockExpr * c);
  virtual void visit(FuncDef * c);
  virtual void visit(Extern * c);
  virtual void visit(Return * c);  
  virtual void visit(If * c);

  // simulate possible side-effects
  virtual void visit(Call * c) { VAL(c); }
  virtual void visit(BinOp * c) { VAL(c); }
  virtual void visit(Var * c) { VAL(c); }  
  virtual void visit(String * c) { VAL(c); }
  virtual void visit(Long * c) { VAL(c); }
  virtual void visit(Double * c) { VAL(c); }
};
