#include "constraints.hh"
#include "typegraph.hh"
#include "unify.hh"
#include "instantiate.hh"
#include "substitution.hh"
#include "solution.hh"

namespace typegraph {
  // show verbose debugging log
  bool showSteps = true;

  Solution::Solution(TypeGraph * gg) : gg(gg) {
    for(auto & pair: gg->relations) {
      auto term = pair.first;
      auto constraint = pair.second;
      if (isConcreteType(constraint))
        knowns.insert(std::make_pair(term, dynamic_cast<Type *>(constraint)));
      else
        unknowns.insert(term);
    }

  START:
    count++;
    for(auto & term: unknowns) {
      // substitute knowns
      int subcount = 0;
      auto range = gg->relations.equal_range(term);
      for(auto it = range.first; it != range.second; it++) {
        auto sub = SubstituteKnowns(&knowns, gg, it->second);
        it->second = sub.output;
        if (showSteps) {
          std::cerr << "Substituting: " << it->first << " :: " << it->second << "\n";
          std::cerr << "wtf         : " << sub.output << "\n";
        }
        subcount += sub.count;
      }

      // convert unknown to known if we substituted
      for(auto it = range.first; it != range.second; it++) {
        if (isConcreteType(it->second)) {
          knowns.insert(std::make_pair(it->first, dynamic_cast<Type *>(it->second)));
          unknowns.erase(unknowns.find(term));
          // std::cerr
          //   << "\033[32m adding " << it->first << " :: "
          //   << it->second << "\033[0m\n";
          goto START;
        }
      }

      // if we're a call, we can substitute in even non-concrete types
      // range = gg->relations.equal_range(term);
      for(auto it = range.first; it != range.second; it++)
        if (Call * c = dynamic_cast<Call *>(it->second))
          SubstituteTerm(gg, c);

      range = gg->relations.equal_range(term);
      // Apply Calls
      // Warning: The following block of code is pretty sketchy.
      // The intention is that it will be "functional" enough to support coral-alpha
      for(auto it = range.first; it != range.second; it++) {
/*
Term term;
match constraint:
  Call(Type("Func", params), args):
     for i in range(params.length - 1):
        if params[i].name == "...": break
           Unify(args[i], params[i])
     Unify(term, params.back())
  Call(Type("Member", Type(field)), Type("Tuple", tupleargs)):
  Call(Type("Member", Type(field)), Type(typename)):
 */
        if (Call * call = dynamic_cast<Call *>(it->second)) {
          if (Type * callee = dynamic_cast<Type *>(call->callee)) {
            if (callee->name == "Func") {
              applyFunction(term, call, callee);
              gg->relations.erase(it);
              goto START;
            }
            else if (callee->name == "Member") {
              bool skip = false;
              if (dynamic_cast<Type *>(callee->params[0]) &&
                  dynamic_cast<Type *>(call->arguments[0])) {
                auto field = dynamic_cast<Type *>(callee->params[0])->name;
                auto type = dynamic_cast<Type *>(call->arguments[0]);

                if (type->name == "Tuple") {
                  for(size_t i = 0; i < type->params.size(); i++) {
                    if (field == "Item" + std::to_string(i)) {
                      auto tuple_field = type->params[i];
                      gg->constrain(it->first, tuple_field);
                      auto instance_index_term = gg->addTerm(
                        it->first->name + ".index", it->first->expr);
                      knowns.insert(std::make_pair(
                                      instance_index_term,
                                      gg->type("Index",  {gg->type(std::to_string(i))})));
                      gg->constrain(
                        instance_index_term,
                        gg->type("Index",  {gg->type(std::to_string(i))}));
                      if (showSteps) {
                        std::cerr << "Deleting: " << it->first << " :: " << it->second << "\n";
                      }
                      gg->relations.erase(it);
                      goto START;
                    }
                  }
                }
                else if (gg->termByName[type->name + "::" + field] && gg->termByName[type->name + "::" + field + ".index"])
                {
                  auto type_term =
                    gg->relations.find(gg->termByName[type->name + "::" + field]);
                  auto index_term =
                    gg->relations.find(gg->termByName[type->name + "::" + field + ".index"]);

                  if (type_term != gg->relations.end() && index_term != gg->relations.end()) {
                    auto out_type = type_term->second;
                    auto out_index = std::stoi(dynamic_cast<Type *>(index_term->second)->name);
                    gg->constrain(it->first, out_type);
                    auto instance_index_term = gg->addTerm("index", it->first->expr);
                    gg->constrain(
                      instance_index_term,
                      gg->type("Index",  {index_term->second}));
                    knowns.insert(std::make_pair(instance_index_term,
                                                 gg->type("Index",  {index_term->second})));
                    if (showSteps) {
                      std::cerr << "Member " << instance_index_term
                                << " :: " << gg->type("Index",  {index_term->second}) << "\n";
                      std::cerr << "Deleting: " << it->first << " :: " << it->second << "\n";
                    }
                    gg->relations.erase(it);
                    goto START;
                  } else if (type_term != gg->relations.end()) {
                    std::cerr << "warning: index field not found for "
                              << type->name << "::" << field << "\n";
                  } else {
                    std::cerr << "warning: data not found for "
                              << type->name << "::" << field << "\n";
                  }
                }
                else if (gg->termByName[type->name + "::" + field]) {
                  auto term = gg->termByName[type->name + "::" + field];
                  auto constraint = gg->relations.find(term)->second;
                  if (Type * func = dynamic_cast<Type *>(constraint)) {
                    if (func->name == "Func") {
                      // add a pointer to the referent method
                      auto funcptr = gg->addTerm(it->first->name + ".func", it->first->expr);
                      gg->constrain(funcptr, gg->type("Term", {gg->type(type->name + "::" + field)}));
                      knowns.insert(std::make_pair(
                                      funcptr,
                                      gg->type("Term", {gg->type(type->name + "::" + field)})));
                      gg->constrain(it->first, func);
                      gg->relations.erase(it);
                      goto START;
                    }
                  }
                  std::cerr << "No index? " << type->name << "::" << field << "\n";
                } else {
                  std::cerr << "Where am I? " << type->name << "::" << field << "\n";
                }
              }
            }
            else if (callee->name == "Or") {
              std::cerr << "\033[31m Applying OR!!! \033[0m\n";
            }
          }
        }
      }
    }
  }

