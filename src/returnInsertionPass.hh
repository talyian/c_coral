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
  InstructionHandler(Expr * e) : Visitor("retins-instr "), out(e) { e->accept(this); }

  void visit(If * a) {
    a->ifbody = (BlockExpr *)RETURN(a->ifbody);
    a->elsebody = (BlockExpr *)RETURN(a->elsebody);
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
  void visit(MatchExpr * match) {
    foreach(match->cases, it) (*it)->accept(this);
  }
  void visit(MatchCaseTagsExpr * tag) {
    tag->body = RETURN(tag->body);
  }
};

class ReturnInsBuilder : public Visitor {
  Module * module;
  FuncDef * func;
public:
  ReturnInsBuilder(Module * m) : Visitor("retins "), module(m), func(0) {
    foreach(m->lines, it) (*it)->accept(this);
  }
  void visit(Let * a) { }
  void visit(DeclTypeEnum * a) { }
  void visit(FuncDef * a) {
    if (
      a->rettype == 0 ||
      getTypeName(a->rettype) == "Unknown" ||
      getTypeName(a->rettype) == "Void"
    ) return;
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
