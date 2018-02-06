#include "expr.hh"

#include <cstdio>
#include <regex>

namespace coral {
  namespace ast {
	Module::Module() { }

	std::regex string_unescape("\\\\n");

	std::string StringLiteral::getString() {
	  auto s = value.substr(1, value.size() - 2);
	  s = std::regex_replace(s, string_unescape, "\n");
	  return s;
	}
  }
}
