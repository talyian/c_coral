#pragma once
#include "utils/ansicolor.hh"
#include "core/expr.hh"

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <iostream>

class TypeTerm {
public:
  std::string name;
  coral::ast::BaseExpr * expr;
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
  virtual ~Constraint() { }
};

class Term : public Constraint {
  public:
  std::string name;
  TypeTerm * term = 0;
  Term(std::string name, TypeTerm * term): name(name), term(term) { }
  Term(TypeTerm * term): name(term->name), term(term) { }
  std::string toString() {
    return "\033[38;5;128m" + name + "\033[0m";
    return "\033[38;5;128m" + name + "(" + std::to_string((unsigned long)term) + ")" + "\033[0m";
  }
  virtual void accept(ConstraintVisitor *);
};

class Type : public Constraint {
  public:
  std::string name;
  std::vector<Constraint *> params;
  Type(std::string name) : name(name) { }
  Type(std::string name,   std::vector<Constraint *> params) : name(name), params(params) { }
  virtual void accept(ConstraintVisitor *);
  std::string toString();
};

class Free : public Constraint {
public:
  int v;
  Free(int v): v(v) { }
  std::string toString();
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
  std::string toString();
};

// 2D Visitor
template<class R, class TLeft>
class TypeEqualityRight : public ConstraintVisitor {
  public:
  R res;
  TLeft lhs;
  TypeEqualityRight(R res, TLeft lhs) : res(res), lhs(lhs) { }
#define VISIT { res->equal(lhs, c); }
  void visit(Type * c) VISIT
  void visit(Term * c) VISIT
  void visit(Free * c) VISIT
  void visit(Call * c) VISIT
#undef VISIT
};

template<class R>
class TypeEqualityWrapper : public ConstraintVisitor {
  public:
  R res;
  Constraint * c, *d;

  TypeEqualityWrapper(R res, Constraint * c, Constraint * d) : res(res), c(c), d(d) {
    c->accept(this); }
#define VISIT { TypeEqualityRight<R, decltype(c)> ter(res, c); d->accept(&ter); }
  void visit(Type * c) VISIT
  void visit(Term * c) VISIT
  void visit(Free * c) VISIT
  void visit(Call * c) VISIT
#undef VISIT
};

// Checks if two constraints have equal values
class ConstraintEqualsImpl {
public:
  static bool of(Constraint * a, Constraint * b);
  bool out = false;
  void equal(Constraint * a, Constraint * b) { out = false; }
  void equal(Free * a, Free * b);
  void equal(Term * a, Term * b);
  void equal(Type * a, Type * b);
};


std::ostream& operator << (std::ostream&out, TypeTerm * tt);

std::ostream& operator << (std::ostream&out, Constraint * cc);
