/*
  core/expr contains all nodes of the syntax tree.
  depends on core/type because expressions have types
 */
#pragma once

#include "type.hh"
#include "utils.hh"
#include <vector>
#include <memory>
#include <string>
#include <iostream>

namespace coral {

#define EXPR_NODE_LIST(M) /* */ M(Expr) /* */ M(BinOp) /* */ M(Call) /* */ M(Index) /* */ M(Extern) /* */ M(String) /* */ M(Long) /* */ M(VoidExpr) /* */ M(BoolExpr) /* */ M(Double) /* */ M(Module) /* */ M(FuncDef) /* */ M(BlockExpr) /* */ M(Var) /* */ M(If) /* */ M(For) /* */ M(Return) /* */ M(Cast) /* */ M(Let) /* */ M(AddrOf) /* */ M(DeclClass) /* */ M(ImplType) /* */ M(ImplClassFor) /* */ M(DeclTypeEnum) /* */ M(DeclTypeAlias) /* */ M(MatchExpr) /* */ M(MatchCaseTagsExpr) /* */ M(Tuple) /* */ M(EnumCase) /* */ M(MatchEnumCaseExpr) /* */ M(Member) /* */ M(Struct) /* */ M(Set) /* */ M(MultiLet)

  enum ExprType {
#define TYPEDEC(EXPR) EXPR##Kind,
    EXPR_NODE_LIST(TYPEDEC)
#undef TYPEDEC
  };

  class ExprNotes {
  public:
	std::vector<std::string> messages;
	std::vector<Type *> types;
	Type* getBestType();

	void add(std::string msg);
	void isType(std::string type);
	void isType(Type * t);
	void returns(std::string type);
	void returns(Type * t);
	void isTypeOf(std::string expr);
	void isTypeOf(class Expr * e);
	void isNamed(std::string name);
	void merge(ExprNotes & o);
	void mergeReturn(ExprNotes & o);
  };

  class Expr {
  public:
	ExprNotes notes;
    virtual std::string toString() { return "[expr]"; }
    virtual void accept(class AbstractVisitor * v);
    virtual ~Expr() { }
    ExprType getType();
  };

  class Struct : public Expr {
  public:
	std::string name;
	std::vector<std::string> classParams;
	std::vector<Expr *> fields;
	std::vector<Expr *> methods;
	Struct(
	  std::string name,
	  std::vector<std::string> classParams,
	  std::vector<Expr *> lines);
    virtual std::string toString() { return "[struct]"; }
    virtual void accept(class AbstractVisitor * v);
  };

  class Call : public Expr {
  public:
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> arguments;
    Call(Expr * a, std::vector<Expr *> b) : callee(a) {
      foreach(b, it) arguments.push_back(std::unique_ptr<Expr>(*it));
    }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString() {
      auto v = callee->toString() + "(";
      for(auto i = arguments.begin(); i != arguments.end(); i++) {
	if (*i) v += (i != arguments.begin() ? ", " : "") + (*i)->toString();
	else v = v + (i != arguments.begin() ? ", " : "")  +  "(null)";
      }
      v += ")";
      return v;
    }
    ~Call() { } //for(auto i = arguments.begin(); i < arguments.end(); i++) { delete *i; }  }
  };

  class Member : public Expr {
  public:
    Expr * base;
    std::string memberName;
    Member(Expr * base, std::string memberName)
      : base(base), memberName(memberName) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString() { return base->toString() + "." + memberName; }
	~Member() { }
  };

  class Index : public Expr {
  public:
    Expr *base;
    std::vector <Expr *> indices;
    Index(Expr * base, std::vector<Expr *> indices) : base(base), indices(indices) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString() {
      auto v = base->toString() + "[";
      for(auto i = indices.begin(); i != indices.end(); i++) {
	v += (i != indices.begin() ? ", " : "");
	if (*i) v += (*i)->toString();
	else v += "(null)";
      }
      v += "]";
      return v;
    }
  };

  class BinOp : public Expr {
  public:
    std::string op;
    Expr * lhs, *rhs;
    BinOp(std::string oper, Expr * a, Expr * b) : op(oper), lhs(a), rhs(b) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString() {
      if (!lhs || !rhs)
	return (
	  "(" + (lhs ? lhs->toString() : "??") +
	  "|" + (rhs ? rhs->toString() : "??") +
	  "|" + op + ")"
	  );
      return "(" + lhs->toString() + op + rhs->toString() + ")";
    }
    int getPrecedence();
    int showParens(BinOp * outer);

    ~BinOp() { delete lhs; delete rhs; }
  };

  class Extern : public Expr {
  public :
    std::string linkage;
    std::string name;
    Type *type;
    Extern(std::string a, std::string b, Type * type) :
      linkage(a), name(b), type(type) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString() { return "extern " + linkage + " " + name; }
    ~Extern() { delete type; }
  };

  class String : public Expr {
  public :
    std::string value;
    String(std::string a);
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString();
  };

