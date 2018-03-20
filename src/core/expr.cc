#include "expr.hh"
#include "core/utils.hh"

#include <cstdio>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

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
      // if (tt.name == "Field") {
      //   os << tt.params[0] << ":" << tt.params[1];
      //   return os;
      // }
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
      std::vector<std::string> path,
      Type * rtype,
      vector<coral::ast::Def *> params,
      Block * body) : body(body) {

      name = path.back();
      path.pop_back();
      container = path;

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

    Call::Call(BaseExpr * callee, TupleLiteral * arguments) : callee(callee) {
        if (arguments)
          for(auto && p : arguments->items)
            this->arguments.push_back(std::unique_ptr<BaseExpr>(p.release()));
	  }
    Call::Call(BaseExpr * callee, vector<BaseExpr *> arguments): callee(callee) {
		for(auto && ptr : arguments)
		  if (ptr) this->arguments.push_back(std::unique_ptr<BaseExpr>(ptr));
	  }
    // flips the AST tree -- from call(member(foo, bar), [a, b])
    // to call(Foo:bar, [foo, a, b])
    void Call::methodCallInvert() {
      auto member = dynamic_cast<Member *>(callee.get());
      if (!member) return;
      if (member->methodPtr) {
        auto instance = member->base.release();
        auto method = member->methodPtr;
        // std::cerr << method-> name << " " << method->isInstanceMethod << "\n";
        if (method->isInstanceMethod) {
          this->arguments.insert(this->arguments.begin(), std::make_unique(instance));
        }
        auto vv = new ast::Var(method->name);
        vv->expr = method;
        this->callee.reset(vv);
      } else  {
        std::cerr << "methodinverting " << member->member << "\n";
        std::cerr << member->memberIndex << "\n";
        auto var = dynamic_cast<Var *>(member->base.get());
        if (var) {
          if (Union *union_def = dynamic_cast<Union *>(var->expr)) {
            std::cerr << "Union: " << member->base.get() << "\n";
          } else {
            std::cerr << "???" << __LINE__ << "\n";
          }
        } else {
          std::cerr << "No base found\n";
        }
      }
    }

    FloatLiteral::FloatLiteral(std::string value) : value(value) {

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
      std::string item = "item";
      for(auto &f: fields)
        this->fields.emplace_back(_typeToDef(f));
    }

    Tuple::Tuple(std::string name, std::vector<Def *> fields) : name(name) {
      for(auto &f: fields) this->fields.emplace_back(f);
    }

    Union::Union(std::string name, ast::Block * block) : name(name) {
      for(auto &line: block->lines)
        if (ast::Def * def = dynamic_cast<ast::Def *>(line.get()))
          this->cases.emplace_back(def);
    }

    Union::Union(std::string name, std::vector<type::Type> cases) : name(name) {
      for(auto &caseitem : _typeArgsToDef(cases))
        this->cases.emplace_back(caseitem);
    }

    Match::Match(BaseExpr * condition, vector<MatchCase *> cases) : condition(condition) {
      for(auto &line: cases)
        this->cases.emplace_back(line);
    }

    Type _defToType(Def * def) {
      if (def->name.empty())
        return *(def->type);
      else
        return type::Type { "Field", { type::Type { def->name }, *(def->type) }};
    }

    Def * _typeToDef(Type t) {
      if (t.name == "Field")
        return new Def(t.params[0].name, new type::Type(t.params[1]), 0);
      else
        return new Def("", new type::Type(t), 0);
    }

    std::vector<Type> _defsToTypeArg(std::vector<Def *> defs) {
      std::vector<Type> out;
      for(auto &def: defs) {
        out.push_back(_defToType(def));
        delete def;
      }
      return out;
    }

    std::vector<Def *> _typeArgsToDef(std::vector<Type> defs) {
      std::vector<Def *> out;
      for(size_t i =0; i < defs.size(); i++) {
        out.push_back(_typeToDef(defs[i]));
        if (out.back()->name == "")
          out.back()->name = "Item" + std::to_string(i);
      }
      return out;
    }
  }
}
