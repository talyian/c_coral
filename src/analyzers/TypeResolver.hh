#include "core/expr.hh"

#include <iostream>

namespace coral {
  namespace analyzers {
    class TypeInfo {
    public:
	  ast::BaseExpr * expr;
      coral::Type type = coral::Type("");
    };

    class TypeResolver : public ast::ExprVisitor {
    public:
      ast::Module * module = 0;
      ast::BaseExpr * target;
      std::string name;
      std::map<ast::BaseExpr *, TypeInfo> info;

      TypeResolver(ast::Module * m);
      virtual std::string visitorName() { return "TypeResolver"; }
    };
  }
}
