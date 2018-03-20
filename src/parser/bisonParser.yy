%define api.value.type variant
%define parse.error verbose
%language "c++"
%lex-param {pp}
%name-prefix "coral"
%parse-param {ParserParam pp}

%{
#include "parser/lexer-internal.hh"
#include <cstdio>
%}

%token <std::string> OP OP4 OP_ADD OP_SUB OP2 OP1
%token <std::string> OP_EQ OP_PIPE OP_AMP
%token <std::string> COMMENT
%token <std::string> STRING
%token <std::string> IDENTIFIER
%token <std::string> INTEGER FLOAT
%token NEWLINE INDENT DEDENT
%token FUNC
%token LET SET
%token FOR IN
%token IF ELIF ELSE
%token WHILE CONTINUE MATCH
%token RETURN
%token ELLIPSIS C_EXTERN IMPORT
%token TYPE
%token ENDOFFILE 0

// TODO split between Statements and Exprs
%type <coral::ast::BaseExpr *> Expr Atom Expr2 Expr3 Expr4
%type <coral::ast::BaseExpr *> Function ModuleLine ModuleRule ForLoop Argument UnionLine
%type <coral::ast::IfExpr *> IfStatement
%type <coral::ast::Block *> StatementBlock UnionBlock
%type <coral::ast::TupleLiteral *> Tuple

%type <coral::ast::Match *> MatchExpr
%type <coral::ast::MatchCase *> MatchLine
%type <std::vector<coral::ast::MatchCase *>> MatchBlock

%type <std::vector<coral::ast::BaseExpr *>> BlockLines ArgumentsListInner UnionLines
%type <std::vector<std::string>> IdentifierList VarPath
%type <coral::type::Type *> TypeDef
%type <std::vector<coral::type::Type>> TypeDefList

%type <coral::ast::Def *> Param NamedTypeDef
%type <std::vector<coral::ast::Def *>> ParamsListInner ParamsListOuter NamedTypeDefList
%type <std::string> GeneralIdentifier Operator

%%

ModuleRule : BlockLines ENDOFFILE {
  pp->module = new coral::ast::Module($1);
  return 0;
}

BlockLines : { }
| BlockLines ModuleLine NEWLINE { $$ = $1; $$.push_back($2); }
| BlockLines NEWLINE { $$ = $1; $$.push_back(0); }

StatementBlock
: ':' NEWLINE INDENT BlockLines DEDENT { $$ = new ast::Block($4); }
| ':' ModuleLine { $$ = new ast::Block({ $2 }); }

GeneralIdentifier
: IDENTIFIER { $$ = $1; }
| MATCH { $$ = "match"; }
| IMPORT { $$ = "import"; }
| '(' Operator ')' { $$ = $2; }

Operator
: OP { $$ = $1; }
| OP4 { $$ = $1; }
| OP_ADD { $$ = $1; }
| OP_SUB { $$ = $1; }
| OP_PIPE { $$ = $1; }
| OP_AMP { $$ = $1; }
| OP2 { $$ = $1; }
| OP1 { $$ = $1; }
| OP_EQ { $$ = $1; }

Atom // Expr0 - Can be called without parens
: GeneralIdentifier { $$ = new ast::Var($1); }
| INTEGER { $$ = new ast::IntLiteral($1); }
| FLOAT { $$ = new ast::FloatLiteral($1); }
| STRING { $$ = new ast::StringLiteral($1); }
| '[' ArgumentsListInner ']' { $$ = new ast::ListLiteral($2); }
| '(' Expr ')' { $$ = $2; }

Tuple : '(' ArgumentsListInner ')' { $$ = new ast::TupleLiteral($2); }

Expr2 : Atom { $$ = $1; } // Expr2s can be unparenthesized in binary ops
| Expr2 '.' IDENTIFIER { $$ = new ast::Member($1, $3); }
| Expr2 Tuple { $$ = new ast::Call($1, $2); delete $2; }
| Expr2 '(' ')' { $$ = new ast::Call($1, {}); }
| Expr2 Atom { $$ = new ast::Call($1, { $2 }); }
| OP_SUB Expr2 { $$ = new ast::Call(new ast::Var("negate"), { $2 }); }
Expr3 : Expr2 { $$ = $1; } | Tuple { $$ = $1; } | Expr3 OP2 Expr3 { $$ = new ast::BinOp($1, $2, $3); }
Expr4 : Expr3 { $$ = $1; }
| Expr4 OP_ADD Expr4 { $$ = new ast::BinOp($1, $2, $3); }
| Expr4 OP_SUB Expr4 { $$ = new ast::BinOp($1, $2, $3); }
Expr : Expr4 { $$ = $1; }
| Expr4 OP4 Expr4  { $$ = new ast::BinOp($1, $2, $3); }
| Expr4 OP_EQ Expr4  { $$ = new ast::BinOp($1, $2, $3); }
| Expr4 OP Expr4 { $$ = new ast::BinOp($1, $2, $3); }
| MatchExpr { $$ = $1; }

ArgumentsListInner : Argument { $$.push_back($1); }
| ArgumentsListInner ',' Argument { $$ = $1; $$.push_back($3); }

Argument : Expr { $$ = $1; } | IDENTIFIER '=' Expr { $$ = $3; }

Param : IDENTIFIER { $$ = new ast::Def($1, 0, 0); }
| IDENTIFIER ':' TypeDef { $$ = new ast::Def($1, $3, 0); }

