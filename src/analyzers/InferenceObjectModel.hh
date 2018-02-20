#include "core/expr.hh"
#include <iostream>
#include <vector>

namespace frobnob {
  // A TypeTerm is a variable we're solving for in the constraint system
  class TypeTerm {
  public:
    std::string name;
    coral::ast::BaseExpr * expr;
    TypeTerm(std::string name, coral::ast::BaseExpr * expr) : name(name), expr(expr) { }
  };

  // A typeconstraint is a formula on TypeTerms
  class TypeConstraint {
  public:
    virtual void print_to(std::ostream& out) { out << "(constraint)"; }
  };
  class Term;
  class Type;
  class Call;
  class FreeType;
  class And;
  class Equal;

  std::ostream & operator<<(std::ostream &out, TypeTerm &tptr);
  std::ostream & operator<<(std::ostream &out, TypeTerm * tptr);
  std::ostream & operator<<(std::ostream &out, TypeConstraint &tc);
  std::ostream & operator<<(std::ostream &out, TypeConstraint *tc);
  std::ostream & operator<<(std::ostream &out, std::vector<TypeConstraint *> &vv);
  TypeConstraint * Global_Ops(std::string op);

  class Type : public TypeConstraint {
  public:
    std::string name;
    std::vector<std::unique_ptr<TypeConstraint>> params;
    Type(std::string name) : name(name) { }
    Type(std::string name, std::vector<TypeConstraint *> pp) : name(name) {
      for(auto &&p:pp) params.emplace_back(p);
    }
    virtual void print_to(std::ostream& out) {
      out << name;
      if (params.size()) {
        out << "(";
        for(auto &&p: params) {
          if (&p != &params.front()) out << ", ";
          p->print_to(out);
        }
        out << ")"; }
    }
  };

  class Term : public TypeConstraint {
  public: TypeTerm * term;
    Term(TypeTerm * term) : term(term) { }
    virtual void print_to(std::ostream &out) {
      out << COL_RGB(5, 3, 4) << term->name << COL_CLEAR;
    }
  };

  class Call : public TypeConstraint {
  public:
    TypeConstraint * callee;
    std::vector<TypeConstraint *> args;
    Call(TypeConstraint * callee, std::vector<TypeConstraint *> args)
      : callee(callee), args(args) { }
    virtual void print_to(std::ostream &out) {
      out << "Call(" << callee << ", " << args << ")";
    }
  };

  class FreeType : public TypeConstraint {
  public:
    int id;
    FreeType() {
      static int maxid = 0;
      id = maxid++;
    }
    virtual void print_to(std::ostream &out) { out << "T" << id; }
  };

  class And : public TypeConstraint {
  public:
    std::vector<TypeConstraint *> terms;
    And(std::vector<TypeConstraint *> terms) : terms(terms) { }
    virtual void print_to(std::ostream & out) { out << "And(" << terms << ")"; }
  };

}
