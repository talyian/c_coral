#include <cstdio>

struct ModuleT {

};

#define Module ModuleT *

#include "parser.hh"
#include "lexer-internal.hh"

int coral_parse(ParserParam scanner);

Module coralParseModule(const char * infile) {
  ParserParam pp = new ParserParamStruct();
  pp->lexer = lexerCreate(infile);
  coral_parse(pp);
  lexerDestroy(pp->lexer);
  delete pp;
  return 0;
}

void coralJsonModule(Module m, const char * outfile) {

}

void coralDestroyModule(Module m) {

}
