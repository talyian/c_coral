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
    void TypeEnvironment::Solve() {
      // Okay, at this point we have the initial set of constraints
      for(auto && tt : terms)
        std::cerr << tt << "\n";
      for(auto &&pair: constraints) {
        TypeTerm * term = pair.first;
        std::cerr << std::setw(15) << pair.first << " :: " << pair.second.get() << "\n";
      }

      // we should really be making a copy of the constraint set,
      // since we need the original after all the reductions are done.
      std::map<TypeTerm *, TypeConstraint *> constraints;
      for(auto &&pair: this->constraints)
        constraints[pair.first] = pair.second.get();

      // FindAllSubstititions(<all>) -> <terms>
      // apply all substititions on <terms> -> <terms2>
      // FindAllSubstitutions(<terms2>)

      // DoSubstitutions(constraints);
      DoInstantiations(constraints);
      DoSubstitutions(constraints);
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
      std::map<TypeTerm *, TypeConstraint *> replacements;
      for(auto &&pair:constraints) {
        for(auto &&type: getDescendants<Type *>(pair.second))
          replacements[pair.first] = type;
        for(auto &&term: getDescendants<Term *>(pair.second))
          replacements[pair.first] = term;
      }
      for(auto &&replace_pair: replacements) {
        subcount = 0;
        for(auto &pair: constraints) {
          pair.second = SubstituteTerm(pair.second, replace_pair.first, replace_pair.second);
          // constraints[replace_pair.first] = SubstituteTerm(
          //   replace_pair.second,
          //   replace_pair.second, 0);
        }
        if (subcount > 0) {
          std::cerr << COL_MAGENTA << "SUBSTITUTED " << replace_pair.first << "\n";
          constraints.erase(constraints.find(replace_pair.first));
        }
      }

      std::cerr << COL_RGB(5, 5, 4)
                << "-[ Substitutions ] ------------------------------------------------------------------"
                << COL_CLEAR << "\n";
      for(auto &&pair: constraints) {
        TypeTerm * term = pair.first;
        std::cerr << std::setw(15) << pair.first << " :: " << pair.second << "\n";
      }
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
                if (i < call->args.size())
                  AddConstraint(constraints, newterms[f], call->args[i]);
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

    //////////////////////////////////////// Constraints
    void TypeEnvironment::AddConstraint(TypeTerm * term, TypeConstraint * tcons) {
      if (constraints.find(term) != constraints.end()) {
        constraints[term].reset(new And({ constraints[term].release(), tcons }));
        std::cerr << COL_LIGHT_RED << "multi-constraint on " << term << COL_CLEAR << "\n";
      } else
        constraints[term] = std::unique_ptr<TypeConstraint>(tcons);
    }
    void TypeEnvironment::AddConstraint(std::map<TypeTerm *, TypeConstraint *> &constraints, TypeTerm * term, TypeConstraint * tcons) {
      if (constraints.find(term) != constraints.end()) {
        constraints[term] = new And({ constraints[term], tcons });
        std::cerr << COL_LIGHT_RED << "multi-constraint on " << term << COL_CLEAR << "\n";
      } else
        constraints[term] = tcons;
    }
}
