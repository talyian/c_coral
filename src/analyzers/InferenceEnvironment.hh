#pragma once
#include "utils/ansicolor.hh"
#include "core/expr.hh"
#include "InferenceObjectModel.hh"

#include <iostream>
#include <iomanip>
#include <set>
#include <algorithm>

namespace frobnob {
  class TypeEnvironment {
  public:
    int subcount;
    std::set<std::string> names;
    std::vector<TypeTerm *> terms;

    // the constraints that are necessary for solving current unknown terms
    std::multimap<TypeTerm *, TypeConstraint *> critical_constraints;

    TypeTerm * AddTerm(std::string name, coral::ast::BaseExpr * expr);
    TypeTerm * FindTerm(coral::ast::BaseExpr * expr);
    void AddConstraint(TypeTerm * term, TypeConstraint * tcons);
    void AddConstraint(
      std::map<TypeTerm *, TypeConstraint *> &constraints,
      TypeTerm * term,
      TypeConstraint * tcons);
    TypeConstraint * AddEquality(TypeConstraint * lhs, TypeConstraint * rhs);

    void Solve();

    TypeConstraint * SubstituteTerm(
      TypeConstraint * subject, TypeTerm * search, TypeConstraint * replacement);
    bool DoSubstitutions(std::map<TypeTerm *, TypeConstraint *> &constraints);
    bool DoInstantiations(std::map<TypeTerm *, TypeConstraint *> &constraints);

  };
}
