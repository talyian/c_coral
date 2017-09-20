%skeleton "lalr1.cc"
%require "3.0"
%error-verbose

%define api.value.type variant
%locations

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
%token FUNC
%token IDENTIFIER
%token STRING
%token SYMBOL
%token FLOAT
%token INTEGER
%token NEWLINE
%token INDENT
%token DEDENT

%type <int64_t> INTEGER
%type <double> FLOAT
%type <std::string> STRING IDENTIFIER;
%type <Expr*> expr line;
%type <std::vector<int>> args_list_inner args_list;
%type <std::vector<Expr*>> lines;
%type <std::vector<Expr*>> param_list;

%{
#include "parser.hh"
int yylex(yy::parser::semantic_type * pp, yy::location * loc);
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
| IDENTIFIER param_list { $$ = new Call($1, $2); }
| FUNC IDENTIFIER args_list ':' NEWLINE line { $$ = new FuncDef($2, $3, $6); }

argument : IDENTIFIER { }
args_list_inner : /* empty */ { }
| argument { $$.push_back(1); }
| args_list_inner ',' argument { $$.push_back(2); }
args_list : '(' args_list_inner ')' { $$ = $2; }

param_list
: expr { $$.push_back($1); }
| '(' expr ')' { $$.push_back($2); }
| '(' expr ',' expr ')' { $$.push_back($2); $$.push_back($4); }
| '(' ')' { }

expr
: STRING { $$ = new String($1); }
| INTEGER { $$ = new Long($1); }
| FLOAT { $$ = new Double($1); }
| expr '+' expr { $$ = new BinOp(std::string("+"), $1, $3); }
| expr '-' expr { $$ = new BinOp(std::string("-"), $1, $3); }
| expr '*' expr { $$ = new BinOp(std::string("*"), $1, $3); }
| expr '/' expr { $$ = new BinOp(std::string("/"), $1, $3); }
| expr '%' expr { $$ = new BinOp(std::string("%"), $1, $3); }

%%

void yy::parser::error(const yy::location &loc, const std::string& m) {
  std::cout << m << std::endl;
}
