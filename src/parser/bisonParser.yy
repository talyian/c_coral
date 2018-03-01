%define api.value.type variant
%define parse.error verbose
%language "c++"
%lex-param {pp}
%name-prefix "coral"
%parse-param {ParserParam pp}

%{
#include "lexer-internal.hh"
#include <cstdio>
%}

%token <std::string> OP OP4 OP3 OP2 OP1
%token <std::string> OP_EQ
%token <std::string> COMMENT
%token <std::string> STRING
%token <std::string> IDENTIFIER
%token <std::string> INTEGER
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
%type <coral::ast::BaseExpr *> Function ModuleLine ModuleRule ForLoop Argument StructLine
%type <coral::ast::IfExpr *> IfStatement
%type <coral::ast::Block *> StatementBlock StructBlock
%type <coral::ast::TupleLiteral *> Tuple
%type <std::vector<coral::ast::BaseExpr *>> BlockLines ArgumentsListInner StructBlockLines
%type <std::vector<std::string>> IdentifierList VarPath
%type <coral::type::Type *> TypeDef
%type <std::vector<coral::type::Type>> TypeDefList

%type <coral::ast::Def *> Param
%type <std::vector<coral::ast::Def *>> ParamsListInner
%type <std::string> GeneralIdentifier

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

Atom // Expr0 - Can be called without parens
: GeneralIdentifier { $$ = new ast::Var($1); }
| INTEGER { $$ = new ast::IntLiteral($1); }
| STRING { $$ = new ast::StringLiteral($1); }
| '[' ArgumentsListInner ']' { $$ = new ast::ListLiteral($2); }
| '(' Expr ')' { $$ = $2; }

Tuple : '(' ArgumentsListInner ')' { $$ = new ast::TupleLiteral($2); }

Expr2 : Atom { $$ = $1; } // Expr2s can be unparenthesized in binary ops
| Expr2 '.' IDENTIFIER { $$ = new ast::Member($1, $3); }
| Expr2 Tuple { $$ = new ast::Call($1, $2); delete $2; }
| Expr2 '(' ')' { $$ = new ast::Call($1, std::vector<ast::BaseExpr *>()); }
| Expr2 Atom { $$ = new ast::Call($1, { $2 }); }
Expr3 : Expr2 { $$ = $1; } | Tuple { $$ = $1; } | Expr3 OP2 Expr3 { $$ = new ast::BinOp($1, $2, $3); }
Expr4 : Expr3 { $$ = $1; } | Expr4 OP3 Expr4 { $$ = new ast::BinOp($1, $2, $3); }
Expr : Expr4 { $$ = $1; }
| Expr4 OP4 Expr4  { $$ = new ast::BinOp($1, $2, $3); }
| Expr4 OP_EQ Expr4  { $$ = new ast::BinOp($1, $2, $3); }
| Expr4 OP Expr4 { $$ = new ast::BinOp($1, $2, $3); }

ArgumentsListInner : Argument { $$.push_back($1); }
| ArgumentsListInner ',' Argument { $$ = $1; $$.push_back($3); }

Argument : Expr { $$ = $1; } | IDENTIFIER '=' Expr { $$ = $3; }

Param : IDENTIFIER { $$ = new ast::Def($1, 0, 0); }
| IDENTIFIER ':' TypeDef { $$ = new ast::Def($1, $3, 0); }

TypeDef : IDENTIFIER { $$ = new coral::type::Type($1); }
| IDENTIFIER '[' TypeDefList ']' { $$ = new coral::type::Type($1, $3); }
| ELLIPSIS { $$ = new coral::type::Type("..."); }
TypeDefList : TypeDef { $$.push_back(*$1); }
| TypeDefList ',' TypeDef {  $$ = $1; $$.push_back(*$3); }
ParamsListInner : Param { $$.push_back($1); }
| ParamsListInner ',' Param { $$ = $1; $$.push_back($3); }

ModuleLine
: COMMENT { $$ = new ast::Comment($1); }
| C_EXTERN GeneralIdentifier ':' TypeDef { $$ = new ast::Extern($2, $4); }
| Expr { $$ = $1; }
| Function { $$ = $1; }
| LET GeneralIdentifier OP_EQ Expr { $$ = new ast::Let(new ast::Var($2), $4); }
| LET GeneralIdentifier ':' TypeDef OP_EQ Expr {
    $$ = new ast::Let(new ast::Var($2), $6); ((ast::Let *)$$)->type = *$4; }
| SET GeneralIdentifier OP_EQ Expr { $$ = new ast::Set(new ast::Var($2), $4); }
| ForLoop { $$ = $1; }
| IfStatement { $$ = $1; }
| WHILE Expr StatementBlock { $$ = new ast::While($2, $3); }
| RETURN Expr { $$ = new ast::Return($2); }
| TYPE GeneralIdentifier StructBlock { $$ = new ast::Struct($2, $3); }
| IMPORT VarPath { $$ = new ast::Import($2); }
Function
: FUNC IDENTIFIER '(' ParamsListInner ')' StatementBlock { $$ = new ast::Func($2, new Type(""), $4, $6); }
| FUNC IDENTIFIER ':' TypeDef '(' ParamsListInner ')' StatementBlock {
   $$ = new ast::Func($2, $4, $6, $8); }
| FUNC IDENTIFIER ':' TypeDef '(' ')' StatementBlock {
   $$ = new ast::Func($2, $4, {}, $7); }
| FUNC IDENTIFIER '(' ')' StatementBlock { $$ = new ast::Func($2, new Type(""), {}, $5); }
| '`' IDENTIFIER FUNC IDENTIFIER ':' TypeDef '(' ParamsListInner ')' {
  $$ = new ast::Func($4, $6, $8, 0); }
| '`' IDENTIFIER FUNC IDENTIFIER ':' TypeDef '(' ')' {
  $$ = new ast::Func($4, $6, {}, 0); }

ForLoop
: FOR GeneralIdentifier IN Expr StatementBlock { $$ = new ast::ForExpr(new ast::Var($2), $4, $5); }
| FOR IdentifierList IN Expr StatementBlock { $$ = new ast::ForExpr(new ast::Var($2), $4, $5); }

IfStatement
: IF Expr StatementBlock { $$ = new ast::IfExpr($2, $3, 0); }
| IF Expr StatementBlock ELSE StatementBlock %prec ELSE { $$ = new ast::IfExpr($2, $3, $5); }
| IF Expr StatementBlock ELSE IfStatement %prec ELSE { $$ = new ast::IfExpr($2, $3, new ast::Block({ $5 })); }

IdentifierList : GeneralIdentifier { $$.push_back($1); }
| IdentifierList ',' GeneralIdentifier { $$ = $1; $$.push_back($3); }

VarPath
: GeneralIdentifier { $$.push_back($1); }
| VarPath '.' GeneralIdentifier { $$ = $1; $$.push_back($3); }

StructBlock : ':' NEWLINE INDENT StructBlockLines DEDENT { $$ = new ast::Block($4); }
StructBlockLines : { }
| StructBlockLines StructLine NEWLINE { $$ = $1; $$.push_back($2); }
| StructBlockLines NEWLINE { $$ = $1; $$.push_back(0); }
StructLine
: GeneralIdentifier ':' TypeDef { $$ = new ast::Def($1, $3, 0); }
| Function { $$ = $1; }

%%
