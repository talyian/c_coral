#include "utils/ansicolor.hh"
#include "analyzers/typegraph/constraint.hh"
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <iostream>

class TypeEquality {
public:
  int counter = 0;
  void equal(Constraint * a, Constraint * b)  {
    std::cerr << "oops\n" << a << ", " << b << "\n";
  }
  void equal(Term * a, Free * b) {
    counter++; std::cerr << "termfree match\n";
  }
};

class TypeGraph {
private:
  std::vector<std::unique_ptr<TypeTerm>> terms;
  std::map<std::string, TypeTerm *> termnames;
  std::vector<std::unique_ptr<Constraint>> constraintStore;
  std::map<TypeTerm *, Constraint*> relations;
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
  Term * term(std::string name) { return addcons(new Term(name)); }

  void AddConstraint(std::string termname, Constraint * c) {
    if (termnames.find(termname) == termnames.end()) {
      std::cerr << "not found " << termname << "\n"; return; }
    auto term = termnames[termname];
    relations[term] = c;
  }
  void Show(std::string header) {
    std::cout << "------------------------------" << header << '\n';
    for(auto &&pair: relations) {
      std::cerr << pair.first << "  ::  " << pair.second << "\n";
    }
  }
  void Step() {
    for(auto && c : relations) {
      if (Call * cc = dynamic_cast<Call *>(c.second))
        if (Type * f = dynamic_cast<Type *>(cc->callee)) {
          Apply(c.first, cc, f);
        }
    }
}
  void Apply(TypeTerm * t, Call * call, Type * f) {
    std::cerr << COL_RGB(4, 5, 2) << call << " Applying\n" << COL_CLEAR;
    for(size_t i = 0; i < call->args.size(); i++) {
      AddEquality(call->args[i], f->params[i]);
    }
    AddEquality(term(t->name), f->params.back());
  }
  void MarkAllDirty() { }
  void AddEquality(Constraint * a, Constraint * b) {
    TypeEquality teq;
    new TypeEqualityWrapper<TypeEquality *>(&teq, a, b);
  }
};

class ResolvePass {

};
