#include "TypeGraph.hh"

// void TypeUnify::equal(Term * tt, Free * ff) {
//   graph->AddConstraint(tt->name, ff);
// }

// void TypeUnify::equal(Type * a, Free * b) {

// }
void TypeUnify::equal(Type * a, Term * b) {
  graph->AddConstraint(b->name, a);
}

void TypeUnify::equal(Term * a, Term * b) {
  graph->AddConstraint(a->name, b);
}

void TypeUnify::equal(Type * a, Type * b) {
  if (a->name != b->name) return;
  if (a->params.size() != b->params.size()) return;
  // TODO
  // for(size_t i = 0; i < a->params.size(); i++)
  //   if (a->params[i]
}




void TypeGraph::Show(std::string header) {
  std::cout << "------------------------------" << header << '\n';
  for(auto &&pair: relations) {
    std::cerr << COL_RGB(5, 2, 3) << std::setw(20) << pair.second
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
  auto it = vars.find(f);
  if (it == vars.end()) {
    auto newterm = graph->AddTerm("T" + std::to_string(f->v));
    vars[f] = graph->term(newterm->name);
    vars[f]->term = newterm;
  }
  out = vars[f];
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
