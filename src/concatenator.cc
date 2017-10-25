#include "concatenator.hh"

Expr * StaticConcat(Expr * m)  { return Concatenator(m).out; }

void Concatenator::visit(Module * m) {
  foreach(m->lines, it) { (*it) = StaticConcat(*it); }
  out = m;
}

void Concatenator::visit(FuncDef * m) {
  m->body = StaticConcat(m->body);
  out = m;
}

void Concatenator::visit(Let * m) {
  m->value = StaticConcat(m->value);
  out = m;
}

void Concatenator::visit(BinOp * m) {
  if (EXPRNAME(m->lhs) == "String" &&
      EXPRNAME(m->rhs) == "String") {
    String * l = (String *)m->lhs;
    String * r = (String *)m->rhs;
    out = new String(l->value + r->value);
  } else {
    m->lhs = StaticConcat(m->lhs);
    m->rhs = StaticConcat(m->rhs);
    out = m;
  }
}

void Concatenator::visit(If * m) {
  m->cond = StaticConcat(m->cond);
  m->ifbody = (BlockExpr *)StaticConcat(m->ifbody);
  m->elsebody = (BlockExpr *)StaticConcat(m->elsebody);
  out = m;
}

void Concatenator::visit(BlockExpr * m) {
  foreach(m->lines, it) (*it) = StaticConcat(*it);
  out = m;
}

void Concatenator::visit(Call * c) {
  foreach(c->arguments, it) (*it) = StaticConcat(*it);
}

void Concatenator::visit(Var * c) { }
void Concatenator::visit(Long * c) {}
void Concatenator::visit(String * c) { }
void Concatenator::visit(VoidExpr * c) { }
