#include "core/expr.hh"

namespace coral {

  // changes a typegraph type back to a coral type
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
      ::Type * inferredType;
    public:
      std::string visitorName() { return "TypeResultWriter"; }
      static void write(std::map<coral::ast::BaseExpr *, ::Type *> expr_terms) {
        TypeResultWriter writer { expr_terms };
      }
      TypeResultWriter (std::map<coral::ast::BaseExpr *, ::Type *> expr_terms) {
        for(auto &pair : expr_terms) {
          inferredType = pair.second;
          if (pair.second && pair.first)
            pair.first->accept(this);
        }
      }
      void visit(ast::IntLiteral *) { /* ignore */ }
      void visit(ast::StringLiteral *) { /* ignore */ }
      void visit(ast::Func * f) {
        if (!inferredType) return;
        // std::cout << COL_GREEN << std::setw(25) << "func: " << f->name << " :: "
        //           << inferredType << COL_CLEAR << "\n";
        Type t = convert_Type(inferredType);
        if (t.name == "") return;
        f->type.reset(new Type(t));

        for(size_t i = 0; i < t.params.size() - 1; i++) {
          f->params[i]->type.reset(new Type(t.params[i]));
        }
      }
      void visit(ast::Def * d) {
        if (!inferredType) return;
        d->type.reset(new Type(convert_Type(inferredType)));
        // std::cout << COL_GREEN << std::setw(25) << "def: " << d->name << " :: "
        //           << inferredType << COL_CLEAR << "\n";
      }
      void visit(ast::Extern *) { }
      void visit(ast::Let * l) {
        if (!inferredType) return;
        // std::cout << COL_GREEN << std::setw(25) << "let: " << l->var->name << " :: "
        //           << inferredType << COL_CLEAR << "\n";
        auto t = convert_Type(inferredType);
        if (t.name == "") return;
        l->type = t;
      }
      void visit(ast::TupleLiteral *) { }
      void visit(ast::Tuple *) { }
      void visit(ast::FloatLiteral *) { }
      // void visit(ast::Def * def) { def->type.reset(new coral::type::Type(inferredType->
      void visit(ast::Member * m) {
        if (inferredType->name == "Index")
          m->memberIndex = std::stoi(dynamic_cast<::Type *>(inferredType->params[0])->name);
        std::cerr << m->member << ".member writing " << inferredType << "\n";
      }
    };
  }
}
