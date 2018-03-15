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
    if (count++ > 1000) {
      std::cerr << "\033[31mToo many iterations of type checker!\033[0m\n";
      return;
    }
    // if (showSteps) {
    //   showUnknowns();
    //   showKnowns();
    // }
    for(auto & term: unknowns) {
      // substitute knowns
      int subcount = 0;
      auto range = gg->relations.equal_range(term);
      for(auto it = range.first; it != range.second; it++) {
        auto sub = SubstituteKnowns(&knowns, gg, it->second);
        it->second = sub.output;
        if (sub.count && showSteps) {
          std::cerr << "Substituting: " << it->first << " :: " << it->second << "\n";
        }
        subcount += sub.count;
      }

      // convert unknown to known if we substituted
      for(auto it = range.first; it != range.second; it++) {
        if (isConcreteType(it->second)) {
          knowns.insert(std::make_pair(it->first, dynamic_cast<Type *>(it->second)));
          // If we're going to erase all the constraints of a term,
          // we should unify each of them against the "solid type"
          // to avoid losing information. This lets us get around the
          // [HACK] of generating doubled terms in TypeResolver.cc
          std::vector<Constraint *> constraints;
          for(auto other_constraint = range.first;
              other_constraint != range.second;
              other_constraint++) {
            constraints.push_back(other_constraint->second);
          }
          for(auto &constraint : constraints)
            Unify(this, term, it->second, constraint);
          if (showSteps)
            std::cerr
              << "\033[32m adding " << it->first << " :: "
              << it->second << "\033[0m\n";
          unknowns.erase(unknowns.find(term));
          goto START;
        }
      }

      if (subcount) goto START;

      // if we're a call, we can substitute in even non-concrete types
      range = gg->relations.equal_range(term);
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
            if (callee->name == "BoundMethod") {
              // boundmethods are basically functions as far as the type inferrer is concerned
              applyFunction(term, call, callee);
              gg->relations.erase(it);
              goto START;
            }
            else if (callee->name == "Func") {
              applyFunction(term, call, callee);
              gg->relations.erase(it);
              goto START;
            }
            // else if (callee->name == "MethodCall") {
            //   // gg->show();
            //   if (applyMethod(term, call, callee)) {
            //     gg->relations.erase(it);
            //     goto START;
            //   }
            // }
            else if (callee->name == "Member") {
              bool skip = false;
              if (dynamic_cast<Type *>(callee->params[0]) &&
                  dynamic_cast<Type *>(call->arguments[0])
                ) {
                auto field = dynamic_cast<Type *>(callee->params[0])->name;
                auto type = dynamic_cast<Type *>(call->arguments[0]);
                auto instance = dynamic_cast<Term *>(call->arguments[1]);
                if (type->name == "Type" && isConcreteType(type)) {
                  std::cerr << "member on a type\n";
                  std::cerr << it->first << " :: " << it->second << "\n";
                  auto constructor = dynamic_cast<Type*>(type->params[0]);
                  auto type_name = dynamic_cast<Type *>(constructor->params.back());
                  call->arguments[0] = type_name;
                  goto START;
                }
                if (type->name == "Tuple") {
                  for(size_t i = 0; i < type->params.size(); i++) {
                    if (field == "Item" + std::to_string(i)) {
                      auto tuple_field = type->params[i];
                      gg->constrain(it->first, tuple_field);
                      auto instance_index_term = gg->addTerm(
                        it->first->name + ".index", it->first->expr);
                      knowns.insert(
                        std::make_pair(
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
                // dereference a field from a tuple
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
                    auto instance_index_term = gg->addTerm(
                      it->first->name + ".index", it->first->expr);
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
                // methodcall
                else if (gg->termByName[type->name + "::" + field]) {
                  auto referred_name = type->name + "::" + field;
                  auto referred_term = gg->termByName[referred_name];
                  auto constraint = gg->relations.find(referred_term)->second;
                  if (Type * func = dynamic_cast<Type *>(constraint)) {
                    if (func->name == "Func") {
                      // add a pointer to the referent method
                      auto funcptr = gg->addTerm(it->first->name + ".func", it->first->expr);
                      gg->constrain(funcptr, gg->type("FuncTerm", {gg->type(type->name + "::" + field)}));
                      knowns.insert(
                        std::make_pair(
                          funcptr,
                          gg->type("FuncTerm", {gg->type(type->name + "::" + field)})));
                      gg->constrain(it->first, func);
                      gg->relations.erase(it);
                      goto START;
                    }
                    if (func->name == "Method") {
                      auto funcptr = gg->addTerm(it->first->name + ".method", it->first->expr);

                      // the functerm allow typeresultwriter to point to the right method
                      gg->constrain(
                        funcptr,
                        gg->type("FuncTerm", {gg->type(type->name + "::" + field)}));
                      knowns.insert(
                        std::make_pair(
                          funcptr,
                          gg->type("FuncTerm", {gg->type(type->name + "::" + field)})));

                      auto member_term = it->first; // fooInst.bar
                      std::cerr << "\033[36m Method Call " << member_term << "\n";
                      std::cerr << call << "\n";
                      std::cerr << "referred:  "<< referred_term << "\n";
                      gg->constrain(
                        member_term,
                        gg->type("BoundMethod", { func->params.back() }));
                      gg->relations.erase(it);
                      // gg->constrain(
                      //   member_term,
                      //   gg->type("BoundMethod", {
                      //       gg->type("Term", {
                      //   }));
                      // gg->constrain(
                      //   tt,
                      //   gg->type("MethodCall", {
                      //       gg->term(referred_term),
                      //         gg->type("Term", {gg->type(referred_name)}),
                      //         gg->type("Term", {gg->type("p")})}));
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
              auto args = call->arguments;
              int option_id = -1;
              for(auto &option:callee->params) {
                option_id++;
                Type * func = 0;
                if ((func = dynamic_cast<Type *>(option)) && func->name == "Func") {
                  if (showSteps)
                    std::cerr << "\033[31m Applying "<< callee << " \033[0m\n";
                  bool mismatch = false;
                  for(size_t i = 0; i < call->arguments.size(); i++) {
                    if (!ConsEquals(func->params[i], call->arguments[i]).out)
                      mismatch = true;
                  }
                  if (!mismatch) {
                    for(size_t i = 0; i < call->arguments.size(); i++) {
                      Unify(this, term, func->params[i], call->arguments[i]);
                    }
                    Unify(this, term, gg->term(term), func->params.back());
                    auto overload_id = gg->addTerm(term->name + ".overload", term->expr);
                    gg->constrain(
                      overload_id,
                      gg->type("OverloadID", {gg->type(std::to_string(option_id))}));
                    knowns.insert(std::make_pair(
                      overload_id,
                      gg->type("OverloadID", {gg->type(std::to_string(option_id))})));
                    gg->relations.erase(it);
                    goto START;
                  }
                }
              }
            }
            else if (callee->name == "Type") {
              auto constructor = dynamic_cast<Type*>(callee->params[0]);
              if (constructor) {
                applyFunction(term, call, constructor);
                gg->relations.erase(it);
                goto START;
              }
            }
            else {
              std::cerr << "\033[31mUnknown callee "
                        << it->first << " :: " << it->second
                        << "\033[0m\n";
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
        // std::cerr << "Unifying call argument\n";
        // std::cerr << callee << " at index " << i << "\n";
        Unify(this, term, param, arg);
      }
    }
    // std::cerr << "Unifying call retval\n";
    Unify(this, term, gg->term(term), callee->params.back());
    if (showSteps)
      std::cerr << "Deleting: " << term << " :: " << call << "\n";
  }

  bool Solution::applyMethod(TypeTerm * term, Call * call, Type * callee) {
    if (!isConcreteType(callee->params[0])) return false;
    if (showSteps)
      std::cerr << "\033[35mcalling method " << callee << "\033[0m\n";

    auto method_type = dynamic_cast<Type *>(callee->params[0]);
    auto func_ptr_term = dynamic_cast<Type *>(
      dynamic_cast<Type *>(callee->params[1])->params[0]);
    auto self_term = dynamic_cast<Type *>(
      dynamic_cast<Type *>(callee->params[2])->params[0]);

    // Deliverables:
    // Call(callee, args)
    // we should emit a "call.inst.bar.method" => MethodTerm[Type::bar, &inst]
    // so that the result writer can do the method-call inversion
    // call(member(a, b), {c}) -> call(A::b, {b, c})
    auto method_term = gg->addTerm(term->name + ".method", term->expr);
    gg->constrain(method_term, gg->type("MethodTerm", {func_ptr_term, self_term}));
    unknowns.insert(method_term);
    // this if we add the self-param, method-call reduces to a regular function call
    auto args = call->arguments;
    args.insert(args.begin(), gg->term(self_term->name));
    auto newcall = gg->call(method_type, args);
    applyFunction(term, newcall, method_type);
    // method_type->params.insert(method_type->params.begin(), gg->term(self_term));

    std::cerr << "\033[35mapplying Method " << term << "\033[0m\n";
    return false;

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
        // std::cerr << "Unifying call argument\n";
        // std::cerr << callee << " at index " << i << "\n";
        Unify(this, term, param, arg);
      }
    }
    // std::cerr << "Unifying call retval\n";
    Unify(this, term, gg->term(term), callee->params.back());
    if (showSteps)
      std::cerr << "Deleting: " << term << " :: " << call << "\n";
    return true;
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
  void Solution::showUnknowns() {
    std::cout << "Unknowns ("<< unknowns.size() << "):\n";
    for (auto &x: unknowns) {
      auto range = gg->relations.equal_range(x);
      for(auto it = range.first; it != range.second; it++)
        std::cout << std::setw(20) << x << " :: " << it->second << "\n";
    }
  }
  void Solution::showKnowns() {
    std::cout << "Knowns:\n";
    for (auto &x: knowns) {
      std::cout << std::setw(20) << x.first << " :: " << x.second << "\n";
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
