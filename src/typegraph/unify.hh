#pragma once
#include "constraints.hh"
#include "typegraph.hh"

namespace typegraph {
  extern bool showSteps;
  class Unify : public ConstraintVisitorDouble {
  public:
    TypeGraph * gg;
    Solution * solution;
    TypeTerm * info;
    int count = 0;
    Unify(Solution * solution, TypeTerm * info, Constraint * a, Constraint * b)
      : ConstraintVisitorDouble(a, b), solution(solution), gg(solution->gg), info(info) {
      if (showSteps)
        std::cerr << info << ": \033[34munifying " << a << " :: " << b << "\033[0m\n";
      run();
    };
    void visit2(Term * a, Type * b) {
      gg->constrain(a->term, b);
      solution->unknowns.insert(a->term);
      count++;
    }
    void visit2(Type * a, Term * b) { visit2(b, a); }
    void visit2(Type * a, Type * b) {
      if (a->name != b->name)
        ;
      else if (a->params.size() != b->params.size())
        ;
      else {
        for(size_t i = 0; i<a->params.size(); i++) {
          count += Unify(solution, info, a->params[i], b->params[i]).count;
        }
        return;
      }
      std::cerr << "\033[31mWarning! Unifying " << a << " and " << b << "\033[0m\n";
    }
    void visit2(Term * a, Term * b) {
      if (a->term == b->term) return;
      gg->constrain(a->term, b);
      gg->constrain(b->term, a);
      solution->unknowns.insert(a->term);
      solution->unknowns.insert(b->term);
      count++;
    }
  };
}
