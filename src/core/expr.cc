#include "expr.hh"

#include <cstdio>
#include <iostream>
#include <regex>

namespace coral {

  namespace type {
	Type Type::returnType() {
	  if (name == "Func") return params.back();
	  return Type("");
	}

	std::ostream & operator << (std::ostream &os, Type * tt)	{
      if (tt == 0) os << "(nulltype)";
      else os << *tt;
      return os;
    }

	std::ostream & operator << (std::ostream &os, Type & tt)	{
      if (tt.name == "Field") {
        os << tt.params[0] << ":" << tt.params[1];
        return os;
      }
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

    Func::Func(
      std::string name,
      Type * rtype,
      vector<coral::ast::Def *> params,
      Block * body) : name(name), body(body) {

      this->type = std::unique_ptr<coral::type::Type>(new Type("Func"));
      for(auto && p : params) {
        if (p->type)
          this->type->params.push_back(*(p->type));
        else
          this->type->params.push_back(Type(""));
        this->params.push_back(std::unique_ptr<Def>(p));
      }
      this->type->params.push_back(*rtype);
      // our contract is we own all constructor pointers;
      delete rtype;
    }

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

    Tuple::Tuple(std::string name, std::vector<type::Type> fields) : name(name) {
      int i = 0;
      std::string item = "item";
      for(auto &f: fields)
        this->fields.emplace_back(new Def("", new type::Type(f), 0));
    }

    Tuple::Tuple(std::string name, std::vector<Def *> fields) : name(name) {
      for(auto &f: fields) this->fields.emplace_back(f);
    }

    std::vector<Type> _defsToTypeArg(std::vector<Def *> defs) {
      std::vector<Type> out;
      for(auto &def: defs) {
        out.push_back(type::Type { "Field", { type::Type { def->name }, *(def->type) } });
      }
      return out;
    }
  }
}
