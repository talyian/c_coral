#include "core/expr.hh"

#include <iostream>

namespace coral {
  namespace analyzers {
    class TypeResolver {
    public:
      ast::Module * module = 0;
      ast::BaseExpr * target;
      std::string name;

      TypeResolver(ast::Module * m);
      virtual std::string visitorName() { return "TypeResolver"; }
    };
  }
}
