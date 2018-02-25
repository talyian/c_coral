#include "analyzers/typegraph/constraint.hh"

std::ostream& operator << (std::ostream&out, TypeTerm * tt) {
  return tt ? (out << tt->name) : (out << "(null)"); }

std::ostream& operator << (std::ostream&out, Constraint * cc) {
  return cc ? (out << cc->toString()) : (out << "(null)"); }

void Constraint::accept(ConstraintVisitor * v) { v->visit(this); }
void Type::accept(ConstraintVisitor * v) { v->visit(this); }
void Term::accept(ConstraintVisitor * v) { v->visit(this); }
void Free::accept(ConstraintVisitor * v) { v->visit(this); }
void Call::accept(ConstraintVisitor * v) { v->visit(this); }
