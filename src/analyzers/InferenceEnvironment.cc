#include "utils/ansicolor.hh"
#include "core/expr.hh"
#include "core/prettyprinter.hh"
#include "TypeResolver.hh"

#include <iostream>
#include <iomanip>
#include <set>
#include <algorithm>

#include "InferenceEnvironment.hh"

using namespace coral;

namespace frobnob {

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

  void DoSimplifyM(
    TypeEnvironment * env,
    std::multimap<TypeTerm *, TypeConstraint *> constraints)
  // TODO: come up with a better name than "simplify"
/*
* T :: Type
* T :: B
* T :: C
becomes
* B :: Type
* C :: Type
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
            std::map<FreeType *, TypeTerm*> newtypes;
            auto args(call->args);
            for(size_t i = 0; i < args.size(); i++) {
              auto param = callee->params[i].get();
              auto arg = args[i];
              if (auto fparam = dynamic_cast<FreeType *>(param)) {
                auto t = newtypes[fparam];
                if (!t) {
                  t = env->AddTerm(pair.first->name + ".T" + std::to_string(fparam->id), 0);
                  newtypes[fparam] = t;
                }
                env->AddConstraint(t, arg);
                env->subcount++;
              } else {
                env->AddEquality(param, arg);
              }
            }
            auto rparam = callee->params[args.size()].get();
            auto arg = pair.first;
            if (!rparam || callee->params.size() < args.size() + 1) {
              std::cerr << COL_LIGHT_RED << "Something is really wrong\n";
              std::cerr << call << "\n";
            }
            if (auto fparam = dynamic_cast<FreeType *>(rparam)) {
              auto t = newtypes[fparam];
              if (!t) {
                t = env->AddTerm(pair.first->name + ".T" + std::to_string(fparam->id), 0);
                newtypes[fparam] = t;
              }
              env->AddConstraint(arg, new Term(t));
                env->subcount++;
            } else {
              env->AddEquality(rparam, new Term(arg));
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

  void TypeEnvironment::Solve() {
    // Okay, at this point we have the initial set of constraints
    printHeader("Original");
    for(auto &&pair: critical_constraints) print(std::cerr, pair);

    subcount = 1;
    for(int i=0; subcount && i<30; i++) {
      this->subcount = 0;
      DoApplicationsM(this, critical_constraints);
      // printHeader("Application");
      // for(auto &&pair: critical_constraints) print(std::cerr, pair);

      // if [term] has a single value that equates to a type or other term
      // we can substitite that value in for all the places where [term] appears
      DoSubstitutionsM(this, critical_constraints);
      // printHeader("Substitution");
      // for(auto &&pair: critical_constraints) print(std::cerr, pair);

      // If [term] has multiple values that are
      DoSimplifyM(this, critical_constraints);
      // printHeader("Simplification");
      // for(auto &&pair: critical_constraints) print(std::cerr, pair);
    }
    printHeader("Solutions");
    for(auto &&pair: critical_constraints) print(std::cerr, pair);
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
    critical_constraints.insert(std::make_pair(term, tcons));
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
