%define api.pure full
%lex-param {pp}
%name-prefix "coral_"
%parse-param {ParserParam pp}

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
%token FOR
%token IN

%type <expr> Function ModuleLine ModuleRule Expr ForLoop Let
%type <exprlines> StatementLinesRule

%%

ModuleRule : StatementLinesRule {
  std::vector<coral::ast::BaseExpr *> bb = *$1;
  pp->module = new coral::ast::Module(bb);
  delete $1;
  for(auto v: bb) delete v;
  printf("done loading module\n");
  return 0;
}

StatementLinesRule : { $$ = new std::vector<coral::ast::BaseExpr *>(); }
| StatementLinesRule ModuleLine NEWLINE { $$ = $1; $$->push_back($2); }

Expr : IDENTIFIER { $$ = 0; }
| Expr '.' IDENTIFIER { $$ = 0; }
| Expr '(' ArgumentsListInner ')' { $$ = 0; }
| Expr '(' ')' { $$ = 0; }
| INTEGER { $$ = 0; }
| IDENTIFIER '[' ArgumentsListInner ']' { $$ = 0; }
| Expr '+' '=' Expr { $$ = 0; }
| Expr '=' '>' Expr { $$ = 0; }
| Expr '-' Expr { $$ = 0; }
| Expr Expr { $$ = 0; }

ArgumentsListInner : Argument { } | ArgumentsListInner ',' Argument { }

Argument : Expr { } | IDENTIFIER '=' Expr { printf("Argument Name %s", $1); }

Param   : IDENTIFIER { }

ParamsListInner : Param { }
| ParamsListInner ',' Param { }

ModuleLine : Expr { $$ = $1; }
| Function { $$ = $1; }
| Let { $$ = $1; }
| ForLoop { $$ = $1; }

Function
: FUNC IDENTIFIER '(' ParamsListInner ')' ':' NEWLINE INDENT StatementLinesRule DEDENT { $$ = 0; }
| FUNC IDENTIFIER '(' ')' ':' NEWLINE INDENT StatementLinesRule DEDENT { $$ = 0; }

ForLoop : FOR IDENTIFIER IN Expr ':' NEWLINE INDENT StatementLinesRule DEDENT { $$ = 0; }

Let : LET IDENTIFIER '=' Expr { $$ = 0; }
%%
