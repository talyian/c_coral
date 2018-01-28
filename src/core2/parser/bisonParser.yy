%define api.pure full
%name-prefix "zz"
%lex-param {scanner}

%parse-param {ParserParam scanner}

%{
#include "lexer-internal.hh"

#include <cstdio>

void yyerror(ParserParam pp, const char * msg) {
	printf("Error: %s\n", msg, pp);
}

%}

%token STRING 1001
%token IDENTIFIER 1002
%token NEWLINE 1003
%token INDENT 1004
%token DEDENT 1005
%token FUNC 1006

%%

line : IDENTIFIER { }
	 | line IDENTIFIER { }
	 | line '(' { }

program : NEWLINE { printf("[RULE] Program\n"); }
		| program line NEWLINE { printf("[RULE] Identifier\n"); }

%%
