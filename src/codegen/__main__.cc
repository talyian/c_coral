#include <iostream>
#include <llvm-c/Core.h>
#include "codegen.hh"

using namespace coral;
using namespace std;

int main() {
  cout << "---- [ Codegen Test ] ----\n";
  auto m = new Module(vector<Expr *>{
	  new FuncDef(
		"Main",
		new IntType(32),
		vector<BaseDef *>{ },
		new Return(new Long(1)),
		false),
  });
  m->name = "codegen-test";
  cout << ModuleBuilder(m).finalize();
}
