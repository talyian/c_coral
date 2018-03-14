#include "analyzers/NameResolver.hh"
#include "utils/ansicolor.hh"

#include <iostream>
#include <string>
using namespace coral;

void analyzers::NameResolver::visit(ast::Module * m) { m->body->accept(this); }
void analyzers::NameResolver::visit(ast::Block * m) { for(auto && line : m->lines) if (line) line->accept(this); }
void analyzers::NameResolver::visit(ast::Comment * m) { }
void analyzers::NameResolver::visit(ast::IfExpr * m) {
  m->cond->accept(this);
  m->ifbody->accept(this);
  if (m->elsebody) m->elsebody->accept(this); }
void analyzers::NameResolver::visit(ast::Let * e) {
  // if we recurse before setting the name, this lets let a = foo a work
  e->value->accept(this);
  scope->insert(e->var->name, e);
}
void analyzers::NameResolver::visit(ast::Extern * x) {
  scope->insert(x->name, x);
}
void analyzers::NameResolver::visit(ast::BinOp * m) {
  m->lhs->accept(this);
  m->rhs->accept(this);

  if (scope->get(m->op).expr) {
    std::cerr << "Found op " << m->op << "\n";
    m->funcptr = dynamic_cast<ast::Func*>(scope->get(m->op).expr);
  } else if (scope->get("(" + m->op + ")").expr) {
    std::cerr << "Found parenthesized op " << m->op << "\n";
  } else {
    // we didn't get any custom definitions! we'll use builtin logic
    // (which honestly is a bit of a hack)
  }
}
void analyzers::NameResolver::visit(ast::Return * m) { if (m->val) m->val->accept(this); }
void analyzers::NameResolver::visit(ast::Call * c) {
  if (c->callee) c->callee->accept(this);
  for(auto && arg: c->arguments) if (arg) arg->accept(this);
}

void analyzers::NameResolver::visit(ast::Var * v) {
  // this is the core feature of NameResolver!
  v->expr = scope->get(v->name).expr;
}

void analyzers::NameResolver::visit(ast::Func * f) {
  // add self-param
  // TODO: This  is a bit poopy
  // should we really insert a self-param in name-resolution?
  // The issue is we want to insert it before resolving names
  // in the body since they will refer to it.
  if (!f->container.empty()) {
    auto container_type = type::Type(f->container.back());
    ast::Var Klass(f->container.back());
    Klass.accept(this);
    if (Klass.expr) {
      ast::Tuple * t = dynamic_cast<ast::Tuple *>(Klass.expr);
      f->tuple = t;
      if (t) {
        auto def = new ast::Def("self", new Type(t->name), 0);
        f->params.insert(
          f->params.begin(),
          std::unique_ptr<ast::Def>(def));
        scope->insert("self", def);
      } else {
        std::cerr << COL_LIGHT_RED << "unknown type kind " << f->container.back() << "\n";
      }
    } else {
      auto def = new ast::Def("self", new type::Type(container_type), 0);
      f->params.insert(
        f->params.begin(),
        std::unique_ptr<ast::Def>(def));
      scope->insert("self", def);
    }
  }

  // if a function is defined multiple times, we add it to
  // a OverloadedFunc type that will be removed by the typeresolver.
  if (!scope->get(f->name).expr) {
    scope->insert(f->name, f);
    for(auto && param : f->params) {
      scope->insert(param->name, param.get());
    }
  }
  else if (scope->get(f->name).kind == ast::ExprTypeKind::OverloadedFuncKind) {
    auto overload = dynamic_cast<ast::OverloadedFunc *>(scope->get(f->name).expr);
    overload->addOverload(f);
  }
  else {
    auto existing_expr = scope->get(f->name).expr;
    auto overload = new ast::OverloadedFunc(f->name);
    overload->addOverload(f);
    overload->addOverload(dynamic_cast<ast::Func *>(existing_expr));
    scope->insert(f->name, overload);
  }
  // this must happen after we've registered our name in info
  // in order for recursion to work.
  // DESIGN: we could require manual
  if (f->body) f->body->accept(this);
}

void analyzers::NameResolver::visit(ast::StringLiteral * e) { }

void analyzers::NameResolver::visit(ast::IntLiteral * i) { }

void analyzers::NameResolver::visit(ast::FloatLiteral * i) { }

void analyzers::NameResolver::visit(ast::Set * s) {
  s->var->accept(this);
  s->value->accept(this);
}

void analyzers::NameResolver::visit(ast::While * w) {
  w->cond->accept(this);
  w->body->accept(this);
}

void analyzers::NameResolver::visit(ast::Member * w) {
  w->base->accept(this);
}

void analyzers::NameResolver::visit(ast::Tuple * w) {
  scope->insert(w->name, w);
}

void analyzers::NameResolver::visit(ast::TupleLiteral * w) {
  for(auto &item:w->items)
    item->accept(this);
}
