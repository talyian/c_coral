#include "constraints.hh"
namespace typegraph {
  std::ostream& operator <<(std::ostream& out, TypeTerm * tt) {
    if (tt) out << tt->name;
    else out << "(0)";
    return out;
  }

  std::ostream& operator <<(std::ostream& out, Constraint * c) {
    if (c) c->print(out);
    else out << "(null constraint)";
    return out;
  }

  void ConstraintVisitor::visit(Free *) { }
  void ConstraintVisitor::visit(Call * c) {
    c->callee->accept(this);
    for(auto &&arg: c->arguments) arg->accept(this); }
  void ConstraintVisitor::visit(Type * t) {
    for(auto &&arg: t->params) arg->accept(this); }
  void ConstraintVisitor::visit(Term *) { }


  class IsConcreteType : public ConstraintVisitor {
  public:
    IsConcreteType(Constraint * c) { c->accept(this); }
    bool out = true;
    void visit(Term *) { out = false; }
    void visit(Call *) { out = false; }
  };
  bool isConcreteType(Constraint * c) { return IsConcreteType(c).out; }

  class HasFreeVars : public ConstraintVisitor {
  public:
    bool out = false;
    HasFreeVars(Constraint * c) { c->accept(this); }
    void visit(Free *) { out = true; }
  };
  bool hasFreeVars(Constraint * c) { return HasFreeVars(c).out; }

}
