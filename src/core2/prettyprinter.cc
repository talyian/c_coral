#include "expr.hh"

#include <iostream>

using std::cout;

namespace coral {
  class PrettyPrinter : coral::ast::ExprVisitor {
  public:
	int indent = 0;
	bool line = false;
	PrettyPrinter();

	std::string IND() { return line ? "" : std::string(indent * 3, ' '); }
	std::string END() { return line ? "" : "\n"; }

	virtual void visit(ast::Module * m);
	virtual void visit(ast::Extern * e) {
	  cout << IND() << "extern 'C' " << e->name << " : " << e->type.name << END();
	}

	virtual void visit(ast::Func * e) {
	  cout << IND() << "func " << e->name << "():\n";
	  indent++;
	  e->body->accept(this);
	  indent--;
	}
	virtual void visit(ast::Block * e) {
	  for(auto && line : e->lines) line->accept(this);
	}

	virtual void visit(ast::Call * e) {
	  cout << IND();
	  line = true;
	  e->callee->accept(this);
	  if (e->arguments.size() != 1) cout << '('; else cout << ' ';
	  for(auto &&arg : e->arguments) {
		if (arg != e->arguments.front()) cout << ", ";
		arg->accept(this);
	  }
	  if (e->arguments.size() != 1) cout << ')';
	  line = false;
	  cout << END();
	}

	virtual void visit(ast::Var * v) { cout << IND() << v->name << END(); }

	virtual void visit(ast::Return * e) {
	  cout << IND() << "return ";
	  line = true;
	  e->val->accept(this);
	  line = false;
	  cout << END();
	}

	virtual void visit(ast::IntLiteral * e) { cout << IND() << e->value << END(); }
	virtual void visit(ast::StringLiteral * s) { cout << IND() << '"' << s->value << '"' << END(); }
  };
}

void coral::PrettyPrinter::visit(ast::Module * m) {
  for(auto && def: m->externs) def->accept(this);
  for(auto && def: m->imports) def->accept(this);
  cout << END();
  for(auto && def: m->functions) def->accept(this);
  if (m->init) m->init->accept(this);
}

coral::PrettyPrinter::PrettyPrinter() {
  indent = 0;
}
