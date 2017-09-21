#include "ast.hh"
#include <regex>

void Expr::accept(Visitor * v) { v->visit(this); }
void BinOp::accept(Visitor * v) { v->visit(this); }
void Call::accept(Visitor * v) { v->visit(this); }
void Extern::accept(Visitor * v) { v->visit(this); }
void String::accept(Visitor * v) { v->visit(this); }
void Long::accept(Visitor * v) { v->visit(this); }
void Double::accept(Visitor * v) { v->visit(this); }
void Module::accept(Visitor * v) { v->visit(this); }
void FuncDef::accept(Visitor * v) { v->visit(this); }
void BlockExpr::accept(Visitor * v) { v->visit(this); }
void Var::accept(Visitor * v) { v->visit(this); }
void If::accept(Visitor * v) { v->visit(this); }
void Return::accept(Visitor * v) { v->visit(this); }

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
  return name + "(" + ")" + "\n" + body->toString();
}
std::string Return::toString() {
  return "return " + value->toString();
}
