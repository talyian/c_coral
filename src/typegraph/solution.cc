#include "constraints.hh"
#include "solution.hh"
#include "typegraph.hh"
#include "utils/ansicolor.hh"

namespace typegraph {
  // show verbose debugging log
  bool showSteps = true;

  class Unify : public ConstraintVisitorDouble {
  public:
    TypeGraph * gg;
    Solution * solution;
    Unify(Solution * solution, Constraint * a, Constraint * b)
      : ConstraintVisitorDouble(a, b), solution(solution), gg(solution->gg) {
      if (showSteps)
        std::cerr << "unifying " << a << " :: " << b << "\n";
      run();
    };
    void visit2(Term * a, Type * b) {
      gg->constrain(a->term, b);
      solution->unknowns.insert(a->term);
    }
    void visit2(Type * a, Term * b) { visit2(b, a); }
    void visit2(Type * a, Type * b) {
      if (ConsEquals(a, b).out) return;
      std::cerr << "\033[31mWarning! Unifying " << a << " and " << b << "\033[0m\n";
    }
    void visit2(Term * a, Term * b) {
      if (a->term == b->term) return;
      gg->constrain(a->term, b);
      gg->constrain(b->term, a);
      solution->unknowns.insert(a->term);
      solution->unknowns.insert(b->term);
    }
  };

  class SubstituteKnowns : public ConstraintVisitor {
  public:
    TypeGraph * gg;
    Constraint * input;
    Constraint * output;
    int count = 0;
    std::unordered_map<TypeTerm *, Type *> * knowns;
    SubstituteKnowns(std::unordered_map<TypeTerm *, Type *> * knowns,
                     TypeGraph * gg,
                     Constraint *c) : knowns(knowns), gg(gg), input(c) {
      v(input);
    }
    Constraint * v(Constraint * c) { output = c; c->accept(this); return output; }
    void visit(Type * t) {
      for(auto &param: t->params)
        param = v(param);
      output = t;
    }
    void visit(Term * term) {
      if (knowns->find(term->term) != knowns->end()) {
        output = (*knowns)[term->term];
        count++;
      }
    }
    void visit(Call * c) {
      c->callee = v(c->callee);
      for(auto &param: c->arguments)
        param = v(param);
      output = c;
    }
  };

  class SubstituteTerm : public ConstraintVisitor {
  public:
    TypeGraph * gg;
    Constraint * in;
    Constraint * out;
    SubstituteTerm(TypeGraph * gg, Constraint * cc) : gg(gg), in(cc), out(cc) {
      cc->accept(this);
    }
    void visit(Call * c) {
      out = c->callee; c->callee->accept(this); c->callee = out;
      for(auto &p: c->arguments)
      { out = p; p->accept(this); p = out; }
      out = c;
    }
    void visit(Term * t) {
      auto cc = gg->relations.equal_range(t->term);
      for(auto it = cc.first; it != cc.second; it++) {
        if (dynamic_cast<Type *>(it->second)) {
          out = it->second;
          return;
        }
      }
    }
  };

  class Instantiate : public ConstraintVisitor {
  public:
    TypeGraph * gg;
    Constraint * input;
    Constraint * out;
    std::map<int, Term *> terms;
    Instantiate(TypeGraph * gg, Constraint * c) : gg(gg), input(c) { c->accept(this); }
    void visit(Term * t) { out = t; }
    void visit(Call * c) { out = c; }
    void visit(Type * t) {
      auto t2 = new Type(t->name, {});
      for(auto &p: t->params) {
        p->accept(this);
        t2->params.push_back(out);
      }
      out = t2;
    }
    void visit(Free * f) {
      if (terms.find(f->index) == terms.end()) {
        auto newterm = gg->addTerm("free" + std::to_string(f->index), 0);
        out = terms[f->index] = gg->term(newterm);
      } else
        out = terms[f->index];
    }
  };

  Solution::Solution(TypeGraph * gg) : gg(gg) {
    for(auto & pair: gg->relations) {
      auto term = pair.first;
      auto constraint = pair.second;
      if (isConcreteType(constraint))
        knowns.insert(std::make_pair(term, dynamic_cast<Type *>(constraint)));
      else
        unknowns.insert(term);
    }

    int count = 0;
  START:
    count++;
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
          unknowns.erase(unknowns.find(term));
          if (showSteps)
            std::cerr << "\033[32m adding " << it->first << " :: "
                      << it->second << "\033[0m\n";
          goto START;
        }
      }

      // if we're a call, we can substitute in even non-concrete types
      // range = gg->relations.equal_range(term);
      for(auto it = range.first; it != range.second; it++) {
        if (Call * c = dynamic_cast<Call *>(it->second)) {
          // std::cerr << it->first << "call!\n";
          c->callee = SubstituteTerm(gg, c->callee).out;
          for(auto &p: c->arguments)
            p = SubstituteTerm(gg, p).out;
        }
      }

      range = gg->relations.equal_range(term);
      // Apply Calls
      // Warning: The following block of code is pretty sketchy.
      // The intention is that it will be "functional" enough to support coral-alpha
      for(auto it = range.first; it != range.second; it++) {
        if (Call * call = dynamic_cast<Call *>(it->second)) {
          if (Type * callee = dynamic_cast<Type *>(call->callee)) {
            if (callee->name == "Func") {
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
                  std::cerr << "\033[31mWarning: size mismstach in call arguments: "
                            << call << "\033[0m\n";
                else {
                  auto arg = call->arguments[i];
                  Unify(this, param, arg);
                }
              }
              Unify(this, gg->term(term), callee->params.back());
              if (showSteps)
                std::cerr << "Deleting: " << it->first << " :: " << it->second << "\n";
              gg->relations.erase(it);
              goto START;
            } else if (callee->name == "Member") {
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
                    std::cerr << COL_LIGHT_RED << "warning: index field not found for "
                              << type->name << "::" << field << "\n" << COL_CLEAR;
                  } else {
                    std::cerr << COL_LIGHT_RED << "warning: data not found for "
                              << type->name << "::" << field << "\n" << COL_CLEAR;
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
                  std::cerr
                    << COL_LIGHT_RED << "No index? "
                    << type->name << "::" << field << "\n" << COL_CLEAR;
                } else {
                  std::cerr
                    << COL_LIGHT_RED << "where am I? "
                    << type->name << "::" << field << "\n" << COL_CLEAR;
                }
              }
            }
          }
        }
      }
    }

    if (showSteps) {
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
