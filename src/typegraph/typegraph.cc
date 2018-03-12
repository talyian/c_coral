#include "constraints.hh"
#include "solution.hh"
#include "typegraph.hh"

#include <memory>

#ifndef ExprType
#define ExprType void *
#endif
namespace typegraph {
  TypeTerm * TypeGraph::findTerm(ExprType expr) {
    auto t = termByValue.find(expr);
    if (t == termByValue.end())
      return 0;
    return t->second;
  }
  TypeTerm * TypeGraph::addTerm(std::string original_name, ExprType expr) {
    auto t = new TypeTerm();
    int i = 0;
    auto name = original_name;
    while (termByName.find(name) != termByName.end())
      name = original_name + "#" + std::to_string(i++);
    termByName.insert(std::make_pair(name, t));
    termByValue.insert(std::make_pair(expr, t));
    t->name = name;
    t->expr = expr;
    terms.push_back(std::unique_ptr<TypeTerm>(t));
    return t;
  }
}
