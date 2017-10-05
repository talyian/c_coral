#include "type.hh"
#include <iostream>
#include <iomanip>
#include <string>
#include <map>

using std::map;
using std::string;

class Scope {
public:
  Scope * parent = 0;
  Type * type = 0;
  map<string, Scope> names;

  Scope * nested() {
    auto n = new Scope();
    n->parent = this;
    return n;
  }
  
  Type * get(string name) {
    if (names.find(name) != names.end()) return names[name].get(name);
    if (type) return type;
    if (parent) return parent->get(name);
    return 0;
  }

  void add(string name, Type * t) {
    // cout << "[scope: adding] " << std::setw(15) << name << " " << t << endl;
    names[name].type = t;
  }

  bool set(string name, Type * t) {
    if (names.find(name) != names.end()) { 
      names[name].type = t;
      return true;
    }
    if (parent && parent->set(name, t)) return true;
    return false;
  }
  
  void show() { show(0); }
  void show(int n) {
    foreach(names, it) {
      if (it->second.type) {
	cout << string(n * 2, ' ');
	cout << std::setw(20) << it->first << " : " << it->second.type << endl;
      }
    }
  }
  void showAll() { showAll(0); }
  void showAll(int n) {
    show(n);
    if (parent) parent->show(n);
  }
};

void testScope() {
  cout << "-[Main Scope]--------------------------\n";
  Scope ss;
  ss.add("foo", new VoidType());
  ss.add("x", new IntType(32));
  vector<Type *> args;
  args.push_back(new PtrType(new IntType(8)));
  ss.add("printf", new FuncType(new VoidType(), args, true));
  ss.show();
  
  cout << "-[Nested Scope with new variable]----------------------\n";
  Scope ns = *(ss.nested());
  ns.add("y", new IntType(7));
  ns.showAll();

  cout << "-[Nested Scope shadows base variable]----------------------\n";
  Scope ns2 = *(ss.nested());
  ns2.add("y", new IntType(7));
  ns2.add("x", new IntType(6));  
  ns2.showAll();
  cout << "ns2.get(x) = " << ns2.get("x") << endl;

  cout << "-[Nested Scope and mutate parent variable]----------------\n";
  Scope ns3 = *(ss.nested());
  ns3.add("y", new IntType(7));
  ns3.set("x", new IntType(6));  
  ns3.showAll();
  cout << "ns2.get(x) = " << ns3.get("x") << endl;
}

