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
      v += (i != arguments.begin() ? ", " : "") + (*i)->toString();      
    }
    v += ")";
    return v;
  }
};

class BinOp : public Expr {
 public:
  std::string op;
  Expr * lhs, *rhs;
  BinOp(std::string oper, Expr * a, Expr * b) : op(oper), lhs(a), rhs(b) { }
  virtual void accept(class Visitor * v);
  virtual std::string toString() {
    return "a + b";
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

class Double : public Expr {
 public :
  double value;
 Double(double a) : value(a) { }
  virtual void accept(class Visitor * v);
  virtual std::string toString();
};

class Long : public Expr {
 public :
  int64_t value;
 Long(int64_t a) : value(a) { }
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

class FuncDef : public Expr {
 public:
  std::string name;
  std::vector<int> args;
  Expr * body;
 FuncDef(std::string name, std::vector<int> args, Expr * body) :
  name(name),
    args(args),
    body(body) { }
  virtual void accept(class Visitor * v);
};
class Visitor {
 public:
  virtual void visit(Expr * c) { std::cout << c->toString() << std::endl; }

  virtual void visit(Module * c) { std::cout << c->toString() << std::endl; }
  virtual void visit(Extern * c) { std::cout << c->toString() << std::endl; }

  virtual void visit(FuncDef * c) { std::cout << c->toString() << std::endl; }
  virtual void visit(Call * c) { std::cout << c->toString() << std::endl; }
  virtual void visit(BinOp * c) { std::cout << c->toString() << std::endl; }

  virtual void visit(String * c) { std::cout << c->toString() << std::endl; }
  virtual void visit(Long * c) { std::cout << c->toString() << std::endl; }
  virtual void visit(Double * c) { std::cout << c->toString() << std::endl; }        
};

