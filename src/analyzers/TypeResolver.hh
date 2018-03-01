#pragma once

#include "core/expr.hh"
#include <iostream>

#include "analyzers/typegraph/constraint.hh"
#include "analyzers/typegraph/TypeGraph.hh"

namespace coral {
  namespace analyzers {
    class TypeResolver : public coral::ast::ExprVisitor {
    public:
      ast::Module * module = 0;
      ast::BaseExpr * target;
      std::string name;

      TypeGraph gg;
      std::map<TypeTerm *, ast::BaseExpr *> term_map;
      TypeTerm * out;

      TypeResolver(ast::Module * m);
      virtual std::string visitorName() { return "TypeResolver"; }
      void visit(ast::Module * m) { m->body->accept(this); }
      void visit(ast::Block * m) {
        for(auto &line : m->lines) if (line) line->accept(this); }
      void visit(ast::Def * d) {
        out = gg.AddTerm(d->name, d);
        if (d->type)
          gg.AddConstraint(out, gg.type(d->type.get()));
      }
      void visit(ast::Func * f) {
        auto term = gg.AddTerm(f->name, f);
        auto constraint = gg.type("Func");
        term_map[term] = f;
        for(auto &p: f->params) {
          p->accept(this);
          constraint->params.push_back(gg.term(out));
        }
        if (f->body) {
          f->body->accept(this);
          constraint->params.push_back(gg.term(out));
        }
        else if (f->type) {
          auto t = gg.type(f->type.get());
          constraint->params = t->params;
        } else {
          constraint->params.push_back(gg.free(100));
        }

        gg.AddConstraint(term, constraint);
        out = term;
      }

      void visit(ast::IfExpr * ifexpr) {
        ifexpr->cond->accept(this);
        gg.AddConstraint(out, gg.type("Bool"));
        auto term = gg.AddTerm("if");
        ifexpr->ifbody->accept(this);
        gg.AddConstraint(term, gg.term(out));
        ifexpr->elsebody->accept(this);
        gg.AddConstraint(term, gg.term(out));
        out = term;
      }

      void visit(ast::Var * var) {
        if (!(out = gg.FindTerm(var->expr)))
          std::cerr << "Missing Type " << var->name << "\n";
      }

      void visit(ast::BinOp * op) {
        op->lhs->accept(this);
        auto lvar = out;
        op->rhs->accept(this);
        auto rvar = out;

        out = gg.AddTerm("op." + op->op);

        if (op->op == "%" ||
            op->op == "+" ||
            op->op == "-" ||
            op->op == "*" ||
            op->op == "/")
          gg.AddConstraint(
            out, gg.call(
              gg.type("Func", {gg.free(0), gg.free(0), gg.free(0)}),
              {gg.term(lvar), gg.term(rvar)}));
        else if (op->op == "=")
          gg.AddConstraint(
            out, gg.call(
              gg.type("Func", {gg.free(0), gg.free(0), gg.type("Bool")}),
              {gg.term(lvar), gg.term(rvar)}));
      }
      void visit(ast::IntLiteral * op) {
        out = gg.AddTerm("i" + (op->value), op);
        gg.AddConstraint(out, gg.type("Int32"));
      }
      void visit(ast::Call * call) {
        call->callee->accept(this);
        auto calleevar = out;
        std::vector<Constraint *> args;
        for(auto &a: call->arguments) {
          a->accept(this);
          args.push_back(gg.term(out));
        }
        if (calleevar) {
          out = gg.AddTerm("call." + calleevar->name);
          gg.AddConstraint(out, gg.call(gg.term(calleevar), args));
        }
      }
      void visit(ast::Return * r) {
        r->val->accept(this);
      }
      void visit(ast::StringLiteral * s) {
        out = gg.AddTerm("str." + s->value, s);
        gg.AddConstraint(out, gg.type("Ptr"));
      }
      void visit(ast::Let * l) {
        l->value->accept(this);
        auto valueterm = out;
        out = gg.AddTerm(l->var->name, l);
        if (l->type.name.size())
          gg.AddConstraint(out, gg.type(&l->type));
        gg.AddConstraint(out, gg.term(valueterm));
      }
      void visit(ast::While * w) {
        w->body->accept(this);
        w->cond->accept(this);
        gg.AddConstraint(out, gg.type("Bool"));
        out = gg.AddTerm("while", w);
      }
      void visit(ast::Set * s) {
        s->value->accept(this);
        auto valueterm = out;
        auto term = gg.FindTerm(s->var->expr);
        if (term)
          gg.AddConstraint(term, gg.term(valueterm));
        else
          std::cerr << "Warning: term not found for set: " << s->var->name << "\n";
        out = gg.AddTerm("set", s);
      }
      void visit(ast::Extern * e) {
        out = gg.AddTerm(e->name, e);
        gg.AddConstraint(out, gg.type(e->type.get()));
      }
    };
  }
}
