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

std::queue<int> tokenq; // a queue to allow multiple return tokens;
std::stack<int> indents({ 0 });
%}

int [0-9]+
num [0-9]+[.][0-9]+
str \"([^\\\"]|\\\\|\\\"|\\n)*\"
id  [$]?[_a-zA-Z][_a-zA-Z0-9]*
br  \n[ ]*
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
{comment} { }
{int}  { yylval->build(std::stoll(yytext)); tok(INTEGER); }
{num}  { yylval->build(std::stod(yytext)); tok(FLOAT); }
{str}  { yylval->build(std::string(yytext, yyleng)); tok(STRING); }
{id}   { yylval->build(std::string(yytext, yyleng)); tok(IDENTIFIER); }
[&]    { tok(ADDR_OF); }

{br}   {
  lineno++; colno = yyleng - 1;
  
  if (indents.top() < colno) { tokenq.push(yy::parser::token::INDENT); indents.push(colno); }
  else if (indents.top() == colno) { }
  else while(indents.top() > colno) {
      indents.pop();
      tokenq.push(yy::parser::token::DEDENT);
    }
  tok(NEWLINE);
}
[ \t]  { }
.      { return *yytext; }
%%
