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

%token COMMENT
%token FOR
%token IN
%token IMPL
%token CLASS
%token MATCH
%token TYPE
%token ADDR_OF
%token LET
%token SET
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

%type <std::string> OP_OPERATOR

%type <std::vector<Expr *>> lines ArgumentList
%type <std::vector<Expr *>> enumBlock matchBlock
%type <std::vector<Expr *>> enumLines matchLines
%type <std::vector<Expr *>> Tuple_inner
%type <Extern *> ExternLine
%type <Expr *> enumLine matchLine TypeDeclLine FuncDefLine LetDeclLine
%type <Expr *> line expr IfExpr ElseSequence UnaryExpr NonUnaryExpr BinaryExpr Atom
%type <Tuple *> Tuple
%type <Type *> typesig AtomType ComplexType FuncSigType
%type <std::vector<Type *>> TypeList
%type <Def *> classLine
%type <std::vector<Def *>> classLines classBlock
%type <BaseDef *> Parameter
%type <Def *> SingleParameter
%type <std::vector<BaseDef *>> ParameterList_inner ParameterList
%type <BlockExpr *> block

%type <Expr *> StructLine
%type <std::vector<Expr *>> StructLines
%type <std::string> TypeParameter
%type <std::vector<std::string>> TypeParameterList

%{
#include "../../core/expr.hh"
using namespace coral;
#include "parser.hh"

int yylex(yy::parser::semantic_type * pp, yy::location * loc, void * yyscanner);
%}

%%

program : lines { module = new Module($1); }

lines
	: NEWLINE { }
	| COMMENT { }
	| line { $$.push_back($1); }
	| lines NEWLINE { $$ = $1; }
	| lines COMMENT { $$ = $1; }
	| lines line { $$ = $1; $$.push_back($2); }

ExternLine
	: EXTERN STRING IDENTIFIER { $$ = new Extern($2, $3, 0); }
	| EXTERN STRING IDENTIFIER ':' typesig { $$ = new Extern($2, $3, $5); }

TypeDeclLine
	// Type Alias
	: TYPE IDENTIFIER OP_EQ typesig { $$ = new DeclTypeAlias($2, $4); }
	// Enum Type
	| TYPE IDENTIFIER enumBlock { $$ = new DeclTypeEnum($2, { 0 }, $3); }
	| TYPE IDENTIFIER '(' TypeParameterList ')' enumBlock {
	    $$ = new DeclTypeEnum($2, $4, $6);
    }

    // Class Type
	// TODO - settle on either parentheses or brackets
	| TYPE IDENTIFIER ':' NEWLINE INDENT StructLines NEWLINE DEDENT {
	    $$ = new Struct($2, std::vector<std::string> { } , $6); }
	// | TYPE IDENTIFIER '(' IDENTIFIER ')' ':' NEWLINE INDENT StructLines NEWLINE DEDENT {
	//     $$ = new Struct($2, std::vector<std::string> { $4 } , $9); }
	// | TYPE IDENTIFIER '[' IDENTIFIER ']' ':' NEWLINE INDENT StructLines NEWLINE DEDENT {
	//     $$ = new Struct($2, std::vector<std::string> { $4 } , $9); }

StructLines
	: NEWLINE { }
	| StructLine { $$.push_back($1); }
	| StructLines NEWLINE { $$ = $1; }
	| StructLines StructLine { $$ = $1; $$.push_back($2); }
StructLine
	: IDENTIFIER IDENTIFIER ':' typesig { $$ = new Let(new Def($2, $4), 0); }
	| IDENTIFIER ':' typesig { $$ = new Let(new Def($1, $3), 0); }
	| FuncDefLine { $$ = $1; }
	| ExternLine { $$ = $1; }
	| LetDeclLine { $$ = $1; }

FuncDefLine
	: FUNC IDENTIFIER ParameterList block { $$ = BuildFunc($2, 0, $3, $4); }
	| FUNC IDENTIFIER ':' FuncSigType ParameterList block { $$ = BuildFunc($2, $4, $5, $6); }

LetDeclLine
	: LET SingleParameter { $$ = new Let($2, 0); }
	| LET SingleParameter OP_EQ expr { $$ = new Let($2, $4); }
	| LET '(' ParameterList_inner ')' OP_EQ expr { $$ = new MultiLet(new TupleDef($3), $6); }

