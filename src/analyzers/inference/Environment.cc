#include "utils/ansicolor.hh"
#include "utils/opts.hh"
#include "core/expr.hh"
#include "analyzers/TypeResolver.hh"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <set>
#include <algorithm>
#include <typeinfo>

using namespace coral;

namespace coral {
  namespace typeinference {

    void print(std::ostream &out, std::pair<TypeTerm *, TypeConstraint *> pair ) {
      std::cerr << std::setw(15) << pair.first << " :: " << pair.second << "\n";
    }

    void printHeader(std::string title) {
      if (title.empty()) return;
      std::cerr  << COL_RGB(3, 3, 5) << '[' << title << "] " << std::setw(60) << fill('-')
                 << COL_CLEAR << "\n";
    }

    void ReplaceTerm(
      TypeEnvironment * env,
      TypeTerm * rule,
      TypeTerm * search,
      TypeConstraint * replace) {

      for(auto &pair : env->critical_constraints)
        if (pair.first == rule)
          pair.second = pair.second->replaceTerm(search, replace);
    }

    std::set<TypeTerm *> GetDependents(TypeEnvironment * env, TypeTerm * search) {
      std::set<TypeTerm *> t;
      for(auto &&pair: env->critical_constraints) if (pair.second->findTerm(search)) t.insert(pair.first);
      return t;
    }

    void RemoveReflexiveRule(
      TypeEnvironment * env,
      std::multimap<TypeTerm *, TypeConstraint *> constraints) {

      std::set<std::pair<TypeTerm *, TypeConstraint *>> toErase;
      for(auto it = constraints.begin(); it != constraints.end(); it++) {
        if (auto tt = dynamic_cast<Term *>(it->second))
          if (tt->term == it->first)
            toErase.insert(*it);
      }
      for(auto ccIter = env->critical_constraints.begin(); ccIter != env->critical_constraints.end();) {
        if (toErase.find(*ccIter) != toErase.end())
          ccIter = env->critical_constraints.erase(ccIter);
        else
          ccIter++;
      }
      // for(auto &&pair : toErase)
      //   for(env->critical_constraints
      // for each rule : constraint of type
      // x : Term(x)
    }

    void DoSimplifyM(
      TypeEnvironment * env,
      std::multimap<TypeTerm *, TypeConstraint *> constraints)
    // TODO: come up with a better name than "simplify"
    /* {T :: Type, T :: B, T :: C} -> {B :: Type, C :: Type}
     if T is solely rules that are a single type and some other terms, we can remove T
     *
     */
    {
      for(auto it = constraints.begin(); it != constraints.end();) {
        auto key = it->first;
        std::vector<TypeConstraint *> values;
        for(auto end = constraints.upper_bound(it->first); it != end; it++) {
          values.push_back(it->second);
        }
        // skip if we're not all of type [Type*] or [Term*]
        bool skip = false;
        for(auto &&v : values)
          if (!dynamic_cast<Type *>(v) && !dynamic_cast<Term *>(v)) skip = true;
        if (skip) continue;
        if (values.size() < 2) continue;

        if (coral::opt::ShowTypeSolution) std::cerr << COL_LIGHT_RED
                                                    << "simplifying " << key << "\n";
        env->critical_constraints.erase(key);
        for(size_t i=0; i<values.size() - 1; i++) {
          auto out = env->AddEquality(values[i], values[i + 1]);
          if (out)
            env->subcount++;
          env->AddConstraint(key, out);
        }
      }
    }

#define HEADER(s) {if (coral::opt::ShowTypeSolution) { \
        std::cerr << "[" << i << "] "; printHeader(s); }}
#define SHOW(s) {if (coral::opt::ShowTypeSolution) { \
        for(auto &&pair: critical_constraints) print(std::cerr, pair);}}
