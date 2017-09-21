#include <string>
#include <vector>

class TypeVisitor;

class Type {
public:
  virtual std::string toString() { return "type?"; }
  virtual void accept(TypeVisitor * v);
  const char * tostr() { return this->toString().c_str(); }
};

std::ostream& operator << (std::ostream& os, Type * tptr);

class FuncType : public Type {
public:
  Type * ret;
  std::vector<Type *> args;
  bool variadic;
  FuncType(Type * ret, std::vector<Type *> args, bool isvar) : ret(ret), args(args), variadic(isvar) {
  }
  virtual void accept(TypeVisitor * v);  
  virtual std::string toString();
};

class PtrType : public Type {
public:
  Type * inner;
  PtrType(Type * inner) : inner(inner) { }
  virtual void accept(TypeVisitor * v);
  virtual std::string toString();
};

class ArrType : public Type {
public:
  Type * inner; int len;
  ArrType(Type * inner, int len) : inner(inner), len(len) { }
  virtual void accept(TypeVisitor * v);
  virtual std::string toString() { return "Arr[" + inner->toString() + "," + std::to_string(len) + "]"; }
};

class IntType : public Type {
public:
  int bits;
  IntType(int length) : bits(length) { }
  virtual void accept(TypeVisitor * v);
  virtual std::string toString();
};

class VoidType : public Type {
public:
  virtual void accept(TypeVisitor * v);
  virtual std::string toString();
};

class TypeVisitor {
public:
  virtual void visit(Type * t) { }
  virtual void visit(FuncType * t) { }
  virtual void visit(PtrType * t) { }
  virtual void visit(ArrType * t) { }  
  virtual void visit(IntType * t) { }
  virtual void visit(VoidType * t) { }  
};
