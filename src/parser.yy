%skeleton "lalr1.cc"
%require "3.0"
%error-verbose

%{
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include "ast.hh"
%}

%token-table
%parse-param { Module *&module } // pass in our output result
%parse-param { void * yyscanner }
%lex-param { yyscanner }
%define api.value.type variant
%locations

%token ADDR_OF
%token LET
%token AS
%token EXTERN
%token FUNC
%token <std::string> IDENTIFIER
%token <std::string> STRING
%token SYMBOL
%token <double> FLOAT
%token <int64_t> INTEGER
%token NEWLINE
%token INDENT
%token DEDENT
%token IF
%token ELSE
%token RETURN


/* %type <int64_t> INTEGER */
/* %type <double> FLOAT */
/* %type <Expr*> expr line Argument BlockOrExpr; */
/* %type <Def *> Parameter; */
/* %type <std::vector<Def *>> ParameterList_inner ParameterList; */
/* %type <std::vector<Expr*>> lines; */
/* %type <BlockExpr *> block; */
/* %type <Type *> typedef; */
/* %type <std::vector<Type *>> TypeList */

%type <std::vector<Expr *>> lines ArgumentList ArgumentList_inner
%type <Expr *> line expr Argument BlockOrExpr
%type <Type *> typesig
%type <std::vector<Type *>> TypeList
%type <Def *> Parameter;
%type <std::vector<Def *>> ParameterList_inner ParameterList
%type <BlockExpr *> block
%{
#include "parser.hh"
int yylex(yy::parser::semantic_type * pp, yy::location * loc, void * yyscanner);
%}

%%

program : lines { module = new Module($1); }

lines
:       NEWLINE { }
| 	lines NEWLINE { $$ = $1; }
| 	line { $$.push_back($1); }
| 	lines line { $$ = $1; $$.push_back($2); }

line
: EXTERN STRING IDENTIFIER { $$ = new Extern($2, $3, new Type()); }
| EXTERN STRING IDENTIFIER ':' typesig { $$ = new Extern($2, $3, $5); }
| FUNC IDENTIFIER ':' typesig ParameterList block { $$ = BuildFunc($2, $4, $5, $6); }
| FUNC '[' ']' IDENTIFIER ':' typesig ParameterList block { $$ = BuildVarFunc($4, $6, $7, $8); }
| LET Parameter '=' expr { $$ = new Let($2, $4); }
| RETURN expr { $$ = new Return($2); }
| expr { $$ = $1; }

block : ':' NEWLINE INDENT lines DEDENT { $$ = new BlockExpr($4); }

Parameter
: IDENTIFIER { $$ = new Def($1, new Type()); }
| IDENTIFIER ':' typesig { $$ = new Def($1, $3); }

ParameterList_inner
: Parameter { $$.push_back($1); }
| ParameterList_inner ',' Parameter { $$ = $1; $$.push_back($3); }

ParameterList
: '(' ')' { }
| '(' ParameterList_inner ')' { $$ = $2; }

Argument : expr { $$ = $1; }
ArgumentList
: '(' ')' { }
| Argument { $$.push_back($1); }
| '(' ArgumentList_inner ')' { $$ = $2; }
ArgumentList_inner
: Argument { $$.push_back($1); }
| ArgumentList_inner ',' Argument { $$ = $1; $$.push_back($3); }

expr
: STRING { $$ = new String($1); }
| INTEGER { $$ = new Long($1); }
| FLOAT { $$ = new Double($1); }
| IDENTIFIER { $$ = new Var($1); }
| expr '+' expr { $$ = new BinOp("+", $1, $3); }
| expr '-' expr { $$ = new BinOp("-", $1, $3); }
| expr '*' expr { $$ = new BinOp("*", $1, $3); }
| expr '/' expr { $$ = new BinOp("/", $1, $3); }
| expr '%' expr { $$ = new BinOp("%", $1, $3); }
| expr '<' expr { $$ = new BinOp("<", $1, $3); }
| expr '>' expr { $$ = new BinOp(">", $1, $3); }
| expr '=' expr { $$ = new BinOp("=", $1, $3); }
| expr '<' '=' expr { $$ = new BinOp("<=", $1, $4); }
| expr '>' '=' expr { $$ = new BinOp(">=", $1, $4); }
| expr '!' '=' expr { $$ = new BinOp("!=", $1, $4); }

| IDENTIFIER ArgumentList { $$ = new Call($1, $2); }
| IF expr BlockOrExpr ELSE BlockOrExpr { $$ = new If($2, $3, $5); }
| expr AS typesig { $$ = new Cast($1, $3); }
| ADDR_OF IDENTIFIER { $$ = new AddrOf($2); }

BlockOrExpr
: block { $$ = $1; }
| expr { $$ = $1; }

typesig
: IDENTIFIER { $$ = BuildType($1); }
| '.' '.' '.' { $$ = BuildType("..."); }
| IDENTIFIER '[' TypeList ']' { $$ = BuildType($1, $3); }
TypeList
: typesig { $$.push_back($1); }
| TypeList ',' typesig { $$ = $1; $$.push_back($3); }

%%

void yy::parser::error(const yy::location &loc, const std::string& m) {
  std::cout << '[' << loc << "]: " << m << std::endl;
}
