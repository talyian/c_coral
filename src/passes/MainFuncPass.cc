// Main Function creation pass --------------------
// If a module does not have a main function defined,
// we add all the little bits of code in the module root
// into a main function

#include "core/expr.hh"
#include "core/treeprinter.hh"
#include "parsing/lexer.hh"
#include <iostream>
#include <map>
#include <algorithm>

using namespace coral;
using std::map;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

class MainFuncPass : public Visitor {
  map<string, Expr *> decls;
  std::vector<Expr *> mainLines;
public:
  Module * out;
  MainFuncPass(Module * m) : Visitor("mainfunc "), out(m) {
    foreach(m->lines, it) (*it)->accept(this);
	finalize();
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
      for(auto it = out->lines.begin(); it != out->lines.end();) {
		if (std::find(mainLines.begin(), mainLines.end(), *it) != mainLines.end()) {
		  lines.push_back(*it);
		  out->lines.erase(it);
		}
		else it++;
      }
	  if (lines.empty()) lines.push_back(new Return(new Long(0)));
      FuncDef * func = new FuncDef(
		"main",
		new IntType(32),
		std::vector<BaseDef *>(),
		new BlockExpr(lines),
		false);
      out->lines.push_back(func);
    }
  }
};

Module * doMainFuncPass(Module * m) { return MainFuncPass(m).out; }
