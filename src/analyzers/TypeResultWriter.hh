#include "core/expr.hh"

namespace coral {

  coral::type::Type convert_Type(::Type * tt) {
    coral::type::Type f(tt->name);
    for(auto &p: tt->params) {
      if (::Type * pt = dynamic_cast<::Type *>(p))
        f.params.push_back(convert_Type(pt));
      else {
        f.name = "";
        break;
      }
    }
    return f;
  }

  namespace analyzers {
    // Given the results of a Typegraph analysis, write it back out into the type fields
    class TypeResultWriter : public ast::ExprVisitor {
    private:
      ::Type * current_term;
    public:
      std::string visitorName() { return "TypeResultWriter"; }
      static void write(std::map<coral::ast::BaseExpr *, ::Type *> expr_terms) {
        TypeResultWriter writer { expr_terms };
      }
      TypeResultWriter (std::map<coral::ast::BaseExpr *, ::Type *> expr_terms) {
        for(auto &pair : expr_terms) {
          current_term = pair.second;
          if (pair.second)
            pair.first->accept(this);
        }
      }
      void visit(ast::IntLiteral *) { /* ignore */ }
      void visit(ast::StringLiteral *) { /* ignore */ }
      void visit(ast::Func * f) {
        if (!current_term) return;
        // std::cout << COL_GREEN << std::setw(25) << "func: " << f->name << " :: "
        //           << current_term << COL_CLEAR << "\n";
        Type t = convert_Type(current_term);
        if (t.name == "") return;
        f->type.reset(new Type(t));

        for(size_t i = 0; i < t.params.size() - 1; i++) {
          f->params[i]->type.reset(new Type(t.params[i]));
        }
      }
      void visit(ast::Def * d) {
        if (!current_term) return;
        d->type.reset(new Type(convert_Type(current_term)));
        // std::cout << COL_GREEN << std::setw(25) << "def: " << d->name << " :: "
        //           << current_term << COL_CLEAR << "\n";
      }
      void visit(ast::Extern * l) { }
      void visit(ast::Let * l) {
        if (!current_term) return;
        // std::cout << COL_GREEN << std::setw(25) << "let: " << l->var->name << " :: "
        //           << current_term << COL_CLEAR << "\n";
        auto t = convert_Type(current_term);
        if (t.name == "") return;
        l->type = t;
      }
      // void visit(ast::Def * def) { def->type.reset(new coral::type::Type(current_term->
    };
  }
}
