// This is the [Apply] Reduction step of the Type Inference algorithm

#include "analyzers/inference/ObjectModel.hh"
#include "analyzers/inference/Environment.hh"

#include <map>

using namespace coral::typeinference;

namespace coral {
  namespace typeinference {
bool freecmp (FreeType * a, FreeType * b) {
  return (
    a == b ? false :
    a == 0 ? true :
    b == 0 ? false :
    a->id < b->id);
};

// Converts FreeTypes to Terms at the point of application
class Instantiate : public AbstractTypeConstraintVisitor {
public:
  TypeEnvironment * env;
  TypeConstraint * out = 0;
  std::map<FreeType *, Term *, decltype(&freecmp)> terms { &freecmp };

  Instantiate(TypeEnvironment * env, TypeConstraint * expr) : env(env) { expr->accept(this); }

  void visit(TypeConstraint * t) {
    out = t;
  }

  void visit(Type * t) {
    for(auto &&p: t->params) { p->accept(this); p = out; }
    out = t;
  }

  void visit(Call * t) {
    t->callee->accept(this); t->callee = out;
    out = t;
  }

  void visit(FreeType * t) {
    if (terms.find(t) == terms.end())
      terms[t] = env->newTerm(env->AddTerm("T" + std::to_string(t->id), 0));
    out = terms[t];
  }

  void visit(Term * t) {
    out = t;
  }
};


std::pair<TypeTerm *, Call *> Apply(TypeEnvironment * env, TypeTerm * tt, Call * call) {
  auto callee = Instantiate { env,  call->callee }.out;
  auto func = dynamic_cast<Type *>(callee);
  if (! func || func->params.size() != call->args.size() + 1) {
    if (func) std::cerr << "Error: Argument Type Mismatch " << callee << ", " << call->args << "\n";
    return std::make_pair((TypeTerm *)0, (Call *)0);
  }

  for(size_t i=0; i<call->args.size(); i++)
    env->AddEquality(call->args[i], func->params[i]);
  env->AddConstraint(tt, func->params[call->args.size()]);
  return std::make_pair(tt, call);
}
  }
}
