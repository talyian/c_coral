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
  class TypeResolver : public ast::ExprVisitor {
  public:
    TypeEnvironment env;

    TypeTerm * out;

    std::string visitorName() { return "FrobResolver"; }

    TypeResolver(ast::Module * m) {
      m->accept(this);
      env.Solve();
    }

    void visit(ast::Module * m) {
      for(auto && line: m->body->lines)
        if (line) line->accept(this);
    }

    void visit(ast::Func * m) {
      auto func_term = env.AddTerm(m->name, m);
      auto func_type = env.newType("Func");
      env.AddConstraint(func_term, func_type);

      for(auto && p: m->params) {
        out = env.AddTerm(m->name + "." + p->name, p.get());
        if (p->type)
          env.AddConstraint(out, env.newType(*(p->type)));
        func_type->params.emplace_back(env.newTerm(out));
      }
      if (!m->body) {
        type::Type t = m->type->params.back();
        func_type->params.emplace_back(env.newType(t));
      } else {
        m->body->accept(this);
        func_type->params.emplace_back(env.newTerm(out));
      }
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
      env.AddConstraint(out, env.newCall(env.Global_Ops(op->op), std::vector<TypeConstraint *> {
            env.newTerm(lvar),
            env.newTerm(rvar)}));
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
        env.AddConstraint(out, env.newTerm(if_var));
        env.AddConstraint(out, env.newTerm(else_var));
      }
    }

    void visit(ast::IntLiteral * e) {
      out = env.AddTerm("i" + e->value, e);
      // TODO: each int value should be its own type,
      // and Int32 should be inferred as a supertype
      // This lets us get value constraints out of our system
      env.AddConstraint(out, env.newType("Int32"));
    }

    void visit(ast::Call * c) {
      c->callee->accept(this);
      auto callee_term = out;
      if (!callee_term) return;
      auto c_out = env.AddTerm("call." + callee_term->name, c);
      auto call_con = env.newCall(env.newTerm(callee_term));
      for(auto &&arg: c->arguments) {
        arg->accept(this);
        call_con->args.push_back(env.newTerm(out));
      }
      env.AddConstraint(c_out, call_con);
      out = c_out;
    }

    void visit(ast::Let * l) {
      auto letterm = env.AddTerm(l->var->name, l);
      l->value->accept(this);
      if (l->type.name != "") env.AddConstraint(letterm, env.newType(l->type));
      else env.AddConstraint(letterm, env.newTerm(out));
    }

    void visit(__attribute__((unused)) ast::Comment * c) { out = 0; }
    void visit(ast::Return * r) {
      r->val->accept(this);
    }
    void visit(ast::StringLiteral * s) {
      out = env.AddTerm("str", s);
      env.AddConstraint(out, env.newType("Ptr"));
    }

    void visit(ast::While * w) {
      w->cond->accept(this);
      w->body->accept(this);
      out = env.AddTerm("while", w);
      env.AddConstraint(out, env.newType("Void"));
    }

    void visit(ast::Set * s) {
      s->value->accept(this);
      auto valueterm = out;
      s->var->accept(this);
      auto varterm = out;
      env.AddConstraint(valueterm, env.newTerm(varterm));
      out = env.AddTerm("set." + valueterm->name, s);
    }

  };
}

coral::analyzers::TypeResolver::TypeResolver(ast::Module * m): module(m) {
  frobnob::TypeResolver resolver(m);

  // rewrite types back into the AST
  for(auto &&pair : resolver.env.critical_constraints) {
    if (!pair.first) continue;
    auto expr = pair.first->expr;
    auto tvalue = dynamic_cast<frobnob::Type *>(pair.second);
    if (tvalue) {
      auto tvaluetype = tvalue->concrete_type();
      if (tvaluetype){
        if (auto let = dynamic_cast<ast::Let *>(expr)) {
          if (let->type.name == "")
            let->type = *tvaluetype;
        }
        else if (auto func = dynamic_cast<ast::Func *>(expr)) {
          if (func->type->returnType().name == "")
            func->type.reset(tvalue->concrete_type());
          for(size_t i = 0; i < func->params.size(); i++) {
            if (!func->params[i]->type || func->params[i]->type->name == "") {
              frobnob::Type * v = (frobnob::Type *)(tvalue->params[i]);
              func->params[i]->type.reset(v->concrete_type());
            }
          }
        }
        delete tvaluetype;
      }
    }
  }
}
