#include "ast.h"
#include <regex>

void Expr::accept(Visitor * v) { v->visit(this); }
void Call::accept(Visitor * v) { v->visit(this); }
void Extern::accept(Visitor * v) { v->visit(this); }
void String::accept(Visitor * v) { v->visit(this); }
void Module::accept(Visitor * v) { v->visit(this); }

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
