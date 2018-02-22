#include "utils/ansicolor.hh"
#include "core/expr.hh"
#include "core/prettyprinter.hh"
#include "InferenceEnvironment.hh"
#include "TypeResolver.hh"

#include <iostream>
#include <iomanip>
#include <set>
#include <algorithm>

namespace coral {
  namespace typeinference {
    TypeResolver::TypeResolver(ast::Module * m) {
      m->accept(this);
      env.Solve();
    }

    void TypeResolver::visit(ast::Module * m) {
      for(auto && line: m->body->lines)
        if (line) line->accept(this);
    }

    void TypeResolver::visit(ast::Func * m) {
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

    void TypeResolver::visit(ast::Block * b) {
      for(auto &&line: b->lines)
        if (line)
          line->accept(this);
    }

    void TypeResolver::visit(ast::BinOp * op) {
      op->lhs->accept(this);
      auto lvar = out;
      op->rhs->accept(this);
      auto rvar = out;
      out = env.AddTerm("op:" + op->op, op);
      env.AddConstraint(out, env.newCall(env.Global_Ops(op->op), std::vector<TypeConstraint *> {
            env.newTerm(lvar),
            env.newTerm(rvar)}));
    }

    void TypeResolver::visit(ast::Var * var) {
      out = env.FindTerm(var->expr);
      if (!out) {
        if (var->name == "struct") {
          out = env.AddTerm("global::struct", var);
          return;
        }
        std::cerr << COL_LIGHT_MAGENTA << var->name << COL_CLEAR << "\n";
      }
    }

    void TypeResolver::visit(ast::IfExpr * e) {
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

    void TypeResolver::visit(ast::IntLiteral * e) {
      out = env.AddTerm("i" + e->value, e);
      // TODO: each int value should be its own type,
      // and Int32 should be inferred as a supertype
      // This lets us get value constraints out of our system
      env.AddConstraint(out, env.newType("Int32"));
    }

    void TypeResolver::visit(ast::Call * c) {
      c->callee->accept(this);
      auto callee_term = out;
      if (!callee_term) {
        for(auto &&arg: c->arguments) {
          arg->accept(this);
        }
        return;
      }
      auto c_out = env.AddTerm("call." + callee_term->name, c);
      auto call_con = env.newCall(env.newTerm(callee_term));
      for(auto &&arg: c->arguments) {
        arg->accept(this);
        call_con->args.push_back(env.newTerm(out));
      }
      env.AddConstraint(c_out, call_con);
      out = c_out;
    }

    void TypeResolver::visit(ast::Let * l) {
      auto letterm = env.AddTerm(l->var->name, l);
      l->value->accept(this);
      if (l->type.name != "") env.AddConstraint(letterm, env.newType(l->type));
      else env.AddConstraint(letterm, env.newTerm(out));
    }

    void TypeResolver::visit(__attribute__((unused)) ast::Comment * c) { out = 0; }
    void TypeResolver::visit(ast::Return * r) {
      r->val->accept(this);
    }
    void TypeResolver::visit(ast::StringLiteral * s) {
      out = env.AddTerm("str", s);
      env.AddConstraint(out, env.newType("Ptr"));
    }

    void TypeResolver::visit(ast::While * w) {
      w->cond->accept(this);
      w->body->accept(this);
      out = env.AddTerm("while", w);
      env.AddConstraint(out, env.newType("Void"));
    }

    void TypeResolver::visit(ast::Set * s) {
      s->value->accept(this);
      auto valueterm = out;
      s->var->accept(this);
      auto varterm = out;
      env.AddConstraint(valueterm, env.newTerm(varterm));
      out = env.AddTerm("set." + valueterm->name, s);
    }

    void TypeResolver::visit(ast::Member * m) {
      m->base->accept(this);
      out = 0;
      // s->value->accept(this);
      // auto valueterm = out;
      // s->var->accept(this);
      // auto varterm = out;
      // env.AddConstraint(valueterm, env.newTerm(varterm));
      // out = env.AddTerm("set." + valueterm->name, s);
    }

  };
}

coral::analyzers::TypeResolver::TypeResolver(ast::Module * m): module(m) {
  typeinference::TypeResolver resolver(m);

  // rewrite types back into the AST
  for(auto &&pair : resolver.env.critical_constraints) {
    if (!pair.first) continue;
    auto expr = pair.first->expr;
    auto tvalue = dynamic_cast<typeinference::Type *>(pair.second);
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
              typeinference::Type * v = (typeinference::Type *)(tvalue->params[i]);
              func->params[i]->type.reset(v->concrete_type());
            }
          }
        }
        delete tvaluetype;
      }
    }
  }
}
