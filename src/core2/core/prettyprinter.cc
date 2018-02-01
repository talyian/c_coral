#include "prettyprinter.hh"
#include "expr.hh"

#include <iostream>

using std::cout;

namespace coral {
  void coral::PrettyPrinter::visit(ast::Extern * e) {
	cout << IND() << "extern 'C' " << e->name << " : " << e->type.name << END();
  }

  void PrettyPrinter::visit(ast::Func * e) {
	cout << IND() << "func " << e->name << "():\n";
	indent++;
	if (e->body) e->body->accept(this);
	indent--;
  }
  void PrettyPrinter::visit(ast::Block * e) {
	for(auto && line : e->lines)
	  if (line) line->accept(this);
	  else cout << "\n";
  }

  void PrettyPrinter::visit(ast::BinOp * e) {
	cout << IND();
	withline(e->lhs);
	cout << " " << e->op << " ";
	withline(e->rhs);
	cout << END();
  }

  void PrettyPrinter::visit(ast::Call * e) {
	auto l = line;
	cout << IND();
	line = true;
	if (e->callee) e->callee->accept(this);
	else cout << "(null callee)";
	// TODO: atomize
	bool showParens = true;
	// if (e->arguments.size() == 1 && (
	// 	  ExprTypeVisitor::of(e->arguments[0]) == ExprTypeKind::ListLiteralKind))
	if (e->arguments.size() == 1) {
	  auto type = ast::ExprTypeVisitor::of(e->arguments[0].get());
	  if (type == ast::ExprTypeKind::ListLiteralKind ||
		  type == ast::ExprTypeKind::IntLiteralKind ||
		  type == ast::ExprTypeKind::StringLiteralKind ||
		  type == ast::ExprTypeKind::VarKind ||
		  type == ast::ExprTypeKind::TupleLiteralKind
		) showParens = false;
	}
	if (showParens) cout << '('; else cout << ' ';
	for(auto &&arg : e->arguments) {
	  if (!arg) { cout << "(null arg)"; continue; }
	  if (arg != e->arguments.front()) cout << ", ";
	  arg->accept(this);
	}
	if (showParens) cout << ')';
	line = l;
	cout << END();
  }

  void PrettyPrinter::visit(ast::Var * v) { cout << IND() << v->name << END(); }

  void PrettyPrinter::visit(ast::Return * e) {
	cout << IND() << "return ";
	line = true;
	if (e->val) e->val->accept(this);
	line = false;
	cout << END();
  }

  void PrettyPrinter::visit(ast::IntLiteral * e) {
	cout << IND() << e->value << END(); }

  void PrettyPrinter::visit(ast::StringLiteral * s) {
	cout << IND()  << s->value << END(); }

  void PrettyPrinter::visit(ast::IfExpr * s) {
	cout << IND() << "if ";
	withline(s->cond);
	cout << ":" << END();
	indent++;
	s->ifbody->accept(this);
	indent--;
	cout << IND() << "else:" << END();
	indent++;
	s->elsebody->accept(this);
	indent--;
  }

  void PrettyPrinter::visit(ast::ForExpr * s) {
	cout << IND() << "for ";
	withline(s->var);
	cout << " in ";
	withline(s->sequence);
	cout << ":" << END();
	indent++;
	if (s->body) s->body->accept(this);
	indent--;
  }

  void PrettyPrinter::visit(ast::Comment * c) {
	cout << IND() << c->value << END();
  }

  void PrettyPrinter::visit(ast::Let * e) {
	cout << IND() << "let ";
	withline(e->var);
	cout << " = ";
	withline(e->value);
	cout << END();
  }

  void PrettyPrinter::visit(ast::Module * m) {
	if (m->body) m->body->accept(this);
  }

  void PrettyPrinter::visit(ast::Member * m) {
	cout << IND();
	m->base->accept(this);
	cout << "." << m->member << END();
  }

  void PrettyPrinter::visit(ast::ListLiteral * m) {
	cout << IND() << '[';
	for(auto && item : m->items) {
	  if (item != m->items.front())
		cout << ", ";
	  withline(item);
	}
	cout << ']' << END();
  }

  void PrettyPrinter::visit(ast::TupleLiteral * m) {
	cout << IND() << '(';
	for(auto && item : m->items) {
	  if (item != m->items.front())
		cout << ", ";
	  withline(item);
	}
	cout << ')' << END();
  }

  PrettyPrinter::PrettyPrinter() {
	indent = 0;
  }
}
