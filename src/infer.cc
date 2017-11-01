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
  map<string, Type *> data;
  vector<string> rules;
  map<string, Scope *> children;

  Scope * nested(string name) {
	auto s = new Scope();
	s->parent = this;
	children[name] = s;
	return children[name];
  }

  void add(string name, Type * t) { data[name] = t; }
  void addRule(string rule) { rules.push_back(rule); }
  void showData(string ctx, int indent) {
    foreach(data, it) cout << "$ " << ctx << it->first << ": " << it->second << endl;
	foreach(children, it) it->second->showData(ctx + it->first + ".", indent + 1);
  }
  void showInfo(string ctx, int indent) {
	foreach(rules, it) cout << "#$ " << ctx << *it << endl;
	foreach(children, it) it->second->showInfo(ctx + it->first + ".", indent + 1);
  }
  void show() { show("", 0); }
  void show(string ctx, int indent) {
	showInfo(ctx, indent);
    // foreach(data, it) cout << string(indent + 2, '*') << ctx << '.' << it->first << ": " << it->second << endl;
	// cout << endl;
	// foreach(rules, it) cout << string(indent * 2 + 1, "$"[0]) << " (" << ctx << ") "<< *it << endl;
	// foreach(children, it) {
	//   it->second->show(it->first, indent + 1);
	// }
  }
};

class ExtraVisitor : public Visitor {
public:
  Visitor * inner = 0;
  ExtraVisitor(string name, Visitor * inner) : Visitor(name), inner(inner) { }
  void visit(Module * m) {
	cout << inner << endl;
	inner->visit(m);
	foreach(m->lines, it) (*it)->accept(this);
  }
};

class ScopeVisitor : public Visitor {
public:
  ScopeVisitor * parent = 0;
  Module * module = 0;
  Scope scope;
  string scopeStack = "";

  ScopeVisitor(Module * m, Scope s) : Visitor("scoper "), module(m), scope(s) {
	m->accept(this);
  }

  ScopeVisitor(ScopeVisitor * parent, string name) : Visitor("scoper "), parent(parent) {

  }

  string scoped(string name) {
	return scopeStack + "." + name;
  }


  void visit(Module * m) {
	foreach(m->lines, it) (*it)->accept(this);
  }

  void visit(Struct * s) {
	scope.addRule(s->name + " = Type(Struct)");
	scope.add(s->name, new BaseType());
	ScopeVisitor v2(this, "test");
	v2.parent = this;
	v2.module = module;
	foreach(s->fields, f) (*f)->accept(&v2);
	scope.children[s->name] = new Scope(v2.scope);
  }

  void visit(DeclTypeEnum * d) {
	scope.add(d->name, new BaseType());
  }

  void visit(DeclTypeAlias * d) {
	scope.addRule(d->name + " = " + d->wrapped->toString());
	scope.add(d->name, d->wrapped);
  }


  void visit(Extern * e) {
	scope.addRule(e->name + " :: " + e->type->toString());
	scope.add(e->name, e->type);
  }

  void visit(Let * e) {
	if (e->var)
	  scope.addRule(e->var->toString() + " :: ");
  }
};

int main() {
  auto module = parse(fopen("tests/libs/syncio.coral", "r"), 0);
  // TreePrinter(module, cout).print();
  Scope s;
  ScopeVisitor scopev(module, s);
  // s.add("a", new IntType(32));
  // s.add("frob", new UnknownType());
  // s.add("printf", new FuncType(new VoidType(), vec(new PtrType(new PtrType(new IntType(8)))), true));
  // cout << "\x1B[2J\x1B[H";
  scopev.scope.show();
}
