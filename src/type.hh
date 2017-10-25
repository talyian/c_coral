#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace std;

#define foreach(COLL, IT) for(auto IT = COLL.begin(); IT != COLL.end(); IT++)

#define TYPE_LOOP(MACRO) MACRO() MACRO(Void) MACRO(Int) MACRO(Func) MACRO(Ptr) MACRO(Unknown) MACRO(Arr) MACRO(Float) MACRO(User) MACRO(Tuple)

class TypeVisitor;

class Type {
public:
  virtual string toString() { return "type"; }
  virtual void accept(TypeVisitor * v);
  virtual ~Type() { }
};

class UserType : public Type {
public:
  string name;
  UserType(string name) : name(name) { }
  virtual string toString() { return name; }
  virtual void accept(TypeVisitor * v);
};

class VoidType : public Type {
public:
  virtual void accept(TypeVisitor * v);
  virtual string toString() ;
};

class IntType : public Type {
public:
  int bits;
  IntType(int bits) : bits(bits) { }
  virtual void accept(TypeVisitor * v);
  string toString() ;
};

class FloatType : public Type {
public:
  int bits;
  FloatType(int bits) : bits(bits) { }
  virtual void accept(TypeVisitor * v);
  string toString() ;
};

class FuncType : public Type {
public:
  Type * ret;
  vector<Type *> args;
  bool variadic;
  FuncType(Type * ret, vector<Type *> args, bool variadic)
    : ret(ret), args(args), variadic(variadic) {
  }
  virtual void accept(TypeVisitor * v);
  string toString() ;
};

class UnknownType : public Type {
public:
  string info;
  UnknownType() : info ("unknown") { }
  UnknownType(string info) : info(info) { }
  virtual void accept(TypeVisitor * v);
  string toString();
};

class PtrType : public Type {
public:
  Type * inner;
  PtrType(Type * inner) : inner(inner) { }
  virtual void accept(TypeVisitor * v);
  string toString();
};

class ArrType : public Type {
public:
  Type * inner;
  int len = 0;
  ArrType(Type * inner) : inner(inner) { }
  virtual void accept(TypeVisitor * v);
  string toString();
};

class TupleType : public Type {
public:
  std::vector<Type *> inner;
  TupleType(std::vector<Type *> inner) : inner(inner) { }
  virtual void accept(TypeVisitor * v);
  string toString();
};

class VariadicTypeInfo : public Type {
public:
  string toString() { return "..."; }
};

class TypeVisitor {
public:
#define VISITDEF(TYPE) virtual void visit(TYPE##Type * t) { cerr << #TYPE << endl; }
TYPE_LOOP(VISITDEF)
#undef VISITDEF
};

class TypeName : public TypeVisitor {
public:
  string out;
  #define VISITDEF(TYPE) virtual void visit(TYPE##Type * t);
  TYPE_LOOP(VISITDEF)
  #undef VISITDEF
};

class TypeEqualsVisitor : public TypeVisitor {
public:
  string left_id = "", right_id = "", val = "";
  Type * lhs, *rhs;
  bool result = false;
  TypeEqualsVisitor(Type * a, Type * b) : lhs(a), rhs(b) {
    a->accept(this);
    left_id = val;
    b->accept(this);
    right_id = val;
    check();
  }

  void visit(IntType * i) { val = i->toString(); }
  void check() {
    result = left_id.length() > 0 && left_id == right_id;
  }
};

bool typeEquals(Type * a, Type * b);
std::string getTypeName(Type * t);


ostream& operator << (ostream & os, Type * t);
Type * BuildType(std::string name);
Type * BuildType(std::string name, std::vector<Type *> typeparams);
