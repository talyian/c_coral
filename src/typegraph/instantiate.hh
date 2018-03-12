#include "constraints.hh"
#include "typegraph.hh"

namespace typegraph {
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
}
