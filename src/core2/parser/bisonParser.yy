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
%token LET 1007

%%

ModuleRule
        : { printf("[RULE] Program Empty\n"); }
		| ModuleRule NEWLINE { printf("[RULE] Program continued \n"); }
		| ModuleRule line NEWLINE { printf("[RULE] Program continued \n"); }

StatementLinesRule : { printf("Statement Lines \n"); }
		| StatementLinesRule line NEWLINE { printf ("line\n"); }

BodyRule : INDENT StatementLinesRule DEDENT { }

function
   : FUNC IDENTIFIER '(' ParamsListInner ')' ':' NEWLINE BodyRule { printf("[RULE] function\n"); }
   | FUNC IDENTIFIER '(' ')' ':' NEWLINE BodyRule { printf("[RULE] function\n"); }

Param   : IDENTIFIER { }

ParamsListInner : Param { }
		|		ParamsListInner ',' Param { }

line
 : IDENTIFIER { }
 | function { }
 | LET IDENTIFIER '=' IDENTIFIER { }
 | line IDENTIFIER { }
 | line '(' { }

%%
