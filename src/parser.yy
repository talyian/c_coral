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

%token FOR
%token IN
%token IMPL
%token CLASS
%token MATCH
%token MODULE
%token TYPE
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

%type <std::vector<Expr *>> lines ArgumentList ArgumentList_inner
%type <std::vector<Expr *>> enumBlock matchBlock
%type <std::vector<Expr *>> enumLines matchLines
%type <Expr *> enumLine matchLine
%type <Expr *> line expr Argument BlockOrExpr 
%type <Type *> typesig
%type <std::vector<Type *>> TypeList
%type <Def *> Parameter classLine
%type <std::vector<Def *>> ParameterList_inner ParameterList classLines classBlock
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
: EXTERN STRING IDENTIFIER { $$ = new Extern($2, $3, new UnknownType()); }
| EXTERN STRING IDENTIFIER ':' typesig { $$ = new Extern($2, $3, $5); }
| FUNC IDENTIFIER ParameterList block { $$ = BuildFunc($2, 0, $3, $4); }
| FUNC IDENTIFIER ':' typesig ParameterList block { $$ = BuildFunc($2, $4, $5, $6); }
| FUNC '[' ']' IDENTIFIER ':' typesig ParameterList block { $$ = BuildVarFunc($4, $6, $7, $8); }
| MODULE IDENTIFIER { $$ = new Expr(); }
| MODULE IDENTIFIER '.' IDENTIFIER { $$ = new Expr(); }
| TYPE IDENTIFIER '=' typesig { $$ = new DeclTypeAlias($2, $4); }
| TYPE IDENTIFIER enumBlock { $$ = new DeclTypeEnum($2, "??", $3); }
| TYPE IDENTIFIER ':' IDENTIFIER enumBlock { $$ = new DeclTypeEnum($2, $4, $5); }
| CLASS IDENTIFIER classBlock { $$ = new DeclClass($2, $3); }
| CLASS IDENTIFIER FOR IDENTIFIER classBlock { $$ = new DeclClass($2, $5); }
| IMPL IDENTIFIER block { $$ = new ImplType($2, $3); }
| IMPL IDENTIFIER FOR IDENTIFIER block { $$ = new ImplClassFor($2, $4, $5); }
| LET Parameter '=' expr { $$ = new Let($2, $4); }
| RETURN expr { $$ = new Return($2); }
| expr { $$ = $1; }
| FOR ParameterList_inner IN expr block { $$ = new For($2, $4, $5); }

classBlock : ':' NEWLINE INDENT classLines DEDENT { $$ = $4; }
classLines : NEWLINE { }
           | classLine { $$.push_back($1); }
	   | classLines NEWLINE { $$ = $1; }
	   | classLines classLine { $$ = $1; $$.push_back($2); }
classLine  : Parameter { $$ = $1; }

// A block for a match body
matchBlock :  ':' NEWLINE INDENT matchLines DEDENT { $$ = $4; }
matchLines
        : NEWLINE { }
	|	matchLines NEWLINE { $$ = $1; }
	|	matchLine { $$.push_back($1); }
	|	matchLines matchLine { $$= $1; $$.push_back($2); }
matchLine
: IDENTIFIER block { $$ = new MatchCaseTagsExpr(new Var($1), $2); }

// A block defining an enumtype
enumBlock: ':' NEWLINE INDENT enumLines DEDENT { $$ = $4; }
enumLines
  : NEWLINE { }
  | enumLines NEWLINE { $$ = $1; }
  | enumLine { $$.push_back($1); }
  | enumLines enumLine { $$ = $1; $$.push_back($2); }
enumLine
  : '|' IDENTIFIER { $$ = new EnumCase($2); }
  | '|' IDENTIFIER '(' ParameterList_inner ')' { $$ = new EnumCase($2, $4); }
  | '|' IDENTIFIER '=' expr { $$ = new EnumCase($2); }
		
block
: ':' NEWLINE INDENT lines DEDENT { $$ = new BlockExpr($4); }
| ':' line { $$ = new BlockExpr(std::vector<Expr *>()); $$->lines.push_back($2); }

Parameter
: IDENTIFIER { $$ = new Def($1, new UnknownType()); }
| IDENTIFIER ':' typesig { $$ = new Def($1, $3); }
| IDENTIFIER '.' IDENTIFIER ':' typesig { $$ = new Def($3, $5); }
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
| expr '+' '=' expr { $$ = new BinOp("+=", $1, $4); }
| expr '*' '=' expr { $$ = new BinOp("*=", $1, $4); }
| expr '-' '=' expr { $$ = new BinOp("-=", $1, $4); }
| expr '/' '=' expr { $$ = new BinOp("/=", $1, $4); }
| expr '@' expr { $$ = new BinOp("@", $1, $3); }
// TODO uhhh this is not a regular binop
| expr '=' '>' expr { $$ = new BinOp("=>", $1, $4); }

| expr '.' IDENTIFIER { $$ = new Call(new Var($3), std::vector<Expr*>{ $1 }); }
| expr '.' IDENTIFIER '(' ArgumentList ')' {
     $5.insert($5.begin(), $1);
     $$ = new Call(new Var($3), $5); }
| expr '[' expr ']' { $$ = new Index($1, std::vector<Expr *>{ $3 }); }
| expr '[' expr ',' expr ']' { $$ = new Index($1, std::vector<Expr *>{ $3, $5 }); }

| expr ArgumentList { $$ = new Call($1, $2); }
| IF expr BlockOrExpr ELSE BlockOrExpr { $$ = new If($2, $3, $5); }
| expr AS typesig { $$ = new Cast($1, $3); }
| ADDR_OF IDENTIFIER { $$ = new AddrOf($2); }
| MATCH expr matchBlock { $$ = new MatchExpr($2, $3); }

| '[' ArgumentList_inner ']' { $$ = new Call(new Var("List.create"), $2); }


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
