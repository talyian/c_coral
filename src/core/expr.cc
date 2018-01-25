#include "expr.hh"
#include <regex>

using namespace coral;

#define ACCEPT_MACRO(NODE) void NODE::accept(AbstractVisitor * v) { v->visit(this); }
EXPR_NODE_LIST(ACCEPT_MACRO)
#undef ACCEPT_MACRO

template <typename T> Expr * mapExpr(Expr * e) { T t(e); return t.out; }

Struct::Struct(
	  std::string name,
	  std::vector<std::string> classParams,
	  std::vector<Expr *> lines) : name(name), classParams(classParams) {
  foreach(lines, line) {
	if (EXPRTYPE(*line) == FuncDefKind)
	  methods.push_back(*line);
	else
	  fields.push_back(*line);
  }
}

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

String::String(std::string a) {
  value = a;
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
  return name;
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
  return expr->toString() + " as " + coral::typeToString(to_type);
}
std::string Let::toString() {
  return var->toString() + ":=" + value->toString();
}

std::string Set::toString() {
  return var->toString() + "<:=" + value->toString();
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

int BinOp::getPrecedence() {
  if (op == "=" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=")
    return 10;

  if (op == "+" || op == "-") return 20;

  if (op == "*" || op == "*" || op == "%") return 30;

  return 0;
}

int BinOp::showParens(BinOp * outer) {
  if (!outer) return false;
  // if the outer precedence is higher, we *must* show parens
  if (outer->getPrecedence() > getPrecedence()) return true;
  if (outer->getPrecedence() < getPrecedence()) return false;
  // if the precedences are equal, then we *must* show parens
  // to force right-hand-first evaluation
  if (outer->rhs == this) return true;
  // return false;
  return false;
}

FuncDef* coral::BuildVarFunc(std::string name, Type* return_type, std::vector<BaseDef *> params, Expr * body) {
  return new FuncDef(name, return_type, params, body, true);
}
FuncDef* coral::BuildFunc(std::string name, Type* return_type, std::vector<BaseDef *> params, Expr * body) {
  return new FuncDef(name, return_type, params, body, false);
}

std::string coral::getName(Expr * e) {
  NameGetter ng;
  e->accept(&ng);
  return ng.out;
}

ExprType coral::Expr::getType() { return ExprTypeVisitor(this).out; }
