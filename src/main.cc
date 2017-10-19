#include <string>
#include <stack>
#include <cstdio>
#include <unistd.h>
#include <vector>
#include "ast.hh"
#include "parser.hh"
#include "lexer.hh"
#include "../src/lib/_include_all.cc"
#include "treeprinter.hh"
#include "mainfuncPass.hh"
#include "inferTypePass.hh"

std::string ir_module(Module *m);
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
  if (command == "lex") {
    void * scanner;
    YYSTYPE lval;
    YYLTYPE loc;
    yylex_init(&scanner);
    yyset_in(fopen(argv[2], "r"), scanner);
    int t = 0;
    while((t = yylex(&lval, &loc, scanner))) {
      string text(yyget_text(scanner));
      replace(text.begin(), text.end(), '\n', ' ');
      printf( "[%4d] %s\n", t, text.c_str());
    }
    yylex_destroy(scanner);
  }
  else if (command == "parse") {
    auto module = get_module(fopen(argv[2], "r"));
    // module = buildMainFunction(module);
    module = inferTypes(module);
    TreePrinter tp(module, std::cerr);
    tp.print();
    delete module;
  }
  if (command == "ir") { 
    auto module = get_module(fopen(argv[2], "r"));
    std::cerr << ir_module(module) << std::endl;
    delete module;
  } else if (command == "jit") {
    auto modules = std::vector<Module *>();
    modules.push_back(get_module(fopen("samples/prelude/extern.coral", "r")));
    for(int i=0; i<argc - 2; i++) {
      std::cerr << argv[i + 2] << "\n";
      auto module = get_module(fopen(argv[i + 2], "r"));
      modules.push_back(inferTypes(module));
    }
    jit_modules(modules);
  }
}
