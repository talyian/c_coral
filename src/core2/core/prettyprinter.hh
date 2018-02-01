#include "expr.hh"

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

	virtual void visit(ast::Module * m);
	virtual void visit(ast::Extern * m);
	virtual void visit(ast::Func * m);
	virtual void visit(ast::Block * m);
	virtual void visit(ast::Call * m);
	virtual void visit(ast::Var * m);
	virtual void visit(ast::Return * m);
	virtual void visit(ast::IntLiteral * m);
	virtual void visit(ast::StringLiteral * m);
	virtual void visit(ast::Comment * m);
	virtual void visit(ast::IfExpr * m);
	virtual void visit(ast::ForExpr * m);
	virtual void visit(ast::BinOp * m);
	virtual void visit(ast::Let * m);
	virtual void visit(ast::Member * m);
	virtual void visit(ast::ListLiteral * m);
	virtual void visit(ast::TupleLiteral * m);
  };
}
