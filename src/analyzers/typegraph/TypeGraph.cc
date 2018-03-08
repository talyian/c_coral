#include "utils/opts.hh"
#include "TypeGraph.hh"
#include "TypeGraphExtensions.hh"
#include <algorithm>

void TypeUnify::equal(Type * a, Term * b) {
  graph->AddConstraint(b->term, a);
}

void TypeUnify::equal(Term * b, Type * a) {
  graph->AddConstraint(b->term, a);
}

void TypeUnify::equal(Term * a, Term * b) {
  if (a->term != b->term)
    graph->AddConstraint(a->term, b);
}

void TypeUnify::equal(Type * a, Type * b) {
  if (b->name == "Field") {
    // HACK: if we're unifying a T against x:T, this should type-check
    // TODO: I wonder if we need type-level strings?
    // probably slightly lower priority than type-level integers.
    if (ConstraintEqualsImpl::of(a, b->params[1])) return;
  }
  if (!ConstraintEqualsImpl::of(a, b)) {
    std::cerr << COL_LIGHT_RED << "Warning: Trying to unify " << a << ", " << b << COL_CLEAR << "\n";
    return;
  }
}

void TypeGraph::Show(std::string header) {
  std::cout << "------------------------------" << header << '\n';
  for(auto &&pair: relations) {
    std::cerr << COL_RGB(5, 2, 3) << std::setw(20) << pair.second
      // << "(" << std::to_string((unsigned long)pair.second) << ")"
              << COL_RGB(3, 3, 3) << "  ::  "
              << pair.first << COL_CLEAR "\n";
  }
}

TypeTermReplacer::TypeTermReplacer(
  TypeGraph * graph, Constraint * subject, TypeTerm * search, Constraint * replace)
  : graph(graph), search(search), subject(subject), replace(replace) {
  out = subject;
  if (search) {
    auto repterm = dynamic_cast<Term *>(replace);
    if (repterm && repterm->term == search)
      return;
    subject->accept(this);
  }
}

void TypeTermReplacer::visit(Type * t) {
  for(auto &p : t->params) { out = p; p->accept(this); p = out; }
  out = t;
}
void TypeTermReplacer::visit(Term * t) {
  if (t->term == search) {
    if (coral::opt::ShowTypeSolution)
      std::cerr
        << COL_YELLOW << "replacing " << std::setw(20) << subject << " "
        << COL_CLEAR << search << " with " << replace << "\n";
    graph->changes++;
    out = replace;
  } else
    out = t;
}
void TypeTermReplacer::visit(Free * f) { out = f; }
void TypeTermReplacer::visit(Call * c) {
  out = c->callee; c->callee->accept(this);
  auto newcallee = out;
  std::vector<Constraint *> newargs;
  for(auto &p : c->args) { out = p; p->accept(this); newargs.push_back(out); }
  out = graph->call(newcallee, newargs);
}

void InstantiateFree::visit(Free * f) {
  auto it = vars.find(f->v);
  if (it == vars.end()) {
    auto newterm = graph->AddTerm("T" + std::to_string(f->v));
    vars[f->v] = graph->term(newterm->name);
    vars[f->v]->term = newterm;
  }
  out = vars[f->v];
}

void InstantiateFree::visit(Type * t) {
  auto newtype = graph->type(t->name);
  for(auto &p : t->params) {
    out = p; p->accept(this); newtype->params.push_back(out);
  }
  out = newtype;
}


Dependents::Dependents(TypeGraph * graph, TypeTerm * term) : graph(graph), subject(term) {
  for(auto &&pair: graph->GetRelations()){
    currentRoot = pair.first;
    currentTerm = pair.second;
    pair.first->accept(this);
  }
}
void Dependents::visit(Type * t) {
  for(auto &p: t->params) p->accept(this);
}
void Dependents::visit(Term * t) {
  if (t->term == subject)
    out.insert(currentRoot);
}
void Dependents::visit(Free * f) { }
void Dependents::visit(Call * c) {
  c->callee->accept(this);
  for(auto &a: c->args) a->accept(this);
}

