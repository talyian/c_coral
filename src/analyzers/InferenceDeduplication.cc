// This is the [Dedup] Reduction step of the Type Inference algorithm

#include "analyzers/InferenceObjectModel.hh"
#include "analyzers/InferenceEnvironment.hh"

#include <map>
#include <vector>
using namespace coral::typeinference;

void Deduplicate(
  TypeEnvironment * env,
  std::multimap<TypeTerm *, TypeConstraint *> constraints) {

  for(auto it = constraints.begin(); it != constraints.end();) {
    auto key = it->first;
    auto values = std::vector<std::pair<TypeTerm *, TypeConstraint*>>(it, constraints.upper_bound(key));
    std::cerr << COL_RGB(4, 2, 1) << "Dedoop "<< key
              << COL_RGB(4, 2, 1) << " :: "
              << COL_RGB(4, 2, 1) << values.size() << COL_CLEAR << "\n";
    it = constraints.upper_bound(key);
    // auto values = std::vector(it, constraints->upp
  }

  // // Some of the other steps naively create multiple copies of the same constraint
  // std::vector<std::pair<TypeTerm *, TypeConstraint *>> newconstraints;
  // for(auto it = constraints.begin(); it != constraints.end();) {
  //   auto key = it->first;
  //   auto compare_constraints = [](TypeConstraint * a, TypeConstraint * b) {
  //     if (typeid(a).hash_code() < typeid(b).hash_code()) return true;
  //     if (typeid(a).hash_code() > typeid(b).hash_code()) return false;
  //     Type *ta, *tb;
  //     if ((ta = dynamic_cast<Type *>(a)) && (tb = dynamic_cast<Type *>(b))) {
  //       std::stringstream ss;
  //       std::stringstream st;
  //       ss << ta;
  //       auto s = ss.str();
  //       st << tb;
  //       auto t = st.str();
  //       return s < t;
  //     }
  //     return false;
  //   };
  //   std::set<TypeConstraint *, decltype(compare_constraints)> values(compare_constraints);
  //   auto end = constraints.upper_bound(it->first);
  //   for(; it != end; it++) {
  //     values.insert(it->second);
  //   }
  //   for(auto &&constraint : values)
  //     newconstraints.push_back(std::make_pair(key, constraint));
  // }
  // env->critical_constraints.clear();
  // for(auto &&pair: newconstraints)
  //   env->critical_constraints.insert(pair);
}
