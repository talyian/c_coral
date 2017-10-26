#include "expr.hh"
#include <regex>

using namespace coral;

#define ACCEPT_MACRO(NODE) void NODE::accept(Visitor * v) { v->visit(this); }
EXPR_NODE_LIST(ACCEPT_MACRO)
#undef ACCEPT_MACRO

template <typename T> Expr * mapExpr(Expr * e) { T t(e); return t.out; }

std::string Escape(std::string s) {
  if (s == "\\\\") return "\\";
  if (s == "\\n") return "\n";
  if (s == "\\\"") return "\"";
  return "?";
}

std::string Translate(std::string s) {
  static std::regex re("\\\\.");
  std::string c = s.substr(1, s.size() - 2);
  std::sregex_iterator begin(c.begin(), c.end(), re), bend;
  std::string res;
  std::string suffix;
  int matches = 0;
  std::for_each(begin, bend, [&res, &suffix, &matches] (std::smatch const & match) {
      res += match.prefix();
      res += Escape(match.str());
      suffix = match.suffix();
      matches++;
    });
  if (matches) return res + suffix;
  return c;
}

std::string Module::toString()  {
    std::string s("");
    s += std::to_string(lines.size()) + " lines\n";
    for(auto iter = lines.begin(); iter != lines.end(); iter++)
      if (*iter) {
	s += (*iter)->toString() + "\n";
      } else {
	s += "(null line?) \n";
      }
    return s;
}
std::string String::toString() {
  return Translate(value);
}

std::string Long::toString() {
  return std::to_string(value);
}
std::string Double::toString() {
  return std::to_string(value);
}
std::string Var::toString() {
  return value;
}
std::string If::toString() {
  return "(" + cond->toString() + "?" + ifbody->toString() + ":" + elsebody->toString() + ")";
}
std::string FuncDef::toString() {
  return name + "(" + ")" + "\n" + (body ? body->toString() : " = 0");
}
std::string Return::toString() {
  return "return " + (value ? value->toString() : "(null)");
}
std::string Cast::toString() {
  return expr->toString() + " as " + to_type->toString();
}
std::string Let::toString() {
  return var->name + ":=" + value->toString();
}
std::string AddrOf::toString() {
  return "&" + var;
}
std::string BlockExpr::toString() {
    std::string s("block:\n");
    for(auto iter = lines.begin(); iter != lines.end(); iter++) {
      if (*iter) s += (*iter)->toString() + "\n";
      else s += "(null)\n";
    }
    return s;
}

FuncDef* coral::BuildVarFunc(std::string name, Type* return_type, std::vector<Def *> params, Expr * body) {
  return new FuncDef(name, return_type, params, body, true);
}
FuncDef* coral::BuildFunc(std::string name, Type* return_type, std::vector<Def *> params, Expr * body) {
  return new FuncDef(name, return_type, params, body, false);
}

std::string getName(Expr * e) {
  NameGetter ng;
  e->accept(&ng);
  return ng.out;
}
