#include <string>
#include <vector>
#include <iostream>

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

class IntGeneric : public Type {
};
  
class IntType : public Type {
public:
  int bits;
  IntType(int length) : bits(length) { }
  virtual void accept(TypeVisitor * v);
  virtual std::string toString();
};

class FloatType : public Type {
public:
  char type;
  int bits;
  FloatType(char type, int length) : type(type), bits(length) { }
  virtual void accept(TypeVisitor * v);
  virtual std::string toString();
};

class VoidType : public Type {
public:
  virtual void accept(TypeVisitor * v);
  virtual std::string toString();
};

#define ACTION(t) std::cerr << "unhandled: " << (t ? t->toString() : "(null)") << std::endl
class TypeVisitor {
public:
  virtual void visit(Type * t) { ACTION(t); }
  virtual void visit(FuncType * t) { ACTION(t); }
  virtual void visit(PtrType * t) { ACTION(t); }
  virtual void visit(ArrType * t) { ACTION(t); }
  virtual void visit(IntType * t) { ACTION(t); }
  virtual void visit(VoidType * t) { ACTION(t); }
  virtual void visit(FloatType * t) { ACTION(t); }  
};
