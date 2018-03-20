#pragma once

#include "core/expr.hh"

#include <iostream>

using std::cout;

namespace coral {

  class PrettyPrinter : public coral::ast::ExprVisitor {
  public:
	int indent = 0;
	bool line = false;
	PrettyPrinter();

	std::string IND() { return line ? "" : std::string(indent * 3, ' '); }
	std::string END() { return line ? "" : "\n"; }

	std::string visitorName() { return "PrettyPrinter"; }

	static void print(ast::BaseExpr * e) { PrettyPrinter pp; e->accept(&pp); }

	template <typename Tn>
	void withline(std::unique_ptr<Tn> & node) { withline(node.get()); }

	template <typename Tn>
	void withline(Tn * node) {
	  auto old_line = line;
	  line = true;
	  if (node) node->accept(this);
	  else cout << "(null)";
	  line = old_line;
	}

	virtual void visit(ast::Module *);
	virtual void visit(ast::Extern *);
	virtual void visit(ast::Func *);
	virtual void visit(ast::Block *);
	virtual void visit(ast::Call *);
	virtual void visit(ast::Var *);
	virtual void visit(ast::Return *);
	virtual void visit(ast::IntLiteral *);
	virtual void visit(ast::FloatLiteral *);
	virtual void visit(ast::StringLiteral *);
	virtual void visit(ast::Comment *);
	virtual void visit(ast::IfExpr *);
	virtual void visit(ast::ForExpr *);
	virtual void visit(ast::BinOp *);
	virtual void visit(ast::Let *);
	virtual void visit(ast::Member *);
	virtual void visit(ast::ListLiteral *);
	virtual void visit(ast::TupleLiteral *);
	virtual void visit(ast::Def *);
	virtual void visit(ast::While *);
	virtual void visit(ast::Set *);
	virtual void visit(ast::Tuple *);
	virtual void visit(ast::Import *);
	virtual void visit(ast::Union *);
	virtual void visit(ast::Match *);
 	virtual void visit(ast::MatchCase *);
  };
}
