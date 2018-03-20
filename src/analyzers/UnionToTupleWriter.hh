#include "core/expr.hh"

namespace coral {
  namespace analyzers {
    class UnionToTupleWriter : public ast::ExprVisitor {
    public:
      ast::Module * module;
      UnionToTupleWriter(ast::Module * m) : module(m) {
        m->accept(this);
      }
      void accept(ast::Module * m) { m->body->accept(this); }
      void accept(ast::Block * b) { for(auto &line: b->lines) if (line) line->accept(this); }
      void accept(ast::Union * u) {
        if (u->constructors.empty())
          // for(size_t i = 0; i<u->cases.size(); i++) {
          //   u->constructors.emplace_back(
          //     new ast::Func(
          //       u->name + "::construct_" + u->cases[i]->name,
          //       type::Type(u->name),
          //       u->cases[i]->type,
          //       new ast::Block({

          //       })));
          // }
      }
    };
  }
}