line
	: ExternLine { $$ = (Expr *)$1; }
	| TypeDeclLine { $$ = $1; }
	| FuncDefLine { $$ = $1; }
	| LetDeclLine { $$ = $1; }
	| CLASS IDENTIFIER classBlock { $$ = new DeclClass($2, $3); }
	| CLASS IDENTIFIER FOR IDENTIFIER classBlock { $$ = new DeclClass($2, $5); }
	| IMPL IDENTIFIER block { $$ = new ImplType($2, $3); }
	| IMPL IDENTIFIER FOR IDENTIFIER block { $$ = new ImplClassFor($2, $4, $5); }
	| SET SingleParameter OP_EQ expr { $$ = new Set(new Var($2->name), $4); }
	| SET expr '.' IDENTIFIER OP_EQ expr { $$ = new Set(new Member($2, $4), $6); }
	| SET '(' ParameterList_inner ')' OP_EQ expr { $$ = new Set(new Var("TODO"), $6); }
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
classLine  : Parameter { $$ = (Def *)$1; /* TODO */ }

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


// P A R A M E T E R S --------------------
SingleParameter
	: IDENTIFIER { $$ = new Def($1, 0); }
	| IDENTIFIER ':' typesig { $$ = new Def($1, $3); }

Parameter
	: SingleParameter { $$ = $1; }
	| '(' ParameterList_inner ')' { $$ = new TupleDef($2); }

ParameterList_inner
	: Parameter { $$.push_back($1); }
	| ParameterList_inner ',' Parameter { $$ = $1; $$.push_back($3); }

ParameterList
	: '(' ')' { }
	| '(' ParameterList_inner ')' { $$ = $2; }

// A R G U M E N T S --------------------
ArgumentList : NonUnaryExpr  { $$.push_back($1); }
	     | Tuple { $$ = $1->items; }


expr : UnaryExpr { $$ = $1; }
     | Atom { $$ = $1; }
     | BinaryExpr { $$ = $1; }


UnaryExpr
	: OP_SUB Atom { $$ = new Call(new Var("negate"), std::vector<Expr *>{$2}); }

NonUnaryExpr
     : Atom { $$ = $1; }
     | BinaryExpr { $$ = $1; }

Atom
	: STRING { $$ = new String($1); }
	| INTEGER { $$ = new Long($1); }
	| FLOAT { $$ = new Double($1); }
	| IDENTIFIER { $$ = new Var($1); }
	| Tuple { $$ = $1; if ($1->items.size() == 1) $$ = $1->items[0]; }

BinaryExpr
	: expr OP_ADD expr { $$ = new BinOp("+", $1, $3); }
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
	| expr OP_OPERATOR expr { $$ = new BinOp($2, $1, $3); }

	| expr '.' IDENTIFIER { $$ = new Member($1, $3); }
	| expr '[' expr ']' { $$ = new Index($1, std::vector<Expr *>{ $3 }); }
	| expr '[' expr ',' expr ']' { $$ = new Index($1, std::vector<Expr *>{ $3, $5 }); }
	| expr ArgumentList { $$ = new Call($1, $2); }
	| expr AS typesig { $$ = new Cast($1, $3); }
	| ADDR_OF IDENTIFIER { $$ = new AddrOf($2); }
	| MATCH expr matchBlock { $$ = new MatchExpr($2, $3); }
	| '[' Tuple_inner ']' { $$ = new Call(new Var("_list"), $2); }


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

// T Y P E S
typesig
	: AtomType { $$ = $1; }
	| ComplexType { $$ = $1; }
	| '.' '.' '.' { $$ = BuildType("..."); }

FuncSigType
	: AtomType  { $$ = $1; }
	| IDENTIFIER '[' TypeList ']' { $$ = BuildType($1, $3); }

AtomType
	: IDENTIFIER { $$ = BuildType($1); }
	| '(' ComplexType ')' { $$ = $2; }

ComplexType
	: IDENTIFIER '[' TypeList ']' { $$ = BuildType($1, $3); }
	| IDENTIFIER '(' TypeList ')' { $$ = BuildType($1, $3); }

TypeList
	: typesig { $$.push_back($1); }
	| TypeList ',' typesig { $$ = $1; $$.push_back($3); }

// type Dictionary(T, U) :: TYPE IDENTIFIER '(' TypeParameterList ')'
TypeParameter
	: IDENTIFIER { $$ = $1; }

TypeParameterList
    : TypeParameter { $$.push_back($1); }
	| TypeParameterList ',' TypeParameter { $$ = $1; $$.push_back($3); }
