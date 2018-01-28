#include <cstdio>

struct ModuleT {
  FILE * fp = 0;
};

#define Module ModuleT *

#include "parser.hh"
#include "lexer-internal.hh"

int zzparse(ParserParam scanner);

Module coralParseModule(char * infile) {
  // auto lexer = lexerCreate(infile);
  // Position position;
  // int length;
  // char * text;
  // while(lexerRead(lexer, &text, &length, &position)) {
  // 	printf(
  // 	  "Token: [%d:%d] %s\n",
  // 	  position.start.row,
  // 	  position.start.col, text);
  // }
  // lexerDestroy(lexer);

  ParserParam scanner = new ParserParamStruct();
  scanner->lexer = lexerCreate(infile);
  zzparse(scanner);
  return 0;

  auto m = new ModuleT();
  if (infile) m->fp = fopen(infile, "r");
  return 0;
}

void coralJsonModule(Module m, char * outfile) {

}

void coralDestroyModule(Module m) {

}
