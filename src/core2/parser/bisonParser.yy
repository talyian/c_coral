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

%type <expr> Function ModuleLine
%%

ModuleRule : { printf("[RULE] Program Empty\n"); }
| ModuleRule NEWLINE { printf("[RULE] Program continued \n"); }
| ModuleRule ModuleLine NEWLINE { printf("[RULE] Program continued \n"); }

StatementLinesRule : { printf("Statement Lines \n"); }
| StatementLinesRule ModuleLine NEWLINE { printf ("line\n"); }

BodyRule : INDENT StatementLinesRule DEDENT { }

Expr : IDENTIFIER { }
| Expr '.' IDENTIFIER { }
| Expr '(' ArgumentsListInner ')' { }
| Expr '(' ')' { }
| INTEGER { }

ArgumentsListInner : Argument { } | ArgumentsListInner ',' Argument { }

Argument : Expr { } | IDENTIFIER '=' Expr { printf("Argument Name %s", $1); }

Param   : IDENTIFIER { }

ParamsListInner : Param { }
| ParamsListInner ',' Param { }

ModuleLine : Expr { }
| Function { $$ = $1; }
| Let { }

Function
: FUNC IDENTIFIER '(' ParamsListInner ')' ':' NEWLINE BodyRule { printf("[RULE] function\n"); }
| FUNC IDENTIFIER '(' ')' ':' NEWLINE BodyRule { printf("[RULE] function\n"); }

Let : LET IDENTIFIER '=' Expr { printf("LET [%.*s]\n\n", $2.len, $2.buf); }
%%
