#include "ast.hh"
#include <string>
Expr * StaticConcat(Expr * m);

class Concatenator : public FunctorVisitor {
public:
  Concatenator (Expr * e) : FunctorVisitor("concat ", e) { e->accept(this); }
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
