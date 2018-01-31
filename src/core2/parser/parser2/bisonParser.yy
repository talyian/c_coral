%language "c++"
%define api.value.type variant
%lex-param {scanner}
%parse-param {void * scanner}

%token <std::string> STRING
%token <std::string> IDENTIFIER
%token <std::string> INTEGER
%token NEWLINE
%token INDENT
%token DEDENT
%token FUNC
%token LET
%token FOR
%token IN
%token IF
%token ELIF
%token ELSE

%type <ast::coral::Expr *> Function ModuleLine ModuleRule Expr ForLoop Let
%type <std::vector<void *>> StatementLinesRule
%{
#include "bisonParser.tab.hh"
int yylex(yy::parser::semantic_type * val, void * scanner);
%}

%%

ModuleRule : StatementLinesRule { }

StatementLinesRule : { }
| StatementLinesRule ModuleLine NEWLINE { }

Expr : IDENTIFIER { }
| Expr '.' IDENTIFIER { }
| Expr '(' ArgumentsListInner ')' { }
| Expr '(' ')' { }
| INTEGER { }
| IDENTIFIER '[' ArgumentsListInner ']' { }
| Expr '+' '=' Expr { }
| Expr '=' '>' Expr { }
| Expr '-' Expr { }
| Expr Expr { }

ArgumentsListInner : Argument { } | ArgumentsListInner ',' Argument { }

Argument : Expr { } | IDENTIFIER '=' Expr { }

Param   : IDENTIFIER { }

ParamsListInner : Param { }
| ParamsListInner ',' Param { }

ModuleLine : Expr { }
| Function { }
| Let { }
| ForLoop { }

Function
: FUNC IDENTIFIER '(' ParamsListInner ')' ':' NEWLINE INDENT StatementLinesRule DEDENT { }
| FUNC IDENTIFIER '(' ')' ':' NEWLINE INDENT StatementLinesRule DEDENT { }

ForLoop : FOR IDENTIFIER IN Expr ':' NEWLINE INDENT StatementLinesRule DEDENT { }

Let : LET IDENTIFIER '=' Expr { }
%%
