#pragma once
#include "constraints.hh"

#include <unordered_set>
#include <unordered_map>

namespace typegraph {
  class TypeGraph;
  class Solution {
  public:
    TypeGraph * gg;
    std::unordered_set<TypeTerm *> unknowns;
    std::unordered_map<TypeTerm *, Type *> knowns;
    std::unordered_set<TypeTerm *> work;
    Solution(TypeGraph * gg);
    Type * getType(TypeTerm *);
    void showKnowns();
  };
}
