#include "core/expr.hh"

#include <iostream>

namespace coral {
  namespace analyzers {
    class TypeInfo {
    public:
	  ast::BaseExpr * expr;
      coral::Type type = coral::Type("");
    };

    // Populates all ast::Var var nodes with a pointer to
    // the expression that it is referring to
    class TypeResolver : public ast::ExprVisitor {
    public:
      ast::Module * module = 0;
      ast::BaseExpr * target;
      std::string name;
      std::map<ast::BaseExpr *, TypeInfo> info;

      TypeResolver(ast::Module * m) : module(m) { visit(m); }
      virtual std::string visitorName() { return "TypeResolver"; }

      void visit(ast::Module * m);
      void visit(ast::Block * m);
      void visit(ast::Comment * m);
      void visit(ast::IfExpr * m);
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
      void visit(ast::Def * w);
    };
  }
}
