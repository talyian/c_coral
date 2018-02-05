#include "../core/expr.hh"
#include "../core/prettyprinter.hh"
#include "LLVMModuleCompiler.hh"
#include <cstdio>

#include "analyzers/NameResolver.cc"
#include "analyzers/ReturnInserter.hh"
#include "parser/parser.hh"
using namespace coral;

int main() {
  printf("Coral Codegen\n");
  auto parser =  coralParseModule("tests/cases/simple/factorial.coral");
  auto module = (ast::Module *)_coralModule(parser);

  PrettyPrinter::print(module);
  analyzers::NameResolver resolver(module);
  analyzers::ReturnInserter returner(module);
  PrettyPrinter::print(module);

  // codegen::LLVMModuleCompiler codegen(module);
  coralDestroyModule(parser);
}
