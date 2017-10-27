#pragma once

/*
  Types
  -----

  These are primitive types used in the AST.
*/

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "utils.hh"

namespace coral {

#define TYPE_LOOP(MACRO) /*
*/ MACRO(Base) /*
*/ MACRO(Unknown) /*
*/ MACRO(Void) /*
*/ MACRO(Int) /*
*/ MACRO(Float) /*
*/ MACRO(Func) /*
*/ MACRO(Ptr) /*
*/ MACRO(Arr) /*
*/ MACRO(User) /*
*/ MACRO(Tuple)

class TypeVisitor;
class TypeNameVisitor;

class BaseType {
public:
  virtual std::string toString();
  virtual void accept(TypeVisitor * v);
  virtual ~BaseType() { }
};

typedef BaseType Type;

class UserType : public BaseType {
public:
  std::string name;
  UserType(std::string name) : name(name) { }
  virtual std::string toString() { return name; }
  virtual void accept(TypeVisitor * v);
};

class VoidType : public BaseType {
public:
  virtual void accept(TypeVisitor * v);
};

class IntType : public BaseType {
public:
  int bits;
  IntType(int bits) : bits(bits) { }
  virtual void accept(TypeVisitor * v);
  virtual std::string toString() { return "Int" + std::to_string(bits); }
};

class FloatType : public BaseType {
public:
  int bits;
  FloatType(int bits) : bits(bits) { }
  virtual void accept(TypeVisitor * v);
};

class FuncType : public BaseType {
public:
  BaseType * ret;
  std::vector<BaseType *> args;
  bool variadic;
  FuncType(BaseType * ret, std::vector<BaseType *> args, bool variadic)
    : ret(ret), args(args), variadic(variadic) {
  }
  virtual void accept(TypeVisitor * v);
  virtual std::string toString() { return "Func[" + ret->toString() + "]"; }
};

class UnknownType : public BaseType {
public:
  std::string info;
  UnknownType() : info ("unknown") { }
  UnknownType(std::string info) : info(info) { }
  virtual void accept(TypeVisitor * v);
};

class PtrType : public BaseType {
public:
  BaseType * inner;
  PtrType(BaseType * inner) : inner(inner) { }
  virtual void accept(TypeVisitor * v);
};

class ArrType : public BaseType {
public:
  BaseType * inner;
  int len = 0;
  ArrType(BaseType * inner) : inner(inner) { }
  virtual void accept(TypeVisitor * v);
};

class TupleType : public BaseType {
public:
  std::vector<BaseType *> inner;
  TupleType(std::vector<BaseType *> inner) : inner(inner) { }
  virtual void accept(TypeVisitor * v);
};

class VariadicTypeInfo : public BaseType {
public:
  std::string toString() { return "..."; }
};

enum TypeKind {
#define DEFKIND(TYPE) TYPE##TypeKind,
TYPE_LOOP(DEFKIND)
#undef DEFKIND
};

class TypeVisitor {
public:
#define VISITDEF(TYPE) virtual void visit(__attribute__((unused)) TYPE##Type * t) { }
  TYPE_LOOP(VISITDEF)
#undef VISITDEF
};

class TypeKindVisitor : public TypeVisitor {
public:
  TypeKind out = UnknownTypeKind;
  TypeKindVisitor(BaseType * t) { t->accept(this); }
#define VISITDEF(TYPE) void visit(__attribute__((unused)) TYPE##Type * t) { out = TYPE##TypeKind; }
  TYPE_LOOP(VISITDEF)
#undef VISITDEF
};

class TypeNameVisitor : public TypeVisitor {
public:
  std::string out = "";
  TypeNameVisitor(BaseType * t) { t -> accept(this); }
#define VISITDEF(TYPE) virtual void visit(__attribute__((unused)) TYPE##Type * t) { out = #TYPE; }
  TYPE_LOOP(VISITDEF)
#undef VISITDEF
};

  TypeKind getTypeKind(BaseType * t);

  std::string getTypeName(BaseType * t);

  std::ostream& operator << (std::ostream& os, BaseType * t);

  Type * BuildType(std::string name, std::vector<Type *> typeparams);

  Type * BuildType(std::string name);

}
