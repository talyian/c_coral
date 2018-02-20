#include "utils/ansicolor.hh"
#include "core/expr.hh"
#include "core/prettyprinter.hh"
#include "TypeResolver.hh"
#include "InferenceEnvironment.hh"

#include <iostream>
#include <iomanip>
#include <set>
#include <algorithm>


using namespace coral;

namespace frobnob {
  class TypeResolver : ast::ExprVisitor {
  public:
    TypeEnvironment env;

    TypeTerm * out;

    std::string visitorName() { return "TypeEnvironment"; }

    TypeResolver(ast::Module * m) {
      m->accept(this);
      env.Solve();
    }

    void visit(ast::Module * m) { for(auto && line: m->body->lines) if (line) line->accept(this); }

    void visit(ast::Func * m) {
      auto func_term = env.AddTerm(m->name, m);
      auto func_type = new Type("Func");
      env.AddConstraint(func_term, func_type);

      for(auto && p: m->params) {
        out = env.AddTerm(m->name + "." + p->name, p.get());
        func_type->params.push_back(std::unique_ptr<Term>(new Term(out)));
      }
      if (!m->body) return;
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
      out = env.AddTerm("op:" + op->op, op);
      env.AddConstraint(out, new Call(Global_Ops(op->op), {new Term(lvar), new Term(rvar)}));
    }

    void visit(ast::Var * var) {
      out = env.FindTerm(var->expr);
    }

    void visit(ast::IfExpr * e) {
      e->cond->accept(this);
      e->ifbody->accept(this);
      if (e->elsebody) {
        auto if_var = out;
        e->elsebody->accept(this);
        auto else_var = out;
        out = env.AddTerm("if", e);
        env.AddConstraint(out, new Type("UNION", {new Term(if_var), new Term(else_var)}));
      }
    }

    void visit(ast::IntLiteral * e) {
      out = env.AddTerm("i" + e->value, e);
      // TODO: each int value should be its own type,
      // and Int32 should be inferred as a supertype
      // This lets us get value constraints out of our system
      env.AddConstraint(out, new Type("Int32"));
    }

    void visit(ast::Call * c) {
      c->callee->accept(this);
      auto callee_term = out;
      if (!callee_term) return;
      auto c_out = env.AddTerm("call." + callee_term->name, c);
      auto call_con = new Call(new Term(callee_term), {});
      for(auto &&arg: c->arguments) {
        arg->accept(this);
        call_con->args.push_back(new Term(out));
      }
      env.AddConstraint(c_out, call_con);
    }

    void visit(__attribute__((unused)) ast::Comment * c) { out = 0; }
  };
}

coral::analyzers::TypeResolver::TypeResolver(ast::Module * m): module(m) {
  frobnob::TypeResolver env(m);
}
