#include "zero_ast.hh"
#include "lib/nlohmann_json.hh"

#include <iostream>

namespace coral_zero {
  class JsonParser {
  public:
    static void parse_file(std::istream &in) {
      nlohmann::json json_doc;
      in >> json_doc;
      std::cout << json_doc;
    }
  };
}
