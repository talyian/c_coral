#pragma once
#include <map>
#include <memory>
#include <vector>
#include <cstdio>

namespace coral {
  namespace type {
	class Type {
	public:
	  std::string name;
	  Type(std::string name) : name(name) { }
	};
  }

  using Type = type::Type;

  namespace ast {
	using std::vector;
	using std::unique_ptr;
	using std::map;

#define MAP_ALL_EXPRS(F) F(Module) F(Extern) F(Import) F(Let) F(Func) F(Block) F(Var) F(Call) F(StringLiteral) F(IntLiteral) F(FloatLiteral) F(Return) F(Comment) F(IfExpr) F(ForExpr) F(BinOp) F(Member) F(ListLiteral) F(TupleLiteral) F(Def) F(While) F(Set)

	// Forward-declare all Expr classes
#define F(E) class E;
	MAP_ALL_EXPRS(F)
	F(BaseExpr);
#undef F

	// Visitor
	class ExprVisitor {
	public:
	  virtual std::string visitorName() { return "ExprVisitor"; }
#define F(E) virtual void visit(__attribute__((unused)) E * expr) { \
	    fprintf(stderr, "%s: %s", visitorName().c_str(), #E "\n");	\
	  }
      MAP_ALL_EXPRS(F)
	  F(BaseExpr)
#undef F
	};

	enum class ExprTypeKind {
#define F(E) E##Kind,
	MAP_ALL_EXPRS(F)
	F(BaseExpr)
#undef F
	};

	template<typename T>
	void moveTo(vector<T *> source, vector<unique_ptr<T>> & target) {
	  for(auto && ptr : source) if (ptr) target.push_back(unique_ptr<coral::ast::BaseExpr>(ptr));
	}

	class BaseExpr {
	public:
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	  virtual ~BaseExpr() { }
	};

	class Expr : public BaseExpr { };

	class Statement : public BaseExpr { };

	class ExprTypeVisitor : public ExprVisitor {
	public:
	  ExprTypeKind out;

	  static ExprTypeKind of(BaseExpr * e) {
		ExprTypeVisitor v;
		v.out = ExprTypeKind::BaseExprKind;
		if (e) e->accept(&v);
		return v.out;
	  }
#define F(E) virtual void visit(__attribute__((unused)) E * expr) { out = ExprTypeKind::E##Kind; }
	  MAP_ALL_EXPRS(F)
#undef F
	};

	class ExprNameVisitor : public ExprVisitor {
	public:
	  std::string out;

	  static std::string of(BaseExpr * e) {
		ExprNameVisitor v;
		v.out = "(null)";
		if (e) e->accept(&v);
		return v.out;
	  }
#define F(E) virtual void visit(__attribute__((unused)) E * expr) { out = #E; }
	  MAP_ALL_EXPRS(F)
#undef F
	};

	class Comment : public Statement {
	public:
	  std::string value;
	  Comment(std::string value) : value(value) { }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};

	class Block : public Statement {
	public:
	  std::vector<unique_ptr<coral::ast::BaseExpr>> lines;
	  Block(std::vector<coral::ast::BaseExpr *> lines) {
		for(auto && ptr : lines) if (ptr) this->lines.push_back(unique_ptr<coral::ast::BaseExpr>(ptr));
	  }
	  virtual void accept(ExprVisitor * v) {
		v->visit(this);
	  }
	};

	class Module : public BaseExpr {
	public:
	  unique_ptr<Block> body;
	  Module();
	  Module(vector<BaseExpr *> lines) {
		body = unique_ptr<Block>(new Block(lines));
	  }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};

	class Func : public Expr {
	public:
	  std::string name;
	  std::unique_ptr<coral::Type> type;
	  vector<unique_ptr<coral::ast::Def>> params;
	  unique_ptr<Block> body;
	  Func(std::string name,
		   Type * type,
		   vector<coral::ast::Def *> params,
		   Block * body)
		: name(name), type(type), body(body) {
		for(auto && p : params) this->params.push_back(std::unique_ptr<Def>(p));
	  }

	  virtual void accept(ExprVisitor * v) {
		v->visit(this);
	  }
	};

