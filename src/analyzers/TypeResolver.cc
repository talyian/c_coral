#include "utils/ansicolor.hh"
#include "core/expr.hh"
#include "core/prettyprinter.hh"
#include "TypeResolver.hh"

#include <iostream>
#include <iomanip>

using namespace coral;

void analyzers::TypeResolver::visit(ast::Module * m) {
  m->body->accept(this);
}
void analyzers::TypeResolver::visit(ast::Block * m) {
  for(auto && line : m->lines) if (line) line->accept(this);
  info[m].expr = m;
  info[m].type = info[m->LastLine()].type;
  // std::cerr << "getting last expressoin " << ast::ExprNameVisitor::of(m->LastLine()) << "\n";
}
void analyzers::TypeResolver::visit(ast::Comment * m) {
  // no type, not even Void Type
  info[m].expr = m;
  info[m].type.name = "";
  // std::swap(info[m].type, 0);
  // info[m].type.name = "asdf";
}

void analyzers::TypeResolver::visit(ast::IfExpr * m) {
  m->cond->accept(this);
  auto t1 = info[m->cond.get()];
  m->ifbody->accept(this);
  auto t2 = info[m->ifbody.get()];
  TypeInfo t3;
  if (m->elsebody) {
	m->elsebody->accept(this);
	t3 = info[m->elsebody.get()];
  }
  info[m].expr = m;
  info[m].type = t2.type;
  if (t2.type != t3.type) {
	// warning: ifblock and elseblock have different types!
	std::cerr << "Warning: if block has inferred different block types: ["
			  << t2.type.name << ", " << t3.type.name << "]\n";
  }
}
void analyzers::TypeResolver::visit(ast::Let * e) {
  info[e].expr = e;
  e->value->accept(this);
  e->var->accept(this);
  if (info[e->var.get()].type.name == "") {
    info[e].type = e->type = info[e->var.get()].type = info[e->value.get()].type;
    std::cout << " [" << ((ast::Var *)e->var.get())->name;
    std::cout << "]  = " << e->type << "\n";
  } else {
    std::cout << " [" << ((ast::Var *)e->var.get())->name << "] ";
    std::cout << " skipping: " << info[e->var.get()].type << "\n";
  }
}
void analyzers::TypeResolver::visit(ast::BinOp * e) {
  e->lhs->accept(this);
  e->rhs->accept(this);
  info[e].expr = e;
  // TODO: this isn't quite right though
  info[e].type = info[e->lhs.get()].type;
}

void analyzers::TypeResolver::visit(ast::Return * m) {
  m->val->accept(this);
  info[m].expr = m;
  info[m].type = info[m->val.get()].type;
}

void analyzers::TypeResolver::visit(ast::Call * c) {
  if (ast::ExprTypeVisitor::of(c->callee.get()) == ast::ExprTypeKind::VarKind) {
    ast::Var * var = (ast::Var *)c->callee.get();
    if (var->name == "struct") {
      info[c].expr = c;
      info[c].type = Type("Struct");
      int i = 0;
      for(auto && arg : c->arguments) {
        auto binop = dynamic_cast<ast::BinOp *>(arg.get());
        if (!binop) return;
        if (binop->op != "=") return;
        auto lhs = dynamic_cast<ast::Var *>(binop->lhs.get());
        if (!lhs) return;
        binop->lhs->accept(this);
        binop->rhs->accept(this);
        info[lhs].expr = lhs;
        info[lhs].type = info[binop->rhs.get()].type;
        info[c].type.params.push_back(info[binop->rhs.get()].type);
        lhs->accept(this);
      }
      return;
    }
  }
  c->callee->accept(this);
  for(auto && a : c->arguments) a->accept(this);
  info[c].expr = c;
  info[c].type = info[c->callee.get()].type.returnType();
  Type tt = info[c->callee.get()].type;
  // std::cerr << "returntype " << tt << "\n";
}

void analyzers::TypeResolver::visit(ast::Var * v) {
  // std::cerr << "Var [" << v->name << "] :: " << info[v->expr].type << "\n";
  info[v].expr = v;
  info[v].type = info[v->expr].type;
}

using namespace coral::type;
void analyzers::TypeResolver::visit(ast::Func * f) {
  info[f].expr = f;
  Type ret_type = *(f->type);
  auto np = f->params.size();
  Type ftype("Func");
  for(ulong i=0; i < np; i++) {
	f->params[i]->accept(this);
	ftype.params.push_back(info[f->params[i].get()].type);
  }
  ftype.params.push_back(ret_type);

  if (f->body) {
    f->body->accept(this);
    if (ret_type.name == "") ftype.params.back() = info[f->body.get()].type;
    // std::cerr << "           inferring return type " << info[f->body.get()].type << "\n";
  }
  std::cerr << COL_RED << std::setw(30) << f->name << COL_CLEAR << " :: " << ftype << "\n";
  info[f].type = ftype;
  f->type = std::unique_ptr<Type>(new Type(ftype));
}

void analyzers::TypeResolver::visit(ast::Def * e) {
  info[e].expr = e;
  info[e].type = *(e->type);
}

void analyzers::TypeResolver::visit(ast::StringLiteral * e) {
  info[e].expr = e;
  info[e].type = Type("String");
}

void analyzers::TypeResolver::visit(ast::IntLiteral * e) {
  info[e].expr = e;
  info[e].type = Type("Int32");
}

void analyzers::TypeResolver::visit(ast::FloatLiteral * e) {
  info[e].expr = e;
  info[e].type = Type("Float");
}

void analyzers::TypeResolver::visit(ast::Set * s) {
  s->var->accept(this);
  s->value->accept(this);
  info[s].type = Type("");
  info[s].expr = s;
  info[s->var.get()].type = info[s->value.get()].type;
}

void analyzers::TypeResolver::visit(ast::While * w) {
  w->cond->accept(this);
  w->body->accept(this);
  info[w].expr = w;
  info[w].type = Type("");
}
