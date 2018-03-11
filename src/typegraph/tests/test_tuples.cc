#include "../typegraph.hh"
#include "testcase.hh"
#include <sstream>

using namespace typegraph;
namespace typegraph { extern bool showSteps; }

class TupleTest : public TestCase {
public:
  TupleTest() {
    name = "Tuple Test";
    test_basic_tuple();
  }
  void test_basic_tuple() {
    TypeGraph gg;
    auto b_y = gg.addTerm("b.y", 0);
    gg.constrain("Bar::y", gg.type("String"));
    gg.constrain("Bar::y.index", gg.type("0"));
    gg.constrain("b", gg.type("Bar"));
    gg.constrain("b.y", gg.call(gg.type("Member", { gg.type("y") }), {gg.term("b")}));
    // typegraph::showSteps = true;
    auto solution = gg.solve();
    Assert(solution.getType(b_y)->name == "String", "named tuple member index");
  }
};