AllConstraints::AllConstraints(TypeGraph * graph, TypeTerm * tt) {
  for(auto &pair : graph->GetRelations()) {
    if (pair.second == tt)
      out.insert(pair.first);
  }
}


void TypeGraph::SideboardConstraint(TypeTerm * tt, Constraint * cc) {
  // std::cerr << "side" << std::setw(16) << tt << " :: " << cc << "\n";
}

void TypeGraph::RemoveConstraint(TypeTerm * tt, Constraint * cc) {
  auto it = relations.find(cc);
  if (coral::opt::ShowTypeSolution)
    std::cerr <<COL_LIGHT_RED << "Removing! " << tt << ":" << cc << COL_CLEAR << "\n";
  if (it == relations.end()) {
    std::cerr << "could not remove " << tt << " :: " << cc << "\n";
    exit(1);
  }
  else
    relations.erase(relations.find(cc));
  changes++;
}


bool ApplyTuple(TypeGraph * graph, TypeTerm * t, Call * call, Type * field, Type * tuple) {
  // this is called when we have a Call(Member(Item0), TupleType)
  // here we both have to register the output type of the field
  // but also register the lookup index so the codegenerator
  // knows what the offset is
  int fieldindex = 0;
  auto memberName = dynamic_cast<Type *>(field)->name;

  // Option1: Literal Tuple
  if (tuple->name == "Tuple") {
    for(size_t i = 0; i < tuple->params.size(); i++) {
      auto tf = dynamic_cast<Type *>(tuple->params[i]);
      auto name = tf->name == "Field" ? (dynamic_cast<Type *>(tf->params[0])->name) : "Item" + std::to_string(i);
      auto type = tf->name == "Field" ? (dynamic_cast<Type *>(tf->params[1])) : tf;

      std::cerr
        << COL_RGB(0, 5, 5) << name << ", "
        << tuple->name << ", "
        << field->name << COL_CLEAR << "\n";
      if (name == field->name) {
        graph->RemoveConstraint(t, call);
        graph->AddConstraint(t, type);
        auto member = dynamic_cast<coral::ast::Member *>(t->expr);
        member->memberIndex = i;
        return true;
      }
    }
  }

  // Option2: Named Tuple
  auto field_term_name = tuple->name + "::" + field->name;
  auto field_term = graph->GetTermByName(field_term_name);
  if (field_term) {
    graph->RemoveConstraint(t, call);
    graph->AddConstraint(t, graph->term(field_term));
    auto member = dynamic_cast<coral::ast::Member *>(t->expr);
    auto indexData = graph->GetTermByName(field_term_name + ".index");
    auto type = graph->GetTypeConstraintForTerm(indexData);
    member->memberIndex = std::stoi(type->name);
    return true;
  }

  // for(auto * _field : tuple->params) {
  //   if (auto field = dynamic_cast<Type *>(_field)) {
  //     // if (field->name == "Field") {
  //     //   auto fieldname = dynamic_cast<Type *>(field->params[0])->name;
  //     //   auto fieldtype = dynamic_cast<Type *>(field->params[1]);
  //     //   if (fieldname == memberName) {
  //     //     // std::cerr << COL_LIGHT_RED << fieldname << COL_CLEAR << "\n";
  //     //     graph->AddEquality(graph->term(t), fieldtype);
  //     //     auto indexTerm = graph->AddTerm(t->name + ".index", t->expr);
  //     //     graph->AddEquality(graph->term(indexTerm), graph->type("Index", {graph->type(std::to_string(fieldindex))}));
  //     //     graph->RemoveConstraint(t, call);
  //     //     return true;
  //     //   }
  //     // } else {
  //     //   if (memberName == "Item" + std::to_string(fieldindex)) {
  //     //     graph->AddEquality(graph->term(t), field);
  //     //     auto indexTerm = graph->AddTerm(t->name + ".index", t->expr);
  //     //     graph->AddEquality(graph->term(indexTerm), graph->type("Index", {graph->type(std::to_string(fieldindex))}));
  //     //     graph->RemoveConstraint(t, call);
  //     //     return true;
  //     //   }
  //     // }
  //   }
  //   fieldindex++;
  // }
  return false;
}

