%option noyywrap
%option bison-bridge
%option reentrant
%option yylineno
%{
#include "bisonParser.tab.hh"
#define YYSTYPE yy::parser::semantic_type
#define YY_USER_ACTION { printf("[%s]", yytext); }
%}
%%

#.* { }
\"(\\\"|[^\"])*\" { }
[a-zA-Z][a-zA-Z0-9]* { }
[0-9][0-9]*L? { }
[ ]       { }
\n[ \t]*  { }
.         { }

%%

void yy::parser::error(const std::string & s) { }

int main() {
	FILE * f = fopen("../../tests/cases/shootout/knucleotide.coral", "r");
	void * scanner;
	yylex_init(&scanner);
	yyset_in(f, scanner);
	yy::parser P(scanner);
	P.parse();
	yylex_destroy(scanner);
}
