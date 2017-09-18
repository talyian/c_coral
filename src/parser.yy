%skeleton "lalr1.cc"
%require "3.0"
%error-verbose

%define api.value.type variant

%{
#define YYDEBUG 1
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>  
#include "../src/ast.h"
%}

%token EXTERN
%token IDENTIFIER
%token STRING
%token SYMBOL
%token NUMBER
%token NEWLINE

%type <std::string> STRING IDENTIFIER;
%type <Expr*> expr line;
%type <std::vector<Expr*>> lines;
%type <std::vector<Expr*>> argumentlist;
%{
#include "parser.hh"
int yylex(yy::parser::semantic_type * pp);
void handle_module(Module & m);
%}

%%

program : lines { Module m($1); handle_module(m); return 0; }

lines
:       NEWLINE { }
| 	line { $$.push_back($1); }
| 	lines line { $$ = $1; $$.push_back($2); }
| 	lines NEWLINE { $$ = $1; }

line
: EXTERN STRING IDENTIFIER { $$ = new Extern($2, $3); }
| IDENTIFIER argumentlist { $$ = new Call($1, $2); }
     
argumentlist
: expr { $$.push_back($1); }
| '(' expr ')' { $$.push_back($2); }
| '(' expr ',' expr ')' { $$.push_back($2); $$.push_back($4); }
| '(' ')' { }

expr : STRING { $$ = new String($1); }

%%

void yy::parser::error(const std::string& m) {
  std::cout << m << std::endl;
}
