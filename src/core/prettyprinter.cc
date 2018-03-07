#include "prettyprinter.hh"
#include "expr.hh"
#include "utils/ansicolor.hh"

#include <iostream>
#include <string>

using std::cout;

namespace coral {
#define COL_KEYWORD COL_RGB(2, 5, 1)
#define COL_STRING  COL_RGB(4, 3, 1)
#define COL_TYPE COL_RGB(4, 4, 2)
#define COL_COMMENT COL_RGB(0, 4, 0)
#define COL_PRIMITIVE COL_RGB(3, 3, 5)
  auto COL_NORMAL = COL_CLEAR;

  void coral::PrettyPrinter::visit(ast::Extern * e) {
    cout << IND() << "extern 'C' " << e->name << " : " << *(e->type) << END();
  }

  void PrettyPrinter::visit(ast::Func * e) {
    cout << IND() << COL_KEYWORD << "func " << COL_NORMAL << e->name;
    if (e->type && e->type->params.size() && e->type->params.back().name != "")
      cout << ": " << COL_TYPE << e->type->params.back() << COL_NORMAL;
    cout << "(";
    for(auto && def : e->params) {
      if (def != e->params.front()) cout << ", ";
      if (def) def->accept(this);
    }
    cout << ")";
    indent++;
    if (e->body) {
      cout << ":\n";
      e->body->accept(this);
    } else cout << "\n";
    indent--;
  }
  void PrettyPrinter::visit(ast::Block * e) {
    for(auto && line : e->lines)
      if (line) line->accept(this);
      else cout << "\n";
  }

  void PrettyPrinter::visit(ast::BinOp * e) {
    cout << IND();
    withline(e->lhs);
    cout << " " << e->op << " ";
    withline(e->rhs);
    cout << END();
  }

  void PrettyPrinter::visit(ast::Call * e) {
    auto l = line;
    cout << IND();
    line = true;
    if (e->callee) e->callee->accept(this);
    else cout << "(null callee)";
    // TODO: atomize
    bool showParens = true;
    // if (e->arguments.size() == 1 && (
    // 	  ExprTypeVisitor::of(e->arguments[0]) == ExprTypeKind::ListLiteralKind))
    if (e->arguments.size() == 1) {
      auto type = ast::ExprTypeVisitor::of(e->arguments[0].get());
      if (type == ast::ExprTypeKind::ListLiteralKind ||
          type == ast::ExprTypeKind::IntLiteralKind ||
          type == ast::ExprTypeKind::FloatLiteralKind ||
          type == ast::ExprTypeKind::StringLiteralKind ||
          type == ast::ExprTypeKind::VarKind ||
          type == ast::ExprTypeKind::TupleLiteralKind
          ) showParens = false;
    }
    if (showParens) cout << '('; else cout << ' ';
    for(auto &&arg : e->arguments) {
      if (!arg) { cout << "(null arg)"; continue; }
      if (arg != e->arguments.front()) cout << ", ";
      arg->accept(this);
    }
    if (showParens) cout << ')';
    line = l;
    cout << END();
  }

  void PrettyPrinter::visit(ast::Var * v) { cout << IND() << v->name << END(); }

  void PrettyPrinter::visit(ast::Return * e) {
    cout << IND() << COL_KEYWORD << "return " << COL_NORMAL;
    line = true;
    if (e->val) e->val->accept(this);
    line = false;
    cout << END();
  }

  void PrettyPrinter::visit(ast::IntLiteral * e) {
    cout << IND() << COL_PRIMITIVE << e->value << COL_NORMAL << END(); }

  void PrettyPrinter::visit(ast::FloatLiteral * e) {
    cout << IND() << COL_PRIMITIVE << e->value << COL_NORMAL << END(); }

  void PrettyPrinter::visit(ast::StringLiteral * s) {
    cout << IND()  << COL_STRING << s->value << COL_NORMAL << END(); }

  void PrettyPrinter::visit(ast::IfExpr * s) {
    cout << IND() <<COL_KEYWORD<< "if " << COL_NORMAL;
    withline(s->cond);
    cout << ":" << END();
    indent++;
    s->ifbody->accept(this);
    indent--;
    if (s->elsebody) {
      cout << IND() << COL_KEYWORD << "else:" << COL_NORMAL << END();
      indent++;
      s->elsebody->accept(this);
      indent--;
    }
  }

  void PrettyPrinter::visit(ast::ForExpr * s) {
    cout << IND() << COL_KEYWORD << "for " << COL_NORMAL;
    withline(s->var);
    cout << " in ";
    withline(s->sequence);
    cout << ":" << END();
    indent++;
    if (s->body) s->body->accept(this);
    indent--;
  }

  void PrettyPrinter::visit(ast::Comment * c) {
    cout << COL_COMMENT;
    cout << IND() << c->value << END();
    cout << COL_NORMAL;
  }

  void PrettyPrinter::visit(ast::Let * e) {
    cout << IND() << COL_KEYWORD << "let " << COL_NORMAL;
    withline(e->var);
    if (e->type.name != "") {
      cout << " : ";
      cout << COL_TYPE << e->type << COL_NORMAL;
    }
    cout << " = ";
    withline(e->value);
    cout << END();
  }

  void PrettyPrinter::visit(ast::Set * e) {
    cout << IND() << COL_LIGHT_CYAN << "set " << COL_NORMAL;
    withline(e->var);
    cout << " = ";
    withline(e->value);
    cout << END();
  }

  void PrettyPrinter::visit(ast::Module * m) {
    if (m->body) m->body->accept(this);
  }

  void PrettyPrinter::visit(ast::Member * m) {
    cout << IND();
    withline(m->base);
    cout << "." << m->member << END();
  }

  void PrettyPrinter::visit(ast::ListLiteral * m) {
    cout << IND() << '[';
    for(auto && item : m->items) {
      if (item != m->items.front())
        cout << ", ";
      withline(item);
    }
    cout << ']' << END();
  }

  void PrettyPrinter::visit(ast::Def * d) {
    cout << d->name;
    if (d->type && d->type->name != "") {
      cout << ":";
      cout << COL_TYPE << *(d->type) << COL_NORMAL;
    }
  }

  void PrettyPrinter::visit(ast::TupleLiteral * m) {
    cout << IND() << '(';
    for(auto && item : m->items) {
      if (item != m->items.front())
        cout << ", ";
      withline(item);
    }
    cout << ')' << END();
  }

  void PrettyPrinter::visit(ast::While * w) {
    cout << IND() << COL_KEYWORD << "while " << COL_NORMAL;
    withline(w->cond);
    cout << ":" << END();
    indent++;
    w->body->accept(this);
    indent--;
  }

  void PrettyPrinter::visit(ast::Tuple * t) {
    cout << IND() << COL_KEYWORD << "type " << COL_TYPE;
    cout << t->name << COL_NORMAL;
    cout << " = {";
    for(auto &field: t->fields) {
      if (field->name.size())
        cout << field->name << ":";
      cout << COL_TYPE << *(field->type) << COL_NORMAL;
      if (&field != &t->fields.back())
        cout << ", ";
    }
    cout << "}" << END();
  }

  void PrettyPrinter::visit(ast::Import * m) {
    cout << IND() << COL_KEYWORD << "import " << COL_NORMAL;
    for(auto &part: m->path) {
      cout << part;
      if (&part != &m->path.back()) cout << ".";
    }
    cout << END();;
  }
  PrettyPrinter::PrettyPrinter() {
    indent = 0;
  }
}
