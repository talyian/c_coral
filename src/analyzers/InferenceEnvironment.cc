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

    for(auto &pair : env->mconstraints)
      if (pair.first == rule)
        pair.second = pair.second->replaceTerm(search, replace);
  }

  std::set<TypeTerm *> GetDependents(TypeEnvironment * env, TypeTerm * search) {
    std::set<TypeTerm *> t;
    for(auto &&pair: env->mconstraints) if (pair.second->findTerm(search)) t.insert(pair.first);
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
        std::cerr << "mooo " << pair.first << dependents.size() << "\n";
        env->mconstraints.erase(pair.first);
      }

      for(auto &&dep: dependents)
        ReplaceTerm(env, dep, pair.first, pair.second);
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
              }
            }
            auto rparam = callee->params[args.size()].get();
            auto arg = pair.first;
            if (auto fparam = dynamic_cast<FreeType *>(rparam)) {
              auto t = newtypes[fparam];
              if (!t) {
                t = env->AddTerm(pair.first->name + ".T" + std::to_string(fparam->id), 0);
                newtypes[fparam] = t;
              }
              env->AddConstraint(arg, new Term(t));
            }
            for(auto to_erase = env->mconstraints.begin();
                to_erase != env->mconstraints.end();
                to_erase++)
              if(*to_erase == pair) env->mconstraints.erase(to_erase);
          }
        }
      }
    }
  }

  void TypeEnvironment::Solve() {
    // Okay, at this point we have the initial set of constraints
    printHeader("Original");
    for(auto &&pair: mconstraints) print(std::cerr, pair);

    DoApplicationsM(this, mconstraints);

    printHeader("Application");
    for(auto &&pair: mconstraints) print(std::cerr, pair);

    DoSubstitutionsM(this, mconstraints);

    printHeader("Substitution");
    for(auto &&pair: mconstraints) print(std::cerr, pair);

    // DoInstantiations(constraints);
    // DoSubstitutions(constraints);
  }


  TypeConstraint * TypeEnvironment::SubstituteTerm(TypeConstraint * subject, TypeTerm * search, TypeConstraint * replacement)  {
    // TODO: this should be a virtual method on each typeconstraint
    if (Call * c = dynamic_cast<Call *>(subject)) {
      c->callee = SubstituteTerm(c->callee, search, replacement);
      for(auto argi = c->args.begin(); argi != c->args.end(); argi++) {
        *argi = SubstituteTerm(*argi, search, replacement); } }
    if (Term * t = dynamic_cast<Term *>(subject)) {
      if (t->term == search) {
        subcount++;
        return replacement; } }
    if (Type * t = dynamic_cast<Type *>(subject)) {
      for(auto &p: t->params)
        p.reset(SubstituteTerm(p.release(), search, replacement)); }
    if (And * t = dynamic_cast<And *>(subject)) {
      for(auto &p: t->terms)
        p = SubstituteTerm(p, search, replacement); }
    return subject;
  }

  bool TypeEnvironment::DoSubstitutions(std::map<TypeTerm *, TypeConstraint *> &constraints) {
    // std::map<TypeTerm *, TypeConstraint *> replacements;
    // for(auto &&pair:constraints) {
    //   for(auto &&type: getDescendants<Type *>(pair.second))
    //     replacements[pair.first] = type;
    //   for(auto &&term: getDescendants<Term *>(pair.second))
    //     replacements[pair.first] = term;
    // }
    // for(auto &&replace_pair: replacements) {
    //   subcount = 0;
    //   for(auto &pair: constraints) {
    //     pair.second = SubstituteTerm(pair.second, replace_pair.first, replace_pair.second);
    //     // constraints[replace_pair.first] = SubstituteTerm(
    //     //   replace_pair.second,
    //     //   replace_pair.second, 0);
    //   }
    //   if (subcount > 0) {
    //     std::cerr << COL_MAGENTA << "SUBSTITUTED " << replace_pair.first << "\n";
    //     constraints.erase(constraints.find(replace_pair.first));
    //   }
    // }
    return subcount > 0;
  }

  bool TypeEnvironment::DoInstantiations(std::map<TypeTerm *, TypeConstraint *> &constraints) {
    // get rid of all the free variables. Generate a direct typeterm at every callsite.
    for(auto &&pair:constraints) {
      if (Call * call = dynamic_cast<Call *>(pair.second)) {
        auto callee = dynamic_cast<Type *>(call->callee);
        if (callee && std::count_if(
              callee->params.begin(),
              callee->params.end(),
              [] (std::unique_ptr<TypeConstraint> &p) { return dynamic_cast<FreeType *>(p.get()); })) {

          auto new_callee = new Type(callee->name);
          new_callee->params.reserve(callee->params.size());
          std::map<FreeType *, TypeTerm *> newterms;

          for(size_t i =0; i < callee->params.size(); i++) {
            auto &p = callee->params[i];
            if (auto f = dynamic_cast<FreeType *>(p.get())) {
              if (!newterms[f]) { newterms[f] = AddTerm(pair.first->name + ".T", 0); }
              // if (i < call->args.size())
              //   AddConstraint(constraints, newterms[f], call->args[i]);
              new_callee->params.emplace_back(new Term(newterms[f]));
            } else {
              new_callee->params.emplace_back(callee->params[i].release());
            }
          }
          call->callee = new_callee;
          pair.second = new_callee->params.back().get();
        }
      }
    }

    std::cerr << COL_RGB(5, 5, 4)
              << "-[ Abstraction ] --------------------------------------------------------------------"
              << COL_CLEAR << "\n";
    for(auto &&pair: constraints) {
      TypeTerm * term = pair.first;
      std::cerr << std::setw(15) << pair.first << " :: " << pair.second << "\n";
    }
    return false;
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
    mconstraints.insert(std::make_pair(term, tcons));
  }
  // void TypeEnvironment::AddConstraint(
  //   std::map<TypeTerm *, TypeConstraint *> &constraints,
  //   TypeTerm * term,
  //   TypeConstraint * tcons) {
  //   if (constraints.find(term) != constraints.end()) {
  //     constraints[term] = new And({ constraints[term], tcons });
  //     std::cerr << COL_LIGHT_RED << "multi-constraint on " << term << COL_CLEAR << "\n";
  //   } else
  //     constraints[term] = tcons;
  // }
}
