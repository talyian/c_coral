%option never-interactive
%option noyywrap

%{
#include <string>
#include <queue>
#include <stack>
#include "../src/ast.h"  
#include "../obj/parser.hh"

#define YY_NO_UNISTD_H
#define YYSTYPE yy::parser::semantic_type
#define YYLTYPE yy::location
#define tok(TOKEN_NAME) return yy::parser::token::TOKEN_NAME
#define YY_USER_ACTION update(yylloc);

int lineno = 0, colno = 0;
void update(yy::location * yylloc) {
  yylloc->begin.line = lineno;
  yylloc->begin.column = colno;
  yylloc->end.line = lineno;
  colno += yyleng;
  yylloc->end.column = colno;
}

using std::vector;
using std::string;

std::queue<int> tokenq;
std::stack<int> indents({ 0 });
%}

%option bison-bridge
%option bison-locations

int [0-9]+
num [0-9]+[.][0-9]+
str \"([^\\\"]|\\\\|\\\"|\\n)*\"
id  [a-zA-Z][a-zA-Z0-9]+
br  \n[ ]*

%%

%{
  if (!tokenq.empty()) { 
    auto val = tokenq.front();
    tokenq.pop();
    return val;
  }
%}

extern { tok(EXTERN); }
func   { tok(FUNC); }
{int}  { yylval->build(std::stoll(yytext)); tok(INTEGER); }
{num}  { yylval->build(std::stod(yytext)); tok(FLOAT); }
{str}  { yylval->build(std::string(yytext, yyleng)); tok(STRING); }
{id}   { yylval->build(std::string(yytext, yyleng)); tok(IDENTIFIER); }
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
