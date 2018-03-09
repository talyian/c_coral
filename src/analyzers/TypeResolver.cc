#include "utils/ansicolor.hh"
#include "utils/opts.hh"
#include "core/expr.hh"
#include "core/prettyprinter.hh"
#include "analyzers/TypeResolver.hh"

#include <iostream>
#include <iomanip>
#include <set>
#include <algorithm>

#include "analyzers/typegraph/TypeGraph.hh"
#include "analyzers/TypeResultWriter.hh"

coral::analyzers::TypeResolver::TypeResolver(ast::Module * m): module(m) {
  m->accept(this);
  if (coral::opt::ShowTypeSolution) gg.Show("module");
  gg.Step();
  if (coral::opt::ShowTypeSolution) gg.Show("module");
  std::map<ast::BaseExpr *, ::Type *> translation;
  for(auto &pair : gg.expr_terms)
    translation[pair.first] = gg.GetTypeConstraintForTerm(pair.second);
  TypeResultWriter::write(translation);
}


void coral::analyzers::TypeResolver::visit(ast::Call * call) {
  out = 0;
  call->callee->accept(this);
  auto calleevar = out;
  std::vector<Constraint *> args;
  for(auto &a: call->arguments) {
    a->accept(this);
    args.push_back(gg.term(out));
  }
  if (calleevar) {
    out = gg.AddTerm("call." + calleevar->name, call);
    gg.AddConstraint(out, gg.call(gg.term(calleevar), args));
  }
}

void coral::analyzers::TypeResolver::visit(ast::Return * r) {
  r->val->accept(this);
}

void coral::analyzers::TypeResolver::visit(ast::StringLiteral * s) {
  out = gg.AddTerm("str." + s->value, s);
  gg.AddConstraint(out, gg.type("Ptr"));
}

void coral::analyzers::TypeResolver::visit(ast::Let * l) {
  l->value->accept(this);
  auto valueterm = out;
  out = gg.AddTerm(l->var->name, l);
  if (l->type.name.size())
    gg.AddConstraint(out, gg.type(&l->type));
  gg.AddConstraint(out, gg.term(valueterm));
}

void coral::analyzers::TypeResolver::visit(ast::While * w) {
  w->body->accept(this);
  w->cond->accept(this);
  gg.AddConstraint(out, gg.type("Bool"));
  out = gg.AddTerm("while", w);
}

void coral::analyzers::TypeResolver::visit(ast::Set * s) {
  s->value->accept(this);
  auto valueterm = out;
  auto term = gg.FindTerm(s->var->expr);
  if (term)
    gg.AddConstraint(term, gg.term(valueterm));
  else
    std::cerr << "Warning: term not found for set: " << s->var->name << "\n";
  out = gg.AddTerm("set", s);
}

void coral::analyzers::TypeResolver::visit(ast::Extern * e) {
  out = gg.AddTerm(e->name, e);
  gg.AddConstraint(out, gg.type(e->type.get()));
}

void coral::analyzers::TypeResolver::visit(ast::Var * var) {
  if (var->name == "negate") {
    out = gg.AddTerm(var->name, var);
    auto free = gg.free(0);
    gg.AddConstraint(out, gg.type("Func", {free, free}));
  }
  else if (var->name == "addrof") {
    out = gg.AddTerm(var->name, var);
    auto free = gg.free(101);
    gg.AddConstraint(out, gg.type("Func", {free, gg.type("Ptr", {free})}));
  }
  else if (var->name == "derefi") {
    out = gg.AddTerm(var->name, var);
    gg.AddConstraint(out, gg.type("Func", {gg.type("Ptr"), gg.type("Int32")}));
  }
  else if (var->name == "int64") {
    out = gg.AddTerm(var->name, var);
    gg.AddConstraint(out, gg.type("Func", {gg.type("Int32"), gg.type("Int64")}));
  }
  else if (var->name == "ptr") {
    out = gg.AddTerm(var->name, var);
    gg.AddConstraint(out, gg.type("Func", {gg.type("Int32"), gg.type("Ptr")}));
  }
  else if (!var->expr)
    std::cerr << "Undefined Reference " << var->name << "\n";
  else if ((out = gg.FindTerm(var->expr)))
    return;
  else
    std::cerr << "Missing Type Term " << var->name << ":" << var->expr << "\n";

}


void coral::analyzers::TypeResolver::visit(ast::Module * m) { m->body->accept(this); }