  class Var : public Expr {
  public :
    std::string name;
	// ref is the FuncDef/Let/Def/Extern that initially declared the var.
	Expr * ref = 0;
    Var(std::string a) : name(a) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString();
  };

  class Double : public Expr {
  public :
    double value;
    Double(double a) : value(a) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString();
  };

  class Long : public Expr {
  public :
    int64_t value;
    Long(int64_t a) : value(a) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString();
  };

  class Module : public Expr {
  public:
    std::string name;
    std::vector<Expr *> lines;
    Module(std::vector<Expr *> a) : name("module"), lines(a) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString();
    ~Module() {
      for(auto i = lines.begin(), e = lines.end(); i != e; i++) delete *i; }
  };

  class BlockExpr : public Expr {
  public:
    std::vector<Expr *>lines;
    BlockExpr(std::vector<Expr *>lines) : lines(lines) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString();
    ~BlockExpr() { for(auto i = lines.begin(), e = lines.end(); i != e; i++) delete *i; }
  };

  enum DefKindEnum { DefKind, TupleDefKind };
  class BaseDef : public Expr {
  public:
	DefKindEnum kind;
    virtual ~BaseDef() { }
    virtual std::string toString() {
	  return "oops";
	}
  };

  class Tuple : public Expr {
  public:
    std::vector<Expr *> items;
    Tuple(std::vector<Expr *> items) : items(items) { }
    virtual void accept(class AbstractVisitor * v);
  };

  class Def : public BaseDef {
  public:
    std::string name;
    Type * type;
    Def(std::string name, Type * t) : name(name), type(t) { kind = DefKind; }
    ~Def() { delete type; }
    virtual std::string toString() {
      return ((type && getTypeKind(type) != UnknownTypeKind) ? name + ": " + type->toString() : name);
	}
  };

  class TupleDef : public BaseDef {
  public:
    std::vector<BaseDef *> items;
    TupleDef(std::vector<BaseDef *> items) : items(items) { kind = TupleDefKind; }
    ~TupleDef() { foreach(items, it) if (*it) delete *it; }
    virtual std::string toString() {
      return "(" + join<BaseDef *>(", ", items, [] (BaseDef * d) { return d->toString(); }) + ")";
    }
  };

  class FuncDef : public Expr {
  public:
    std::string name;
    Type * rettype;
    std::vector<BaseDef *> args;
    Expr * body;
    bool multi;
    FuncDef(std::string name, Type * rettype,
	    std::vector<BaseDef *> args,
	    Expr * body,
	    bool multi) :
      name(name), rettype(rettype), args(args), body(body), multi(multi) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString();
    ~FuncDef() {
      if (body) { delete body; }
      // if (rettype) { delete rettype; }
      for(auto i = args.begin(), e = args.end(); i != e; i++) if (*i) { delete *i; }
    }
  };

  class Cast : public Expr {
  public:
    Expr * expr;
    Type * to_type;
    Cast(Expr * expr, Type * to_type) : expr(expr), to_type(to_type) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString();
    ~Cast() { delete expr; delete to_type; }
  };

  class If : public Expr {
  public:
	bool ifterminated = 0;
	bool elseterminated = 0;
    Expr * cond;
    BlockExpr *ifbody, *elsebody;
    If(Expr * cond, Expr * ifbody, Expr * elsebody) :
      cond(cond), ifbody((BlockExpr *)ifbody), elsebody((BlockExpr *)elsebody) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString();
    ~If() { delete cond; delete ifbody; delete elsebody; }
  };

  class For : public Expr {
  public:
    std::vector<BaseDef *> var;
    Expr *source, *body;
    For(std::vector<BaseDef *> var, Expr * source, Expr * body) : var(var), source(source), body(body) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString() { return "for"; }
    ~For() { delete source; delete body; }
  };

  class MultiLet : public Expr {
  public:
	TupleDef * var;
	// While we should be able to destructure tuples at compile time,
    // this isn't a Tuple * because we can destructure lists, arrays, etc. at runtime
	Expr * val;
	MultiLet(TupleDef * var, Expr * val) : var(var), val(val) { }
    virtual void accept(class AbstractVisitor * v);
    ~MultiLet() { if (var) delete var; if (val) delete val; }
  };

  class Let : public Expr {
  public:
    Def * var;
    Expr * value;
    Let(Def * var, Expr * value) : var(var), value(value) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString();
    ~Let() { delete var; delete value; }
  };

  class Set : public Expr {
  public:
    std::vector<BaseDef *> tuplevar;
    Expr * value;
	Expr * var = 0;
    Set(Var * var, Expr * value) : value(value), var(var) { }
	Set(Member * var, Expr * value) : value(value), var(var) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString();
    ~Set() { delete var; delete value; }
  };

  class AddrOf : public Expr {
  public:
    std::string var;
    AddrOf(std::string var) : var(var) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString();
  };

  class Return : public Expr {
  public:
    Expr * value;
    Return(Expr * value) : value(value) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString();
    ~Return() { delete value; }
  };

