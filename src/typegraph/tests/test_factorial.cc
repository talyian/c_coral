// const char * r = R"RULES(
// gg.constrain("printf", gg.type("Func", {gg.type("Ptr"), gg.tye("..."), gg.type("Void")}));

// gg.constrain("factorial", gg.type("Func", gg.type("n, if)
// gg.constrain("i2", gg.type("Int32
// gg.constrain("op.<", gg.type("Bool
// gg.constrain("i1", gg.type("Int32
// gg.constrain("op.-", gg.type("Call", gg.type("Func", gg.type("*T0, *T0, *T0), n, i1.1)

// gg.constrain("if", gg.type("i1
// gg.constrain("i1.1", gg.type("Int32
// gg.constrain("i4", gg.type("Int32
// gg.constrain("call.factorial", gg.type("Call", gg.type("factorial, op.-)
// gg.constrain("op.*", gg.type("Call", gg.type("Func", gg.type("*T0, *T0, *T0), n, call.factorial)

// gg.constrain("if", gg.type("op.*
// gg.constrain("main", gg.type("Func(i0)
// gg.constrain("str."%d! = %d\n"", gg.type("Ptr
// gg.constrain("i4.1", gg.type("Int32
// gg.constrain("call.factorial.1", gg.type("Call(factorial, i4.1)
// gg.constrain("call.printf", gg.type("Call(printf, str."%d! = %d\n", i4, call.factorial.1)
// gg.constrain("str."%d! = %d\n".1", gg.type("Ptr
// gg.constrain("i6", gg.type("Int32
// gg.constrain("i6.1", gg.type("Int32

// gg.constrain("call.factorial.2", gg.type("Call(factorial, i6.1)
// gg.constrain("call.printf.1", gg.type("Call(printf, str."%d! = %d\n".1, i6, call.factorial.2)
// gg.constrain("str."%d! = %d\n".2", gg.type("Ptr
// gg.constrain("i8", gg.type("Int32
// gg.constrain("i8.1", gg.type("Int32
// gg.constrain("call.factorial.3", gg.type("Call(factorial, i8.1)
// gg.constrain("call.printf.2", gg.type("Call(printf, str."%d! = %d\n".2, i8, call.factorial.3)
// gg.constrain("i0", gg.type("Int32"));

// )RULES";

#include "../typegraph.hh"
#include "testcase.hh"

using namespace typegraph;

class Factorial : public TestCase {
public:
  void test_typegraph_terms() {
    TypeGraph gg;
    auto n = gg.addTerm("n", 0);
    auto t_if = gg.addTerm("if", 0);
    auto i11 = gg.addTerm("i1.1", 0);
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

    gg.show();
    auto solution = gg.solve();
  }
};
