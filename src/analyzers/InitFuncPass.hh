#include "core/expr.hh"

namespace coral {
  namespace analyzers {

    // Allows Let and Call at a module top level.
    class InitFuncPass : ast::ExprVisitor {
    private:
      bool remove = false;
      ast::Func * func = 0;
      ast::Module * module = 0;
    public:
      InitFuncPass(ast::Module * m);
      ast::Func * getInitFunc();
      // Noop for regular module-level statements
      void visit(ast::Func *) { }
      void visit(ast::Import *) { }
      void visit(ast::Comment *) { }
      void visit(ast::Extern *) { }

      void visit(ast::Call * call);
      void visit(ast::Let * let);
    };
  }
}
