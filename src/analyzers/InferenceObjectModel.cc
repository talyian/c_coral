#include "core/expr.hh"
#include "utils/ansicolor.hh"
#include "InferenceObjectModel.hh"

#include <iostream>
#include <iomanip>
#include <vector>

using namespace coral;

namespace frobnob {

  std::ostream & operator<<(std::ostream &out, TypeTerm &tptr) {
    std::streamsize s =  out.width();
    out.width(0);
    return out << COL_RGB(5, 3, 4) << std::setw(s) << tptr.name << COL_CLEAR;
  }

  std::ostream & operator<<(std::ostream &out, TypeTerm * tptr) {
    return out << *tptr;
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
      out << tc;
    }
    return out;
  }


  TypeConstraint * Global_Ops(std::string op) {
    static auto T0 = new FreeType();
    static auto ArithOp = new Type("Func", { T0, T0, T0 });
    if (op == "<")
      return new Type("Func", {T0, T0, new Type("Int1")});
    return ArithOp;
  }
}
