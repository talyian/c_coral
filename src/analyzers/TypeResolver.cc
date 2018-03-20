#include "utils/ansicolor.hh"
#include "utils/opts.hh"
#include "core/expr.hh"
#include "core/prettyprinter.hh"
#include "analyzers/TypeResolver.hh"

#include <iostream>
#include <iomanip>
#include <set>
#include <algorithm>

#include "typegraph/typegraph.hh"
#include "analyzers/TypeResultWriter.hh"

using namespace typegraph;

namespace typegraph {
  extern bool showSteps;
}

typegraph::Type * typeconvert(TypeGraph * gg, coral::type::Type * ct) {
  auto t = gg->type(ct->name);
  for(auto &p : ct->params)
    t->params.push_back(typeconvert(gg, &p));
  return t;
}

coral::type::Type * typeRevConvert(typegraph::Type * ct) {
  auto t = new coral::type::Type(ct->name);
  for(auto &p : ct->params) {
    t->params.push_back(*typeRevConvert(dynamic_cast<typegraph::Type *>(p)));
  }
  return t;
}

coral::analyzers::TypeResolver::TypeResolver(ast::Module * m): module(m) {
  m->accept(this);
  if (coral::opt::ShowTypeSolution) gg.show();
  typegraph::showSteps = coral::opt::ShowTypeSolution;
  auto solution = gg.solve();
  if (coral::opt::ShowTypeSolution) gg.show();
  std::vector<std::pair<coral::ast::BaseExpr *, typegraph::Type *>> expr_terms;
  for(auto &pair: solution.allKnownTypes()) {
    // std::cerr << "|  " << std::setw(40) << pair.first << "\t" << pair.second << "\n";
    if (pair.first)
      expr_terms.push_back(std::make_pair((coral::ast::BaseExpr*)pair.first->expr, pair.second));
  }
  TypeResultWriter::write(&gg, expr_terms);
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
    // auto func = dynamic_cast<coral::ast::Func *>((coral::ast::BaseExpr *)calleevar->expr);
    // if (func && func->params.size() && func->params[0]->name == "self") {
    //   args.insert(args.begin(),
    // }
    out = gg.addTerm("call." + calleevar->name, call);
    gg.constrain(out, gg.call(gg.term(calleevar), args));
    // HACK: we duplicate this to force one of  them to be kept
    // even after return type inference removes the other one
    out = gg.addTerm("call." + calleevar->name, call);
    gg.constrain(out, gg.call(gg.term(calleevar), args));
  }
}

void coral::analyzers::TypeResolver::visit(ast::Return * r) {
  if (r->val) r->val->accept(this);
}

void coral::analyzers::TypeResolver::visit(ast::StringLiteral * s) {
  out = gg.addTerm("str." + s->value, s);
  gg.constrain(out, gg.type("Ptr"));
}

void coral::analyzers::TypeResolver::visit(ast::Let * l) {
  l->value->accept(this);
  auto valueterm = out;
  out = gg.addTerm(l->var->name, l);
  if (l->type.name != "")
    gg.constrain(out, (typeconvert(&gg, &(l->type))));
  gg.constrain(out, gg.term(valueterm));
}

void coral::analyzers::TypeResolver::visit(ast::While * w) {
  w->body->accept(this);
  w->cond->accept(this);
  gg.constrain(out, gg.type("Bool"));
  out = gg.addTerm("while", w);
}

void coral::analyzers::TypeResolver::visit(ast::Set * s) {
  s->value->accept(this);
  auto valueterm = out;
  auto term = gg.findTerm(s->var->expr);
  if (term)
    gg.constrain(term, gg.term(valueterm));
  else
    std::cerr << "Warning: term not found for set: " << s->var->name << "\n";
  out = gg.addTerm("set", s);
}

void coral::analyzers::TypeResolver::visit(ast::Extern * e) {
  out = gg.addTerm(e->name, e);
  gg.constrain(out, typeconvert(&gg, e->type.get()));
}

