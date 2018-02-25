#pragma once
#include "utils/ansicolor.hh"
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <iostream>

class TypeTerm {
public:
  std::string name;
  TypeTerm(std::string name) : name(name) { }
};

class Constraint;
class Type;
class Term;
class Free;
class Call;

class ConstraintVisitor {
  public:
  virtual void visit(Constraint * c) { std::cerr << "error!\n"; }
  virtual void visit(Type * c) = 0;
  virtual void visit(Term * c) = 0;
  virtual void visit(Free * c) = 0;
  virtual void visit(Call * c) = 0;
};

class Constraint {
public:
  virtual std::string toString() { return "constraint"; }
  virtual void accept(ConstraintVisitor *);
};

class Term : public Constraint {
  public:
  std::string name;
  TypeTerm * term = 0;
  Term(std::string name, TypeTerm * term): name(name), term(term) { }
  std::string toString() { return name; }
  virtual void accept(ConstraintVisitor *);
};
class Type : public Constraint {
  public:
  std::string name;
  std::vector<Constraint *> params;
  Type(std::string name) : name(name) { }
  Type(std::string name,   std::vector<Constraint *> params) : name(name), params(params) { }
  virtual void accept(ConstraintVisitor *);
  std::string toString() {
    auto s = name;
    if(params.size()) {
      s += "(";
      for(auto &p : params) { if (&p != &params.front()) s += ", "; s += p->toString();}
      s += ")";
    }
    return s;
  }
};
class Free : public Constraint {
public:
  int v;
  Free(int v): v(v) { }
  std::string toString() { return "*T" + std::to_string(v); }
virtual void accept(ConstraintVisitor *);
};
class Call : public Constraint {
public:
  Constraint * callee;
  std::vector<Constraint *> args;
  Call(
    Constraint * callee,
    std::vector<Constraint *> args
    ): callee(callee), args(args) { }
  virtual void accept(ConstraintVisitor *);
  std::string toString() {
    auto s = "Call(" + callee->toString();
    if(args.size()) {
      s += ", ";
      for(auto &p : args) { if (&p != &args.front()) s += ", "; s += p->toString();}
    }
    s += ")";
    return s;
  }
};

// 2D Visitor!
template<class R, class TLeft>
class TypeEqualityRight : public ConstraintVisitor {
  public:
  R res;
  TLeft * lhs;
  TypeEqualityRight(R res, TLeft * lhs) : res(res), lhs(lhs) { }
#define VISIT { res->equal(lhs, c); }
  void visit(Type * c) VISIT
  void visit(Term * c) VISIT
  void visit(Free * c) VISIT
  void visit(Call * c) VISIT
};

template<class R, class T>
TypeEqualityRight<R, T> * makeTQR(R r, T * o) { return new TypeEqualityRight<R, T>(r, o); }

template<class R>
class TypeEqualityWrapper : public ConstraintVisitor {
  public:
  R res;
  Constraint * c, *d;
  TypeEqualityWrapper(R res, Constraint * c, Constraint * d) : res(res), c(c), d(d) {
    c->accept(this); }
#define VISIT { d->accept(makeTQR(res, c)); }
  void visit(Type * c) VISIT
  void visit(Term * c) VISIT
  void visit(Free * c) VISIT
  void visit(Call * c) VISIT
#undef VISIT
};


std::ostream& operator << (std::ostream&out, TypeTerm * tt);

std::ostream& operator << (std::ostream&out, Constraint * cc);
