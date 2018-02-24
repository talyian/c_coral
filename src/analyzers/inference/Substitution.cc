#include "utils/ansicolor.hh"
#include "utils/opts.hh"
#include "core/expr.hh"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <set>
#include <algorithm>
#include <typeinfo>

#include "analyzers/inference/Environment.hh"

using namespace coral;

namespace coral {
  namespace typeinference {
    // Given a lookup term, find all the other terms it depends on.
    class FindTermRequirements : public AbstractTypeConstraintVisitor {
    public:
      TypeEnvironment * env;
      TypeTerm * root;
      bool lookUpTerms = false;
      std::set<TypeTerm *> found;
      std::set<TypeConstraint *> visited;
      FindTermRequirements(TypeEnvironment * env, TypeTerm * tt)
        : env(env), root(tt), lookUpTerms(true) {
        visitTerm(tt);
      }
      virtual void visitTerm(TypeTerm * tt) {
        if (lookUpTerms)
          for(auto it = env->critical_constraints.find(tt);
              it != env->critical_constraints.end() &&
                it != env->critical_constraints.upper_bound(tt);
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
        }
      }
    };


    // Given {a :: Func[b, c], d :: Func[a, c], e::Int, F :: d}
    // FindDependents a returns d
    class FindTermDependents : public TypeConstraintVisitor {
    public:
      TypeEnvironment * env;
      TypeTerm * root;
      std::set<TypeConstraint *> out;
      FindTermDependents(TypeEnvironment * env, TypeTerm * tt)
        : env(env), root(tt) {
        for(auto it = env->critical_constraints.begin();
            it != env->critical_constraints.end();
            it++) {
          it->second->accept(this);
        }
      }
      void visit(Term * t) { if (t->term == root) out.insert(t); }
    };

    class IsConcreteType : public TypeConstraintVisitor {
    public:
      bool out = true;
      TypeConstraint * root;
      IsConcreteType (TypeConstraint * e) : root(e) { e->accept(this); }
      void visit(Type * t) { out = (root == t); TypeConstraintVisitor::visit(t); }
      void visit(FreeType * f) { out = false; }
    };
    // given { a :: Foo[b, Int32], b: Bar[Int32] }
    // replace(a, b) should result in { a :: Foo[Bar[Int32], Int32], b: Bar[Int32] }
    class ReplaceTermVisitor : public AbstractTypeConstraintVisitor {
    public:
      TypeConstraint * out;
      TypeTerm * find;
      TypeConstraint * replace;
      int counter = 0;
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
          if (coral::opt::ShowTypeSolution) std::cerr << COL_LIGHT_RED << "Replacing " << find << COL_CLEAR << "\n";
          counter++;
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
        if (!dynamic_cast<Type *>(pair.second) && !dynamic_cast<Term *>(pair.second)) {
          // std::cout << "Type Mismatch: " << pair.first << "\n";
          continue;
        }

        // only substitute single-rule terms for now
        if (env->critical_constraints.count(pair.first) > 1) {
          // std::cout << "Multirule: " << pair.first << "\n";
          continue; }

        // do not substitute recursively-defined items since that just propagates the loop
        auto depends = FindTermRequirements { env, pair.first }.found;
        if (depends.find(pair.first) != depends.end()) {
          // std::cout << "Skipping Recursive: " << pair.first << "\n";
          continue; }

        std::cerr << "replacing " << pair.first << "\n";

        for(auto &subject : env->critical_constraints) {
          ReplaceTermVisitor rep { env, subject.second, pair.first, pair.second };
          subject.second = rep.out;
        }

        if (FindTermDependents { env, pair.first }.out.size()) {
          std::cerr << COL_LIGHT_RED << "ERROR: dependencies still exist\n";
          std::cerr << pair.first << COL_CLEAR << "\n";
          exit(1);
        } else if (IsConcreteType { pair.second }.out) {
          env->Sideboard(pair.first);
        }
      }
    }
  }
}
