// Main Function extraction pass
#include "ast.hh"
#include <iostream>
#include <map>
#include <algorithm>

using std::map;
using std::string;

#define RETURN(expr) InstructionHandler(expr).out
class InstructionHandler : public Visitor {
public:
  Expr *out;
  InstructionHandler(Expr * e) : out(e) { visitorName = "retinst "; e->accept(this); }

  void visit(If * a) {
    a->ifbody = RETURN(a->ifbody);
    a->elsebody = RETURN(a->elsebody);
  }

  void visit(BlockExpr * a) {
    if (a->lines.empty()) return;
    a->lines.back() = RETURN(a->lines.back());
  }

  void visit(Call * b) {
    out = new Return(b);
  }
  void visit(BinOp * b) {
    out = new Return(b);
  }
  void visit(VoidExpr * b) {
    
  }
};

class ReturnInsBuilder : public Visitor {
  Module * module;
  FuncDef * func;
public:
  ReturnInsBuilder(Module * m) : module(m), func(0) {
    foreach(m->lines, it) (*it)->accept(this);
  }
  void visit(FuncDef * a) {
    if (getTypeName(a->rettype) == "Void") return;
    this->func = a;
    a->body->accept(this);
  }
  void visit(Call * a) { }
  void visit(BlockExpr * a) {
    if (a->lines.empty()) return;
    Expr *expr = a->lines.back();
    if (expr) { 
      a->lines.back() = RETURN(expr);
    }
  }
  void visit(Extern * a) { }
};

Module * insertReturns(Module * m) {
  ReturnInsBuilder bb(m);
  return m;
}
