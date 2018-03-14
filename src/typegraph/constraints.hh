#pragma once
#include <string>
#include <vector>
#include <iostream>

#ifndef ExprType
#define ExprType void *
#endif
namespace typegraph {
  class TypeTerm {
  public:
    std::string name;
    ExprType expr;
  };
  std::ostream& operator <<(std::ostream& out, TypeTerm * t);

#define CONS_LOOP(F) F(Type) F(Term) F(Free) F(Call)
#define CONS_LOOP_F1(F, T1) F(T1, Free) F(T1, Term) F(T1, Call) F(T1, Type)
  class Constraint;
  class Type;
  class Free;
  class Term;
  class Call;

  class ConstraintVisitor {
  public:
#define VISIT(T) virtual void visit(T * f);
    CONS_LOOP(VISIT)
#undef VISIT
    virtual ~ConstraintVisitor() { }
  };

  class Constraint {
  public:
    Constraint() { }
    virtual ~Constraint() { }
    virtual void print(std::ostream& out) { out << "constraint"; }
    virtual void accept(ConstraintVisitor *) { }
  };
  std::ostream& operator <<(std::ostream& out, Constraint * c);

  class Type : public Constraint {
  public:
    Type(std::string name, std::vector<Constraint *> params) : name(name), params(params) { }
    std::string name;
    std::vector<Constraint *> params;
    virtual void print(std::ostream& out) {
      out << name;
      if (params.empty()) return;
      out << "[";
      for(auto & c: params) {
        c->print(out);
        if (&c != &params.back()) out << ", ";
      }
      out << "]";
    }
    virtual void accept(ConstraintVisitor * v) { v->visit(this); }
  };

  class Free : public Constraint {
  public:
    Free(int index) : index(index) { }
    int index;
    virtual void print(std::ostream& out) { out << "\033[38;5;96mτ" << index << "\033[0m"; }
    virtual void accept(ConstraintVisitor * v) { v->visit(this); }
  };

  class Term : public Constraint {
  public:
    Term(TypeTerm * term) : term(term) { }
    TypeTerm * term;
    virtual void print(std::ostream& out) { out << "\033[38;5;45m" << term << "\033[0m"; }
    virtual void accept(ConstraintVisitor * v) { v->visit(this); }
  };

  class Call : public Constraint {
  public:
    Call(Constraint * callee, std::vector<Constraint *> arguments)
      : callee(callee), arguments(arguments) { }
    Constraint * callee;
    std::vector<Constraint *> arguments;
    virtual void print(std::ostream& out) {
      out << "call(" << callee;
      for(auto * a: arguments)
        out << ", " << a;
      out << ")";
    }
    virtual void accept(ConstraintVisitor * v) { v->visit(this); }
  };


  bool isConcreteType(Constraint *);

  class ConstraintVisitorDouble : public ConstraintVisitor {
    virtual void visit2(Constraint *, Constraint *) { std::cout << "hmmmm\n"; }
#define VISIT2(T1, T2) virtual void visit2(T1 *, T2 *) { }
#define VISIT(T1) CONS_LOOP_F1(VISIT2, T1)
    CONS_LOOP(VISIT)
#undef VISIT
#undef VISIT2
    template <class T>
    class RightVisitor : public ConstraintVisitor {
    public:
      ConstraintVisitorDouble * dd;
      T * right;
      RightVisitor(ConstraintVisitorDouble * dd, T * rhs) : dd(dd), right(rhs) { }
      void visit(Type * left) { dd->visit2(left, right); }
      void visit(Term * left) { dd->visit2(left, right); }
      void visit(Free * left) { dd->visit2(left, right); }
      void visit(Call * left) { dd->visit2(left, right); }
    };
#define VISIT(T) virtual void visit(T * f) { RightVisitor<T> rv(this, f); left->accept(&rv); }
    CONS_LOOP(VISIT)
#undef VISIT
  public:
    Constraint * left, * right;
    ConstraintVisitorDouble(Constraint * left, Constraint * right) : left(left), right(right) { }
    void run() { right->accept(this); }
    virtual ~ConstraintVisitorDouble() { }
  };

  class ConsEquals : public ConstraintVisitorDouble {
  public:
    bool out = false;
    ConsEquals(Constraint * a, Constraint * b) : ConstraintVisitorDouble(a, b) {
      if (a && b) run(); };
    void visit2(Free * f, Free * g) { out = f->index == g->index; }
    void visit2(Term * x, Term * y) { out = x->term == y->term; }
    void visit2(Type * t, Type * u) {
      out = true;
      if (t->name != u->name) { out = false; return; }
      if (t->params.size() != u->params.size()) { out = false; return; }
      for(size_t i = 0; i < t->params.size(); i++) {
        if (!ConsEquals(t->params[i], u->params[i]).out) { out = false; return; }
      }
    }
  };
}
