#include <cstdio>

struct ModuleT {

};

#define Module ModuleT *

#include "parser.hh"
#include "lexer-internal.hh"

int zzparse(ParserParam scanner);

Module coralParseModule(const char * infile) {
  ParserParam pp = new ParserParamStruct();
  pp->lexer = lexerCreate(infile);
  zzparse(pp);
  lexerDestroy(pp->lexer);
  delete pp;
  return 0;
}

void coralJsonModule(Module m, const char * outfile) {

}

void coralDestroyModule(Module m) {

}
