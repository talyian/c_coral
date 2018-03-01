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
  graph->AddConstraint(a->term, b);
}

void TypeUnify::equal(Type * a, Type * b) {
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
  // std::cerr << t->term << " !! term \n";
  if (t->term == search) {
    // std::cerr << COL_YELLOW << std::setw(30)
    //           << subject << ".." << t << " " << COL_CLEAR
    //           << "\tReplacing " << search << " with " << replace << "\n";
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


void TypeGraph::Apply(TypeTerm * t, Call * call, Type * callfunc) {
  // TODO: this can be implemented as a unify operation on two Funcs
  // get rid of free types in F's type signature

  Type * callee = (Type *)InstantiateFree::of(this, callfunc);
  if (coral::opt::ShowTypeSolution)
    std::cerr << "applying" << COL_RGB(4, 5, 2) << std::setw(22) << t << " :: "
              << std::setw(20) << callfunc << " -> "
              << std::setw(20) << callee << COL_CLEAR << "\n";
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
      auto representative = *shared_constraints.begin();
      for(auto &c: shared_constraints) graph->RemoveConstraint(0, c);
      for(auto &c: shared_constraints)
        if (c != representative)
          graph->AddEquality(c, representative);
      return;
    }
  }

  // Sometimes a term appears on the right hand side multiple times.
  // In this case, we can remove all those constraints and replace with
  // unifying all the remaining terms against a representative
  // (they should all be equivalent)
  if (Term * right_term = dynamic_cast<Term *>(cons)) {
    auto shared_terms = AllTerms::of(graph, right_term);
    if (shared_terms.size() > 1) {
      auto representative = *shared_terms.begin();
      for(auto &&pair: shared_terms) graph->RemoveConstraint(pair.first, pair.second);
      for(auto &&pair: shared_terms)
        if (pair.first != representative.first)
          graph->AddEquality(graph->term(pair.first), graph->term(representative.first));
      return;
    }
  }

  // If we're Calling a term, we might as well substitute its definition into the
  // call.

  if (Type * func = dynamic_cast<Type *>(cons)) {
    if (func->name == "Func") {
      auto dependents = Dependents::of(graph, term);
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
    }
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

void DuplicateConstraint::visit(Type * c)  { out = new Type(c->name, c->params); }
void DuplicateConstraint::visit(Term * c) { out = new Term(c->term); }
void DuplicateConstraint::visit(Free * c) { out = new Free(c->v); }
void DuplicateConstraint::visit(Call * c) { out = new Call(c->callee, c->args); }

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