	class IfExpr : public Expr {
	public:
	  unique_ptr<BaseExpr> cond, ifbody, elsebody;
	  IfExpr(BaseExpr* cond,
			 BaseExpr* ifbody,
			 BaseExpr* elsebody)
		: cond(cond), ifbody(ifbody), elsebody(elsebody) { }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};

	class ForExpr : public Expr {
	public:
	  unique_ptr<BaseExpr> var, sequence, body;
	  ForExpr(BaseExpr* var, BaseExpr* sequence, BaseExpr* body)
		: var(var), sequence(sequence), body(body) { }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};

	class Var : public Expr {
	public:
	  std::string name;
      // the declaring expression -- the node that created the name in question
	  // in the current scope
	  ast::BaseExpr * expr;

	  Var(std::vector<std::string> names) {
		name = names.size() ? names[0] : "undefined";
	  }
	  Var(std::string name) : name(name) { }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};
	class StringLiteral : public Expr {
	public:
	  std::string value;
	  StringLiteral(std::string value): value(value) { }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	  std::string getString();
	};
	class IntLiteral : public Expr {
	public:
	  std::string value;
	  IntLiteral(std::string value) : value(value) {  }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};
	class FloatLiteral : public Expr {
	public:
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};

	class BinOp : public Expr {
	public:
	  std::unique_ptr<BaseExpr> lhs, rhs;
	  std::string op;
	  BinOp(BaseExpr * lhs, std::string op, BaseExpr * rhs) : lhs(lhs), rhs(rhs), op(op) { }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};

	class Let : public Statement {
	public:
	  unique_ptr<Var> var;
	  unique_ptr<BaseExpr> value;
	  Let(Var * var, BaseExpr * value) : var(var), value(value) { }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};

	class Set : public Statement {
	public:
	  unique_ptr<Var> var;
	  unique_ptr<BaseExpr> value;
	  Set(Var * var, BaseExpr * value) : var(var), value(value) { }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};

	class Def : public Expr {
	public:
	  std::string name;
	  std::unique_ptr<type::Type> type;
	  BaseExpr * value;
	  Def(std::string name, type::Type * t, BaseExpr * value) : name(name), type(t), value(value) { }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};

	class Member : public Expr {
	public:
	  unique_ptr<BaseExpr> base = 0;
	  std::string member;
	  Member(BaseExpr * base, std::string member) : base(base), member(member) { }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};

	class ListLiteral : public Expr {
	public:
	  std::vector<unique_ptr<BaseExpr>> items;
	  ListLiteral(std::vector<BaseExpr *> items) {
		for(auto && pp : items) if (pp) this->items.push_back(std::unique_ptr<BaseExpr>(pp));
	  }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};

	class TupleLiteral : public Expr {
	public:
	  std::vector<unique_ptr<BaseExpr>> items;
	  TupleLiteral(std::vector<BaseExpr *> items) {
		for(auto && pp : items) if (pp) this->items.push_back(std::unique_ptr<BaseExpr>(pp));
	  }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};

		class Call : public Expr {
	public:
	  unique_ptr<BaseExpr> callee;
	  vector<std::unique_ptr<BaseExpr>> arguments;

	  Call(BaseExpr * callee, TupleLiteral * arguments) : callee(callee) {
		for(auto && p : arguments->items)
		  this->arguments.push_back(std::unique_ptr<BaseExpr>(p.release()));
	  }
	  Call(BaseExpr * callee, vector<BaseExpr *> arguments): callee(callee) {
		for(auto && ptr : arguments)
		  if (ptr) this->arguments.push_back(std::unique_ptr<BaseExpr>(ptr));
	  }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};


	class Return : public Statement {
	public:
	  unique_ptr<BaseExpr> val;
	  Return(BaseExpr * val) : val(val) { }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};

	class Extern : public Statement {
	public:
	  std::string name;
	  std::string linkage;
	  coral::Type type;
	  Extern(std::string name, coral::Type type) : name(name), type(type) { }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};

	class Import : public Statement {
 	public:
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};

	class While : public Statement {
 	public:
	  unique_ptr<BaseExpr> cond, body;
	  While(BaseExpr* cond, BaseExpr * body) : cond(cond), body(body) { }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};
  }
}
