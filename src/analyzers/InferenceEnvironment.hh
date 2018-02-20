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
    std::map<TypeTerm *, std::unique_ptr<TypeConstraint>> constraints;
    std::multimap<TypeTerm *, TypeConstraint *> mconstraints;

    TypeTerm * AddTerm(std::string name, coral::ast::BaseExpr * expr);
    TypeTerm * FindTerm(coral::ast::BaseExpr * expr);
    void AddConstraint(TypeTerm * term, TypeConstraint * tcons);
    void AddConstraint(
      std::map<TypeTerm *, TypeConstraint *> &constraints,
      TypeTerm * term,
      TypeConstraint * tcons);


    void Solve();

    TypeConstraint * SubstituteTerm(TypeConstraint * subject, TypeTerm * search, TypeConstraint * replacement);

    template<class T>
    std::vector<T> getDescendants(TypeConstraint * root) {
      std::vector<T> a;
      getDescendants<T>(root, a);
      return a;
    }

    template<class T>
    std::vector<T> getDescendants(TypeConstraint * root, std::vector<T> & items) {
      if (T t = dynamic_cast<T>(root)) {
        std::cerr << COL_LIGHT_BLUE << "Adding " << t << ", ";
        std::cerr << typeid(T).name() << "\n";
        items.push_back((T)root);
      } else if (And * t = dynamic_cast<And *>(root)) {
        for(auto &term : t->terms)
          getDescendants<T>(term, items);
      }
      return items;
    };

    bool DoSubstitutions(std::map<TypeTerm *, TypeConstraint *> &constraints);

    bool DoInstantiations(std::map<TypeTerm *, TypeConstraint *> &constraints);

  };
}
