#include <cstdio>

#include "lexer-internal.hh"
#include "bisonParser.tab.hh"
#include "../core/prettyprinter.hh"
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
	  if (!PP.parse()) {
		module = pp->module;
		if (pp->module) {
		  coral::PrettyPrinter pretty;
		  pp->module->accept(&pretty);
		}
	  }
	  lexerDestroy(pp->lexer);
	  delete pp;
	}
	void writeJson(const char * outfile) { }
	~Parser() {
	  if (module) delete module;
	}
  };
}

#define ParserModule coral::Parser *
#include "parser.hh"

ParserModule coralParseModule(const char * infile) { return new coral::Parser(infile); }
void coralJsonModule(ParserModule m, const char * outfile) { m->writeJson(outfile); }
void coralDestroyModule(ParserModule m) { delete m; }
