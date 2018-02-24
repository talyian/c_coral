#include "core/expr.hh"
#include "analyzers/inference/Environment.hh"

#include <string>

namespace coral {
  namespace typeinference {
    class InferenceBuilder : public ast::ExprVisitor {
    public:
      TypeEnvironment env;
      TypeTerm * out;
      std::string visitorName() { return "InferenceBuilder"; }
      InferenceBuilder(ast::Module * m);

      void visit(ast::Module * m);
      void visit(ast::BinOp * m);
      void visit(ast::Block * m);
      void visit(ast::Func * m);
      void visit(ast::Var * m);
      void visit(ast::IfExpr * m);
      void visit(ast::IntLiteral * m);
      void visit(ast::StringLiteral * m);
      void visit(ast::Call * m);
      void visit(ast::Let * m);
      void visit(ast::Set * m);
      void visit(ast::Comment * m);
      void visit(ast::Return * m);
      void visit(ast::While * m);
      void visit(ast::Member * m);
    };
  }
}
