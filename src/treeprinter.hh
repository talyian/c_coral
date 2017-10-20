#pragma once

#include "ast.hh"
#include <iostream>
#include <string>

using std::string;
using std::cerr;
using std::endl;

class TreePrinter : public Visitor {
public:
  Module * module;
  std::ostream & out;
  int indent = 0;
  int line_mode = 0;
  TreePrinter(Module * m, std::ostream & c) : module(m), out(c) { }
  void print() {
    module->accept(this);
  }
  void visit(Return * r) {
    out << IND() << "return ";
    auto t = line_mode;
    line_mode = 1;
    r->value->accept(this);
    line_mode = t;
    out << END();
  }
  void visit(Expr * e) { out << IND() << "# expr: " << EXPRNAME(e) << END(); }
  void visit(Index * i) {
    out << IND();
    auto t = line_mode;
    line_mode = 1;
    i->base->accept(this);
    out << '[';
    foreach(i->indices, it) {
      if (it != i->indices.begin()) { out << ", "; }
      (*it)->accept(this); }
    line_mode = t;    
    out << ']' << END();
    
  }
  void visit(For * f) {
    out << IND() << "for ";
    auto t = line_mode;
    line_mode = 1;
    foreach(f->var, it) {
      visit(*it);
    }
    out << " in ";
    f->source->accept(this);
    line_mode = t;
    out << ":" << END();
    indent++;
    f->body->accept(this);
    indent--;
  }
  void visit(Module * m) {
    foreach (module->lines, line) {
      (*line)->accept(this);
    }
  }

  void visit(FuncDef * m) {
    TreePrinter pp(module, out);
    pp.line_mode = 1;
    if (m->rettype != 0)
      out << IND() << "func " << m->name << " : " << m->rettype->toString();
    else 
      out << IND() << "func " << m->name;
    out << "(";
    foreach(m->args, arg) {
      if (arg != m->args.begin()) out << ", ";      
      pp.visit(*arg);
    }
    out << ")";
    out << ":\n";
    indent++;
    m->body->accept(this);
    indent--;
    out << END();
  }

  void visit(BlockExpr * e) {
    auto lines = e->lines;
    foreach(lines, it) {
      (*it)->accept(this);
    }
  }

  void visit(BinOp * op) {
    out << IND();
    TreePrinter pp(module, out);
    pp.line_mode = 1;
    out << "(";
    op->lhs->accept(&pp);
    out << " " << op->op << " ";
    op->rhs->accept(&pp);
    out << ")";    
    out << END();
  }
  
  void visit(If * e) {
    TreePrinter pp(module, out);
    pp.line_mode = 1;
    out << IND() << "if ";
    e->cond->accept(&pp);
    out << ":\n";
    indent++;
    e->ifbody->accept(this);
    indent--;
    out << IND() << "else:\n";
    indent++;
    e->elsebody->accept(this);
    indent--;
  }

  void visit(MatchCaseTagsExpr * e) {
    TreePrinter pp(module, out);
    pp.line_mode = 1;
    out << IND();
    e->label->accept(&pp);
    out << ":" << END();
    indent++;
    e->body->accept(this);
    indent--;
  }
  
  void visit(MatchExpr * e) {
    TreePrinter pp(module, out);
    pp.line_mode = 1;
    out << IND() << "match ";
    e->cond->accept(&pp);
    out << ":\n";
    indent++;
    foreach(e->cases, c) (*c)->accept(this);
    indent--;    
  }

  void visit(DeclClass * a) {
    out << IND() << "class " << a->name << ":" << END();
    indent++;
    foreach(a->lines, line) {
      visit(*line);
    }
    indent--;
    out << END();
  }
  
  void visit(DeclTypeAlias * a) {
    out << IND() << "type " << a->name << " = " << a->wrapped->toString() << END();
    out << END();    
  }
  void visit(DeclTypeEnum * a) {
    out << IND() << "type " << a->name << ":" << END();
    indent++;
    foreach(a->body, it) {
      Expr * line = *it;
      if (line) line->accept(this);
      else out << IND() << "(expr?)\n";
    }
    indent--;
    out << END();    
  }
  void visit(EnumCase * e) {
    TreePrinter lp(module, out);
    lp.line_mode = 1;
    out << IND() << e->name;
    if (e->defs.size()) {
      out << "(";
      foreach(e->defs, def) lp.visit(*def);
      out << ")";
    }
    out << END();
  }
  void visit(Call * c) {
    out << IND();
    TreePrinter lp(module, out);
    lp.line_mode = 1;
    c->callee->accept(&lp);
    if (c->arguments.size() == 0) out << "()";
    else if (c->arguments.size() == 1) {
      out << '(';      
      c->arguments[0]->accept(&lp);
      out << ')';      
    } else {
      out << '(';
      foreach(c->arguments, it) {
	if (it != c->arguments.begin())	out << ", ";
	(*it)->accept(&lp);
      }
      out << ')';
    }
    out << END();
  }

  void visit(ImplType * a) {
    out << IND() << "impl " << a->name << ":" << END();
    indent++;
    a->body->accept(this);
    indent--;
  }
  
  void visit(ImplClassFor * a) {
    out << IND()
	<< "impl " << a->class_name
        << " for " << a->type_name
	<< ":" << END();
    indent++;
    a->body->accept(this);
    indent--;
  }

  void visit(Extern * e) {
    out << IND() << "extern " << e->name << " : " << e->type << END();
  }
  void visit(Def * d) {
    out << IND() << d->name;
    if (d->type && getTypeName(d->type) != "Unknown")
      out << " : " << d->type->toString();
    out << END();
  }

  void visit(Cast * c) {
    out << IND() << "(";
    c->expr->accept(this);
    out << " as " << c->to_type << ")" << END();
  }
  void visit(Let * d) {
    TreePrinter lp(module, out);
    lp.line_mode = 1;
    out << IND() << "let ";
    lp.visit(d->var);
    out << " = ";
    d->value->accept(&lp);
    out << END();
  }
  
  void visit(Long * d) { out << IND() << d->value << END(); }

  void visit(String * d) { out << IND() << d->value << END(); }  

  void visit(Var * d) {  out << IND() << d->value << END(); }
  
  std::string END() {
    if (line_mode) return "";
    return "\n";
  }
  std::string IND() {
    if (line_mode) return "";
    return std::string(indent * 2, ' ');
  }
};
