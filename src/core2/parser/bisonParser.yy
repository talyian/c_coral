%define api.pure full
%lex-param {scanner}
%name-prefix "coral_"
%parse-param {ParserParam scanner}

%{
#include "lexer-internal.hh"
#include <cstdio>
void yyerror(ParserParam pp, const char * msg) {
	printf("Error: %s\n", msg, pp);
}
%}

%token <str> STRING
%token <str> IDENTIFIER
%token <str> INTEGER
%token NEWLINE
%token INDENT
%token DEDENT
%token FUNC
%token LET

%%

ModuleRule : { printf("[RULE] Program Empty\n"); }
| ModuleRule NEWLINE { printf("[RULE] Program continued \n"); }
| ModuleRule line NEWLINE { printf("[RULE] Program continued \n"); }

StatementLinesRule : { printf("Statement Lines \n"); }
| StatementLinesRule line NEWLINE { printf ("line\n"); }

BodyRule : INDENT StatementLinesRule DEDENT { }

Expr : IDENTIFIER { }
| Expr '.' IDENTIFIER { }
| Expr '(' ArgumentsListInner ')' { }
| Expr '(' ')' { }
| INTEGER { }

ArgumentsListInner : Argument { } | ArgumentsListInner ',' Argument { }

Argument : Expr { } | IDENTIFIER '=' Expr { printf("Argument Name %s", $1); }

function
: FUNC IDENTIFIER '(' ParamsListInner ')' ':' NEWLINE BodyRule { printf("[RULE] function\n"); }
| FUNC IDENTIFIER '(' ')' ':' NEWLINE BodyRule { printf("[RULE] function\n"); }

Param   : IDENTIFIER { }

ParamsListInner : Param { }
| ParamsListInner ',' Param { }

line : Expr { }
| function { }
| LET IDENTIFIER '=' Expr { }
| line IDENTIFIER { }
| line '(' { }

%%
