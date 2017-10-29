#include "treeprinter.hh"

void coral::TreePrinter::print() {
  if (module == 0) out << "empty module\n";
  else module->accept(this);
}

void coral::TreePrinter::visit(Return * r) {
  out << IND() << "return ";
  auto t = line_mode;
  line_mode = 1;
  r->value->accept(this);
  line_mode = t;
  out << END();
}
void coral::TreePrinter::visit(Tuple * t) {
  out << IND();
  auto lm = line_mode;
  line_mode = 1;
  out << '(';
  foreach(t->items, it) {
    if (it != t->items.begin()) out << ", ";
    (*it)->accept(this);
  }
  out << ')';
  line_mode = lm;
  out << END();
}
void coral::TreePrinter::visit(Expr * e) { out << IND() << "# expr: " << EXPRNAME(e) << END(); }
void coral::TreePrinter::visit(Index * i) {
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
void coral::TreePrinter::visit(For * f) {
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
void coral::TreePrinter::visit(Module * m) {
  foreach (module->lines, line) {
    (*line)->accept(this);
  }
}

void coral::TreePrinter::visit(FuncDef * m) {
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

void coral::TreePrinter::visit(BlockExpr * e) {
  auto lines = e->lines;
  foreach(lines, it) {
    (*it)->accept(this);
  }
}

void coral::TreePrinter::visit(BinOp * op) {
  out << IND();
  if (op->showParens(curop)) out << "(";

  TreePrinter pp(module, out);
  pp.curop = op;
  pp.line_mode = 1;
  op->lhs->accept(&pp);
  out << " " << op->op << " ";
  op->rhs->accept(&pp);
  if (op->showParens(curop)) out << ")";
  out << END();
}

void coral::TreePrinter::visit(If * e) {
  TreePrinter pp(module, out);
  pp.line_mode = 1;
  out << IND() << (showElif ? "elif " : "if ");
  showElif = 0;
  e->cond->accept(&pp);
  out << ":\n";
  indent++;
  e->ifbody->accept(this);
  indent--;

  if (e->elsebody->lines.size()) {
    if (e->elsebody->lines.size() == 1 && EXPRNAME(e->elsebody->lines[0]) == "If") {
      If * nextexpr = (If *)e->elsebody->lines[0];
      showElif = 1;
      nextexpr->accept(this);
    } else {
      out << IND() << "else:\n";
      indent++;
      e->elsebody->accept(this);
      indent--;
    }
  }
}

void coral::TreePrinter::visit(MatchCaseTagsExpr * e) {
  TreePrinter pp(module, out);
  pp.line_mode = 1;
  out << IND();
  e->label->accept(&pp);
  out << ":" << END();
  indent++;
  e->body->accept(this);
  indent--;
}

void coral::TreePrinter::visit(MatchExpr * e) {
  TreePrinter pp(module, out);
  pp.line_mode = 1;
  out << IND() << "match ";
  e->cond->accept(&pp);
  out << ":\n";
  indent++;
  foreach(e->cases, c) (*c)->accept(this);
  indent--;
}

void coral::TreePrinter::visit(DeclClass * a) {
  out << IND() << "class " << a->name << ":" << END();
  indent++;
  foreach(a->lines, line) {
    visit(*line);
  }
  indent--;
  out << END();
}

void coral::TreePrinter::visit(DeclTypeAlias * a) {
  out << IND() << "type " << a->name << " = " << a->wrapped->toString() << END();
  out << END();
}
void coral::TreePrinter::visit(DeclTypeEnum * a) {
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
void coral::TreePrinter::visit(EnumCase * e) {
  TreePrinter lp(module, out);
  lp.line_mode = 1;
  out << IND() << "| " << e->name;
  if (e->defs.size()) {
    out << "(";
    foreach(e->defs, def) lp.visit(*def);
    out << ")";
  }
  out << END();
}
void coral::TreePrinter::visit(Call * c) {
  out << IND();
  TreePrinter lp(module, out);
  lp.line_mode = 1;
  if (c->callee->getType() == VarKind) {
    auto vv = (Var *)c->callee.get();
    if (vv->value == "_list") {
      out << '[';
      foreach(c->arguments, it) {
	if (it != c->arguments.begin())	out << ", ";
	(*it)->accept(&lp);
      }
      out << ']' << END();
      return;
    }
  }
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
void coral::TreePrinter::visit(VoidExpr * e) {
  out << IND() << "()" << END();
}

void coral::TreePrinter::visit(ImplType * a) {
  out << IND() << "impl " << a->name << ":" << END();
  indent++;
  a->body->accept(this);
  indent--;
}

void coral::TreePrinter::visit(ImplClassFor * a) {
  out << IND()
      << "impl " << a->class_name
      << " for " << a->type_name
      << ":" << END();
  indent++;
  a->body->accept(this);
  indent--;
}

void coral::TreePrinter::visit(Extern * e) {
  out << IND() << "extern " << e->name << " : " << e->type << END();
}
void coral::TreePrinter::visit(BaseDef * d) {
  out << IND() << d->toString() << END(); return;
}

void coral::TreePrinter::visit(Cast * c) {
  out << IND() << "(";
  c->expr->accept(this);
  out << " as " << c->to_type << ")" << END();
}
void coral::TreePrinter::visit(Let * d) {
  TreePrinter lp(module, out);
  lp.line_mode = 1;
  out << IND() << "let ";
  if (d->tuplevar.size()) {
    out << "(";
    foreach(d->tuplevar, it) {
      if (it != d->tuplevar.begin()) out << ", ";
      lp.visit(*it);
    }
    out << ")";
  } else
    lp.visit(d->var);
  out << " = ";
  d->value->accept(&lp);
  out << END();
}

void coral::TreePrinter::visit(Long * d) { out << IND() << d->value << END(); }

void coral::TreePrinter::visit(String * d) { out << IND() << d->value << END(); }

void coral::TreePrinter::visit(Var * d) {  out << IND() << d->value << END(); }

void coral::TreePrinter::visit(BoolExpr * d) { out << IND() << d->value << END(); }

void coral::TreePrinter::visit(Double * d) { out << IND() << d->value << END(); }

void coral::TreePrinter::visit(AddrOf * d) { out << IND() << "addr " << d->var << END(); }

void coral::TreePrinter::visit(MatchEnumCaseExpr * c) {
  out << IND();
  out << c->name;
  out << "(";
  foreach(c->defs, it) visit(*it);
  out << ")";
  out << END();
}

void coral::TreePrinter::visit(Member * c) {
  out << IND();
  c->base->accept(this);
  out << "." << c->memberName;
  out << END();
}

std::string coral::TreePrinter::END () {
  if (line_mode) return "";
  return "\n";
}
std::string coral::TreePrinter::IND() {
  if (line_mode) return "";
  return std::string(indent * 2, ' ');
}
