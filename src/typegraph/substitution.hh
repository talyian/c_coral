#include "constraints.hh"
#include "typegraph.hh"

namespace typegraph {
  extern bool showSteps;

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
      out = cc;
      cc->accept(this);
    }

    void visit(Type * t) {
      for(auto &x: t->params) {
        out = x; x->accept(this); x = out;
      }
      out = t;
    }
    void visit(Free * f) { }
    void visit(Call * c) {
      out = c->callee;
      c->callee->accept(this);
      c->callee = out;

      for(auto &p: c->arguments) {
        out = p;
        p->accept(this);
        p = out;
      }

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
}
