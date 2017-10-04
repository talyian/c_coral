#include <string>
#include <stack>
#include <cstdio>
#include <unistd.h>
#include <vector>
#include "ast.hh"
#include "parser.hh"
#include "lexer.hh"
#include "compiler.hh"

std::string handle_module(Module *m);
void        jit_modules(std::vector<Module *> args);

Module * get_module(FILE * in) {
  Module * module;
  void * scanner;
  yylex_init(&scanner);
  yyset_in(in, scanner);
  yy::parser coralp(module, scanner);
  coralp.parse();
  yylex_destroy(scanner);
  return module;
}

int main(int argc, char **argv) {
  std::string command(argv[1]);
  if (command == "ir") { 
    auto module = get_module(fopen(argv[2], "r"));
    std::cerr << handle_module(module) << std::endl;
    delete module;
  } else if (command == "jit") {
    auto modules = std::vector<Module *>();
    modules.push_back(get_module(fopen("samples/prelude/extern.coral", "r")));
    for(int i=0; i<argc - 2; i++) {
      std::cerr << argv[i + 2] << "\n";
      modules.push_back(get_module(fopen(argv[i + 2], "r")));
    }
    jit_modules(modules);
  }
}
