#include "NameResolver.hh"
#include <iostream>

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
  info[e->var->name].expr = e;
  info[e->var->name].kind = ast::ExprTypeKind::LetKind;
}
void analyzers::NameResolver::visit(ast::Extern * x) {
  info[x->name].expr = x;
  info[x->name].kind = ast::ExprTypeKind::ExternKind;
}
void analyzers::NameResolver::visit(ast::BinOp * m) { m->lhs->accept(this); m->rhs->accept(this); }
void analyzers::NameResolver::visit(ast::Return * m) { if (m->val) m->val->accept(this); }
void analyzers::NameResolver::visit(ast::Call * c) {
  if (c->callee) c->callee->accept(this);
  for(auto && arg: c->arguments) if (arg) arg->accept(this);
}

void analyzers::NameResolver::visit(ast::Var * v) {
  auto expr = info[v->name].expr;
  // std::cerr << "Var! " << v->name
  // 		  << " ("  << (void *) expr << ") "
  // 		  << ast::ExprNameVisitor::of(expr) << std::endl;
  v->expr = info[v->name].expr;
}

void analyzers::NameResolver::visit(ast::Func * f) {
  info[f->name].expr = f;
  info[f->name].kind = ast::ExprTypeKind::FuncKind;
  for(auto && param : f->params) {
	info[param->name].expr = param.get();
	info[param->name].kind = ast::ExprTypeKind::DefKind;
  }
  if (f->body) f->body->accept(this);
}

void analyzers::NameResolver::visit(ast::StringLiteral * e) { }

void analyzers::NameResolver::visit(ast::IntLiteral * i) { }

void analyzers::NameResolver::visit(ast::FloatLiteral * i) { }

void analyzers::NameResolver::visit(ast::Set * s) {
  s->var->accept(this);
  s->value->accept(this);
  // info[s->var->name].expr = s;
  // info[s->var->name].kind = ast::ExprTypeKind::SetKind;
}

void analyzers::NameResolver::visit(ast::While * w) {
  w->cond->accept(this);
  w->body->accept(this);
}

void analyzers::NameResolver::visit(ast::Member * w) {
  w->base->accept(this);
}

void analyzers::NameResolver::visit(ast::Tuple * w) {
  info[w->name].expr = w;
  info[w->name].kind = ast::ExprTypeKind::TupleKind;
}

void analyzers::NameResolver::visit(ast::TupleLiteral * w) {
  for(auto &item:w->items)
    item->accept(this);
}
