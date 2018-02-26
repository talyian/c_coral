#pragma once
#include "utils/ansicolor.hh"
#include "analyzers/typegraph/constraint.hh"
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <iostream>

class TypeGraph;

// Handles Unifying two constraints
// TODO: replace with IsAssignable semantics later on
class TypeUnify {
public:
  TypeGraph * graph;
  int counter = 0;
  TypeUnify(TypeGraph * graph) : graph(graph) { }

  void equal(Constraint * a, Constraint * b)  {
    std::cerr << "Unhandled Unify: (" << a << ", " << b << ")\n";
  }
  // void equal(Term * a, Free * b);
  // void equal(Type * a, Free * b);
  void equal(Type * a, Term * b);
  void equal(Term * a, Term * b);
  void equal(Type * a, Type * b);
};

class TypeTermReplacer : public ConstraintVisitor {
public:
  TypeGraph * graph;
  TypeTerm * search;
  Constraint * subject;
  Constraint * replace;
  Constraint * out = 0;
  TypeTermReplacer(
    TypeGraph * graph, Constraint * subject, TypeTerm * search, Constraint * replace);
  void visit(Type *);
  void visit(Term *);
  void visit(Free *);
  void visit(Call *);
};

// converts a Func(T, T) to a Func(T0, T0)
// i.e. for each callsite we generate a term that represents
// a particular instance of a free type
class InstantiateFree : public ConstraintVisitor {
public:
  static Type * of(TypeGraph * graph, Type * root) {
    InstantiateFree ff(graph, root); return (Type *)ff.out;
  }
  TypeGraph * graph;
  Type * root;
  Constraint * out = 0;
  std::map<Free *, Term *> vars;
  InstantiateFree(TypeGraph * graph, Type * root)
    : graph(graph), root(root) {
    root->accept(this);
  }
  void visit(Type * t);
  void visit(Term * t) { out = t; }
  void visit(Free * f);
  void visit(Call * c) { out = c; }
};

// Returns all the constraints that contain a particular term
class Dependents : public ConstraintVisitor {
public:
  static std::set<Constraint *> of(TypeGraph * graph, TypeTerm * term) {
    return Dependents(graph, term).out;
  }
  std::set<Constraint *> out;
  TypeGraph * graph;
  // the term we're looking for dependents of
  TypeTerm * subject;
  // a candidate for inclusion in the dependents list
  TypeTerm * currentTerm;
  Constraint * currentRoot;
  Dependents(TypeGraph * graph, TypeTerm * term);
  void visit(Type * t);
  void visit(Term * t);
  void visit(Free * f);
  void visit(Call * c);
};

// returns true if is argument is a type that contains only other types
class SimpleType : public ConstraintVisitor {
public:
  static bool of(Type * t) { return SimpleType(t).out; }
  bool out = true;
  SimpleType(Type * t) { t->accept(this); }
  void visit(Type * t) { for(auto &p: t->params) p->accept(this); }
  void visit(Term * t) { out = false; }
  void visit(Free * f) { out = false; }
  void visit(Call * c) { out = false; }
};

class TypeGraph {
private:
  std::vector<std::unique_ptr<TypeTerm>> terms;
  std::map<std::string, TypeTerm *> termnames;
  std::vector<std::unique_ptr<Constraint>> constraintStore;
  // std::set<std::pair<TypeTerm *, Constraint*>> relations;
  std::map<Constraint *, TypeTerm *> relations;
public:
  TypeTerm * AddTerm(std::string name) {
    terms.emplace_back(new TypeTerm(name));
    return termnames[name] = terms.back().get();
  }

  template <class T>
  T * addcons(T * p) { constraintStore.emplace_back(p); return p; }
  Type * type(std::string name, std::vector<Constraint *> v) {
    return addcons(new Type(name, v)); }
  Type * type(std::string name) { return addcons(new Type(name)); }
  Free * free(int n) { return addcons(new Free(n)); }
  Call * call(Constraint * callee, std::vector<Constraint *> v) {
    return addcons(new Call(callee, v)); }
  Term * term(std::string name) { return addcons(new Term(name, termnames[name])); }

  void AddConstraint(std::string termname, Constraint * c) {
    if (termnames.find(termname) == termnames.end()) {
      std::cerr << "not found " << termname << "\n";
      exit(3);
      return;
    }
    auto term = termnames[termname];
    if (!term) {
      std::cerr << "Null term?!? " << termname << "\n";
      exit(3);
      return;
    }
    if (relations.find(c) != relations.end()) {
      std::cerr << "constraint collision: " << termname << " :: " << c << "\n";
      return;
    }

    std::cerr << COL_RGB(2, 5, 3) << "adding" << std::setw(24) << term
              << " :: " << c << COL_CLEAR << "\n";
    relations[c] = term;
  }

