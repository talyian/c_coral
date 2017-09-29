%skeleton "lalr1.cc"
%require "3.0"
%error-verbose

%{
#define YYDEBUG 1
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>  
#include "ast.hh"
%}

%token-table
%parse-param { Module *&module }
%define api.value.type variant
%locations

%token ADDR_OF
%token LET
%token AS
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
%token IF
%token ELSE
%token RETURN

%type <int64_t> INTEGER
%type <double> FLOAT
%type <std::string> STRING IDENTIFIER 
%type <Expr*> expr line Argument BlockOrExpr;
%type <Def *> Parameter;
%type <std::vector<Def *>> ParameterList_inner ParameterList;
%type <std::vector<Expr*>> lines;
%type <std::vector<Expr*>> ArgumentList ArgumentList_inner;
%type <BlockExpr *> block;

%type <Type *> typedef;
%type <std::vector<Type *>> TypeList

%{
#include "parser.hh"
int yylex(yy::parser::semantic_type * pp, yy::location * loc);
Module * __module;
void handle_module(Module & m);
%}

%%

program : lines { module = new Module($1); }

lines
:       NEWLINE { }
| 	line { $$.push_back($1); }
| 	lines line { $$ = $1; $$.push_back($2); }
| 	lines NEWLINE { $$ = $1; }

line
: EXTERN STRING IDENTIFIER { $$ = new Extern($2, $3, 0); }
| EXTERN STRING IDENTIFIER ':' typedef { $$ = new Extern($2, $3, $5); }
| FUNC IDENTIFIER ':' typedef ParameterList block { $$ = new FuncDef($2, $4, $5, $6, false); }
| FUNC '[' ']' IDENTIFIER ':' typedef ParameterList block { $$ = new FuncDef($4, $6, $7, $8, true); }
| LET Parameter '=' expr { $$ = new Let($2, $4); }
| RETURN expr { $$ = new Return($2); }
| expr { $$ = $1; }

block : ':' NEWLINE INDENT lines DEDENT { $$ = new BlockExpr($4); }

Parameter
: IDENTIFIER { $$ = new Def($1, new Type()); }
| IDENTIFIER ':' typedef { $$ = new Def($1, $3); }

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
| expr '+' expr { $$ = new BinOp(std::string("+"), $1, $3); }
| expr '-' expr { $$ = new BinOp(std::string("-"), $1, $3); }
| expr '*' expr { $$ = new BinOp(std::string("*"), $1, $3); }
| expr '/' expr { $$ = new BinOp(std::string("/"), $1, $3); }
| expr '%' expr { $$ = new BinOp(std::string("%"), $1, $3); }
| expr '<' expr { $$ = new BinOp(std::string("<"), $1, $3); }
| expr '>' expr { $$ = new BinOp(std::string(">"), $1, $3); }
| expr '=' expr { $$ = new BinOp(std::string("="), $1, $3); }
| expr '<' '=' expr { $$ = new BinOp(std::string("<="), $1, $4); }
| expr '>' '=' expr { $$ = new BinOp(std::string(">="), $1, $4); }
| expr '!' '=' expr { $$ = new BinOp(std::string("!="), $1, $4); }

| IDENTIFIER ArgumentList { $$ = new Call($1, $2); }
| IF expr BlockOrExpr ELSE BlockOrExpr { $$ = new If($2, $3, $5); }
| expr AS typedef { $$ = new Cast($1, $3); }
| ADDR_OF IDENTIFIER { $$ = new AddrOf($2); }

BlockOrExpr
: block { $$ = $1; }
| expr { $$ = $1; }

typedef
: IDENTIFIER {
   $$ = (
       $1 == "Void" ? (Type *)new VoidType() :
       $1 == "Bool" ? (Type *)new IntType(1) :       
       $1 == "Int1" ? (Type *)new IntType(1) :
       $1 == "Int8" ? (Type *)new IntType(8) :
       $1 == "Int16" ? (Type *)new IntType(16) :
       $1 == "Int32" ? (Type *)new IntType(32) :
       $1 == "Int64" ? (Type *)new IntType(64) :
       $1 == "Int" ? (Type *)new IntGeneric() :
       $1 == "Float64" ? (Type *)new FloatType('b', 64) :
       $1 == "Float32" ? (Type *)new FloatType('b', 32) :       
   0);
   if (!$$) {
     std::cerr << "unknown type: " << $1 << std::endl;
   }
}
| '.' '.' '.' { $$ = 0; }
| IDENTIFIER '[' TypeList ']' {
  if ($1 == "Ptr") $$ = new PtrType($3[0]);
  else if ($1 == "Arr") $$ = new ArrType($3[0], 0);
  else if ($1 == "Fn") {
    auto retv = $3.back();
    auto var = false;
    $3.pop_back();
    for(auto i=$3.begin(); i!=$3.end(); i++) {
       if (!*i) {
         var = true;
         $3.erase(i);
         break;
       }
    }
    $$ = new FuncType(retv, $3, var);
  }
  else {
  std::cerr << "unknown type: " << $1 << std::endl;
  exit(1);
  $$ = 0;
  }
}
TypeList
: typedef { $$.push_back($1); }
| TypeList ',' typedef {
  $$ = $1;
  if ($3) $$.push_back($3);
}

%%

void yy::parser::error(const yy::location &loc, const std::string& m) {
  std::cout << '[' << loc << "]: " << m << std::endl;
}
