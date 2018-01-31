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
	auto oldline = line;
	line = true;
	if (e->lhs) e->lhs->accept(this);
	cout << " " << e->op << " ";
	if (e->rhs) e->rhs->accept(this);
	line = oldline;
  }

  void PrettyPrinter::visit(ast::Call * e) {
	cout << IND();
	line = true;
	e->callee->accept(this);
	// TODO: atomize
	if (true) cout << '('; else cout << ' ';
	for(auto &&arg : e->arguments) {
	  if (arg != e->arguments.front()) cout << ", ";
	  arg->accept(this);
	}
	if (true) cout << ')';
	line = false;
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
	cout << IND() << '"' << s->value << '"' << END(); }

  void PrettyPrinter::visit(ast::IfExpr * s) {
	cout << IND() << "if ";
	withline(s->cond.get());
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
	if (s->var) s->var->accept(this);
	cout << " in ";
	if (s->sequence) s->sequence->accept(this);
	cout << END() << ":" << "\n";
	if (s->body) s->body->accept(this);
  }


  void PrettyPrinter::visit(ast::Comment * c) {
	cout << IND() << c->value << END();
  }

  void PrettyPrinter::visit(ast::Module * m) {
	printf("Module %d %d %d %d\n",
		   m->externs.size(),
		   m->imports.size(),
		   m->functions.size(),
		   m->init ? m->init->lines.size() : -1
	  );
	for(auto && def: m->externs) def->accept(this);
	for(auto && def: m->imports) def->accept(this);
	cout << END();
	for(auto && def: m->functions) def->accept(this);
	if (m->init) m->init->accept(this);
  }
  PrettyPrinter::PrettyPrinter() {
	indent = 0;
  }
}
