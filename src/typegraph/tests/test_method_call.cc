#include "../typegraph.hh"
#include "testcase.hh"
#include <sstream>

using namespace typegraph;
namespace typegraph { extern bool showSteps; }

class MethodCallTest : public TestCase {
public:
  MethodCallTest() {
    name = "Method Calls Test";
    test_call_instancemethod();
    test_call_classmethod();
  }
  void test_call_instancemethod() {
    // Test that both instancemethods
    // and class methods can be called on an instance.

    // type Person = {age:Int32}
    // func Person.getType() : "Person"
    // func Person.getAge(): self.age
    // let p = Person(33)
    // let a = p.getType()
    // let b = p.getAge()
    TypeGraph gg;
    auto person_age = gg.addTerm("Person::age", 0);
    auto person_age_index = gg.addTerm("Person::age.index", 0);
    gg.constrain(person_age, gg.type("Int32"));
    gg.constrain(person_age_index, gg.type("0"));
    auto person = gg.addTerm("Person", 0);
    gg.constrain(person, gg.type("Func", {gg.type("Int32"), gg.type("Person")}));

    auto getType = gg.addTerm("Person::getType", 0);
    gg.constrain(getType, gg.type("Func", {gg.type("Ptr")}));

    auto getAge = gg.addTerm("Person::getAge", 0);
    auto self_age = gg.addTerm("self.age", 0);
    auto self = gg.addTerm("self", 0);
    gg.constrain(self, gg.type("Person"));
    gg.constrain(self_age, gg.call(gg.type("Member", {gg.type("age")}), {
          gg.term(self),
          gg.type("Term", {gg.type("p")})
    }));
    gg.constrain(getAge, gg.type("Method", {gg.type("Person"), gg.term(self_age)}));

    auto p = gg.addTerm("p", 0);
    gg.constrain(p, gg.type("Person"));
    auto a = gg.addTerm("a", 0);
    auto p_getType = gg.addTerm("p.getType", 0);
    gg.constrain(p_getType, gg.call(gg.type("Member", { gg.type("getType")}), {
          gg.term("p"),
          gg.type("Term", {gg.type("p")})}));
    gg.constrain(a, gg.call(gg.term(p_getType), {}));

    auto b = gg.addTerm("b", 0);
    auto p_getAge = gg.addTerm("p.getAge", 0);
    gg.constrain(p_getAge, gg.call(gg.type("Member", {gg.type("getAge")}), {
          gg.term("p"),
          gg.type("Term", {gg.type("p")})}));
    gg.constrain(b, gg.call(gg.term(p_getAge), {}));

    gg.show();
    showSteps = true;
    auto solution = gg.solve();
    solution.showSummary();

    // Part 1: Calling a typemethod on an instance
    // This generates a FuncTerm[<Fterm>] for the ast::Member node
    // where FTerm points to a Func[TParams, TResult] type
    Assert(
      ConsEquals(
        solution.getType(gg.term("self.age.index")->term),
        gg.type("Index", {gg.type("0")})).out,
      "self.age.index found at index 0");
    Assert(
      ConsEquals(
        solution.getType(gg.term("p.getType.func")->term),
        gg.type("FuncTerm", {gg.type("Person::getType")})).out,
      "p.getType is a pointer to Person::getType");
    Assert(
      ConsEquals(
        solution.getType(gg.term("p.getType")->term),
        gg.type("Func", {gg.type("Ptr")})).out,
      "p.getType is a Func[Ptr]");
    Assert(
      ConsEquals(solution.getType(gg.term("a")->term), gg.type("Ptr")).out,
      "p.getType() returns a string");

    // Part 2: Calling an instance method on an instance
    // This generates a MethodTerm[<Fterm>] for the ast::Member node
    // where FTerm points to a Func[Self, TParams, TResult]
    // ast::Member then gets rewritten to the bound method type Func[TParams, TResult]

    // Assert(
    //   ConsEquals(
    //     solution.getType(gg.term("b.method")->term),
    //     gg.type("MethodTerm", {gg.type(getAge->name), gg.type(p->name)})).out,
    //   "b.method points to Person::getAge and p");

    std::cerr << solution.getType(gg.term(p_getAge->name + ".method")->term) << "\n";
    Assert(
      ConsEquals(
        solution.getType(gg.term(p_getAge->name + ".method")->term),
        gg.type("FuncTerm", {gg.type("Person::getAge")})).out,
      "p.getAge.method points to a method Person::getAge");

    std::cerr << solution.getType(p_getAge) << "\n";
    std::cerr << gg.type("BoundMethod", {gg.type("Int32")}) << "\n";
    Assert(
      ConsEquals(
        solution.getType(p_getAge),
        gg.type("BoundMethod", {gg.type("Int32")})).out,
      "p.getAge is a type BoundMethod[Int32]");

    std::cerr << solution.getType(gg.term("Person::getAge")->term) << "\n";
    Assert(
      ConsEquals(
        solution.getType(gg.term("Person::getAge")->term),
        gg.type("Method", {gg.type("Person"), gg.type("Int32")})).out,
      "Person::getAge is a method on person returning Int32");

    Assert(
      ConsEquals(solution.getType(gg.term("b")->term), gg.type("Int32")).out,
      "p.getAge() returns an Int32");
  }
  void test_call_classmethod() {
    // Test that classmethods can be called on a class
    // type Person = {age:Int32}
    // func Person.getType() : "Person"
    // let a = Person.getType()
    TypeGraph gg;
    auto person_age = gg.addTerm("Person::age", 0);
    auto person_age_index = gg.addTerm("Person::age.index", 0);
    gg.constrain(person_age, gg.type("Int32"));
    gg.constrain(person_age_index, gg.type("0"));
    auto person = gg.addTerm("Person", 0);
    gg.constrain(person, gg.type("Func", {gg.type("Int32"), gg.type("Person")}));

    auto getType = gg.addTerm("Person::getType", 0);
    gg.constrain(getType, gg.type("Func", {gg.type("Ptr")}));

    auto a = gg.addTerm("a", 0);
    auto person_gettype = gg.addTerm("Person.getType", 0);
    gg.constrain(
      person_gettype, gg.call(
        gg.type("Member", {gg.type("getType")}),
        {gg.type("Person")}));
    gg.constrain(a, gg.call(gg.term(person_gettype), {}));

    // gg.show();
    auto solution = gg.solve();
    // solution.showSummary();
    Assert(
      ConsEquals(
        solution.getType(a),
        gg.type("Ptr")).out,
      "Person.getType is a string");
    Assert(
      ConsEquals(
        solution.getType(gg.term(person_gettype->name + ".func")->term),
        gg.type("FuncTerm", {gg.type("Person::getType")})).out,
      "Person.getType points to Person::getType");
  }
};
