#include "constraint.hh"
#include "TypeGraph.hh"

class AllConstraints {
public:
  static std::set<Constraint *> of(TypeGraph * graph, TypeTerm * tt) {
    return AllConstraints(graph, tt).out;
  }
  TypeGraph * graph;
  TypeTerm * tt;
  std::set<Constraint *> out;
  AllConstraints(TypeGraph * graph, TypeTerm * tt);
};

class DuplicateConstraint : public ConstraintVisitor {
public:
  Constraint * out;
  TypeGraph * graph;
  DuplicateConstraint(TypeGraph * graph, Constraint * t) : graph(graph) { t->accept(this); }
  void visit(Type * c) ;
  void visit(Term * c) ;
  void visit(Free * c) ;
  void visit(Call * c) ;
};
