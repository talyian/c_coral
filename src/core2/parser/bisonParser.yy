%language "c++"
%define api.value.type variant
%lex-param {pp}
%name-prefix "coral"
%parse-param {ParserParam pp}

%{
#include "lexer-internal.hh"
#include <cstdio>
%}

%token <std::string> OP
%token <std::string> COMMENT
%token <std::string> STRING
%token <std::string> IDENTIFIER
%token <std::string> INTEGER
%token NEWLINE
%token INDENT
%token DEDENT
%token FUNC
%token LET
%token FOR
%token IN
%token IF
%token ELIF
%token ELSE
%token RETURN

// TODO split between Statements and Exprs
%type <coral::ast::BaseExpr *> Function ModuleLine ModuleRule Expr ForLoop Let IfElse Argument
%type <coral::ast::Block *> StatementBlock
%type <std::vector<coral::ast::BaseExpr *>> StatementLinesRule ParamsListInner ArgumentsListInner

%%

ModuleRule : StatementLinesRule {
  pp->module = new coral::ast::Module($1);
  return 0;
}

StatementLinesRule : { }
| StatementLinesRule ModuleLine NEWLINE { $$ = $1; $$.push_back($2); }

StatementBlock : ':' NEWLINE INDENT StatementLinesRule DEDENT { $$ = new ast::Block($4); }
| ':' ModuleLine { $$ = new ast::Block({ $2 }); }

Expr : IDENTIFIER { $$ = new ast::Var($1); }
| Expr '.' IDENTIFIER { $$ = 0; }
| Expr '(' ArgumentsListInner ')' { $$ = new ast::Call($1, $3); }
| Expr '(' ')' { $$ = 0; }
| Expr Expr { $$ = new ast::Call($1, $2); }
| INTEGER { $$ = new ast::IntLiteral($1); }
| STRING { $$ = new ast::StringLiteral($1); }
| IDENTIFIER '[' ArgumentsListInner ']' { $$ = 0; }
| Expr OP Expr  { $$ = new ast::BinOp($1, $2, $3); }


ArgumentsListInner : Argument { $$.push_back($1); }
| ArgumentsListInner ',' Argument { $$ = $1; $$.push_back($3); }

Argument : Expr { $$ = $1; } | IDENTIFIER '=' Expr { $$ = $3; }

Param : IDENTIFIER { }

ParamsListInner : Param { }
| ParamsListInner ',' Param { }

ModuleLine : { $$ = 0; }
| COMMENT { $$ = new ast::Comment($1); }
| Expr { $$ = $1; }
| Function { $$ = $1; }
| Let { $$ = $1; }
| ForLoop { $$ = $1; }
| IfElse { $$ = $1; }
| RETURN Expr { $$ = new ast::Return($2); }

Function
: FUNC IDENTIFIER '(' ParamsListInner ')' StatementBlock {
    $$ = new ast::Func($2, Type("o"), {}, $6); }
| FUNC IDENTIFIER '(' ')' StatementBlock {
    $$ = new ast::Func($2, Type("o"), {}, $5); }

ForLoop : FOR IDENTIFIER IN Expr StatementBlock { $$ = 0; }

IfElse : IF Expr StatementBlock NEWLINE ELSE StatementBlock {
    $$ = new ast::IfExpr($2, $3, $6); }

Let : LET IDENTIFIER '=' Expr { $$ = 0; }
%%
