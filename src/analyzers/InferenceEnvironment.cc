#include "utils/ansicolor.hh"
#include "core/expr.hh"
#include "TypeResolver.hh"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <set>
#include <algorithm>
#include <typeinfo>

#include "InferenceEnvironment.hh"

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

    void DoSubstitutionsM(
      TypeEnvironment * env,
      std::multimap<TypeTerm *, TypeConstraint *> constraints) {

      for(auto it = constraints.begin(); it != constraints.end(); it++) {
        auto pair = *it;
        // only substitute terms and types
        if (!dynamic_cast<Type *>(pair.second) && !dynamic_cast<Term *>(pair.second)) continue;
        // only substitute single-rule terms for now
        if (constraints.count(pair.first) > 1) continue;
        auto dependents = GetDependents(env, pair.first);
        // ignore recursive terms for now
        if (dependents.find(pair.first) != dependents.end()) continue;

        if (dependents.size()) {
          env->subcount++;
        }

        for(auto &&dep: dependents)
          ReplaceTerm(env, dep, pair.first, pair.second);
      }
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
    void Deduplicate(
      TypeEnvironment * env,
      std::multimap<TypeTerm *, TypeConstraint *> constraints) {
      // Some of the other steps naively create multiple copies of the same constraint
      std::vector<std::pair<TypeTerm *, TypeConstraint *>> newconstraints;
      for(auto it = constraints.begin(); it != constraints.end();) {
        auto key = it->first;
        auto compare_constraints = [](TypeConstraint * a, TypeConstraint * b) {
          if (typeid(a).hash_code() < typeid(b).hash_code()) return true;
          if (typeid(a).hash_code() > typeid(b).hash_code()) return false;
          Type *ta, *tb;
          if ((ta = dynamic_cast<Type *>(a)) && (tb = dynamic_cast<Type *>(b))) {
            std::stringstream ss;
            std::stringstream st;
            ss << ta;
            auto s = ss.str();
            st << tb;
            auto t = st.str();
            return s < t;
          }
          return false;
        };
        std::set<TypeConstraint *, decltype(compare_constraints)> values(compare_constraints);
        auto end = constraints.upper_bound(it->first);
        for(; it != end; it++) {
          values.insert(it->second);
        }
        for(auto &&constraint : values)
          newconstraints.push_back(std::make_pair(key, constraint));
      }
      env->critical_constraints.clear();
      for(auto &&pair: newconstraints)
        env->critical_constraints.insert(pair);
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

        env->critical_constraints.erase(key);
        for(size_t i=0; i<values.size() - 1; i++) {
          auto out = env->AddEquality(values[i], values[i + 1]);
          if (out)
            env->subcount++;
          env->AddConstraint(key, out);
        }
      }
    }

    void DoApplicationsM(
      TypeEnvironment * env,
      std::multimap<TypeTerm *, TypeConstraint *> constraints)
    {
/*
  constraints |> map match
  | Call(Type(t), args) as call ->
*/;
      for(auto &pair: constraints) {
        if (auto call = dynamic_cast<Call *>(pair.second)) {
          if (auto callee = dynamic_cast<Type *>(call->callee)) {
            if (callee->name == "Func") {
              auto freetype_compare = [](FreeType * a, FreeType * b) {
                return (
                  a == b ? false :
                  a == 0 ? true:
                  b == 0 ? false:
                  a->id < b->id);
              };
              std::map<FreeType *, TypeTerm *, decltype(freetype_compare)> newtypes(freetype_compare);
              auto args(call->args);
              for(size_t i = 0; i < args.size(); i++) {
                auto param = callee->params[i];
                auto arg = args[i];
                if (auto fparam = dynamic_cast<FreeType *>(param)) {
                  if (newtypes.find(fparam) == newtypes.end()) {
                    newtypes[fparam] = env->AddTerm(pair.first->name + ".T" + std::to_string(fparam->id), 0);
                  }
                  env->AddConstraint(newtypes[fparam], arg);
                  env->subcount++;
                } else {
                  env->AddEquality(param, arg);
                }
              }
              auto rparam = callee->params[args.size()];
              auto arg = pair.first;
              if (!rparam || callee->params.size() < args.size() + 1) {
                std::cerr << COL_LIGHT_RED << "Something is really wrong\n";
                std::cerr << call << "\n";
              }
              if (auto fparam = dynamic_cast<FreeType *>(rparam)) {
                if (newtypes.find(fparam) == newtypes.end()) {
                  newtypes[fparam] = env->AddTerm(pair.first->name + ".T" + std::to_string(fparam->id), 0);
                }
                env->AddConstraint(arg, env->newTerm(newtypes[fparam]));
                env->subcount++;
              } else {
                env->AddEquality(rparam, env->newTerm(arg));
              }
              for(auto to_erase = env->critical_constraints.begin();
                  to_erase != env->critical_constraints.end();)
                if(*to_erase == pair)
                  to_erase = env->critical_constraints.erase(to_erase);
                else
                  to_erase++;
            }
          }
        }
      }
    }

#define SHOW(s) { printHeader(s); for(auto &&pair: critical_constraints) print(std::cerr, pair); }
// #define SHOW(s) ;w
    void TypeEnvironment::Solve() {
      SHOW("Original");

      subcount = 1;
      for(int i=0; subcount && i<30; i++) {
        this->subcount = 0;
        DoApplicationsM(this, critical_constraints);
        SHOW("Application");

        // if [term] has a single value that equates to a type or other term
        // we can substitite that value in for all the places where [term] appears
        DoSubstitutionsM(this, critical_constraints);
        SHOW("Substitution");

        // If [term] has multiple values that are
        DoSimplifyM(this, critical_constraints);
        SHOW("Simplification");

        Deduplicate(this, critical_constraints);
        RemoveReflexiveRule(this, critical_constraints);
        SHOW("Dedup");
      }
      if (subcount)
        std::cerr << COL_LIGHT_RED
                  << "Warning: Type Solver Limit reached: " << 30 << COL_CLEAR << "\n";
      SHOW("Solution")
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

      if (Term * tt = dynamic_cast<Term *>(tcons)) {
        if (tt->term->expr == term->expr) {
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
  }
}
