#include "../typegraph.hh"
#include "test_tuples.cc"
#include "test_factorial.cc"
#include "testcase.hh"

using namespace typegraph;
#include <iostream>
#include <iomanip>
namespace typegraph {
  extern bool showSteps;
}
class BasicTest : public TestCase {
public:
  BasicTest() {
    test_concrete_type();
    test_typegraph_terms();
    test_type_constraints();
    test_function_call_inference();
    test_simple_polymorphism();
  }

  void test_typegraph_terms() {
    TypeGraph gg;
    std::string frob = "frobString";
    std::string bob = "bobString";
    auto f = gg.addTerm("frob", &frob);
    auto b = gg.addTerm("bob", &bob);
    auto b2 = gg.addTerm("bob2", &bob);
    auto b3 = gg.addTerm("bob", &bob);
    Assert(b3 != b, "terms with same given name are not the same object");
    Assert(b3->name != b->name, "terms with same given name are assigned different names");
    Assert(b2 != b, "terms with the same value are not equivalent");
    Assert(b2->expr == b->expr, "different terms with same expr pointer");
    gg.show();
  }

  void test_concrete_type() {
    // a concrete type is as defined as it's going to get -we can move it from
    // the unknown pile to the known pile
    TypeGraph gg;
    Assert(isConcreteType(gg.type("foo", {})), "type is concrete type");
    Assert(!isConcreteType(gg.type("foo", {gg.term("z")})), "type of term is not concrete type");
    Assert(isConcreteType(gg.type("foo", {gg.free(0), gg.type("z")})), "type is concrete type");
    Assert(isConcreteType(gg.free(0)), "free is not concrete type");
    Assert(!isConcreteType(gg.term("t")), "Term is not concrete type");
    Assert(!isConcreteType(gg.call(gg.term("f"), {})), "Call is not concrete type");
  }

  void test_type_constraints() {
    TypeGraph gg;
    auto foo = gg.addTerm("foo", 0);
    auto bar = gg.addTerm("bar", 0);
    gg.constrain(foo, gg.type("Int32"));
    gg.constrain(bar, gg.type("String"));
    auto solution = gg.solve();
    Assert(solution.getType(bar)->name == "String", "string variable is of type string");
    Assert(solution.getType(foo)->name == "Int32", "int variable is of type int");
  }

  void test_function_call_inference() {
    std::cout << "Testing Simple Function Call Type Inference\n";
    TypeGraph gg;
    auto result = gg.addTerm("result", 0);
    auto unknown_arg = gg.addTerm("unknown_arg", 0);
    gg.constrain("func", gg.type("Func", {gg.type("Int32"), gg.type("Int8")}));
    gg.constrain("func_arg", gg.type("Int32"));
    gg.constrain("arg_2", gg.term("func_arg"));
    gg.constrain(result, gg.call(gg.term("func"), {gg.term("func_arg")}));
    gg.constrain(result, gg.call(gg.term("func"), {gg.term(unknown_arg)}));
    auto solution = gg.solve();
    auto restype = solution.getType(result);
    Assert(restype && restype->name == "Int8", "return type inference");
    auto argtype = solution.getType(unknown_arg);
    Assert(argtype && argtype->name == "Int32", "param type inference");
  }

  void test_simple_polymorphism() {
    std::cout << "Testing Simple Polymorphic functions\n";
    TypeGraph gg;
    auto left = gg.addTerm("left", 0);
    auto right = gg.addTerm("right", 0);
    gg.constrain(left, gg.type("Func", {gg.free(0), gg.free(1), gg.free(0)}));
    gg.constrain(right, gg.type("Func", {gg.free(0), gg.free(1), gg.free(1)}));
  }
};

int main() {
  showSteps = false;
  std::cout << "Basic Tests\n";
  BasicTest().summary();
  Factorial().summary();
  TupleTest().summary();
  return 0;
}
