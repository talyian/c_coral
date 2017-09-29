#include <string>
#include <stack>
#include <cstdio>
#include <unistd.h>
#include "ast.hh"
#include "parser.hh"
#include "lexer.hh"
#include "compiler.hh"

int main(int argc, char **argv) {
  Module * module;
  void * scanner;
  yylex_init(&scanner);
  yyset_in(fopen(argv[2], "r"), scanner);

  yy::parser coralp(module, scanner);
  coralp.parse();
  std::cerr << module->toString() << std::endl;

  ModuleCompiler c(module);
  std::cerr << c.getIR() << std::endl;
  std::cout << c.getIR() << std::endl;

  delete module;
  yylex_destroy(scanner);
}
