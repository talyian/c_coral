#include "analyzers/typegraph/constraint.hh"

#include <sstream>

std::ostream& operator << (std::ostream&out, TypeTerm * tt) {
  return tt ? (out << tt->name) : (out << "(null)"); }

std::ostream& operator << (std::ostream&out, Constraint * cc) {
  return cc ? (out << cc->toString()) : (out << "(null)"); }

void Constraint::accept(ConstraintVisitor * v) { v->visit(this); }
void Type::accept(ConstraintVisitor * v) { v->visit(this); }
void Term::accept(ConstraintVisitor * v) { v->visit(this); }
void Free::accept(ConstraintVisitor * v) { v->visit(this); }
void Call::accept(ConstraintVisitor * v) { v->visit(this); }

std::string Type::toString() {
  std::stringstream out;
  out << COL_RGB(5, 4, 2) << name << COL_CLEAR;
  if(params.size()) {
    out <<  "(";
    for(auto &p : params) { if (&p != &params.front()) out << ", "; out <<  p->toString();}
    out <<  ")";
  }
  auto s = out.str();
  return s;
}
std::string Free::toString() { return "*T" + std::to_string(v); }
std::string Call::toString() {
  auto s = "Call(" + callee->toString();
  if(args.size()) {
    s += ", ";
    for(auto &p : args) { if (&p != &args.front()) s += ", "; s += p->toString();}
  }
  s += ")";
  return s;
}


bool ConstraintEqualsImpl::of(Constraint * a, Constraint * b) {
  ConstraintEqualsImpl ceq;
  TypeEqualityWrapper<ConstraintEqualsImpl *> wceq(&ceq, a, b);
  return ceq.out;
}

void ConstraintEqualsImpl::equal(Term * a, Term * b)  {
  if (a->term && b->term)
    out = a->term == b->term;
  else
    out = a->name == b->name;
}

void ConstraintEqualsImpl::equal(Free * a, Free * b) {
  out = a->v == b->v;
}

void ConstraintEqualsImpl::equal(Type * a, Type * b) {
  if (a->name != b->name) { out = false; return; }
  if (a->params.size() != b->params.size()) { out = false; return; }
  for(size_t i = 0; i < a->params.size(); i++) {
    if (!ConstraintEqualsImpl::of(a->params[i], b->params[i])) { out = false; return; }
  }
  out = true;
}
