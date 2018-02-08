#include "core/expr.hh"

#include <iostream>

namespace coral {
  namespace analyzers {

	class NameInfo {
	public:
	  ast::BaseExpr * expr = 0;
	  ast::ExprTypeKind kind = ast::ExprTypeKind::BaseExprKind;
	};

	// Populates all ast::Var var nodes with a pointer to
	// the expression that it is referring to
	class NameResolver : public ast::ExprVisitor {
	public:
	  ast::Module * module = 0;
	  ast::BaseExpr * target;
	  std::string name;
	  std::map<std::string, NameInfo> info;

	  NameResolver(ast::Module * m) : module(m) { visit(m); }
	  virtual std::string visitorName() { return "NameResolver"; }

	  void visit(ast::Module * m) { m->body->accept(this); }
	  void visit(ast::Block * m) { for(auto && line : m->lines) if (line) line->accept(this); }
	  void visit(ast::Comment * m) { }
	  void visit(ast::IfExpr * m) {
		m->cond->accept(this);
		m->ifbody->accept(this);
		if (m->elsebody) m->elsebody->accept(this); }
	  void visit(ast::Let * e) {
		info[e->var->name].expr = e;
		info[e->var->name].kind = ast::ExprTypeKind::LetKind;
		e->value->accept(this);
	  }
	  void visit(ast::BinOp * m) { m->lhs->accept(this); m->rhs->accept(this); }
	  void visit(ast::Return * m) { if (m->val) m->val->accept(this); }
	  void visit(ast::Call * c) {
		if (c->callee) c->callee->accept(this);
		for(auto && arg: c->arguments) if (arg) arg->accept(this);
	  }

	  void visit(ast::Var * v) {
		auto expr = info[v->name].expr;
		// std::cout << "Var! " << v->name
		// 		  << " ("  << (void *) expr << ") "
		// 		  << ast::ExprNameVisitor::of(expr) << std::endl;
		v->expr = info[v->name].expr;
	  }

	  void visit(ast::Func * f) {
		info[f->name].expr = f;
		info[f->name].kind = ast::ExprTypeKind::FuncKind;
		for(auto && param : f->params) {
		  info[param->name].expr = param.get();
		  info[param->name].kind = ast::ExprTypeKind::DefKind;
		}
		if (f->body) f->body->accept(this);
	  }

	  void visit(ast::StringLiteral * e) { }

      void visit(ast::IntLiteral * i) { }

	  void visit(ast::FloatLiteral * i) { }

	  void visit(ast::Set * s) {
		s->var->accept(this);
		s->value->accept(this);
		// info[s->var->name].expr = s;
		// info[s->var->name].kind = ast::ExprTypeKind::SetKind;
	  }

	  void visit(ast::While * w) {
		w->cond->accept(this);
		w->body->accept(this);
	  }
	};
  }
}
