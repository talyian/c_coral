#include <cstdio>

#include "lexer-internal.hh"
#include "bisonParser.tab.hh"

namespace coral {
  class Parser {
  public:
	Parser(const char * infile) {
	  ParserParam pp = new ParserParamStruct();
	  pp->lexer = lexerCreate(infile);
	  coral_parse(pp);
	  if (pp->module) delete pp->module;
	  lexerDestroy(pp->lexer);
	  delete pp;
	}
	void writeJson(const char * outfile) { }
  };
}

#define ParserModule coral::Parser *
#include "parser.hh"

ParserModule coralParseModule(const char * infile) { return new coral::Parser(infile); }
void coralJsonModule(ParserModule m, const char * outfile) { m->writeJson(outfile); }
void coralDestroyModule(ParserModule m) { delete m; }