void coral::analyzers::TypeResolver::visit(ast::Var * var) {
  if (var->name == "negate") {
    out = gg.addTerm(var->name, var);
    auto free = gg.free(0);
    gg.constrain(out, gg.type("Func", {free, free}));
  }
  else if (var->name == "addrof") {
    out = gg.addTerm(var->name, var);
    auto free = gg.free(101);
    gg.constrain(out, gg.type("Func", {free, gg.type("Ptr", {free})}));
  }
  else if (var->name == "deref") {
    out = gg.addTerm(var->name, var);
    auto free = gg.free(50);
    gg.constrain(out, gg.type("Func", {gg.type("Ptr", {free}), free}));
  }
  else if (var->name == "derefi") {
    out = gg.addTerm(var->name, var);
    gg.constrain(out, gg.type("Func", {gg.type("Ptr"), gg.type("Int32")}));
  }
  else if (var->name == "int64") {
    out = gg.addTerm(var->name, var);
    gg.constrain(out, gg.type("Func", {gg.type("Int32"), gg.type("Int64")}));
  }
  else if (var->name == "ptr") {
    out = gg.addTerm(var->name, var);
    gg.constrain(out, gg.type("Func", {gg.type("Int32"), gg.type("Ptr")}));
  }
  else if (!var->expr) {
    if (var->name.substr(0, 10) != "_llvmBuild") {
      PrettyPrinter::print(module);
      std::cerr << "\033[31mType Resolver: Undefined Reference " << var->name << "\033[0m\n";
      exit(2);
    }
  } else {
    if ((out = gg.findTerm(var->expr))) {
      std::cerr << COL_LIGHT_YELLOW << "resolving term " << var->name << " as reference\033[0m\n";
      return;
    }
    // the expr might not be actually in the Module ifself -- this happens, for example,
    // if we're adding an OverloadedFunc expression
    if (ast::ExprTypeVisitor::of(var->expr) == ast::ExprTypeKind::OverloadedFuncKind) {
      auto overloaded_func = dynamic_cast<ast::OverloadedFunc*>(var->expr);
      auto params = std::vector<typegraph::Constraint *>();
      for(auto &func: overloaded_func->funcs)
        params.push_back(gg.term(gg.findTerm(func)));
      out = gg.addTerm(overloaded_func->name, overloaded_func);
      gg.constrain(out, gg.type("Or", params));
      return;
    }
    else
      std::cerr
        << "\033[31mMissing Type Term " << var->name
        << ":" << var->expr << "\033[0m\n";
  }
  return;
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
  out = gg.addTerm(d->name, d);
  if (d->type)
    gg.constrain(out, typeconvert(&gg, d->type.get()));
}
void coral::analyzers::TypeResolver::visit(ast::Func * f) {
  out = 0;
  auto name = f->name;
  for(auto &part : f->container)
    name = part + "::" + name;
  auto func_name = name;
  auto term = gg.addTerm(func_name, f);

  auto constraint = gg.type(f->isInstanceMethod ? "Method" : "Func");

  term_map[term] = f;
  for(auto &p: f->params) {
    p->accept(this);
    constraint->params.push_back(gg.term(out));
  }
  // return type
  if (f->body) {
    f->body->accept(this);
    constraint->params.push_back(gg.term(out));
  }
  else if (f->type) {
    auto t = typeconvert(&gg, f->type.get());
    constraint->params = t->params;
  } else {
    constraint->params.push_back(gg.free(100));
  }

  gg.constrain(term, constraint);
  out = term;
}

void coral::analyzers::TypeResolver::visit(ast::IfExpr * ifexpr) {
  ifexpr->cond->accept(this);
  gg.constrain(out, gg.type("Bool"));
  auto term = gg.addTerm("if", ifexpr);
  ifexpr->ifbody->accept(this);
  gg.constrain(term, gg.term(out));
  ifexpr->elsebody->accept(this);
  gg.constrain(term, gg.term(out));
  out = term;
}

