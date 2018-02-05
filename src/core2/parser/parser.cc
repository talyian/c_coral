#include <cstdio>

#include "parser/lexer-internal.hh"
#include "bisonParser.tab.hh"
#include "core/prettyprinter.hh"
namespace coral {

  void coral::parser::error(const std::string & msg) {
	this->pp->module = 0;
	auto lexer = this->pp->lexer;
	fprintf(stderr, "[Line %d]: Error: %s", lexer->pos.start.row, msg.c_str());
	fprintf(stderr, "  '%s'\n", lexer->text);
  }


  class Parser {
  public:
	coral::ast::BaseExpr * module = 0;
	Parser(const char * infile) {
	  ParserParam pp = new ParserParamStruct();
	  pp->lexer = lexerCreate(infile);
	  coral::parser PP(pp);
	  if (!PP.parse()) module = pp->module;
	  lexerDestroy(pp->lexer);
	  delete pp;
	}
	void writeJson(const char * outfile) { }
	~Parser() {
	  if (module) delete module;
	}
  };
}

#define ParserType coral::Parser *
#define ModuleType void *
#include "parser.hh"

ParserType coralParseModule(const char * infile) { return new coral::Parser(infile); }
void coralJsonModule(ParserType m, const char * outfile) { m->writeJson(outfile); }
void coralDestroyModule(ParserType m) { delete m; }
ModuleType _coralModule(ParserType m) { return m->module; }