void coral::analyzers::TypeResolver::visit(ast::Block * m) {
  TypeTerm * lastvalid = 0;
  for(auto &line : m->lines)
    if (line) {
      line->accept(this);
      if (out) lastvalid = out;
    }
  out = lastvalid;
}
void coral::analyzers::TypeResolver::visit(ast::Def * d) {
  out = gg.AddTerm(d->name, d);
  if (d->type)
    gg.AddConstraint(out, gg.type(d->type.get()));
}
void coral::analyzers::TypeResolver::visit(ast::Func * f) {
  auto func_name = f->tuple ? f->tuple->name + "::" + f->name : f->name;
  auto term = gg.AddTerm(func_name, f);
  auto constraint = gg.type("Func");
  term_map[term] = f;
  for(auto &p: f->params) {
    p->accept(this);
    constraint->params.push_back(gg.term(out));
  }
  if (f->body) {
    f->body->accept(this);
    constraint->params.push_back(gg.term(out));
  }
  else if (f->type) {
    auto t = gg.type(f->type.get());
    constraint->params = t->params;
  } else {
    constraint->params.push_back(gg.free(100));
  }

  gg.AddConstraint(term, constraint);
  out = term;
}

void coral::analyzers::TypeResolver::visit(ast::IfExpr * ifexpr) {
  ifexpr->cond->accept(this);
  gg.AddConstraint(out, gg.type("Bool"));
  auto term = gg.AddTerm("if");
  ifexpr->ifbody->accept(this);
  gg.AddConstraint(term, gg.term(out));
  ifexpr->elsebody->accept(this);
  gg.AddConstraint(term, gg.term(out));
  out = term;
}

void coral::analyzers::TypeResolver::visit(ast::BinOp * op) {
  op->lhs->accept(this);
  auto lvar = out;
  op->rhs->accept(this);
  auto rvar = out;

  out = gg.AddTerm("op." + op->op);

  if (op->op == "%" ||
      op->op == "+" ||
      op->op == "-" ||
      op->op == "*" ||
      op->op == "/")
    gg.AddConstraint(
      out, gg.call(
        gg.type("Func", {gg.free(0), gg.free(0), gg.free(0)}),
        {gg.term(lvar), gg.term(rvar)}));
  else if (op->op == "=")
    gg.AddConstraint(
      out, gg.call(
        gg.type("Func", {gg.free(0), gg.free(0), gg.type("Bool")}),
        {gg.term(lvar), gg.term(rvar)}));
}

void coral::analyzers::TypeResolver::visit(ast::IntLiteral * op) {
  out = gg.AddTerm("i" + (op->value), op);
  gg.AddConstraint(out, gg.type("Int32"));
}

void coral::analyzers::TypeResolver::visit(ast::FloatLiteral * op) {
  out = gg.AddTerm("f" + (op->value), op);
  gg.AddConstraint(out, gg.type("Float64"));
}

void coral::analyzers::TypeResolver::visit(ast::Tuple * t) {
  // TODO: the name corresponds to both the tuple
  // and the constructor? We probably need to generate different AST
  // nodes for constructor type access
  std::vector<Constraint *> func_fields;
  std::vector<Constraint *> func_args;
  int index = -1;
  for(auto &field: t->fields) {
    index++;
    auto field_info = field->type.get();

    std::string name;
    Type type("");
    if (field_info->name == "Field") {
      type = field_info->params[1];
      name = field_info->params[0].name;
    }else {
      type = *field_info;
      name = "Item" + std::to_string(index);
    }

    auto field_term = gg.AddTerm(t->name + "::" + name, 0);
    gg.AddConstraint(field_term, gg.type(&type));

    auto field_term_index = gg.AddTerm(t->name + "::" + name + ".index", 0);
    gg.AddConstraint(field_term_index, gg.type(std::to_string(index)));
    func_args.push_back(gg.type(&type));
  }
  auto type = gg.AddTerm(t->name, 0);
  func_args.push_back(gg.type(t->name));
  out = gg.AddTerm(t->name + ".new", t);
  gg.AddConstraint(out, gg.type("Func", func_args));
}

void coral::analyzers::TypeResolver::visit(ast::Comment *) { out = 0; }

void coral::analyzers::TypeResolver::visit(ast::Member * m) {
  m->base->accept(this);
  auto basetype = out;
  auto memberpath = m->member;
  out = gg.AddTerm(basetype->name + "." + memberpath, m);
  gg.AddConstraint(
    out,
    gg.call(
      gg.type("Member", {gg.type(memberpath)}),
      {gg.term(basetype)}));
}

void coral::analyzers::TypeResolver::visit(ast::TupleLiteral * tuple) {
  std::vector<Constraint *> terms;
  for(auto &item: tuple->items) {
    item->accept(this);
    terms.push_back(gg.term(out));
  }
  out = gg.AddTerm("tuple", tuple);
  gg.AddConstraint(out, gg.type("Tuple", terms));
}
