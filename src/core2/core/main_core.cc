#include "expr.hh"
#include <cstdio>

namespace coral {
  class PrettyPrinter : coral::ast::ExprVisitor {
  public:
	PrettyPrinter();
	virtual void visit(ast::Module * m);
  };
}

using namespace coral::ast;
using namespace coral;

int main() {
  coral::PrettyPrinter pp;

  auto m = new Module({
	new Extern("printf", Type("Fn[Int]")),
    new Func("factorial", Type("Int32"), {
		new Return(new Call(new Var("factorial"), {})),
	}),
    new Func("main", Type("Int32"), {
	  new Call(new Var("printf"), {
		  new StringLiteral("Hello, %s!\\n"),
		  new StringLiteral("World")
	  }),
      new Return(new IntLiteral(0)),
	})
  });

  pp.visit(m);
}
