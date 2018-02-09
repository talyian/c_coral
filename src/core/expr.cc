#include "expr.hh"

#include <cstdio>
#include <regex>

namespace coral {

  namespace type {
	Type Type::returnType() {
	  if (name == "Func") return params.back();
	  return Type("");
	}

	std::ostream & operator << (std::ostream &os, Type & tt)	{
	  os << tt.name;
	  if (tt.params.size()) {
		os << "[";
        int i = 0;
		for(auto && p : tt.params) {
          if (i++) os << ", ";
          os << p;
        }
		os << "]";
	  }
	  return os;
	}
  }

  namespace ast {
	Module::Module() { }

	std::regex string_unescape("\\\\n");

	std::string StringLiteral::getString() {
	  auto s = value.substr(1, value.size() - 2);
	  s = std::regex_replace(s, string_unescape, "\n");
	  return s;
	}

	BaseExpr * Block::LastLine() {
	  for(auto && iter = lines.rbegin(); iter != lines.rend(); iter++) {
		if (*iter == 0) continue;
		if (ast::ExprTypeVisitor::of(iter->get()) == ast::ExprTypeKind::CommentKind) continue;
		return iter->get();
	  }
	  return 0;
	}
  }
}
