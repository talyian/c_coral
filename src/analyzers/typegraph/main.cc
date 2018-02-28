#include "analyzers/typegraph/TypeGraph.hh"
#include "utils/ansicolor.hh"
#include <iostream>



void check_collatz() {
  TypeGraph gg;
  gg.AddTerm("i1");
  gg.AddTerm("op:=");
  gg.AddTerm("i1");
  gg.AddTerm("op:=");
  gg.AddTerm("op:%");
  gg.AddTerm("i1.0");
  gg.AddTerm("op:=.0");
  gg.AddTerm("call.collatz");
  gg.AddTerm("i3");
  gg.AddTerm("op:*");
  gg.AddTerm("i1.1");
  gg.AddTerm("op:+");
  gg.AddTerm("collatz_loop.a");
  gg.AddTerm("collatz_loop.b");
  gg.AddTerm("i1.2");
  gg.AddTerm("op:+.0");
  gg.AddTerm("call.collatz.0");
  gg.AddTerm("i2.0");
  gg.AddTerm("op:/");
  gg.AddTerm("i1.3");
  gg.AddTerm("op:+.1");
  gg.AddTerm("if");
  gg.AddTerm("if");
  gg.AddTerm("if.0");
  gg.AddTerm("if.0");
  gg.AddTerm("collatz_loop");
  gg.AddTerm("call.collatz.1");
  gg.AddTerm("i0");
  gg.AddTerm("main");
  gg.AddTerm("call.collatz.2");
  gg.AddTerm("i1.4");
  gg.AddTerm("i30");
  gg.AddTerm("i0.0");
  gg.AddTerm("collatz");
  gg.AddTerm("i2");

  gg.AddTerm("collatz.m");
  gg.AddTerm("collatz.n");
  gg.AddTerm("op.*");

  gg.AddConstraint("i1", gg.type("Int32"));
  gg.AddConstraint(
    "op:=", gg.call(
      gg.type("Func", {gg.free(0), gg.free(0), gg.free(0)}),
      {gg.term("collatz.n"), gg.term("i1") }));
  gg.AddConstraint(
    "op:%", gg.call(
      gg.type("Func", { gg.free(1), gg.free(1), gg.free(1)}),
      {gg.term("collatz.n"), gg.term("i2")}));
  gg.AddConstraint("i1.0", gg.type("Int32"));
  gg.AddConstraint(
    "op:=.0", gg.call(
      gg.type("Func", {gg.free(2), gg.free(2), gg.free(2)}),
      {gg.term("op:%"), gg.term("i1.0")}));


  gg.AddConstraint("call.collatz", gg.call(gg.term("collatz"), {gg.term("op:+"), gg.term("op:+.0")}));
  gg.AddConstraint("i3", gg.type("Int32"));
  gg.AddConstraint("op:*", gg.call(
                     gg.type("Func", {gg.free(3), gg.free(3), gg.free(3)}),
                     {gg.term("i3"), gg.term("collatz.n")}));
  gg.AddConstraint("i1.1", gg.type("Int32"));
  gg.AddConstraint("op:+", gg.call(gg.type("Func", {gg.free(4), gg.free(4), gg.free(4)}),
                                   {gg.term("op.*"), gg.term("i1.1")}));
  gg.AddConstraint("collatz_loop.a", gg.type("Int32"));
  gg.AddConstraint("collatz_loop.b", gg.type("Int32"));
  gg.AddConstraint("i1.2", gg.type("Int32"));
  gg.AddConstraint("op:+.0", gg.call(gg.type("Func", {gg.free(5), gg.free(5), gg.free(5)}),
                                     {gg.term("collatz.m"), gg.term("i1.2")}));
  gg.AddConstraint("call.collatz.0", gg.call(gg.term("collatz"), {gg.term("op:/"), gg.term("op:+.1")}));
  gg.AddConstraint("i2.0", gg.type("Int32"));
  gg.AddConstraint("op:/", gg.call(gg.type("Func", {gg.free(6), gg.free(6), gg.free(6)}),
{gg.term("collatz.n"), gg.term("i2.0")}));
  gg.AddConstraint("i1.3", gg.type("Int32"));
  gg.AddConstraint("op:+.1", gg.call(gg.type("Func", {gg.free(7), gg.free(7), gg.free(7)}),
{gg.term("collatz.m"), gg.term("i1.3")}));

  gg.AddConstraint("if", gg.term("call.collatz"));
  gg.AddConstraint("if", gg.term("call.collatz.0"));
  gg.AddConstraint("if.0", gg.term("collatz.m"));
  gg.AddConstraint("if.0", gg.term("if"));
  gg.AddConstraint("collatz_loop", gg.type("Func", {
        gg.term("collatz_loop.a"), gg.term("collatz_loop.b"), gg.term("call.collatz.1") }));
  gg.AddConstraint("call.collatz.1", gg.call(gg.term("collatz"),
                                             {gg.term("collatz_loop.a"), gg.term("i0")}));
  gg.AddConstraint("i0", gg.type("Int32"));
  gg.AddConstraint("main", gg.type("Func", {gg.term("i0.0") }));
  gg.AddConstraint("call.collatz.2", gg.call(gg.term("collatz"), {
        gg.term("i1.4"), gg.term("i30") }));
  gg.AddConstraint("i1.4", gg.type("Int32"));
  gg.AddConstraint("i30", gg.type("Int32"));
  gg.AddConstraint("i0.0", gg.type("Int32"));
  gg.AddConstraint("collatz", gg.type("Func", { gg.term("collatz.n"), gg.term("collatz.m"), gg.term("if.0") }));
  gg.AddConstraint("i2", gg.type("Int32"));

  gg.Show("Initial");
  gg.MarkAllDirty();
  gg.Step();
  gg.Show("Final");
}