  void RemoveConstraint(TypeTerm * tt, Constraint * cc) {
    auto it = relations.find(cc);
    // std::cerr << "Removing! " << tt << "\n";
    if (it == relations.end()) {
      std::cerr << "could not remove " << tt << " :: " << cc << "\n";
      exit(1);
    }
    else
      relations.erase(relations.find(cc));
  }

  std::map<Constraint *, TypeTerm *> GetRelations () {
    return relations;
  }
  void Show(std::string header);

  void Step() {
    /*

How Optimally this should work:

  while RULE = (Term,Constraint) = pop_work_queue():
     if [apply] on term:
        // Conditions: Term is a Call(Func[T])
        // Action: we create new (Term,Constraints) for each parameter and retval
        // Action: we remove RULE
     if [substitute] on term:
        // Conditions [Phase 1]: if Term is a complete type (i.e. no terms, frees, calls)
        // Conditions [Phase 2]: if Term is a non-recursive type (with frees and terms)
        //           : !constraint->recursive_refs->contains(term)
        //           : dynamic_cast<Type *>(constraint)
        // Action: for(rule: term->direct_dependents) -> replace(rule, term, constraint)
        // Action: sideboard RULE
        // Conditions [Phase 3]: if Term is another Term:
        // Action: for(rule: term->direct_dependents) -> replace(rule, term, constraint)
        // Action: sideboard RULE
     */
    int old_changes = -1;
    while(changes != old_changes) {
    BEGIN:
      old_changes = changes;
      // TODO: this should be a work queue instead of scanning the entire set
      for(auto && c : relations) {
        // [Apply] Rule
        if (Call * cc = dynamic_cast<Call *>(c.first)) {
          if (Type * f = dynamic_cast<Type *>(cc->callee)) {
            Apply(c.second, cc, f);
            break;
          }
        }
        // [Substitution - Phase 1] Rule

        if (Type * cons = dynamic_cast<Type *>(c.first))
          if (SimpleType::of(cons)) {
            auto dependents = Dependents::of(this, c.second);
            if (dependents.size()) {
              std::cerr << "substituting " << COL_LIGHT_YELLOW << std::setw(17) << c.second
                        << " :: " << cons << COL_CLEAR << "\n";
              for(auto &dependent: dependents)
                Substitute(dependent, c.second, cons);
              // we unify all the constraints of c.second against c.first
              // e.g. { a:: Int, a:: b} -> { b :: Int}
              for(auto other_constraint: AllConstraints::of(this, c.second)) {
                if (other_constraint == c.first) continue;
                RemoveConstraint(c.second, other_constraint);
                AddEquality(c.first, other_constraint);
              }
              // TODO: sideboard rule
              RemoveConstraint(c.second, c.first);
              break;
            }
          }
        // Substitution - Term
        if (Term * cons = dynamic_cast<Term *>(c.first)) {
          auto dependents = Dependents::of(this, c.second);
          if (dependents.size()) {
            std::cerr << "substituting " << std::setw(17) << c.second << " :: " << cons << "\n";
            for(auto &dependent: dependents) {
              std::cerr << COL_LIGHT_BLUE << c.first << "\n";
              auto replaced = Substitute(dependent, c.second, c.first);
              std::cerr << std::setw(30) << replaced << "\n";
              // std::cerr << " replaced with " << replaced << COL_CLEAR << "\n";
            }
            // RemoveConstraint(c.second, c.first);
            break;
          }
        }
      }
      // std::cerr << changes - old_changes << "\n";
    }
  }

  void Apply(TypeTerm * t, Call * call, Type * callfunc) {
    // TODO: this can be implemented as a unify operation on two Funcs
    // get rid of free types in F's type signature
    Type * callee = (Type *)InstantiateFree::of(this, callfunc);
    std::cerr << "applying" << COL_RGB(4, 5, 2) << std::setw(22) << t << " :: "
              << std::setw(20) << callfunc << " -> "
              << std::setw(20) << callee << COL_CLEAR << "\n";
    for(size_t i = 0; i < call->args.size(); i++)
      AddEquality(call->args[i], callee->params[i]);
    AddEquality(term(t->name), callee->params.back());
    this->RemoveConstraint(t, call);
    changes++;
  }
  int changes = 0;

  Constraint * Substitute(Constraint * subject, TypeTerm * find, Constraint * replace) {
    auto out = TypeTermReplacer (this, subject, find, replace).out;
    this->relations[out] = this->relations[subject];
    this->relations.erase(subject);
    // return TypeTermReplacer (this, subject, find, replace).out;
    return out;
  }

  void MarkAllDirty() { }
  void AddEquality(Constraint * a, Constraint * b) {
    TypeUnify teq(this);
    new TypeEqualityWrapper<TypeUnify *>(&teq, a, b);
  }
};

class ResolvePass {

};
