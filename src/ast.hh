#pragma once

#include <vector>
#include <string>
#include <iostream>
#include "type.hh"

class Expr {
 public:
  virtual std::string toString() { return "[expr]"; }
  virtual void accept(class Visitor * v);
  virtual ~Expr() { }
};

class Call : public Expr {
 public:
  std::string callee;
  std::vector<Expr *> arguments;
 Call(std::string a, std::vector<Expr *> b) : callee(a), arguments(b) { }
  virtual void accept(class Visitor * v);
  virtual std::string toString() {
    auto v = callee + "(";
    for(auto i = arguments.begin(); i != arguments.end(); i++) {
      if (*i) v += (i != arguments.begin() ? ", " : "") + (*i)->toString();
      else v = v + (i != arguments.begin() ? ", " : "")  +  "(null)";
    }
    v += ")";
    return v;
  }
    ~Call() { for(auto i = arguments.begin(); i < arguments.end(); i++) { delete *i; }  }
};

class BinOp : public Expr {
 public:
  std::string op;
  Expr * lhs, *rhs;
  BinOp(std::string oper, Expr * a, Expr * b) : op(oper), lhs(a), rhs(b) { }
  virtual void accept(class Visitor * v);
  virtual std::string toString() {
    if (!lhs || !rhs)
      return (
	      "(" + (lhs ? lhs->toString() : "??") +
	      "|" + (rhs ? rhs->toString() : "??") +
	      "|" + op + ")"
	      );
    return "(" + lhs->toString() + op + rhs->toString() + ")";
  }
    ~BinOp() { delete lhs; delete rhs; }
};

class Extern : public Expr {
 public :
  std::string linkage;
  std::string name;
  Type *type;
  Extern(std::string a, std::string b, Type * type) :
    linkage(a), name(b), type(type) { }
  virtual void accept(class Visitor * v);
  virtual std::string toString() { return "extern " + linkage + " " + name; }
  ~Extern() { delete type; }
};

class String : public Expr {
 public :
  std::string value;
 String(std::string a) : value(a) { }
  virtual void accept(class Visitor * v);
  virtual std::string toString();
};

class Var : public Expr {
 public :
  std::string value;
  Var(std::string a) : value(a) { }
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
  virtual std::string toString();
  ~Module() {
  for(auto i = lines.begin(), e = lines.end(); i != e; i++) delete *i; }
};

class BlockExpr : public Expr {
 public:
  std::vector<Expr *>lines;
 BlockExpr(std::vector<Expr *>lines) : lines(lines) { }
  virtual void accept(class Visitor * v);
  virtual std::string toString() {
    std::string s("");
    for(auto iter = lines.begin(); iter != lines.end(); iter++) {
      if (*iter) s += (*iter)->toString() + "\n";
      else s += "(null)\n";
    }
    return s;
  }
  ~BlockExpr() { for(auto i = lines.begin(), e = lines.end(); i != e; i++) delete *i; }
};

class Def {
 public:
  std::string name;
  Type * type;
  Def(std::string name, Type * t) : name(name), type(t) { }
  ~Def() { delete type; }
};

class FuncDef : public Expr {
 public:
  std::string name;
  Type * rettype;
  std::vector<Def *> args;
  Expr * body;
  bool multi;
  FuncDef(std::string name, Type * rettype,
	  std::vector<Def *> args,
	  Expr * body,
	  bool multi) :
	      name(name), rettype(rettype), args(args), body(body), multi(multi) { }
  virtual void accept(class Visitor * v);
  virtual std::string toString();
  ~FuncDef() {
      delete body;
      delete rettype;
      for(auto i = args.begin(), e = args.end(); i != e; i++) delete *i;
  }
};

class Cast : public Expr {
public:
  Expr * expr;
  Type * to_type;
  Cast(Expr * expr, Type * to_type) : expr(expr), to_type(to_type) { }
  virtual void accept(class Visitor * v);
  virtual std::string toString();
  ~Cast() { delete expr; delete to_type; }
};

class If : public Expr {
public:
  Expr * cond, *ifbody, *elsebody;
  If(Expr * cond, Expr * ifbody, Expr * elsebody) : cond(cond), ifbody(ifbody), elsebody(elsebody) { }
  virtual void accept(class Visitor * v);
  virtual std::string toString();
  ~If() { delete cond; delete ifbody; delete elsebody; }
};

class Let : public Expr {
public:
  Def * var;
  Expr * value;
  Let(Def * var, Expr * value) : var(var), value(value) { }
  virtual void accept(class Visitor * v);
  virtual std::string toString();
  ~Let() { delete var; delete value; }
};

class AddrOf : public Expr {
public:
  std::string var;
  AddrOf(std::string var) : var(var) { }
  virtual void accept(class Visitor * v);
  virtual std::string toString();
};

class Return : public Expr {
public:
  Expr * value;
  Return(Expr * value) : value(value) { }
  virtual void accept(class Visitor * v);
  virtual std::string toString();
  ~Return() { delete value; }
};

class Visitor {
 public:
  virtual void visit(Expr * c) { std::cerr << c->toString() << std::endl; }

  virtual void visit(Module * c) { std::cerr << c->toString() << std::endl; }
  virtual void visit(Extern * c) { std::cerr << c->toString() << std::endl; }
  virtual void visit(BlockExpr * c) { std::cerr << c->toString() << std::endl; }
  virtual void visit(FuncDef * c) { std::cerr << c->toString() << std::endl; }

  virtual void visit(Call * c) { std::cerr << c->toString() << std::endl; }
  virtual void visit(BinOp * c) { std::cerr << c->toString() << std::endl; }
  virtual void visit(Var * c) { std::cerr << c->toString() << std::endl; }
  virtual void visit(String * c) { std::cerr << c->toString() << std::endl; }
  virtual void visit(Long * c) { std::cerr << c->toString() << std::endl; }
  virtual void visit(Double * c) { std::cerr << c->toString() << std::endl; }

  virtual void visit(If * c) { std::cerr << c->toString() << std::endl; }
  virtual void visit(Return * c) { std::cerr << c->toString() << std::endl; }
  virtual void visit(Cast * c) { std::cerr << c->toString() << std::endl; }
  virtual void visit(Let * c) { std::cerr << c->toString() << std::endl; }
  virtual void visit(AddrOf * c) { std::cerr << c->toString() << std::endl; }
  virtual ~Visitor () { }
};

FuncDef* BuildFunc(std::string name, Type* functype, std::vector<Def *> params, Expr * body);
FuncDef* BuildVarFunc(std::string name, Type* functype, std::vector<Def *> params, Expr * body);
