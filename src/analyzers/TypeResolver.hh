#pragma once

#include "core/expr.hh"
#include <iostream>

#include "analyzers/typegraph/constraint.hh"
#include "analyzers/typegraph/TypeGraph.hh"

namespace coral {
  namespace analyzers {
    class TypeResolver : public coral::ast::ExprVisitor {
    public:
      ast::Module * module = 0;
      ast::BaseExpr * target;
      std::string name;

      TypeGraph gg;
      std::map<TypeTerm *, ast::BaseExpr *> term_map;
      TypeTerm * out;

      TypeResolver(ast::Module * m);
      virtual std::string visitorName() { return "TypeResolver"; }

      void visit(ast::Module * m);
      void visit(ast::Block * m);
      void visit(ast::Def * d);
      void visit(ast::Func * f);
      void visit(ast::IfExpr * ifexpr);
      void visit(ast::Var * var);
      void visit(ast::BinOp * op);
      void visit(ast::IntLiteral * op);
      void visit(ast::Call * call);
      void visit(ast::Return * r);
      void visit(ast::StringLiteral * s);
      void visit(ast::Let * l);
      void visit(ast::While * w);
      void visit(ast::Set * s);
      void visit(ast::Extern * e);
      void visit(ast::Tuple * t);
      void visit(ast::TupleLiteral * t);
    };
  }
}
