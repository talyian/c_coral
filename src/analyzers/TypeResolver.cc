#include "utils/ansicolor.hh"
#include "core/expr.hh"
#include "core/prettyprinter.hh"
#include "TypeResolver.hh"

#include <iostream>
#include <iomanip>
#include <set>


using namespace coral;

namespace frobnob {
  // A TypeTerm is a variable we're solving for in the constraint system
  class TypeTerm {
  public:
    std::string name;
    ast::BaseExpr * expr;
    TypeTerm(std::string name, ast::BaseExpr * expr) : name(name), expr(expr) { }
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

  std::ostream & operator<<(std::ostream &out, TypeTerm &tptr) {
    return out << COL_RGB(5, 3, 4) << tptr.name << COL_CLEAR; }
  std::ostream & operator<<(std::ostream &out, TypeTerm * tptr) { return out << *tptr; }

  std::ostream & operator<<(std::ostream &out, TypeConstraint &tc) {
    tc.print_to(out); return out;
  }
  std::ostream & operator<<(std::ostream &out, TypeConstraint *tc) { return out << *tc; }
  std::ostream & operator<<(std::ostream &out, std::vector<TypeConstraint *> &vv) {
    for(auto &&tc : vv) {
      if (&tc != &vv.front()) out << ", ";
      out << tc;
    }
    return out;
  }

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



  class TypeEnvironment : ast::ExprVisitor {
  public:
    TypeEnvironment(ast::Module * m) { m->accept(this); }
    std::string visitorName() { return "TypeEnvironment"; }

    void Solve() {
      for(auto && tt : terms)
        std::cerr << tt << "\n";
      for(auto &&pair: constraints) {
        TypeTerm * term = pair.first;
        std::cerr << COL_RGB(5, 3, 4) << std::setw(15) << pair.first->name << COL_CLEAR
                  << " :: " << pair.second.get() << "\n";
      }
    }


    std::set<std::string> names;
    std::vector<TypeTerm *> terms;
    TypeTerm * AddTerm(std::string name, ast::BaseExpr * expr) {
      int i = 0; std::string name0 = name;
      while (names.find(name) != names.end())
        name = name0 + "." + std::to_string(i++);
      names.insert(name);
      terms.push_back(new TypeTerm(name, expr));
      return terms.back();
    }
    TypeTerm * FindTerm(ast::BaseExpr * expr) {
      for(auto &&term: terms) if (term->expr == expr) return term;
      return 0;
    }

    std::map<TypeTerm *, std::unique_ptr<TypeConstraint>> constraints;
    void AddConstraint(TypeTerm * term, TypeConstraint * tcons) {
      if (constraints.find(term) != constraints.end())
        std::cerr << COL_LIGHT_RED << "WARNING: overwritting constraint on " << term << COL_CLEAR << "\n";
      constraints[term] = std::unique_ptr<TypeConstraint>(tcons);
    }

    // Visitor Parts
    TypeTerm * out;

    void visit(ast::Module * m) { for(auto && line: m->body->lines) if (line) line->accept(this); }

    void visit(ast::Func * m) {
      auto func_term = AddTerm(m->name, m);
      auto func_type = new Type("Func");
      AddConstraint(func_term, func_type);

      for(auto && p: m->params) {
        out = AddTerm(m->name + "." + p->name, p.get());
        func_type->params.push_back(std::unique_ptr<Term>(new Term(out)));
      }

      m->body->accept(this);
      func_type->params.push_back(std::unique_ptr<Term>(new Term(out)));
      out = func_term;
    }

    void visit(ast::Block * b) {
      for(auto &&line: b->lines)
        if (line)
          line->accept(this);
    }

    void visit(ast::BinOp * op) {
      op->lhs->accept(this);
      auto lvar = out;
      op->rhs->accept(this);
      auto rvar = out;
      out = AddTerm("op:" + op->op, op);
      AddConstraint(out, new Call(new Type(op->op), {new Term(lvar), new Term(rvar)}));
    }

    void visit(ast::Var * var) {
      out = FindTerm(var->expr);
    }

    void visit(ast::IfExpr * e) {
      e->cond->accept(this);
      e->ifbody->accept(this);
      if (e->elsebody) {
        auto if_var = out;
        e->elsebody->accept(this);
        auto else_var = out;
        out = AddTerm("if", e);
        AddConstraint(out, new Type("UNION", {new Term(if_var), new Term(else_var)}));
      }
    }

    void visit(ast::IntLiteral * e) {
      out = AddTerm("i" + e->value, e);
      AddConstraint(out, new Type("Int32"));
    }

    void visit(ast::Call * c) {
      c->callee->accept(this);
      auto callee_term = out;
      auto c_out = AddTerm("call." + callee_term->name, c);
      auto call_con = new Call(new Term(callee_term), {});
      for(auto &&arg: c->arguments) {
        arg->accept(this);
        call_con->args.push_back(new Term(out));
      }
      AddConstraint(c_out, call_con);
    }
  };

}

coral::analyzers::TypeResolver::TypeResolver(ast::Module * m): module(m) {
  frobnob::TypeEnvironment env(m);
  env.Solve();
}
