#include <llvm-c/Core.h>
#include <map>

#define lookup(scope, name) mb->load(mb->builder, scope, name)

// Compile a coral file
class ModuleBuilder : public Visitor {
public:

  LLVMContextRef context;
  LLVMModuleRef module;
  LLVMBuilderRef builder;
  LLVMValueRef function;
  LLVMBasicBlockRef bb;

  std::map<LLVMValueRef, std::map<std::string, LLVMValueRef>> names;
  void define_func(LLVMValueRef scope, std::string name, LLVMValueRef value);
  void define(LLVMValueRef scope, std::string name, LLVMValueRef value);
  LLVMValueRef load(LLVMBuilderRef builder, LLVMValueRef scope, std::string name);
  LLVMValueRef loadWithType(LLVMValueRef scope, std::string name, LLVMTypeRef type);

  void init(Module * m);
  ModuleBuilder(Module * m);
  ModuleBuilder(Module * m, std::map<LLVMValueRef, std::map<std::string, LLVMValueRef>> _names);
  ModuleBuilder * getModuleBuilder() { return this; }
  ~ModuleBuilder();

  std::string finalize();

  virtual void visit(BlockExpr * c);
  virtual void visit(FuncDef * c);
  virtual void visit(Extern * c);
  virtual void visit(Return * c);
  virtual void visit(If * c);
  virtual void visit(Let * c);

  // simulate possible side-effects
  virtual void visit(Call * c);
  virtual void visit(BinOp * c);
  virtual void visit(Var * c);
  virtual void visit(String * c);
  virtual void visit(Long * c);
  virtual void visit(Double * c);
  virtual void visit(AddrOf * c);
  virtual void visit(VoidExpr * c);  
};


#define LTYPE(t) GetLLVMType(getModuleBuilder(), t).out
class GetLLVMType : public TypeVisitor {
public:
  ModuleBuilder * mb;
  ModuleBuilder * getModuleBuilder() { return mb; }
  LLVMTypeRef out;
  GetLLVMType(ModuleBuilder *mb, Type * t);
  virtual void visit(Type * f);
  virtual void visit(VoidType * f);
  virtual void visit(FuncType * f);
  virtual void visit(PtrType * f);
  virtual void visit(ArrType * f);
  virtual void visit(IntType * f);
  virtual void visit(FloatType * f);
};

// VAL(e): Convert an Expr * e to LLVMValueRef
#define VAL(e) ExprValue(getModuleBuilder(), e).output
class ExprValue : public Visitor {
public:
  ModuleBuilder * mb;
  ModuleBuilder * getModuleBuilder() { return mb; }
  LLVMValueRef output;
  ExprValue(ModuleBuilder * mb, Expr * e)
    : mb(mb) { visitorName = "VAL "; e->accept(this); }
  virtual void visit(String * s);
  virtual void visit(Long * s);
  virtual void visit(Double * s);
  virtual void visit(BinOp * s);
  virtual void visit(Var * s);
  virtual void visit(Call * s);
  virtual void visit(Cast * s);
  virtual void visit(AddrOf * s);
};

// Type-specialize over different operand types
class BinaryValue : public Visitor {
public:
  ModuleBuilder * mb;
  ModuleBuilder * getModuleBuilder() { return mb; }
  LLVMValueRef output;
  BinOp * op;
  BinaryValue(ModuleBuilder * mb, BinOp * e)
    : mb(mb), op(e) { op->lhs->accept(this); }
  void visitLong();
  void visitDouble();
  virtual void visit(Long * s) { visitLong(); }
  virtual void visit(Double * s) { visitDouble(); }
  virtual void visit(Var * s) { visitLong(); }
};