  void Solution::applyFunction(TypeTerm * term, Call * call, Type * callee) {
    Instantiate instantiate(gg, callee);
    for(auto &pair: instantiate.terms)
      unknowns.insert(pair.second->term);
    callee = dynamic_cast<Type *>(instantiate.out);

    for(size_t i = 0; i < callee->params.size() - 1; i++) {
      auto param = callee->params[i];
      if (Type * type_param = dynamic_cast<Type *>(param))
        if (type_param->name == "...")
          break;
      if (i >= call->arguments.size())
        std::cerr << "\033[31mWarning: size mismatch in call arguments: "
                  << call << "\033[0m\n";
      else {
        auto arg = call->arguments[i];
        std::cerr << "Unifying call argument\n";
        std::cerr << callee << " at index " << i << "\n";
        Unify(this, term, param, arg);
      }
    }
    std::cerr << "Unifying call retval\n";
    Unify(this, term, gg->term(term), callee->params.back());
    if (showSteps)
      std::cerr << "Deleting: " << term << " :: " << call << "\n";
  }

  void Solution::showSummary() {
    std::cout << "Changes Count: " << count << "\n";
    showKnowns();
    if (unknowns.size()) {
      std::cout << "Unknowns:\n";
      for (auto &x: unknowns) {
        auto range = gg->relations.equal_range(x);
        for(auto it = range.first; it != range.second; it++) {
          std::cout << std::setw(20) << it->first << " :: " << it->second << "\n";
        }
      }
    }
  }
  void Solution::showKnowns() {
    if (knowns.size()) {
      std::cout << "Knowns:\n";
      for (auto &x: knowns) {
        std::cout << std::setw(20) << x.first << " :: " << x.second << "\n";
      }
    }
  }
  Type * Solution::getType(TypeTerm * term) {
    auto range = gg->relations.equal_range(term);
    for(auto it = range.first; it != range.second; it++) {
      auto constraint = it->second;
      if (isConcreteType(constraint)) {
        return dynamic_cast<Type *>(constraint);
      }
    }
    return 0;
  }

  std::unordered_multimap<TypeTerm *, Type *> Solution::allKnownTypes() {
    std::unordered_multimap<TypeTerm *, Type *> result;
    for(auto &pair: knowns) {
      auto term = pair.first;
      auto range = gg->relations.equal_range(term);
      for(auto it = range.first; it != range.second; it++) {
        if (isConcreteType(it->second))
          result.insert(std::make_pair(term, dynamic_cast<Type *>(it->second)));
      }
    }
    return result;
  }
}
