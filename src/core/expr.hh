#pragma once
#include <map>
#include <memory>
#include <vector>
#include <cstdio>
#include "type.hh"
#include <iostream>

// Here is a list of all the AST node types.
// We use the X-Macro pattern to eliminate a lot of boilerplate around setting up AST nodes
#define MAP_EXPRS(F) F(Module)                                          \
  F(Extern) F(Import) F(Let) F(Func)                                    \
  F(Block) F(Var) F(Call)                                               \
  F(StringLiteral) F(IntLiteral) F(FloatLiteral)                        \
  F(Return) F(Comment) F(IfExpr) F(ForExpr) F(BinOp) F(Member)          \
  F(ListLiteral) F(TupleLiteral) F(Def) F(While) F(Set) F(Tuple)        \
  F(OverloadedFunc) F(Union) F(Match)

#define MAP_ALL_EXPRS(F) F(BaseExpr) MAP_EXPRS(F)

namespace coral {
  using Type = type::Type;

  namespace ast {
    using std::vector;
    using std::unique_ptr;
    using std::map;

    // Forward-declare all Expr classes
#define RUN_EXPR(E) class E;
    MAP_ALL_EXPRS(RUN_EXPR)
#undef RUN_EXPR
    class BaseExprVisitor {
    public:
      virtual std::string visitorName() { return "ExprVisitor"; }
#define RUN_EXPR(E) virtual void visit(E *) = 0;
      MAP_ALL_EXPRS(RUN_EXPR)
#undef RUN_EXPR
    };

    // Visitor
    class ExprVisitor : public BaseExprVisitor {
    public:
      std::string visitorName() { return "ExprVisitor"; }
#define RUN_EXPR(E) virtual void visit(E *) { \
        fprintf(stderr, "%s: %s", visitorName().c_str(), #E "\n"); }
      MAP_ALL_EXPRS(RUN_EXPR)
#undef RUN_EXPR
    };

    class ActiveExprVisitor : public BaseExprVisitor {
    public:
#define RUN_EXPR(E) virtual void visit(E *);
      MAP_ALL_EXPRS(RUN_EXPR)
#undef RUN_EXPR
    };

    enum class ExprTypeKind {
#define RUN_EXPR(E) E##Kind,
      MAP_ALL_EXPRS(RUN_EXPR)
#undef RUN_EXPR
    };

    class BaseExpr {
    public:
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
      virtual ~BaseExpr() { }
    };

    class Value : public BaseExpr { };

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
#define F(E) virtual void visit(E *) { out = ExprTypeKind::E##Kind; }
      MAP_EXPRS(F)
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
#define F(E) virtual void visit(E *) { out = #E; }
      MAP_EXPRS(F)
#undef F
    };

    class Comment : public Statement {
    public:
      std::string value;
      Comment(std::string value) : value(value) { }
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
    };

    class Block : public Statement {
    public:
      std::vector<unique_ptr<coral::ast::BaseExpr>> lines;
      Block(std::vector<coral::ast::BaseExpr *> lines) {
        for(auto && ptr : lines) this->lines.push_back(unique_ptr<coral::ast::BaseExpr>(ptr));
      }
      BaseExpr * LastLine();
      virtual void accept(BaseExprVisitor * v) {
        v->visit(this);
      }
    };

    class Module : public BaseExpr {
    public:
      std::string name = "module";
      std::string path;
      unique_ptr<Block> body;
      Module();
      Module(vector<BaseExpr *> lines) {
        body = unique_ptr<Block>(new Block(lines));
      }
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
    };

    class Func : public Value {
    public:
      std::vector<std::string> container; // a namespace or container type
      std::string name;
      Tuple * tuple = 0;
      bool isInstanceMethod = false;
      std::unique_ptr<coral::Type> type;
      vector<unique_ptr<coral::ast::Def>> params;
      unique_ptr<Block> body;
      Func(std::vector<std::string> name,
           Type * type,
           vector<coral::ast::Def *> params,
           Block * body);
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
      std::string fullName() {
        std::string s;
        for(auto &t : container) s += t + ".";
        return s + name;
      }
    };

    class OverloadedFunc : public Value {
      public:
      std::vector<Func *> funcs;
      std::string name;
      OverloadedFunc(std::string name) : name(name) { }
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
      void addOverload(Func * f) { funcs.push_back(f); }
    };
    class IfExpr : public Value {
    public:
      unique_ptr<BaseExpr> cond, ifbody, elsebody;
      IfExpr(BaseExpr* cond,
             BaseExpr* ifbody,
             BaseExpr* elsebody)
        : cond(cond), ifbody(ifbody), elsebody(elsebody) { }
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
    };

    class ForExpr : public Value {
    public:
      unique_ptr<BaseExpr> var, sequence, body;
      ForExpr(BaseExpr* var, BaseExpr* sequence, BaseExpr* body)
        : var(var), sequence(sequence), body(body) { }
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
    };

    class Var : public Value {
    public:
      std::string name;
      // the declaring expression -- the node that created the name in question
      // in the current scope
      ast::BaseExpr * expr = 0;

