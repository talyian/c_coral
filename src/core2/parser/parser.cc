#include <cstdio>

struct ModuleT {
  FILE * fp = 0;
};

#define Module ModuleT *

#include "parser.hh"
#include "lexer-internal.hh"

int zzparse(ParserParam scanner);

Module coralParseModule(const char * infile) {
  ParserParam scanner = new ParserParamStruct();
  scanner->lexer = lexerCreate(infile);
  zzparse(scanner);
  lexerDestroy(scanner->lexer);
  delete scanner;
  return 0;
}

void coralJsonModule(Module m, const char * outfile) {

}

void coralDestroyModule(Module m) {

}
