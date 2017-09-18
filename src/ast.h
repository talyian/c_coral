#include <vector>
#include <string>
#include <iostream>

class Expr {
 public:
  virtual std::string toString() { return "[expr]"; }
  virtual void accept(class Visitor * v);
};

class Call : public Expr {
 public:
  std::string callee;
  std::vector<Expr *> arguments;
 Call(std::string a, std::vector<Expr *> b) : callee(a), arguments(b) { }
  virtual void accept(class Visitor * v);
  virtual std::string toString() {
    auto v = callee + "(" ;
    for(auto i = arguments.begin(); i != arguments.end(); i++) {
      if (i != arguments.begin())
	v += ", ";
      v += (*i)->toString();
    }
    v += ")";
    return v;
  }
};

class Extern : public Expr {
 public :
  std::string Linkage;
  std::string Name;
 Extern(std::string a, std::string b) : Linkage(a), Name(b) { }
  virtual void accept(class Visitor * v);  
  virtual std::string toString() { return "extern " + Linkage + " " + Name; }
};

class String : public Expr {
 public :
  std::string value;
 String(std::string a) : value(a) { }
  virtual void accept(class Visitor * v);
  virtual std::string toString();
};

class Module : public Expr {
 public:
  std::string name;
  std::vector<Expr *> lines;
  Module(std::vector<Expr *> a) : name("module"), lines(a) { }
  virtual void accept(class Visitor * v);  
  virtual std::string toString() {
    std::string s("");
    for(auto iter = lines.begin(); iter != lines.end(); iter++) {
      s += (*iter)->toString() + "\n";
    }
    return s;
  }
};

class Visitor {
 public:
  virtual void visit(Expr * c) { std::cout << c->toString() << std::endl; }
  virtual void visit(Extern * c) { std::cout << c->toString() << std::endl; }
  virtual void visit(Call * c) { std::cout << c->toString() << std::endl; }
  virtual void visit(Module * c) { std::cout << c->toString() << std::endl; }      
};

