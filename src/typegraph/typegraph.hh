#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <map>
#include <iomanip>

#include "constraints.hh"
#include "solution.hh"

namespace typegraph {
  class TypeGraph {
  private:
    // find a term by
    std::vector<std::unique_ptr<TypeTerm>> terms;
    std::map<std::string, TypeTerm *> termByName;
    std::unordered_multimap<void *, TypeTerm *> termByValue;

    std::vector<std::unique_ptr<Constraint>> constraints;

    std::unordered_multimap<TypeTerm *, Constraint *> relations;

    friend class Solution;
    friend class SubstituteTerm;;
    template<class T> T * _constraint(T * ptr) {
      constraints.push_back(std::unique_ptr<Constraint>(ptr));
      return ptr;
    }
  public:
    TypeTerm * addTerm(std::string original_name, ExprType expr);
    TypeTerm * findTerm(ExprType expr);

    void constrain(std::string term, Constraint * cons) {
      if (termByName.find(term) == termByName.end()) addTerm(term, 0);
      constrain(termByName[term], cons);
    }
    void constrain(TypeTerm * term, Constraint * cons) {
      relations.insert(std::make_pair(term, cons));
    }

    Type * type(std::string name) { return type(name, {}); }
    Type * type(std::string name, std::vector<Constraint *> params) {
      return _constraint(new Type(name, params));
    }
    Free * free(int ordinal) { return _constraint(new Free(ordinal)); }
    Term * term(std::string name) { return term(termByName[name]); }
    Term * term(TypeTerm * term) {  return _constraint(new Term(term)); }
    Call * call(Constraint * callee, std::vector<Constraint *> arguments) {
      return _constraint(new Call(callee, arguments));
    }

    void show() {
      for (auto &pair: relations) {
        std::cout << std::setw(20) << pair.first << " :: " << pair.second << "\n";
      }
      // for (auto &t: termByName)
      //   std::cout << std::setw(10) << t.first << std::setw(20) << t.second << "\n";
    }
    Solution solve() {
      // show();
      return Solution(this);
    }
  };
}
