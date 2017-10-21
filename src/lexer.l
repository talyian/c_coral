%option never-interactive
%option noyywrap
%option bison-bridge
%option bison-locations
%option reentrant

%{
#include <string>
#include <queue>
#include <stack>
#include "ast.hh"
#include "parser.hh"

#define YY_NO_UNISTD_H
#define YYSTYPE yy::parser::semantic_type
#define YYLTYPE yy::location
#define tok(TOKEN_NAME) return yy::parser::token::TOKEN_NAME
#define YY_USER_ACTION update(yylloc, yyleng);

int lineno = 0, colno = 0;
void update(yy::location * loc, int len) {
  loc->begin.line = lineno;
  loc->begin.column = colno;
  loc->end.line = lineno;
  colno += len;
  loc->end.column = colno;
}

using std::vector;
using std::string;

int oper(string val) {
    if(val == "+") tok(OP_ADD);
    if(val == "*") tok(OP_MUL);
    if(val == "-") tok(OP_SUB);
    if(val == "/") tok(OP_DIV);
    if(val == "%") tok(OP_MOD);
    if(val == "!") tok(OP_NOT);

    if(val == "=") tok(OP_EQ);
    if(val == "<") tok(OP_LT);
    if(val == ">") tok(OP_GT);
    if(val == "<=") tok(OP_LTE);
    if(val == ">=") tok(OP_GTE);
    if(val == "!=") tok(OP_NE);
    tok(OP_OPERATOR);
}


std::queue<int> tokenq; // a queue to allow multiple return tokens;
std::stack<int> indents({ 0 });
%}

int [0-9]+[L]?
num [0-9]+[.][0-9]+
bin 0b[01]+
hex 0x[0-9a-fA-F]+
str \"([^\\\"]|\\\\|\\\"|\\n)*\"
id  [$]?[_a-zA-Z][_a-zA-Z0-9]*
br  \n[ ]*
op  ([-+*/%!^><=])+
comment [#].*$

%%

%{ // our little hack to return multiple tokens from a rule match
  if (!tokenq.empty()) {
    auto val = tokenq.front();
    tokenq.pop();
    return val;
  }
%}

<<EOF>> { // on eof, close all outstanding indent blocks
    if (indents.size() <= 1) return 0;
    while(indents.size() > 2) {
       indents.pop();
       tokenq.push(yy::parser::token::DEDENT);
    }
    indents.pop();
    return yy::parser::token::DEDENT;
}

impl   { tok(IMPL); }
for    { tok(FOR); }
in     { tok(IN); }
class  { tok(CLASS); }
match  { tok(MATCH); }
type   { tok(TYPE); }
module { tok(MODULE); }
let    { tok(LET); }
as     { tok(AS); }
return { tok(RETURN); }
extern { tok(EXTERN); }
func   { tok(FUNC); }
if     { tok(IF); }
else   { tok(ELSE); }
elif   { tok(ELIF); }
pass   { tok(PASS); }
{op}   { return oper(string(yytext, yyleng)); }
{comment} { }
{int}  { yylval->build(std::stoll(yytext)); tok(INTEGER); }
{num}  { yylval->build(std::stod(yytext)); tok(FLOAT); }
{str}  { yylval->build(string(yytext, yyleng)); tok(STRING); }
{id}   { yylval->build(string(yytext, yyleng)); tok(IDENTIFIER); }
[&]    { tok(ADDR_OF); }
{br}   {
  lineno++; colno = yyleng - 1;

  if (indents.top() < colno) { tokenq.push(yy::parser::token::INDENT); indents.push(colno); }
  else if (indents.top() == colno) { }
  else while(indents.top() > colno) {
      indents.pop();
      tokenq.push(yy::parser::token::DEDENT);
      tokenq.push(yy::parser::token::NEWLINE);
    }
  tok(NEWLINE);
}
[ \t]  { }
.      { return *yytext; }
%%
