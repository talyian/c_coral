#include "core/expr.hh"
#include "core/prettyprinter.hh"
#include "ReturnInserter.hh"

#include <iostream>

namespace coral {
  namespace analyzers {
	class FuncFinder : public ast::ExprVisitor {
	public:
	  ast::BaseExpr * original;
	  ast::Func * out;
	  FuncFinder(ast::BaseExpr * expr) { original = expr; expr->accept(this); }
	  void visit(ast::Module * m) { m->body->accept(this); }
	  void visit(ast::Block * b) { for(auto && line : b->lines) if (line) line->accept(this); }
	  void visit(ast::Func * f) { out = f; }
	};

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

	  static ast::BaseExpr * replace(ast::BaseExpr * original) {
		ReturnReplace rr(original);
		// std::cout << "Replacing Return: "
		// 		  << ast::ExprNameVisitor::of(rr.original) << " -- "
		// 		  << ast::ExprNameVisitor::of(rr.out) << std::endl;
		return rr.out;
	  }

	  void visit(ast::Comment * e) { }
	  void visit(ast::IfExpr * e) {
		e->ifbody.reset(ReturnReplace::replace(e->ifbody.release()));
		e->elsebody.reset(ReturnReplace::replace(e->elsebody.release()));
		out = e;
	  }
	  void visit(ast::Block * e) {
		auto m = e->lines.back().get();
		std::cout << ast::ExprNameVisitor::of(m) << "\n";
		e->lines.back().reset(ReturnReplace::replace(e->lines.back().release()));
		out = e;
	  }
	  void visit(ast::Var * e) { out = new ast::Return(e); }
	  void visit(ast::Call * e) { out = new ast::Return(e); }
	};
  }
}

using namespace coral;

void coral::analyzers::ReturnInserter::visit(ast::Module * m) { m->body->accept(this); }

void coral::analyzers::ReturnInserter::visit(ast::Block * m) {
  for(auto && line : m->lines) if (line) line->accept(this);
}

void coral::analyzers::ReturnInserter::visit(ast::Func * m) {
  auto block = m->body.get();
  decltype(block->lines)::iterator iter;
  for(auto i = block->lines.begin(); i != block->lines.end(); i++) if (*i) { iter = i; }
  iter->reset(ReturnReplace::replace(iter->release()));
}
