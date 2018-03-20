#include "utils/ansicolor.hh"
#include "core/expr.hh"
#include "typegraph/constraints.hh"
#include "typegraph/typegraph.hh"

namespace coral {
  // changes a typegraph type back to a coral type
  coral::type::Type convert_Type(typegraph::Type * tt) {
    coral::type::Type f(tt->name);
    for(auto &p: tt->params) {
      if (typegraph::Type * pt = dynamic_cast<typegraph::Type *>(p))
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
      typegraph::Type * inferredType;
      typegraph::TypeGraph * gg;
    public:
      std::string visitorName() { return "TypeResultWriter"; }
      static void write(
        typegraph::TypeGraph * gg,
        std::vector<std::pair<coral::ast::BaseExpr *, typegraph::Type *>> expr_terms) {
        TypeResultWriter writer { gg, expr_terms };
      }
      TypeResultWriter (
        typegraph::TypeGraph * gg,
        std::vector<std::pair<coral::ast::BaseExpr *, typegraph::Type *>> expr_terms) : gg(gg) {
        for(auto &pair : expr_terms) {
          inferredType = pair.second;
          std::cerr
            << COL_LIGHT_BLUE << "writing "
            << ast::ExprNameVisitor::of(pair.first) ;
          if (ast::Var * var = dynamic_cast<ast::Var *>(pair.first))
            std::cerr << " (" << var->name << ")" ;
          std::cerr << " :: " << pair.second << "\033[0m\n";
          if (pair.second && pair.first)
            pair.first->accept(this);
        }
      }

      void visit(ast::IntLiteral *) { /* ignore */ }
      void visit(ast::StringLiteral *) { /* ignore */ }
      void visit(ast::Func * f) {
        if (!inferredType) return;
        Type t = convert_Type(inferredType);
        if (t.name == "") return;
        if (t.name == "Method") {
          // can we rely on nameresolver to insert these?
          // f->params.insert(
          //   f->params.begin(), std::unique_ptr<coral::ast::Def>(
          //     new coral::ast::Def("self", new Type(""), 0)));
        }
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
      void visit(ast::Tuple *) { }
      void visit(ast::FloatLiteral *) { }
      // void visit(ast::Def * def) { def->type.reset(new coral::type::Type(inferredType->
      void visit(ast::Member * m) {
        // std::cout << COL_GREEN << std::setw(25) << "member: " << m->member << " :: "
        //           << inferredType << COL_CLEAR << "\n";
        if (inferredType->name == "Index")
          m->memberIndex = std::stoi(
            dynamic_cast<typegraph::Type *>(
              inferredType->params[0])->name);
        else if (inferredType->name == "FuncTerm") {
          // std::cerr << COL_LIGHT_CYAN << "funcptr "
          //           << m->member << " :: " << inferredType << "\033[0m\n";
          auto func_term = dynamic_cast<typegraph::Type *>(inferredType->params[0])->name;
          auto expr = static_cast<ast::BaseExpr *>(gg->term(func_term)->term->expr);
          m->methodPtr = dynamic_cast<ast::Func *>(expr);
          // PrettyPrinter::print(m->methodPtr);
          // std::cerr << m->methodPtr->name << "\033[0m\n";
        } else {
          // std::cerr
          //   << COL_LIGHT_CYAN
          //   << "typeof  " << m->member
          //   << " :: " << inferredType << "\033[0m\n";
        }
      }
      void visit(ast::Call * call) {
        if (inferredType->name == "OverloadID") {
          auto inferred_index =
            std::stoi(dynamic_cast<typegraph::Type *>(inferredType->params[0])->name);
          auto var = dynamic_cast<ast::Var *>(call->callee.get());
          if (var) {
            auto overload = dynamic_cast<ast::OverloadedFunc*>(var->expr);
            if (overload) {
              // std::cerr << "setting overload to " << inferred_index << "\n";
              var->expr = overload->funcs[inferred_index];
              return;
            }
          }
        }
        // std::cerr << "error calling " << ast::ExprNameVisitor::of(call->callee.get()) << "\n";
      }
      void visit(ast::OverloadedFunc *) { }
      void visit(ast::BinOp *) { }
      void visit(ast::IfExpr *) { }
      void visit(ast::TupleLiteral * t) {
        t->type.reset(new Type(convert_Type(inferredType)));
      }
    };
  }
}
