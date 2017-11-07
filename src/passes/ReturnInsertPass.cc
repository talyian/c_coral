// Return Insertion
// In Coral, functions do not call "return" explicitly --
// Instead the last value in the function is implicitly the return
// This requires inserting "ret" calls at some point. It's easier to do that
// as a self-contained step than in codegen since it has to be propagated
// into if / match and loops correctly.
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

#define RETURN(exprp) ReplaceReturn(exprp).out
class ReplaceReturn : public Visitor {
public:
  Expr *out;
  ReplaceReturn(Expr * e) : Visitor("retins-instr "), out(e) { e->accept(this); }

  void visit(If * a) {
    a->ifbody = (BlockExpr *)RETURN(a->ifbody);
    a->elsebody = (BlockExpr *)RETURN(a->elsebody);
  }

  void visit(BlockExpr * a) {
    if (a->lines.empty()) return;
    a->lines.back() = RETURN(a->lines.back());
  }

  void visit(Var * l) { out = new Return(l); }
  void visit(String * b) { out = new Return(b); }
  void visit(Long * l) { out = new Return(l); }
  void visit(Call * b) { out = new Return(b); }
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

// Find all FuncDefs, and call ReplaceReturn on them
class ReturnInsertPass : public Visitor {
  FuncDef * func;
public:
  Module * out;
  ReturnInsertPass(Module * m) : Visitor("retins "), func(0), out(m) {
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
    a->body = RETURN(a->body);
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

Module * doReturnInsertPass(Module * m) { return ReturnInsertPass(m).out; }
