#include "constraint.hh"
#include "TypeGraph.hh"

// Get all constraints in the graph for a particular term
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

//Get all terms in the graph for a particular constraint
class AllTerms {
public:
  static std::set<std::pair<TypeTerm *, Constraint*>> of(TypeGraph * graph, Term * cons) {
    std::set<std::pair<TypeTerm *, Constraint *>> result;
    for(auto &pair: graph->GetRelations())
      if (ConstraintEqualsImpl::of(pair.first, cons))
        result.insert(std::make_pair(pair.second, pair.first));
    return result;
  }
};

// Duplicate a constraint so we can use it as another key in the relations graph
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
