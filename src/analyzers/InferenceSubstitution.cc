#include "utils/ansicolor.hh"
#include "utils/opts.hh"
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

    class FindDependencies : public AbstractTypeConstraintVisitor {
    public:
      TypeEnvironment * env;
      TypeTerm * root;
      bool lookUpTerms = false;
      std::set<TypeTerm *> found;
      std::set<TypeConstraint *> visited;
      FindDependencies(TypeEnvironment * env, TypeTerm * tt): env(env), root(tt), lookUpTerms(true) {
        for(auto it = env->critical_constraints.find(tt);
            it != env->critical_constraints.end() && it != env->critical_constraints.upper_bound(tt);
            it++)
          it->second->accept(this);
      }
      virtual void visitTerm(TypeTerm * tt) {
        if (lookUpTerms)
          for(auto it = env->critical_constraints.find(tt);
              it != env->critical_constraints.end() && it != env->critical_constraints.upper_bound(tt);
              it++) {
            it->second->accept(this);
          }
      }
      void visit(TypeConstraint * t) {
        std::cerr << "FindDependents: Unknown Type " << t << "\n";
      }
      void visit(Type * t) {
        if (visited.find(t) != visited.end()) return; else { visited.insert(t); }
        for(auto & param: t->params)
          param->accept(this);
      }
      void visit(Call * t) {
        if (visited.find(t) != visited.end()) return; else { visited.insert(t); }
        t->callee->accept(this);
        for(auto & arg : t->args) arg->accept(this);
      }
      void visit(FreeType * t) { }
      void visit(Term * t) {
        if (visited.find(t) != visited.end()) return; else { visited.insert(t); }
        if (found.find(t->term) == found.end()) {
          found.insert(t->term);
          visitTerm(t->term);
        } else {
          /* do nothing */
        }
      }
    };

    // given { a :: Foo[b, Int32], b: Bar[Int32] }
    // replace(a, b) should result in { a :: Foo[Bar[Int32], Int32], b: Bar[Int32] }
    class ReplaceTermVisitor : public AbstractTypeConstraintVisitor {
    public:
      TypeConstraint * out;
      TypeTerm * find;
      TypeConstraint * replace;
      ReplaceTermVisitor(
        TypeEnvironment * env,
        TypeConstraint * subject,
        TypeTerm * find,
        TypeConstraint * replace) : find(find), replace(replace) {
        run(subject);
      }

      TypeConstraint * run(TypeConstraint * v) {
        out = v; v->accept(this); return out;
      }
      void visit(TypeConstraint * t) {
        std::cerr << "FindDependents: Unknown Type " << t << "\n";
      }
      void visit(Type * t) {
        for(TypeConstraint * &param: t->params)
          param = run(param);
        out = t;
      }
      void visit(Call * c) {
        c->callee = run(c->callee);
        for(auto &arg : c->args) arg = run(arg);
        out = c;
      }
      void visit(Term * t) {
        if (t->term == find) {
          std::cerr << COL_LIGHT_RED << "Replacing " << find << COL_CLEAR << "\n";
          out = replace;
        } else
          out = t;
      }
      void visit(FreeType *) { }
    };

    void DoSubstitutionsM(
      TypeEnvironment * env,
      std::multimap<TypeTerm *, TypeConstraint *> ccc) {

      for(auto it = env->critical_constraints.begin(); it != env->critical_constraints.end(); it++) {
        auto pair = *it;
        // only substitute terms and types
        if (!dynamic_cast<Type *>(pair.second) && !dynamic_cast<Term *>(pair.second)) continue;

        // only substitute single-rule terms for now
        if (env->critical_constraints.count(pair.first) > 1) { continue; }

        // do not substitute recursively-defined items since that just propagates the loop
        auto depends = FindDependencies { env, pair.first }.found;
        if (depends.find(pair.first) != depends.end()) { continue; }

        for(auto &subject : env->critical_constraints) {
          subject.second = ReplaceTermVisitor { env, subject.second, pair.first, pair.second }.out;
        }

        // dependents = FindDependents(env, pair.first, false).found;
        // for(auto && dep: dependents) {
        //   std::cerr << COL_LIGHT_YELLOW << "R "
        //             << pair.first << " in " << dep << COL_CLEAR << "\n";
        //   ReplaceTermVisitor { env, dep, pair.first, pair.second };
        // }
      }
    }
  }
}