      Var(std::vector<std::string> names) {
        name = names.size() ? names[0] : "undefined";
      }
      Var(std::string name) : name(name) { }
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
    };
    class StringLiteral : public Value {
    public:
      std::string value;
      StringLiteral(std::string value): value(value) { }
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
      std::string getString();
    };
    class IntLiteral : public Value {
    public:
      std::string value;
      IntLiteral(std::string value) : value(value) {  }
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
    };
    class FloatLiteral : public Value {
    public:
      std::string value;
      FloatLiteral(std::string value);
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
    };

    class BinOp : public Value {
    public:
      std::unique_ptr<BaseExpr> lhs, rhs;
      std::string op;
      ast::Func * funcptr = 0;
      BinOp(BaseExpr * lhs, std::string op, BaseExpr * rhs) : lhs(lhs), rhs(rhs), op(op) { }
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
    };

    class Let : public Statement {
    public:
      unique_ptr<Var> var;
      unique_ptr<BaseExpr> value;
      Type type;
      Let(Var * var, BaseExpr * value) : var(var), value(value), type("") { }
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
    };

    class Set : public Statement {
    public:
      unique_ptr<Var> var;
      unique_ptr<BaseExpr> value;
      Set(Var * var, BaseExpr * value) : var(var), value(value) { }
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
    };

    class Def : public Value {
    public:
      std::string name;
      std::unique_ptr<type::Type> type;
      BaseExpr * value;
      Def(std::string name, type::Type * t, BaseExpr * value) : name(name), type(t), value(value) { }
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
    };

    class Member : public Value {
    public:
      unique_ptr<BaseExpr> base = 0;
      std::string member;
      ast::Func * methodPtr = 0;
      int memberIndex = -1;
      Member(BaseExpr * base, std::string member) : base(base), member(member) { }
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
    };

    class ListLiteral : public Value {
    public:
      std::vector<unique_ptr<BaseExpr>> items;
      ListLiteral(std::vector<BaseExpr *> items) {
        for(auto && pp : items) if (pp) this->items.push_back(std::unique_ptr<BaseExpr>(pp));
      }
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
    };

    class TupleLiteral : public Value {
    public:
      std::vector<unique_ptr<BaseExpr>> items;
      std::unique_ptr<Type> type = 0;
      TupleLiteral(std::vector<BaseExpr *> items) {
        for(auto && pp : items) if (pp) this->items.push_back(std::unique_ptr<BaseExpr>(pp));
      }
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
    };

    class Call : public Value {
    public:
      unique_ptr<BaseExpr> callee;
      vector<std::unique_ptr<BaseExpr>> arguments;

      Call(BaseExpr * callee, TupleLiteral * arguments);
      Call(BaseExpr * callee, vector<BaseExpr *> arguments);
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
      void methodCallInvert();
    };


    class Return : public Statement {
    public:
      unique_ptr<BaseExpr> val;
      Return() : val((BaseExpr *)0) { }
      Return(BaseExpr * val) : val(val) { }
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
    };

    class Extern : public Statement {
    public:
      std::string name;
      std::string linkage;
      std::unique_ptr<coral::Type> type;
      Extern(std::string name, coral::Type * type) : name(name), type(type) { }
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
    };

    class Import : public Statement {
    public:
      std::vector<std::string> path;
      Import(std::vector<std::string> import_path) : path(import_path) { }
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
    };

    class While : public Statement {
    public:
      unique_ptr<BaseExpr> cond, body;
      While(BaseExpr* cond, BaseExpr * body) : cond(cond), body(body) { }
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
    };

    class Tuple : public BaseExpr {
    public:
      std::string name;
      unique_ptr<BaseExpr> body = 0;
      vector<unique_ptr<Def>> fields;
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }

      /* This is for the class-like declaration */
      Tuple(std::string name, Block * body): name(name), body(body) { }

      /* Untagged Tuples */
      Tuple(std::string name, std::vector<type::Type> fields);

      /* Tagged Tuples */
      Tuple(std::string name, std::vector<ast::Def *> fields);
    };

    class Union : public BaseExpr {
    public:
      std::string name;
      std::vector<std::unique_ptr<Def>> cases;
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
      Union(std::string name, ast::Block * block);
    };

    class MatchCase : public BaseExpr {
    public:
      vector<std::string> label;
      vector<unique_ptr<Def>> parameter;
      unique_ptr<Block> body = 0;
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
      MatchCase(vector<std::string> label, vector<Def *> def, Block * body)
        : label(label), body(body) {
        for(auto d: def)
          parameter.emplace_back(d);
      }
    };

    class Match : public BaseExpr {
    public:
      unique_ptr<ast::BaseExpr> condition;
      vector<unique_ptr<ast::MatchCase>> cases;
      virtual void accept(BaseExprVisitor * v) { v->visit(this); }
      Match(BaseExpr * condition, vector<MatchCase *> cases);
    };
    // used for defining tuples
    std::vector<Type> _defsToTypeArg(std::vector<Def *> defs);
  }
}
