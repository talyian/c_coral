#pragma once
#include "core/expr.hh"
#include "utils/ansicolor.hh"
#include <iostream>
#include <set>

namespace coral {
  namespace analyzers {

	class NameInfo {
	public:
	  ast::BaseExpr * expr = 0;
	  ast::ExprTypeKind kind = ast::ExprTypeKind::BaseExprKind;
	};

    class NameScope {
    public:
      NameScope * parent;
      // variables from the current scope
	  std::map<std::string, NameInfo> info;
      // free variables that are pulled from outer scope
      std::multimap<std::string, ast::BaseExpr *> freeVars;

      void insert(std::string name, ast::BaseExpr * expr) {
        insert(name, expr, ast::ExprTypeVisitor::of(expr));
      }
      void insert(
        std::string name,
        ast::BaseExpr * expr,
        ast::ExprTypeKind kind) {
        info[name].expr = expr;
        info[name].kind = kind;
      }
      NameInfo get(std::string name) {
        auto it = info.find(name);
        if (it != info.end()) return it->second;
        if (parent)
          return parent->get(name);
        return NameInfo();
      }
    };

	// Populates all ast::Var var nodes with a pointer to
	// the expression that it is referring to
	class NameResolver : public ast::ExprVisitor {
	public:
	  ast::Module * module = 0;
	  ast::BaseExpr * target;
	  std::string name;

      NameScope * root = new NameScope(), *scope = root;

      NameScope * pushScope(std::string) {
        auto n = new NameScope();
        n->parent = scope;
        scope = n;
        return n;
      }

      NameScope * popScope() {
        auto n = scope;
        if (scope == root) {
          std::cerr << COL_LIGHT_RED << "Warning: trying to pop root name scope\n";
        } else {
          scope = scope->parent;
          delete n;
        }
        return scope;
      }

	  NameResolver(ast::Module * m) : module(m) { visit(m); }
	  virtual std::string visitorName() { return "NameResolver"; }

	  void visit(ast::Module * m);
	  void visit(ast::Block * m);
	  void visit(ast::Comment * m);
	  void visit(ast::IfExpr *);
	  void visit(ast::Match *);
	  void visit(ast::MatchCase *);
	  void visit(ast::Let * e);
	  void visit(ast::BinOp * m);
	  void visit(ast::Return * m);
	  void visit(ast::Call * c);
	  void visit(ast::Var * v);
	  void visit(ast::Func * f);
	  void visit(ast::StringLiteral * e);
      void visit(ast::IntLiteral * i);
	  void visit(ast::FloatLiteral * i);
	  void visit(ast::Set * s);
	  void visit(ast::While * w);
	  void visit(ast::Member * w);
	  void visit(ast::Extern * w);
	  void visit(ast::Tuple * w);
	  void visit(ast::Union * w);
	  void visit(ast::TupleLiteral * w);
	};
  }
}
