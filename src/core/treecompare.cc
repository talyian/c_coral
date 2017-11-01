#include "expr.hh"
#include "treeprinter.hh"
#include "../parsing/lexer.hh"

#include <vector>
#include <string>
#include <iostream>

using namespace coral;
using namespace std;

class TreeCompare : public coral::Visitor {
public:
  Expr * lhs = 0;
  Expr * rhs = 0;
  Expr * mismatch = 0;
  Expr * mismatch_b = 0;
  TreeCompare(Expr * left, Expr * right) : Visitor("compare ") {
	lhs = left;
	rhs = right;
	left->accept(this);
  }
  void finish() {
	if (mismatch == 0) { cout << "Match!\n"; return; }
	cout << "----------------------------------------\n";
	TreePrinter(mismatch, cout).print();
	cout << "----------------------------------------\n";
	TreePrinter(mismatch_b, cout).print();
  }
  void visit(Module * a) {
	if (mismatch) return;
	if (rhs == 0 || rhs->getType() != ModuleKind) {
	  cerr << rhs << ": Module type mismatch\nb";
	  mismatch = a; mismatch_b = rhs; return; }
	if (rhs == 0 || rhs->getType() != ModuleKind) {
	  cerr << rhs << ": Module type mismatch\nb";
	  mismatch = a; mismatch_b = rhs; return; }
	Module * b = (Module *)rhs;
	if (a->lines.size() != b->lines.size()) {
	  cerr << "lines mismatch\n";
	  mismatch = a; mismatch_b = rhs; return; }
	for(int i=0; i<a->lines.size(); i++) {
	  TreeCompare cmp(a->lines[i], b->lines[i]);
	  if (cmp.mismatch) { mismatch =cmp.mismatch; mismatch_b = cmp.mismatch_b; return; }
	}
  }
  void visit(FuncDef * f) {
	if (rhs == 0 || rhs->getType() != FuncDefKind) {
	  cerr << rhs << ": Function type mismatch\nb";
	  mismatch = f; mismatch_b = rhs; return; }
	FuncDef * b = (FuncDef *)rhs;
	cout << f->name << " versus " << b->name << endl;
  }
};

int main() {
  Module * a = parse(0,
				   "func twice(f, x):\n"
				   "  f(f x)\n");
  Module * b = new Module(
	vector<Expr *> {
	  new FuncDef(
		"twice",
		new UnknownType(),
		vector<BaseDef *> {

		},
		new Call(new Var("f"), vector<Expr *> {
			new Call(new Var("f"), vector<Expr *> {
				new Var("x")
				  })
			  }),
		false)
		});
  TreeCompare t(a, b);
  t.finish();
}
