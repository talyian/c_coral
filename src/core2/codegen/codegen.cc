#include "../core/expr.hh"
#include "../core/prettyprinter.hh"
#include "LLVMModuleCompiler.hh"
#include "LLVMJit.hh"
#include <cstdio>

#include "analyzers/NameResolver.cc"
#include "analyzers/ReturnInserter.hh"
#include "parser/parser.hh"
using namespace coral;

void Run(const char * path) {
  auto parser =  coralParseModule(path);
  auto module = (ast::Module *)_coralModule(parser);

  PrettyPrinter::print(module);
  analyzers::NameResolver resolver(module);
  analyzers::ReturnInserter returner(module);
  PrettyPrinter::print(module);

  codegen::LLVMModuleCompiler compiler(module);
  codegen::LLVMRunJIT(compiler.llvmModule);
  compiler.llvmModule = 0;
  coralDestroyModule(parser);
}
int main() {
  printf("Coral Codegen\n");
  Run("tests/cases/simple/collatz.coral");
  return 0;
  Run("tests/cases/simple/factorial.coral");
  Run("tests/cases/simple/hello_world.coral");
  Run("tests/cases/simple/hello_world.coral");
  Run("tests/cases/simple/hello_world.coral");
}
