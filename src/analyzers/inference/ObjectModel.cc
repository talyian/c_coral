#include "core/expr.hh"
#include "utils/ansicolor.hh"
#include "analyzers/inference/ObjectModel.hh"

#include <iostream>
#include <iomanip>
#include <vector>

using namespace coral;

namespace coral {
  namespace typeinference {
    void TypeConstraintVisitor::visit(TypeConstraint * t) { std::cerr << "TypeConstraintVisitor " << t << "\n"; }
    void TypeConstraintVisitor::visit(Term * t) { }
    void TypeConstraintVisitor::visit(Type * t) { for(auto &p: t->params) p->accept(this); }
    void TypeConstraintVisitor::visit(Call * t) { for(auto &a: t->args) a->accept(this); t->callee->accept(this); }
    void TypeConstraintVisitor::visit(FreeType * t) { }


    std::ostream & operator<<(std::ostream &out, TypeTerm &tptr) {
      std::streamsize s =  out.width();
      out.width(0);
      return out << COL_RGB(5, 3, 4) << std::setw(s) << tptr.name << COL_CLEAR;
    }

    std::ostream & operator<<(std::ostream &out, TypeTerm * tptr) {
      return (tptr ? out << *tptr : out << "(null term)");
    }

    std::ostream & operator<<(std::ostream &out, TypeConstraint &tc) {
      tc.print_to(out); return out;
    }

    std::ostream & operator<<(std::ostream &out, TypeConstraint *tc) {
      return tc ? (out << *tc) : (out << "(null)");
    }

    std::ostream & operator<<(std::ostream &out, std::vector<TypeConstraint *> &vv) {
      for(auto &&tc : vv) {
        if (&tc != &vv.front()) out << ", ";
        out << &tc;
      }
      return out;
    }
  }
}