// #define SHOW(s) ;w
    void TypeEnvironment::Solve() {
      int i=0;
      SHOW("Original");
      subcount = 1;
      for(; subcount && i<30; i++) {
        this->subcount = 0;

        HEADER("Application")
        std::vector<std::pair<TypeTerm *, Call *>> to_delete;
        for(auto &pair: critical_constraints)
          if (auto call = dynamic_cast<Call *>(pair.second))
            to_delete.push_back(Apply(this, pair.first, call));
        for(auto & pair: to_delete)
          this->RemoveConstraint(pair.first, pair.second);
        SHOW("Application");

        // if [term] has a single value that equates to a type or other term
        // we can substitite that value in for all the places where [term] appears
        HEADER("Substitution")
        DoSubstitutionsM(this, critical_constraints);
        SHOW("Substitution");

        // If [term] has multiple values that are
        HEADER("Simplification")
          DoSimplifyM(this, critical_constraints);
        SHOW("Simplification");

        HEADER("Dedup + RemoveReflex")
          Deduplicate(this, critical_constraints);
        RemoveReflexiveRule(this, critical_constraints);
        SHOW("Dedup");
      }
      HEADER("Solution")
        if (subcount) {
        std::cerr << COL_LIGHT_RED
                  << "Warning: Type Solver Limit reached: " << 30 << COL_CLEAR << "\n";
        }
      SHOW("Solution")
        for(auto &m : solved_constraints)
          std::cerr << std::setw(20) << m.first << " == " << m.second << "\n";
    }

    TypeTerm * TypeEnvironment::AddTerm(std::string name, ast::BaseExpr * expr) {
      int i = 0; std::string name0 = name;
      while (names.find(name) != names.end())
        name = name0 + "." + std::to_string(i++);
      names.insert(name);
      terms.push_back(new TypeTerm(name, expr));
      return terms.back();
    }
    TypeTerm * TypeEnvironment::FindTerm(ast::BaseExpr * expr) {
      for(auto &&term: terms) if (term->expr == expr) return term;
      return 0;
    }

    void TypeEnvironment::AddConstraint(
      TypeTerm * term,
      TypeConstraint * tcons) {

      if (coral::opt::ShowTypeSolution) {
        std::cerr << COL_RGB(3,5,4) << std::setw(20) << "Adding"
                  << std::setw(10) << term
                  << COL_RGB(3,5,4) << " :: "
                  << tcons << COL_CLEAR << "\n";
      }

      if (Term * tt = dynamic_cast<Term *>(tcons)) {
        if (tt->term->name == term->name) {
          std::cerr << "Redefining term as itself :( " << term << "\n";
          return;
        }
      }

      critical_constraints.insert(std::make_pair(term, tcons));
      if (critical_constraints.size() > 1000) {
        std::cerr << "constraints overflow :(";
        exit(1);
      }
    }

    void TypeEnvironment::RemoveConstraint(TypeTerm * tt, TypeConstraint * tcons) {
      if (!tt) return;
      if (coral::opt::ShowTypeSolution) {
        std::cerr << COL_RGB(5, 3, 2) << std::setw(20) << "Removing"
                  << std::setw(10) << tt
                  << COL_RGB(5, 3, 2) << " :: "
                  << tcons << COL_CLEAR << "\n";
      }

      for(auto iter = critical_constraints.begin();
          iter != critical_constraints.end();) {
        if (iter->first != tt)
          iter = critical_constraints.upper_bound(iter->first);
        else if (iter->second == tcons)
          iter = critical_constraints.erase(iter);
        else
          iter++;
      }
    }

    TypeConstraint * TypeEnvironment::AddEquality(TypeConstraint * lhs, TypeConstraint * rhs) {
      Type *lt, *rt;
      Term *tt, *tt2;
      if ((lt = dynamic_cast<Type *>(lhs)) && (rt = dynamic_cast<Type *>(rhs))) {
        if (lt->name != rt->name)
          std::cerr << "Warning: Unifying two Types " << lhs << " -- " << rhs << "\n";
      }
      else if (dynamic_cast<Type *>(lhs) && dynamic_cast<Term *>(rhs)) {
        return AddEquality(rhs, lhs);
      }
      else if ((rt = dynamic_cast<Type *>(rhs)) && (tt = dynamic_cast<Term *>(lhs))) {
        AddConstraint(tt->term, rhs); return rhs;
      }
      else if ((tt2 = dynamic_cast<Term *>(rhs)) && (tt = dynamic_cast<Term *>(lhs))) {
        AddConstraint(tt->term, rhs); return rhs;
      }
      else {
        std::cerr
          << COL_LIGHT_RED << "Warning: Unhandled Unification: "
          << lhs << " -- " << rhs << COL_CLEAR << "\n";
      }
      return rhs;
    }

    void TypeEnvironment::Sideboard(TypeTerm * tt) {
      auto it = this->critical_constraints.find(tt);
      this->solved_constraints.insert(*it);
      if (coral::opt::ShowTypeSolution)
        std::cerr << COL_LIGHT_BLUE << std::setw(20) << "sideboarded " << it->first << "\t" << it->second << "\n";
      this->critical_constraints.erase(it->first);
    }
  }
}
