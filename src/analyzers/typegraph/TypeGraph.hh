#pragma once
#include "core/type.hh"
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
    exit(1);
  }
  void equal(Type * a, Term * b);
  void equal(Term * a, Type * b);
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
  std::map<int, Term *> vars;
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

// returns true if is argument is a type that contains only other types or Free variables
class SimpleType : public ConstraintVisitor {
public:
  static bool of(Type * t) { return SimpleType(t).out; }
  bool out = true;
  SimpleType(Type * t) { t->accept(this); }
  void visit(Type * t) { for(auto &p: t->params) p->accept(this); }
  void visit(Term *) { out = false; }
  void visit(Free *) { }
  void visit(Call *) { out = false; }
};

class TypeGraph {
private:
  std::vector<std::unique_ptr<TypeTerm>> terms;
  std::map<std::string, TypeTerm *> termnames;
  std::vector<std::unique_ptr<Constraint>> constraintStore;
  // std::set<std::pair<TypeTerm *, Constraint*>> relations;
  std::map<Constraint *, TypeTerm *> relations;
public:
  std::map<coral::ast::BaseExpr *, TypeTerm *> expr_terms;

  Type * GetTypeConstraintForTerm(TypeTerm * term) {
    for(auto & pair: relations)
      if (Type * tt = dynamic_cast<Type *>(pair.first))
        if (pair.second == term)
          return tt;
    return 0;
  }

  TypeTerm * FindTerm(coral::ast::BaseExpr * expr) {
    if (expr_terms.find(expr) == expr_terms.end()) return 0;
    return expr_terms[expr];
  }

  TypeTerm * AddTerm(std::string name, coral::ast::BaseExpr * expr) {
    auto term = AddTerm(name);
    term->expr = expr;
    expr_terms[expr] = term;
    return term;
  }
  TypeTerm * AddTerm(std::string _name) {
    auto name = _name;
    int i = 0;
    while (termnames.find(name) != termnames.end()) {
      name = _name + "." + std::to_string(++i);
    }
    terms.emplace_back(new TypeTerm(name));
    return termnames[name] = terms.back().get();
  }

  template <class T>
  T * addcons(T * p) {
    constraintStore.push_back(std::unique_ptr<Constraint>(p));
    return p;
  }
  TypeTerm * GetTermByName(std::string name) { return termnames[name]; }
  Type * type(std::string name, std::vector<Constraint *> v) {
    return addcons(new Type(name, v)); }
  Type * type(std::string name) { return addcons(new Type(name)); }
  Free * free(int n) { return addcons(new Free(n)); }
  Call * call(Constraint * callee, std::vector<Constraint *> v) {
    return addcons(new Call(callee, v)); }
  Term * term(std::string name) { return addcons(new Term(name, termnames[name])); }
  Term * term(TypeTerm * t) { return addcons(new Term(t->name, t)); }
  ::Type * type(coral::type::Type * ct) {
    auto ret = new ::Type(ct->name);
    for(auto &p : ct->params) ret->params.push_back(type(&p));
    return addcons(ret);
  }

  void AddConstraint(std::string termname, Constraint * c) {
    AddConstraint(termnames[termname], c);
  }

  void AddConstraint(TypeTerm * tt, Constraint * c);

  void SideboardConstraint(TypeTerm * tt, Constraint * cc);

  void RemoveConstraint(TypeTerm * tt, Constraint * cc);

  std::map<Constraint *, TypeTerm *> GetRelations () {
    return relations;
  }
  void Show(std::string header);

  void Step();

  void Apply(TypeTerm * t, Call * call, Type * callfunc);
  int changes = 0;

  Constraint * Substitute(Constraint * subject, TypeTerm * find, Constraint * replace) {
    auto out = TypeTermReplacer (this, subject, find, replace).out;
    this->AddConstraint(this->relations[subject], out);
    this->relations.erase(subject);
    changes++;
    return out;
  }

  void MarkAllDirty() { }
  void AddEquality(Constraint * a, Constraint * b) {
    TypeUnify teq(this);
    TypeEqualityWrapper<TypeUnify *>(&teq, a, b);
  }
};
