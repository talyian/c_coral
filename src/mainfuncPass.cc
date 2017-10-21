// Main Function extraction pass
#include "ast.hh"
#include <iostream>
#include <map>
#include <algorithm>

using std::map;
using std::string;

class MainFuncBuilder : public Visitor {
  map<string, Expr *> decls;
  Module * module;
  std::vector<Expr *> mainLines;
public:
  MainFuncBuilder(Module * m) : Visitor("mainfunc "), module(m) {
    foreach(m->lines, it) (*it)->accept(this);
  }
  void visit(Extern * e) { decls[e->name] = e; }
  void visit(If * e) { mainLines.push_back(e); }
  void visit(Call * e) { mainLines.push_back(e); }
  void visit(Let * a) { mainLines.push_back(a); } //decls[a->var->name] = a; }
  void visit(DeclTypeAlias * a) { decls[a->name] = a; }
  void visit(DeclTypeEnum * a) { decls[a->name] = a; }
  void visit(FuncDef * a) { decls[a->name] = a; }

  void finalize() {
    auto main = decls["main"];
    if (!main) {
      std::vector<Expr *> lines;
      for(auto it = module->lines.begin(); it != module->lines.end();) {
	if (std::find(mainLines.begin(), mainLines.end(), *it) != mainLines.end()) {
	  lines.push_back(*it);
	  module->lines.erase(it);
	}
	else it++;
      }
      FuncDef * func = new FuncDef(
	"main",
	new IntType(32),
	std::vector<Def *>(),
	new BlockExpr(lines),
	false);
      module->lines.push_back(func);
    }
  }
};

Module * buildMainFunction(Module * m) {
  MainFuncBuilder bb(m);
  bb.finalize();
  return m;
}