void TypeGraph::Apply(TypeTerm * t, Call * call, Type * callfunc) {
  // TODO: this can be implemented as a unify operation on two Funcs
  // get rid of free types in F's type signature

  Type * callee = (Type *)InstantiateFree::of(this, callfunc);
  if (callee == 0) return;

  if (coral::opt::ShowTypeSolution)
    std::cerr << "applying" << COL_RGB(4, 5, 2) << std::setw(22) << t << " :: "
              << std::setw(20) << callfunc << " -> "
              << std::setw(20) << callee << COL_CLEAR << "\n";

  if (callee->name == "Member") {
    if (call->args.size() == 1) {
      if (Type * tuple = dynamic_cast<Type *>(call->args[0])) {
        if (ApplyTuple(this, t, call, dynamic_cast<Type *>(callee->params[0]), tuple)) return;
      }
    }
  }

  if (callee->name == "Func") {
    // std::cerr << "hmmm " << call << "\n";
    for(size_t i = 0; i < call->args.size(); i++) {
      if (Type * t = dynamic_cast<Type *>(callee->params[i]))
        if (t->name == "...")
          break;
      AddEquality(call->args[i], callee->params[i]);
    }
    AddEquality(term(t->name), callee->params.back());
    this->RemoveConstraint(t, call);
    changes++;
  }
}

