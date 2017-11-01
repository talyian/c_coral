#include "core/type.hh"
#include "core/expr.hh"
#include "core/treeprinter.hh"
#include "parsing/lexer.hh"

#include <map>
#include <string>

using namespace std;
using namespace coral;

class Scope {
public:
  Scope * parent = 0;
  map<string, Type *> types;
  map<string, Expr *> expr;
  map<string, Scope *> children;
  vector<string> rules;
  string name;

  Scope * nested(string name) {
	auto s = new Scope();
	s->parent = this;
	s->name = name;
	children[name] = s;
	return children[name];
  }
  string get(string name) {
	if (types.find(name) != types.end())
	  return types[name]->toString();
	if (expr.find(name) != expr.end())
	  return ExprNameVisitor(expr[name]).out;
	return "";
  }
  string get(string ns, string name) {
	if (children.find(name) != children.end())
	  return children[name]->get(name);
	return "";
  }

  void add(string name, Expr * t) {
	if (expr.find(name) != expr.end()) {
	  // cerr << "Warning: overwriting name :" << name << endl;
	  for(auto it = expr.begin(); it != expr.end();) {
		auto sname = it->first;
		if (!sname.compare(0, name.length() + 2, name + "::"))
		  it = expr.erase(it);
		else
		  it++;
	  }
	}
	expr[name] = t;
  }
  void publish(string name, Expr * t) {
	add(name, t);
	if (parent) parent->publish(this->name + "::" + name, t);
  }

  void add(string name, Type * t) { types[name] = t; }
  void addRule(string rule) { rules.push_back(rule); }
  void showTypes(string ctx, int indent) {
    foreach(types, it) cout << "$ " << ctx << it->first << ": " << it->second << endl;
	foreach(children, it) it->second->showTypes(ctx + it->first + ".", indent + 1);
  }
  void showExpr(string ctx, int indent) {
    foreach(expr, it) cout << "#$$$ " << it->first << ": " << ExprNameVisitor(it->second).out << endl;
	// foreach(children, it) it->second->showExpr(ctx + it->first + ".", indent + 1);
  }
  void showInfo(string ctx, int indent) {
	foreach(rules, it) cout << "#$ " << ctx << *it << endl;
	foreach(children, it) it->second->showInfo(ctx + it->first + ".", indent + 1);
  }
  void show() { show("", 0); }
  void show(string ctx, int indent) {
	showExpr(ctx, indent);
    // foreach(types, it) cout << string(indent + 2, '*') << ctx << '.' << it->first << ": " << it->second << endl;
	// cout << endl;
	// foreach(rules, it) cout << string(indent * 2 + 1, "$"[0]) << " (" << ctx << ") "<< *it << endl;
	// foreach(children, it) {
	//   it->second->show(it->first, indent + 1);
	// }
  }
};

class ScopeVisitor : public Visitor {
public:
  Expr * expr;
  Scope * scope;
  ScopeVisitor(Expr * e, Scope * s) : Visitor("scope "), expr(e), scope(s) { if (e) e->accept(this); }
  ScopeVisitor * nested(string name) {
	return new ScopeVisitor(0, scope->nested(name));
  }
  void visit(Module * m) { foreach(m->lines, it) (*it)->accept(this); }
  void visit(Struct * e) {
	scope->publish(e->name, e);
	ScopeVisitor * n = nested(e->name);
	foreach(e->fields, it) (*it)->accept(n);
	foreach(e->methods, it) (*it)->accept(n);
  }
  void visit(Let * e) {
	scope->publish(e->var->toString(), e);
  }
};

void massert(string name, bool cond, string msg) {
  printf("%-20s: ", name.c_str());
  if (cond)
	cout << "\e[1;32m" << "OK" << "\e[0m ";
  else
	cout << "\e[1;31m" << "ERROR" << "\e[0m ";
  cout << msg << "\n";
}
void massert(bool cond) { return massert("(test)", cond, ""); }


void test0() {
  auto module = parse( 0,
					   "type container:\n"
					   "  let A = 1\n"
					   "  let B = 2\n");
  ScopeVisitor scopevisitor(module, new Scope());
  massert(
	"0: container",
	"" != scopevisitor.scope->get("container"),
	scopevisitor.scope->get("container"));
  massert(
	"0: container::A",
	"" != scopevisitor.scope->get("container::A"),
	scopevisitor.scope->get("container::A"));
}

void test1() {
  auto module = parse(
	0,
	"let A = 0\n"
	"type container:\n"
	"  let containerA = 1\n"
	"  let B = 2\n"
    "let B = 3\n"
	"let containerA = 3\n"
	"let container = \"c\"\n");

  ScopeVisitor scopevisitor(module, new Scope());
  massert(
	"1: container",
	"" != scopevisitor.scope->get("container"),
	scopevisitor.scope->get("container"));
  massert(
	"1: container::A",
	"" == scopevisitor.scope->get("container::A"),
	"container::A is overwritten");
  massert(
	"1: containerA",
	"" != scopevisitor.scope->get("containerA"),
	"containerA not is overwritten");
}

int main() {
  // TreePrinter(module, cout).print();
  // ScopeVisitor scopevisitor(module, new Scope());
  // scopevisitor.scope->show();
  test0();
  test1();
}
