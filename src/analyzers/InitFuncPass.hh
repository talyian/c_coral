#include "core/expr.hh"
#include <string>

namespace coral {
  namespace analyzers {

    // Allows Let and Call at a module top level.
    class InitFuncPass : ast::ExprVisitor {
    private:
      bool remove = false;
      ast::Func * func = 0;
      ast::Module * module = 0;
    public:
      std::string visitorName() { return "InitFunc"; }
      InitFuncPass(ast::Module * m);
      ast::Func * getInitFunc();
      // Noop for regular module-level statements
      void visit(ast::Func *) { }
      void visit(ast::Import *) { }
      void visit(ast::Comment *) { }
      void visit(ast::Extern *) { }
      void visit(ast::Tuple *) { }
      void visit(ast::Union *) { }

      void visit(ast::Call * call);
      void visit(ast::Let * let);
      void visit(ast::Match *);
      void visit(ast::IfExpr *);
      void visit(ast::While *);
    };
  }
}
