#pragma once
#include <map>
#include <memory>
#include <vector>

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

#define MAP_ALL_EXPRS(F) F(Module) F(Extern) F(Import) F(Let) F(Func) F(Block) F(Var) F(Call) F(StringLiteral) F(IntLiteral) F(FloatLiteral) F(Return) F(Comment) F(IfExpr) F(ForExpr) F(BinOp)

	// Forward-declare all Expr classes
#define F(E) class E;
	MAP_ALL_EXPRS(F)
	F(BaseExpr);
#undef F

	// Visitor
	class ExprVisitor {
	public:
#define F(E) virtual void visit(__attribute__((unused)) E * expr) { printf(#E "\n"); }
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
#define F(E) virtual void visit(__attribute__((unused)) E * expr) { out = ExprTypeKind::E##Kind; }
	MAP_ALL_EXPRS(F)
#undef F
	static ExprTypeKind of(BaseExpr * e) {
	  ExprTypeVisitor v;
	  v.out = ExprTypeKind::BaseExprKind;
	  if (e) e->accept(&v);
	  return v.out;
	}
	};

	class Comment : public Statement {
	public:
	  std::string value;
	  Comment(std::string value) : value(value) { }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};

	class Block : public Statement {
	public:
	  std::vector<coral::ast::BaseExpr *> lines;
	  Block(std::vector<coral::ast::BaseExpr *> lines) {
		for(auto && ptr : lines) this->lines.push_back(ptr);
	  }
	  virtual void accept(ExprVisitor * v) {
		v->visit(this);
	  }
	};

	class Module : public BaseExpr {
	public:
	  vector<unique_ptr<Extern>> externs;
	  vector<unique_ptr<Import>> imports;
	  vector<Func *> functions;
	  unique_ptr<Block> init;
	  Module();
	  Module(vector<BaseExpr *> lines) {
		init = unique_ptr<Block>(new Block(lines));
		for(auto && line : lines) {
		  if (!line) continue;
		  if (ExprTypeVisitor::of(line) == ExprTypeKind::ExternKind)
			externs.push_back(unique_ptr<Extern>((Extern *) line));
		  else if (ExprTypeVisitor::of(line) == ExprTypeKind::FuncKind)
			// functions.push_back(unique_ptr<Func>((Func *) line));
			functions.push_back((Func *)line);
		  else if (ExprTypeVisitor::of(line) == ExprTypeKind::ImportKind)
			imports.push_back(unique_ptr<Import>((Import *) line));
		  else
			; // init->lines.push_back(unique_ptr<BaseExpr>(line));
		}
	  }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};

	class Func : public Expr {
	public:
	  std::string name;
	  coral::Type type;
	  vector<coral::ast::BaseExpr *> params;
	  Block * body;
	  Func(std::string name,
		   Type type,
		   vector<coral::ast::BaseExpr *> params,
		   Block * body)
		: name(name), type(type), params(params), body(body) { }

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
	  ForExpr(BaseExpr* var,
			  BaseExpr* sequence,
			  BaseExpr* body)
		: var(var), sequence(sequence), body(body) { }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};

	class Call : public Expr {
	public:
	  unique_ptr<BaseExpr> callee;
	  vector<BaseExpr *> arguments;
	  Call(BaseExpr * callee, BaseExpr * argument): callee(callee) {
		arguments.push_back(argument);
	  }

	  Call(BaseExpr * callee, vector<BaseExpr *> arguments): callee(callee) {
		for(auto && ptr : arguments)
		  this->arguments.push_back(ptr);
	  }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};

	class Var : public Expr {
	public:
	  std::string name;
	  Var(std::string name) : name(name) { }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};
	class StringLiteral : public Expr {
	public:
	  std::string value;
	  StringLiteral(std::string value): value(value) { }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
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
	  BaseExpr * lhs = 0, * rhs = 0;
	  std::string op;
	  BinOp(BaseExpr * lhs, std::string op, BaseExpr * rhs) : lhs(lhs), rhs(rhs), op(op) { }
	  virtual void accept(ExprVisitor * v) { v->visit(this); }
	};

	class Let : public Statement {
	public:
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

  }
}