void check_apply() {
  TypeGraph gg;
  Constraint * fcons = gg.type("Func", {gg.free(0), gg.free(0), gg.free(0)});
  gg.AddTerm("a");
  gg.AddTerm("b");
  gg.AddTerm("f");
  gg.AddTerm("res");
  gg.AddConstraint("a", gg.type("Int32"));
  gg.AddConstraint("f", fcons);
  gg.AddConstraint("res", gg.call(fcons, {gg.term("a"), gg.term("b")}));

  gg.Show(" [Application test]");
  gg.MarkAllDirty();
  gg.Step();
  gg.Show(" [Application test]");
}

void test_Unify_Type_and_Var() {
  TypeGraph gg;
  auto fterm = gg.AddTerm("id");
  auto gterm = gg.AddTerm("g");
  auto hterm = gg.AddTerm("h");
  auto jterm = gg.AddTerm("j");
  gg.AddConstraint(fterm->name, new Type("Func", {new Free(1), new Free(1)}));
  gg.AddConstraint("h", new Type("int"));
  gg.AddConstraint(gterm->name, new Call(new Term(fterm), {new Term(hterm)}));
  gg.AddConstraint("j", new Term(gterm));
  gg.Show("Type/Term UnifyTest");
  gg.Step();
  gg.Show("Type/Term UnifyTest");
}

void test_ConstraintEquals() {
  auto test = [] (Constraint * a, Constraint * b) {
    auto out = ConstraintEqualsImpl::of(a, b);
    std::cerr << a << (out ? " == " : " != ") << b << "\n";
  };
  test(new Type("asdf"), new Type("asdf"));
  test(new Type("asdf"), new Type("asdf", {new Type("Int32")}));
  test(new Type("asdf"), new Type("fedf"));
}

int main() {
  std::cout << "\033[3m" << COL_RGB(1, 2, 5) << "Type Test\n" << COL_CLEAR;
  // test_ConstraintEquals();
  // test_Unify_Type_and_Var();
  // check_apply();
  check_collatz();
  return 0;
}
