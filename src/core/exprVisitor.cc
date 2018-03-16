#include "core/expr.hh"

namespace coral {
  namespace ast {
    void ActiveExprVisitor::visit(ast::Return * r) { }
    void ActiveExprVisitor::visit(ast::StringLiteral * ) { }
    void ActiveExprVisitor::visit(ast::IntLiteral * ) { }
    void ActiveExprVisitor::visit(ast::FloatLiteral * ) { }
    void ActiveExprVisitor::visit(ast::Comment * ) { }
    void ActiveExprVisitor::visit(ast::Extern * ) { }
    void ActiveExprVisitor::visit(ast::Let * ) { }
    void ActiveExprVisitor::visit(ast::Func * f) {
      for(auto &def: f->params)
        def->accept(this);
      if (f->body) f->body->accept(this);
    }
    void ActiveExprVisitor::visit(ast::Block * b) {
      for(auto &line: b->lines) if(line) line->accept(this);
    }
    void ActiveExprVisitor::visit(ast::Call * ) { }
    void ActiveExprVisitor::visit(ast::Var * ) { }
    void ActiveExprVisitor::visit(ast::IfExpr * ) { }
    void ActiveExprVisitor::visit(ast::ForExpr * ) { }
    void ActiveExprVisitor::visit(ast::BinOp * op) {
      op->lhs->accept(this);
      op->rhs->accept(this);
    }
    void ActiveExprVisitor::visit(ast::Member * ) { }
    void ActiveExprVisitor::visit(ast::ListLiteral * ) { }
    void ActiveExprVisitor::visit(ast::TupleLiteral * ) { }
    void ActiveExprVisitor::visit(ast::Def * ) { }
    void ActiveExprVisitor::visit(ast::While * w) {
      w->cond->accept(this);
      if (w->body) w->body->accept(this);
    }
    void ActiveExprVisitor::visit(ast::Set * ) { }
    void ActiveExprVisitor::visit(ast::Tuple * ) { }
    void ActiveExprVisitor::visit(ast::BaseExpr * ) { }
    void ActiveExprVisitor::visit(ast::Module * m) {
      m->body->accept(this);
    }
    void ActiveExprVisitor::visit(ast::Import * ) { }
    void ActiveExprVisitor::visit(ast::OverloadedFunc *) { }
    void ActiveExprVisitor::visit(ast::Match *) { }
    void ActiveExprVisitor::visit(ast::Union * u) {
      for(auto & c: u->cases)
        c->accept(this);
    }
  }
}
