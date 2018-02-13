#include "core/expr.hh"
#include "core/prettyprinter.hh"
#include "ReturnInserter.hh"

#include <iostream>
#include <algorithm>

namespace coral {
  namespace analyzers {

    class ReturnReplace : public ast::ExprVisitor {
    public:
      ast::BaseExpr * original;
      ast::BaseExpr * out = 0;

      ReturnReplace(ast::BaseExpr * expr) {
        original = expr;
        out = expr;
        if (expr)
          expr->accept(this);
      }

      std::string visitorName() { return "Returnify"; }

      static ast::BaseExpr * replace(ast::BaseExpr * original) {
        ReturnReplace rr(original);
        // std::cout << "Replacing Return: "
        //            << ast::ExprNameVisitor::of(rr.original) << " -- "
        //            << ast::ExprNameVisitor::of(rr.out) << std::endl;
        return rr.out;
      }

      void visit(ast::Comment * e) { }
      void visit(ast::IfExpr * e) {
        if (!e->elsebody) { out = 0; return; }
        e->ifbody.reset(ReturnReplace::replace(e->ifbody.release()));
        e->elsebody.reset(ReturnReplace::replace(e->elsebody.release()));
        out = e;
      }
      void visit(ast::Block * e) {

        auto last_item = std::find_if(
          e->lines.rbegin(), e->lines.rend(),
          [] (std::unique_ptr<ast::BaseExpr> & cur) { return (cur && ast::ExprTypeVisitor::of(cur.get()) != ast::ExprTypeKind::CommentKind); });

        if (!e->lines.empty() && last_item != e->lines.rend()) {
          auto old_ptr = last_item->release();
          auto new_ptr = ReturnReplace::replace(old_ptr);
          if (new_ptr) {
            last_item->reset(new_ptr);
            if (new_ptr != old_ptr) { return; }
            if (ast::ExprTypeVisitor::of(old_ptr) == ast::ExprTypeKind::ReturnKind) return;
            if (ast::ExprTypeVisitor::of(old_ptr) == ast::ExprTypeKind::IfExprKind) return;
          } else {
            last_item->reset(old_ptr);
          }
        }
        e->lines.push_back(
          std::unique_ptr<ast::BaseExpr>(
            new ast::Return(
              new ast::IntLiteral("0"))));
        out = e;
      }
      void visit(ast::Var * e) { out = new ast::Return(e); }
      void visit(ast::Call * e) { out = new ast::Return(e); }
      void visit(ast::BinOp * e) { out = new ast::Return(e); }
      void visit(ast::IntLiteral * e) { out = new ast::Return(e); }
      void visit(ast::StringLiteral * e) { out = new ast::Return(e); }
      void visit(ast::While * e) { }
      void visit(ast::ForExpr * e) { }
      void visit(ast::Return * e) { }
    };
  }
}

using namespace coral;

void coral::analyzers::ReturnInserter::visit(ast::Module * m) { m->body->accept(this); }

void coral::analyzers::ReturnInserter::visit(ast::Block * m) {
  for(auto && line : m->lines) if (line) line->accept(this);
}

void coral::analyzers::ReturnInserter::visit(ast::Func * m) {
  if (!m->body) return;
  ReturnReplace::replace(m->body.get());
}