  class DeclTypeAlias : public Expr {
  public:
    std::string name;
    Type * wrapped;
    DeclTypeAlias(std::string name, Type * wrapped)
      : name(name), wrapped(wrapped) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString() { return "type-alias"; }
    ~DeclTypeAlias() { delete wrapped; }
  };

  class DeclTypeEnum : public Expr {
  public:
    std::string name;
    std::vector<Expr *> body;
    DeclTypeEnum(
      std::string name,
	  __attribute__((unused)) std::string unused,
      std::vector<Expr *> body
      ) : name(name), body(body) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString() { return "type-enum"; }
    ~DeclTypeEnum() { }
  };

  class EnumCase : public Expr {
  public:
    std::string name;
    std::vector<BaseDef *> defs;
    EnumCase(std::string name) : name(name) { }
    EnumCase(std::string name, std::vector<BaseDef *> defs)
      :name(name), defs(defs) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString() { return "enum-case"; }
  };

  class MatchExpr : public Expr {
  public:
    Expr * cond;
    std::vector<Expr *> cases;
    MatchExpr(Expr * cond, std::vector<Expr *> cases)
      : cond(cond), cases(cases) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString() { return "enum-case"; }
  };

  class MatchEnumCaseExpr : public EnumCase {
  public:
    MatchEnumCaseExpr(std::string name) : EnumCase(name) { }
    MatchEnumCaseExpr(std::string name, std::vector<BaseDef *> defs)
      : EnumCase(name, defs) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString() { return "match-enum-case"; }
  };

  class MatchCaseTagsExpr : public Expr {
  public:
    Expr * label;
    Expr * body;
    MatchCaseTagsExpr(Expr * label, Expr * body)
      : label(label), body(body) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString() { return "match-case"; }
    ~MatchCaseTagsExpr() { delete label; delete body; }
  };

  class DeclClass : public Expr {
  public:
    std::string name;
    std::vector<Def *> lines;
    DeclClass(std::string name, std::vector<Def *> lines)
      : name(name), lines(lines) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString() { return "class-def-" + name; }
  };

  class ImplClassFor : public Expr {
  public:
    std::string type_name;
    std::string class_name;
    Expr * body;
    ImplClassFor(
      std::string classname,
      std::string name,
      Expr * body)
      : type_name(name), class_name(classname), body(body) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString() {
      return "impl-class-for-" + type_name + "-" + class_name; }
  };

  class ImplType : public Expr {
  public:
    std::string name;
    Expr * body;
    ImplType(std::string name, Expr * body)
      : name(name), body(body) { }
    virtual void accept(class AbstractVisitor * v);
    virtual std::string toString() { return "impl-" + name; }
  };

  class VoidExpr : public Expr {
  public:
    VoidExpr() { }
    virtual void accept(class AbstractVisitor * v);
  };

  class BoolExpr : public Expr {
  public:
    bool value;
    BoolExpr(bool value) : value(value) { }
    virtual void accept(class AbstractVisitor * v);
  };

  // the base visitor. By Default all visits just write to stderr.
  class AbstractVisitor {
  public:
#define VISIT(NODE) virtual void visit(NODE * c) = 0;
    EXPR_NODE_LIST(VISIT)
#undef VISIT
    virtual ~AbstractVisitor () { }
  };

  // By Default all visits just write to stderr.
  class Visitor : public AbstractVisitor {
  public:
    std::string visitorName;
    Visitor(std::string name) : visitorName(name) { }
#define VISIT(NODE) virtual void visit(__attribute__((unused)) NODE * c) { std::cerr << "\033[1;35m" << visitorName << "visit: " << #NODE << "\033[0m\n"; }
    EXPR_NODE_LIST(VISIT)
#undef VISIT
    virtual ~Visitor () { }
  };

// useful visitors go here

#define EXPRNAME(t) ExprNameVisitor(t).out
  class ExprNameVisitor : public Visitor {
  public:
    std::string out;
    ExprNameVisitor(Expr * e) : Visitor("exprname ") { e->accept(this); }
#define VISIT(NODE) virtual void visit(__attribute__((unused)) NODE * c) { out = #NODE; }
    EXPR_NODE_LIST(VISIT)
#undef VISIT
  };

#define EXPRTYPE(t) ExprTypeVisitor(t).out
  class ExprTypeVisitor : public Visitor {
  public:
    enum ExprType out = ExprKind;
    ExprTypeVisitor(Expr * e) : Visitor("exprtype ") { e->accept(this); }
#define VISIT(NODE) virtual void visit(__attribute__((unused)) NODE * c) { out = NODE##Kind; }
    EXPR_NODE_LIST(VISIT)
#undef VISIT
  };


  class NameGetter : public Visitor {
  public:
    std::string out;
    NameGetter() : Visitor("nameget ") { }
    void visit(Var * v) { out = v->name; }
    void visit(__attribute__((unused)) Expr * e) { out = ""; }
  };

  std::string getName(Expr * e);

  FuncDef* BuildFunc(std::string name, Type* functype, std::vector<BaseDef *> params, Expr * body);

  FuncDef* BuildVarFunc(std::string name, Type* functype, std::vector<BaseDef *> params, Expr * body);

}
