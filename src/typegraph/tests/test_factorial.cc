#include "../typegraph.hh"
#include "testcase.hh"
#include <sstream>

using namespace typegraph;

class Factorial : public TestCase {
public:
  Factorial() {
    name = "Factorial Test";
    test_typegraph_terms();
  }
  void test_typegraph_terms() {
    TypeGraph gg;
    gg.addTerm("n", 0);
    gg.addTerm("if", 0);
    gg.addTerm("i1.1", 0);
    auto factorial = gg.addTerm("factorial", 0);
    auto main = gg.addTerm("main", 0);
    gg.constrain("printf", gg.type("Func", {gg.type("Ptr"), gg.type("..."), gg.type("Void")}));
    gg.constrain("factorial", gg.type("Func", {gg.term("n"), gg.term("if")}));
    gg.constrain("i2", gg.type("Int32"));
    gg.constrain("op.<", gg.type("Bool"));
    gg.constrain("i1", gg.type("Int32"));
    gg.constrain("op.-", gg.call(
                   gg.type("Func", {gg.free(0), gg.free(0), gg.free(0)}),
                   {gg.term("n"), gg.term("i1.1")}));

    gg.constrain("if", gg.term("i1"));
    gg.constrain("i1.1", gg.type("Int32"));
    gg.constrain("i4", gg.type("Int32"));
    gg.constrain("call.factorial", gg.call(gg.term("factorial"), {gg.term("op.-")}));
    gg.constrain("op.*", gg.call(
                   gg.type("Func", {gg.free(0), gg.free(0), gg.free(0)}),
                   {gg.term("n"), gg.term("i1.1")}));

    gg.constrain("i0", gg.type("Int32"));
    gg.constrain("if", gg.term("op.*"));
    gg.constrain("main", gg.type("Func", {gg.term("i0")}));
    gg.constrain("str.\"%d! = %d\\n\"", gg.type("Ptr"));
    gg.constrain("i4.1", gg.type("Int32"));
    gg.constrain("call.factorial.1", gg.call(gg.term("factorial"), {gg.term("i4.1")}));
    gg.constrain("call.printf",
      gg.call(gg.term("printf"), {
          gg.term("str.\"%d! = %d\\n\""),
            gg.term("i4"),
            gg.term("call.factorial.1") }));
    gg.constrain("str.\"%d! = %d\\n\".1", gg.type("Ptr"));
    gg.constrain("i6", gg.type("Int32"));
    gg.constrain("i6.1", gg.type("Int32"));

    gg.constrain("call.factorial.2", gg.call(gg.term("factorial"), {gg.term("i6.1")}));
    gg.constrain("call.printf.1", gg.call(gg.term("printf"),
                                          {gg.term("str.\"%d! = %d\\n\".1"),
                                          gg.term("i6"),
                                              gg.term("call.factorial.2")}));
    gg.constrain("str.\"%d! = %d\\n\".2", gg.type("Ptr"));
    gg.constrain("i8", gg.type("Int32"));
    gg.constrain("i8.1", gg.type("Int32"));
    gg.constrain("call.factorial.3", gg.call(gg.term("factorial"), {gg.term("i8.1")}));
    gg.constrain("call.printf.2", gg.call(gg.term("printf"), {
          gg.term("str.\"%d! = %d\\n\".2"),
            gg.term("i8"),
            gg.term("call.factorial.3")}));

    auto solution = gg.solve();

    auto fact_type = solution.getType(factorial);
    auto main_type = solution.getType(main);
    std::stringstream sout; sout << fact_type;
    std::stringstream mout; mout << main_type;
    Assert(sout.str() == "Func[Int32, Int32]", "Factorial inferred type");
    Assert(mout.str() == "Func[Int32]", "Main inferred type");
  }
};
