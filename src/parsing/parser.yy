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
%token ELIF
%token RETURN
%token PASS

%precedence OP_ARROW
%precedence OP_EQ OP_LTE OP_GTE OP_LT OP_GT OP_NE
%precedence OP_MULEQ OP_DIVEQ OP_ADDEQ OP_SUBEQ OP_MODEQ
%precedence OP_BIND

%precedence OP_OPERATOR

%left OP_ADD OP_SUB
%precedence OP_MOD
%left OP_MUL OP_DIV
%precedence OP_NOT


%type <std::vector<Expr *>> lines ArgumentList
%type <std::vector<Expr *>> enumBlock matchBlock
%type <std::vector<Expr *>> enumLines matchLines
%type <std::vector<Expr *>> Tuple_inner
%type <Expr *> enumLine matchLine
%type <Expr *> line expr IfExpr ElseSequence
%type <Tuple *> Tuple
%type <Type *> typesig
%type <std::vector<Type *>> TypeList
%type <Def *> Parameter classLine
%type <std::vector<Def *>> ParameterList_inner ParameterList classLines classBlock
%type <BlockExpr *> block
%{
#include "../../core/expr.hh"
using namespace coral;
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
| TYPE IDENTIFIER OP_EQ typesig { $$ = new DeclTypeAlias($2, $4); }
| TYPE IDENTIFIER enumBlock { $$ = new DeclTypeEnum($2, "??", $3); }
| TYPE IDENTIFIER ':' IDENTIFIER enumBlock { $$ = new DeclTypeEnum($2, $4, $5); }
| CLASS IDENTIFIER classBlock { $$ = new DeclClass($2, $3); }
| CLASS IDENTIFIER FOR IDENTIFIER classBlock { $$ = new DeclClass($2, $5); }
| IMPL IDENTIFIER block { $$ = new ImplType($2, $3); }
| IMPL IDENTIFIER FOR IDENTIFIER block { $$ = new ImplClassFor($2, $4, $5); }
| LET Parameter OP_EQ expr { $$ = new Let($2, $4); }
| RETURN expr { $$ = new Return($2); }
| PASS { $$ = new VoidExpr(); }
| expr { $$ = $1; }
| FOR ParameterList_inner IN expr block { $$ = new For($2, $4, $5); }
| IfExpr { $$ = $1; }

classBlock : ':' NEWLINE INDENT classLines DEDENT { $$ = $4; }
classLines
 : NEWLINE { }
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
 | IDENTIFIER '(' ParameterList_inner ')' block {
     $$ = new MatchCaseTagsExpr(new MatchEnumCaseExpr($1, $3), $5); }
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
  | '|' IDENTIFIER OP_EQ expr { $$ = new EnumCase($2); }

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

ArgumentList : expr  { $$.push_back($1); }
	     | Tuple { $$ = $1->items; }

expr
: STRING { $$ = new String($1); }
| INTEGER { $$ = new Long($1); }
| FLOAT { $$ = new Double($1); }
| IDENTIFIER { $$ = new Var($1); }

| expr OP_ADD expr { $$ = new BinOp("+", $1, $3); }
| expr OP_SUB expr { $$ = new BinOp("-", $1, $3); }
| expr OP_MUL expr { $$ = new BinOp("*", $1, $3); }
| expr OP_DIV expr { $$ = new BinOp("/", $1, $3); }
| expr OP_MOD expr { $$ = new BinOp("%", $1, $3); }
| expr OP_LT expr { $$ = new BinOp("<", $1, $3); }
| expr OP_GT expr { $$ = new BinOp(">", $1, $3); }
| expr OP_EQ expr { $$ = new BinOp("=", $1, $3); }
| expr OP_LTE expr { $$ = new BinOp("<=", $1, $3); }
| expr OP_GTE expr { $$ = new BinOp(">=", $1, $3); }
| expr OP_NE expr { $$ = new BinOp("!=", $1, $3); }
| expr OP_ADDEQ expr { $$ = new BinOp("+=", $1, $3); }
| expr OP_MULEQ expr { $$ = new BinOp("*=", $1, $3); }
| expr OP_SUBEQ expr { $$ = new BinOp("-=", $1, $3); }
| expr OP_DIVEQ expr { $$ = new BinOp("/=", $1, $3); }
| expr OP_MODEQ expr { $$ = new BinOp("%=", $1, $3); }
| expr OP_BIND expr { $$ = new BinOp("@", $1, $3); }
| expr OP_ARROW expr { $$ = new BinOp("=>", $1, $3); }

| expr '.' IDENTIFIER { $$ = new Call(new Var($3), std::vector<Expr*>{ $1 }); }
| expr '.' IDENTIFIER '(' ArgumentList ')' {
     $5.insert($5.begin(), $1);
     $$ = new Call(new Var($3), $5); }
| expr '[' expr ']' { $$ = new Index($1, std::vector<Expr *>{ $3 }); }
| expr '[' expr ',' expr ']' { $$ = new Index($1, std::vector<Expr *>{ $3, $5 }); }
| expr ArgumentList { $$ = new Call($1, $2); }
| expr AS typesig { $$ = new Cast($1, $3); }
| ADDR_OF IDENTIFIER { $$ = new AddrOf($2); }
| MATCH expr matchBlock { $$ = new MatchExpr($2, $3); }

| '[' Tuple_inner ']' { $$ = new Call(new Var("List.create"), $2); }
| Tuple { $$ = $1; if ($1->items.size() == 1) $$ = $1->items[0]; }

Tuple
: '(' ')' { $$ = new Tuple(std::vector<Expr *>()); }
| '(' Tuple_inner ')' { $$ = new Tuple($2); }

Tuple_inner
: expr { $$.push_back($1); }
| Tuple_inner ',' expr { $$ = $1; $$.push_back($3); }

IfExpr
: IF expr block { $$ = new If($2, $3, new BlockExpr(std::vector<Expr*>{})); }
| IF expr block NEWLINE ElseSequence { $$ = new If($2, $3, $5); }

ElseSequence
: /* empty */ { $$ = new BlockExpr(std::vector<Expr *>{ }); }
| ELIF expr block NEWLINE ElseSequence { $$ = new BlockExpr(std::vector<Expr*>{new If($2, $3, $5)}); }
| ELSE block { $$ = $2; }

typesig
: IDENTIFIER { $$ = BuildType($1); }
| '.' '.' '.' { $$ = BuildType("..."); }
| IDENTIFIER '[' TypeList ']' { $$ = BuildType($1, $3); }
TypeList
: typesig { $$.push_back($1); }
| TypeList ',' typesig { $$ = $1; $$.push_back($3); }

%%

#include <sstream>
std::string yy_src = "";
void yy::parser::error(const yy::location &loc, const std::string& m) {
  std::stringstream ss(yy_src);
  std::string line;
  for(int i=0; i < (int)loc.begin.line - 1; i++) {
    std::getline(ss, line, '\n');
  }
  for(int i=0; i<3; i++) {
    std::getline(ss, line, '\n');
    std::cout << line << std::endl;
  }
  std::cout << '[' << loc.begin << '-' << loc.end << "]: " << m << std::endl;
}
