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
%token <std::string> OP_EQ
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
%type <coral::ast::BaseExpr *> Expr Atom Expr2
%type <coral::ast::BaseExpr *> Function ModuleLine ModuleRule ForLoop Let Argument
%type <coral::ast::IfExpr *> IfElse IfStatement
%type <coral::ast::Block *> StatementBlock
%type <std::vector<coral::ast::BaseExpr *>> StatementLinesRule ArgumentsListInner
%type <std::string> Operator
%type <std::vector<std::string>> IdentifierList

%type <coral::ast::Def *> Param
%type <std::vector<coral::ast::Def *>> ParamsListInner
%%

ModuleRule : StatementLinesRule {
  pp->module = new coral::ast::Module($1);
  return 0;
}

StatementLinesRule : { }
| StatementLinesRule ModuleLine NEWLINE { $$ = $1; $$.push_back($2); }

StatementBlock : ':' NEWLINE INDENT StatementLinesRule DEDENT { $$ = new ast::Block($4); }
| ':' ModuleLine { $$ = new ast::Block({ $2 }); }

Atom // Expr0 - Can be called without parens
: IDENTIFIER { $$ = new ast::Var($1); }
| INTEGER { $$ = new ast::IntLiteral($1); }
| STRING { $$ = new ast::StringLiteral($1); }
| '[' ArgumentsListInner ']' { $$ = new ast::ListLiteral($2); }
// | '(' ArgumentsListInner ')' { $$ = new ast::TupleLiteral($2); }
| '(' Expr ')' { $$ = $2; }

Expr2 : Atom { $$ = $1; }
| Expr2 '.' IDENTIFIER { $$ = new ast::Member($1, $3); }
| Expr2 '(' ArgumentsListInner ')' { $$ = new ast::Call($1, $3); }
| Expr2 '(' ')' { $$ = new ast::Call($1, std::vector<ast::BaseExpr *>()); }
| Expr2 Atom { $$ = new ast::Call($1, $2); }

Expr : Expr2 { $$ = $1; }
| Expr Operator Expr  { $$ = new ast::BinOp($1, $2, $3); }

ArgumentsListInner : Argument { $$.push_back($1); }
| ArgumentsListInner ',' Argument { $$ = $1; $$.push_back($3); }

Argument : Expr { $$ = $1; } | IDENTIFIER '=' Expr { $$ = $3; }

Param : IDENTIFIER { $$ = new ast::Def($1, 0); }

ParamsListInner : Param { $$.push_back($1); }
| ParamsListInner ',' Param { $$ = $1; }

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
    $$ = new ast::Func($2, Type("o"), $4, $6); }
| FUNC IDENTIFIER '(' ')' StatementBlock {
    $$ = new ast::Func($2, Type("o"), {}, $5); }

ForLoop
: FOR IDENTIFIER IN Expr StatementBlock { $$ = new ast::ForExpr(new ast::Var($2), $4, $5); }
| FOR IdentifierList IN Expr StatementBlock { $$ = new ast::ForExpr(new ast::Var($2), $4, $5); }

IfStatement : IF Expr StatementBlock { $$ = new ast::IfExpr($2, $3, 0); }
IfElse
: IfStatement { $$ = $1; }
| IfStatement NEWLINE ELSE StatementBlock { $$ = $1; $$->elsebody.reset($4); }

Let : LET IDENTIFIER OP_EQ Expr { $$ = new ast::Let(new ast::Var($2), $4); }

Operator : OP { $$ = $1; } | OP_EQ { $$ = $1; }

IdentifierList : IDENTIFIER { $$.push_back($1); }
			   | IdentifierList ',' IDENTIFIER { $$ = $1; $$.push_back($3); }
%%
