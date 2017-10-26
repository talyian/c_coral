#include "type.hh"
#include "expr.hh"
#include "treeprinter.hh"

using namespace coral;

int main() {
  auto module = new Module(
    std::vector<Expr *> {
      new Let(
	new Def("a", 0),
	new Long(3)),
	new Let(
	  new Def("b", 0),
	  new Long(458354)),
	new Call(
	  new Var("printf"),
	  std::vector<Expr *> {
	    new String("\"Hello %d\""),
	      new Var("a"),
	      }),
	});
  TreePrinter(module, std::cout).print();
}
