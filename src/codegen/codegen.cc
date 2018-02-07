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
