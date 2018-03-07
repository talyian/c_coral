#include "core/expr.hh"
#include <string>
#include <vector>

namespace coral {
  namespace analyzers {
    class ImportResolver : public ast::ExprVisitor {
    public:
      std::vector<ast::BaseExpr *> imported_lines;
      coral::ast::Module * module;
      std::string visitorName() { return "ImportResolver"; }
      ImportResolver(ast::Module * m);
      void visit(ast::Module *);
      void visit(ast::Block *);
      void visit(ast::Import *);            
    };
  }
}
