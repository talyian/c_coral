#include "ast.hh"

Expr * StaticConcat(Expr * m);

class Concatenator : public Visitor {
public:
  Concatenator (Expr * e) : out(e), Visitor("concat ") { e->accept(this); }
  Expr * out = 0;
  void visit(Module * n);
  void visit(FuncDef * n);
  void visit(Let * n);
  void visit(BinOp * n);
  void visit(If * n);
  void visit(BlockExpr * n);
  void visit(Call * n);
  void visit(Var * n);
  void visit(Long * n);
  void visit(String * n);
  void visit(VoidExpr * n);
  void visit(Extern * n) { }
};