void coral::analyzers::TypeResolver::visit(ast::BinOp * op) {
  op->lhs->accept(this);
  auto lvar = out;
  op->rhs->accept(this);
  auto rvar = out;

  out = gg.addTerm("op." + op->op, op);

  if (op->op == "%" ||
      op->op == "+" ||
      op->op == "-" ||
      op->op == "*" ||
      op->op == "/")
    gg.constrain(
      out, gg.call(
        gg.type("Func", {gg.free(0), gg.free(0), gg.free(0)}),
        {gg.term(lvar), gg.term(rvar)}));
  else if (op->op == "=")
    gg.constrain(
      out, gg.call(
        gg.type("Func", {gg.free(0), gg.free(0), gg.type("Bool")}),
        {gg.term(lvar), gg.term(rvar)}));
}

void coral::analyzers::TypeResolver::visit(ast::IntLiteral * op) {
  out = gg.addTerm("i" + (op->value), op);
  gg.constrain(out, gg.type("Int32"));
}

void coral::analyzers::TypeResolver::visit(ast::FloatLiteral * op) {
  out = gg.addTerm("f" + (op->value), op);
  gg.constrain(out, gg.type("Float64"));
}

void coral::analyzers::TypeResolver::visit(ast::Union * u) {
  auto union_term = gg.addTerm(u->name, u);
  int i = 0;
  auto union_name = union_term->name;
  for(auto &c : u->cases) {
    out = gg.addTerm(union_name + "::" + c->name, 0);
    auto case_type = typeconvert(&gg, c->type.get());
    auto constructor_type = gg.type("Func", {case_type, gg.type(union_term->name)});
    gg.constrain(out, constructor_type);
    auto indexer = gg.addTerm(union_name + "::" + c->name + ".index", 0);
    gg.constrain(indexer, gg.type(std::to_string(i++)));
  }
  gg.constrain(union_term, gg.type("@Union", { gg.type(union_name) }));
  out = union_term;
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
    // std::cerr << COL_LIGHT_BLUE << field->name << "(" << field->name.empty() << ")\033[0m\n";
    Type type = *(field->type);
    std::string name = field->name.empty() ? "Item" + std::to_string(index) : field->name;

    auto field_term = gg.addTerm(t->name + "::" + name, 0);
    gg.constrain(field_term, typeconvert(&gg, &type));

    auto field_term_index = gg.addTerm(t->name + "::" + name + ".index", 0);
    gg.constrain(field_term_index, gg.type(std::to_string(index)));
    func_args.push_back(typeconvert(&gg, &type));
  }
  // auto type = gg.addTerm(t->name, 0);
  func_args.push_back(gg.type(t->name));

  auto constructor = gg.addTerm(t->name + ".@constructor", 0);
  gg.constrain(constructor, gg.type("Func", func_args));

  out = gg.addTerm(t->name, t);
  gg.constrain(out, gg.type("Type", {gg.term(constructor)}));
}

void coral::analyzers::TypeResolver::visit(ast::Comment *) { out = 0; }

void coral::analyzers::TypeResolver::visit(ast::Member * m) {
  m->base->accept(this);
  auto basetype = out;
  auto memberpath = m->member;

  auto member = gg.type("Member", {gg.type(memberpath)});
  // HACK: add this twice to avoid function call unification
  // from eagerly stealing a term from member index calculation
  for(auto i = 0; i < 2; i++) {
    out = gg.addTerm(basetype->name + "." + memberpath, m);
    auto constraint = gg.call(member, {gg.term(basetype), gg.term(out)});
    gg.constrain(out, constraint);
  }
}

void coral::analyzers::TypeResolver::visit(ast::TupleLiteral * tuple) {
  std::vector<Constraint *> terms;
  for(auto &item: tuple->items) {
    item->accept(this);
    terms.push_back(gg.term(out));
  }
  out = gg.addTerm("tuple", tuple);
  gg.constrain(out, gg.type("Tuple", terms));
}

void coral::analyzers::TypeResolver::visit(ast::Match * match) {
  for(auto &match_case : match->cases) {
    auto case_term = gg.addTerm(
      "match." + match_case->label->name + "." + match_case->def->name,
      match_case->def.get());
    gg.constrain(case_term, typeconvert(&gg, match_case->def->type.get()));
    match_case->body->accept(this);
  }
  out = 0;
}
