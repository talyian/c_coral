#include <iostream>
#include <llvm-c/Core.h>


#include "../parsing/lexer.hh"
#include "../core/treeprinter.hh"
#include "../codegen/codegen.hh"

#include "../passes/InferTypesPass.cc"
#include "../passes/ReturnInsertPass.cc"
#include "../passes/MainFuncPass.cc"

using namespace coral;
using namespace std;

int main() {
  cout << "---- [ Codegen Test ] ----\n";
  auto m = parse(0, R"CORAL(
extern "C" printf : Fn[..., Void]

func main ():
  printf "Hello World\n"

)CORAL");
  m->name = "codegen-test";
  m = parse(fopen("tests/libs/syncio.coral", "r"), 0);
  m = (Module *)InferTypesPass(m).out;
  m = (Module *)ReturnInsertPass(m).out;
  m = (Module *)MainFuncPass(m).out;
  TreePrinter(m, cout).print();
  // cout << ModuleBuilder(m).finalize();
}
