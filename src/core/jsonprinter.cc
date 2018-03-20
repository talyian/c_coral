#include "lib/nlohmann_json.hh"
#include "core/expr.hh"

namespace coral {
  class JsonPrinter : public ast::ExprVisitor {
  public:
    JsonPrinter() { }
  };
}