void StepSingleConstraint(TypeGraph * graph, TypeTerm * term, Constraint * cons) {
  // std::cerr
  //   << std::setw(30) << term << " :: " << cons
  //   << " (" << Dependents::of(graph, term).size() << ")\n";
  // [Apply] Rule
  if (Call * cc = dynamic_cast<Call *>(cons)) {
    if (Type * f = dynamic_cast<Type *>(cc->callee)) {
      graph->Apply(term, cc, f);
      return;
    }
  }

  // [Substitution - Phase 1] Rule
  if (Type * istype = dynamic_cast<Type *>(cons))
    if (SimpleType::of(istype)) {
      auto dependents = Dependents::of(graph, term);
      if (dependents.size()) {
        // std::cerr << "substituting " << COL_LIGHT_YELLOW << std::setw(17) << term
        //           << " :: " << cons << COL_CLEAR << "\n";
        for(auto &dependent: dependents)
          graph->Substitute(dependent, term, cons);
        // TODO: sideboard rule
        graph->SideboardConstraint(term, cons);
        return;
      }
    }

  // Substitution - Term
  if (Term * isterm = dynamic_cast<Term *>(cons)) {
    auto dependents = Dependents::of(graph, term);
    if (dependents.size()) {
      // std::cerr << "substituting " << std::setw(17) << term << " :: " << cons << "\n";
      for(auto &dependent: dependents) {
        TypeTerm * tt = graph->GetRelations()[dependent];
        auto replaced = graph->Substitute(dependent, term, cons);
        // std::cerr << "     subbed " << tt << ":" << dependent << " -> " << replaced << "\n";
      }
      // RemoveConstraint(c.second, c.first);
      return;
    }
  }

  // Sometimes a term appears on the right hand side multiple times.
  // In this case, we can remove all those constraints and replace with
  // unifying all the remaining terms against a representative
  // (they should all be equivalent)
  if (Term * right_term = dynamic_cast<Term *>(cons)) {

    auto shared_terms = AllTerms::of(graph, right_term);
    auto shared_constraints = AllConstraints::of(graph, right_term->term);

    auto skip_unify = false;
    for(auto &c: shared_constraints)
      if (dynamic_cast<Call *>(c))
        skip_unify = true;

    if (!skip_unify) {
      auto combined_constraints = std::set<Constraint *>(shared_constraints);
      for(auto &t: shared_terms)
        combined_constraints.insert(graph->term(t.first));

      if (combined_constraints.size() > 1) {
        if (coral::opt::ShowTypeSolution)
          std::cerr << right_term << ": unifying with right-hand terms\n";

        auto representative = *combined_constraints.begin();
        for(auto &&pair : shared_terms)
          if (pair.first != right_term->term)
            graph->RemoveConstraint(pair.first, pair.second);
        for(auto &&c : shared_constraints)
          graph->RemoveConstraint(right_term->term, c);
        for(auto &c: combined_constraints)
          if(c != representative)
            graph->AddEquality(representative, c);
        graph->AddEquality(representative, right_term);
        return;
      }
    }
  }

  // Unification - Terms and Simple Types
  // if {a :: foo, a::bar, a::baz, a::Func[*T -> *T]}
  auto shared_constraints = AllConstraints::of(graph, term);
  if (shared_constraints.size() > 1) {
    // first, make sure that we're not unifying calls (we have to apply them first)
    auto skip_unify = false;
    for(auto &c: shared_constraints)
      if (dynamic_cast<Call *>(c))
        skip_unify = true;

    if (!skip_unify) {
      // to unify, first, we remove all the old constraints
      // then we merge each constraint against a representative constraint.
      if (coral::opt::ShowTypeSolution)
        std::cerr << "unifying with shared constraints\n";
      auto representative = *shared_constraints.begin();
      for(auto &c: shared_constraints)
        if (c != representative)
          graph->RemoveConstraint(term, c);
      for(auto &c: shared_constraints)
        if (c != representative)
          graph->AddEquality(c, representative);
      return;
    }
  }


  // If we're Calling a term, we might as well substitute its definition into the
  // call.
  if (Type * func = dynamic_cast<Type *>(cons)) {
    // if (func->name == "Func") {
    auto dependents = Dependents::of(graph, term);
    // if (func->name == "Tuple")
    //   std::cerr << COL_RGB(2, 2, 5) << term << " is a tuple! " << dependents.size() << "\n";
    for(auto &dep: dependents) {
      if (Call * call = dynamic_cast<Call *>(dep)) {
        if (coral::opt::ShowTypeSolution)
          std::cerr
            << COL_BLUE << call->callee << " "
            << (call->callee == func) << "   call \n" << COL_CLEAR;
        if (ConstraintEqualsImpl::of(call->callee, graph->term(term))) {
          graph->Substitute(call, term, cons);
        }
      }
    }
    // }
  }
  return;
}

void TypeGraph::Step() {
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
    old_changes = changes;
    // TODO: this should be a work queue instead of scanning the entire set each loop
    for(auto && c : relations) {
      StepSingleConstraint(this, c.second, c.first);
      if (changes != old_changes) {
        //   Show("--");
        //   getchar();
        break;
      }
    }
    // std::cerr << changes - old_changes << "\n";
  }
}

void DuplicateConstraint::visit(Type * c)  { out = graph->type(c->name, c->params); }
void DuplicateConstraint::visit(Term * c) { out = graph->term(c->term); }
void DuplicateConstraint::visit(Free * c) { out = graph->free(c->v); }
void DuplicateConstraint::visit(Call * c) { out = graph->call(c->callee, c->args); }

void TypeGraph::AddConstraint(TypeTerm * term, Constraint * c) {
  if (coral::opt::ShowTypeSolution)
    std::cerr
      << COL_RGB(2, 5, 3) << "adding" << std::setw(24) << term << " :: " << c << COL_CLEAR << "\n";
  changes++;
  if (relations.find(c) != relations.end()) {
    relations[DuplicateConstraint(this, c).out] = term;
  } else {
    relations[c] = term;
  }
}