TypeDef
: IDENTIFIER { $$ = new coral::type::Type($1); }
| IDENTIFIER '[' TypeDefList ']' { $$ = new coral::type::Type($1, $3); }
| IDENTIFIER '[' NamedTypeDefList ']' { $$ = new coral::type::Type($1, ast::_defsToTypeArg($3)); }
| ELLIPSIS { $$ = new coral::type::Type("..."); }
| '{' TypeDefList '}' { $$ = new type::Type("Tuple", $2); }
| '{' NamedTypeDefList '}' { $$ = new type::Type("Tuple", ast::_defsToTypeArg($2)); }
NamedTypeDef : GeneralIdentifier ':' TypeDef { $$ = new ast::Def($1, $3, 0); }
NamedTypeDefList
: NamedTypeDef { $$.push_back($1); }
| NamedTypeDefList ',' NamedTypeDef { $$ = $1; $$.push_back($3); }
TypeDefList : TypeDef { $$.push_back(*$1); delete $1; }
| TypeDefList ',' TypeDef { $$ = $1; $$.push_back(*$3); delete $3; }
| TypeDefList ',' INTEGER { $$ = $1; $$.push_back(coral::type::Type($3)); }
ParamsListInner : Param { $$.push_back($1); }
| ParamsListInner ',' Param { $$ = $1; $$.push_back($3); }
ParamsListOuter : '(' ')' { }
| '(' ParamsListInner ')' { $$ = $2; }

ModuleLine
: COMMENT { $$ = new ast::Comment($1); }
| C_EXTERN GeneralIdentifier ':' TypeDef { $$ = new ast::Extern($2, $4); }
| Expr { $$ = $1; }
| Function { $$ = $1; }
| LET GeneralIdentifier OP_EQ Expr { $$ = new ast::Let(new ast::Var($2), $4); }
| LET GeneralIdentifier ':' TypeDef OP_EQ Expr {
    $$ = new ast::Let(new ast::Var($2), $6); ((ast::Let *)$$)->type = *$4; delete $4; }
| SET GeneralIdentifier OP_EQ Expr { $$ = new ast::Set(new ast::Var($2), $4); }
| ForLoop { $$ = $1; }
| IfStatement { $$ = $1; }
| WHILE Expr StatementBlock { $$ = new ast::While($2, $3); }
| RETURN Expr { $$ = new ast::Return($2); }
| TYPE GeneralIdentifier UnionBlock { $$ = new ast::Union($2, $3); }
| TYPE GeneralIdentifier OP_EQ TypeDef {
  // TODO: make a general feature for newtype
  if ($4->name == "Tuple")
    { $$ = new ast::Tuple($2, $4->params); /* delete $4 */ } // new ast::TypeDecl($2, $4);
  else if ($4->name == "Union")
    { $$ = new ast::Union($2, $4->params); /* delete $4 */ }
  else
    { $$ = 0; printf("\033[031mwat[%s - %s]\033[0m\n", $2.c_str(), $4->name.c_str()); }
}
| IMPORT VarPath { $$ = new ast::Import($2); }

Function
: FUNC VarPath ParamsListOuter StatementBlock { $$ = new ast::Func($2, new Type(""), $3, $4); }
| FUNC VarPath ':' TypeDef ParamsListOuter StatementBlock { $$ = new ast::Func($2, $4, $5, $6); }

ForLoop
: FOR GeneralIdentifier IN Expr StatementBlock { $$ = new ast::ForExpr(new ast::Var($2), $4, $5); }
| FOR IdentifierList IN Expr StatementBlock { $$ = new ast::ForExpr(new ast::Var($2), $4, $5); }

IfStatement
: IF Expr StatementBlock { $$ = new ast::IfExpr($2, $3, 0); }
| IF Expr StatementBlock ELSE StatementBlock %prec ELSE { $$ = new ast::IfExpr($2, $3, $5); }
| IF Expr StatementBlock ELSE IfStatement %prec ELSE { $$ = new ast::IfExpr($2, $3, new ast::Block({ $5 })); }

// Match Expression
MatchExpr
: MATCH Expr ':' NEWLINE INDENT MatchBlock NEWLINE DEDENT {
    $$ = new ast::Match($2, $6);
}
MatchBlock
: MatchLine { $$.push_back($1); }
| MatchBlock NEWLINE MatchLine { $$ = $1; $$.push_back($3); }
MatchLine
: OP_PIPE VarPath ParamsListOuter StatementBlock {
  $$ = new ast::MatchCase($2, $3, $4);
}

IdentifierList : GeneralIdentifier { $$.push_back($1); }
| IdentifierList ',' GeneralIdentifier { $$ = $1; $$.push_back($3); }

VarPath
: GeneralIdentifier { $$.push_back($1); }
| VarPath '.' GeneralIdentifier { $$ = $1; $$.push_back($3); }

UnionBlock : ':' NEWLINE INDENT UnionLines DEDENT { $$ = new ast::Block($4); }
UnionLines : { }
| UnionLines UnionLine NEWLINE { $$ = $1; $$.push_back($2); }
| UnionLines NEWLINE { $$ = $1; $$.push_back(0); }

UnionLine
: OP_PIPE GeneralIdentifier ':' TypeDef { $$ = new ast::Def($2, $4, 0); }

%%
