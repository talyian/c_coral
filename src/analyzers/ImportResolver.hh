#include "core/expr.hh"
#include <string>
#include <vector>

namespace coral {
  namespace analyzers {
    class ImportResolver : public ast::ActiveExprVisitor {
    public:
      std::vector<ast::BaseExpr *> imported_lines;
      coral::ast::Module * module;
      std::string visitorName() { return "ImportResolver"; }
      ImportResolver(ast::Module * m);
      void visit(ast::Import *);
    };
  }
}
