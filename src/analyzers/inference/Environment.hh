#pragma once
#include "utils/ansicolor.hh"
#include "core/expr.hh"
#include "analyzers/inference/ObjectModel.hh"

#include <iostream>
#include <iomanip>
#include <set>
#include <algorithm>

namespace coral {
  namespace typeinference {

    class ConstraintMap : public std::multimap<TypeTerm *, TypeConstraint *> { };

    class TypeEnvironment {
    public:
      int subcount;
      std::set<std::string> names;
      std::vector<TypeTerm *> terms;
      std::vector<std::unique_ptr<TypeConstraint>> all_constraints;

      // the constraints that are necessary for solving current unknown terms
      ConstraintMap critical_constraints;
      ConstraintMap solved_constraints;

      TypeTerm * AddTerm(std::string name, coral::ast::BaseExpr * expr);
      TypeTerm * FindTerm(coral::ast::BaseExpr * expr);
      void AddConstraint(TypeTerm * term, TypeConstraint * tcons);
      void AddConstraint(
        std::map<TypeTerm *, TypeConstraint *> &constraints,
        TypeTerm * term,
        TypeConstraint * tcons);
      TypeConstraint * AddEquality(TypeConstraint * lhs, TypeConstraint * rhs);
      void RemoveConstraint(TypeTerm * tt, TypeConstraint * tcons);
      void Sideboard(TypeTerm * tt);

      void Solve();

      TypeConstraint * SubstituteTerm(
        TypeConstraint * subject, TypeTerm * search, TypeConstraint * replacement);
      bool DoSubstitutions(std::map<TypeTerm *, TypeConstraint *> &constraints);
      bool DoInstantiations(std::map<TypeTerm *, TypeConstraint *> &constraints);

      ~TypeEnvironment() { for(auto &&p: terms) if (p) delete p; }

      TypeConstraint * Global_Ops(std::string op) {
        auto T0 = newFreeType();
        auto ArithOp = newType("Func", std::vector<TypeConstraint *>{
            T0,
            T0,
            T0 });
        if (op == "<")
          return newType("Func", std::vector<TypeConstraint *>{T0, T0, newType("Int1")});
        return ArithOp;
      }

      template <class ... Tp>
      Call * newCall(Tp ... args) {
        auto call = new Call(args...);
        all_constraints.emplace_back(call);
        return call;
      }
      template <class ... Tp>
      Term * newTerm(Tp ... args) {
        auto call = new Term(args...); all_constraints.emplace_back(call); return call;
      }
      template <class ... Tp>
      Type * newType(Tp ... args) {
        auto call = new Type(args...); all_constraints.emplace_back(call); return call;
      }
      template <class ... Tp>
      FreeType * newFreeType(Tp ... args) {
        auto call = new FreeType(args...); all_constraints.emplace_back(call); return call;
      }
    };

    void DoSubstitutionsM(TypeEnvironment * env, std::multimap<TypeTerm *, TypeConstraint *> ccc);
    void Deduplicate(TypeEnvironment * env, std::multimap<TypeTerm *, TypeConstraint *> ccc);
    std::pair<TypeTerm *, Call *> Apply(TypeEnvironment * env, TypeTerm * tt, Call * call);
  }
}
