%option never-interactive
%option noyywrap
%option bison-bridge
%option bison-locations
%option reentrant

%{
#include <string>
#include <queue>
#include <stack>
#include "../../core/expr.hh"
using namespace coral;
#include "parser.hh"

#define YY_NO_UNISTD_H
#define YYSTYPE yy::parser::semantic_type
#define YYLTYPE yy::location
#define tok(TOKEN_NAME) return yy::parser::token::TOKEN_NAME
#define YY_USER_ACTION update(yylloc, yyleng);
#define YY_NO_UNPUT
#define YY_NO_INPUT

int lineno = 0, colno = 0, paren_depth = 0;

void push_paren(char c) {
   paren_depth++;
}
void pop_paren(char c) {
   paren_depth--;
}

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

    if (val == "+=") tok(OP_ADDEQ);
    if (val == "-=") tok(OP_SUBEQ);
    if (val == "*=") tok(OP_MULEQ);
    if (val == "/=") tok(OP_DIVEQ);
    if (val == "%=") tok(OP_MODEQ);
    tok(OP_OPERATOR);
}


std::queue<int> tokenq; // a queue to allow multiple return tokens;
std::vector<int> indents({ 0 });
%}

int [0-9]+[L]?
num [0-9]+[.][0-9]+
bin 0b[01]+
hex 0x[0-9a-fA-F]+
str \"([^\\\"]|\\\\|\\\"|\\[a-z])*\"
id  [_a-zA-Z$][$_a-zA-Z0-9]*
br  (\r?\n[ ]*)+
op  ([-+*/%!^><=@$!&?])+
comment (#|†|‡).*$

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
       indents.pop_back();
       tokenq.push(yy::parser::token::DEDENT);
    }
    indents.pop_back();
    return yy::parser::token::DEDENT;
}

impl     { tok(IMPL); }
for      { tok(FOR); }
in       { tok(IN); }
class    { tok(CLASS); }
match    { tok(MATCH); }
type     { tok(TYPE); }
let      { tok(LET); }
set      { tok(SET); }
as       { tok(AS); }
return   { tok(RETURN); }
extern   { tok(EXTERN); }
func     { tok(FUNC); }
if       { tok(IF); }
else     { tok(ELSE); }
elif     { tok(ELIF); }
pass     { tok(PASS); }
{op}     { string val(yytext, yyleng); yylval->build(val); return oper(val); }
{comment} { }
{int}    { yylval->build(std::stoll(yytext)); tok(INTEGER); }
{num}    { yylval->build(std::stod(yytext)); tok(FLOAT); }
{str}    { yylval->build(string(yytext, yyleng)); tok(STRING); }
'([^'])' { yylval->build(string(yytext, yyleng)); tok(STRING); }
{id}     { yylval->build(string(yytext, yyleng)); tok(IDENTIFIER); }
[&]      { tok(ADDR_OF); }
\[       { push_paren('['); return '['; }
\]       { pop_paren(']'); return ']'; }
\(       { push_paren('('); return '('; }
\)       { pop_paren(')'); return ')'; }
{br}     {
  for(int i=0; i<yyleng; i++) {
    if (yytext[i] == '\n') { lineno++; colno = 0; }
    else if (yytext[i] == '\r') { colno = 0; }
    else colno++;
  }

  if (paren_depth) break;
  /* if (last_token_was_newline == 1) break; */
  /* last_token_was_newline = 0; */
  if (indents.empty()) { std::cerr << "mismatched indent/dedent!\n"; }
  if (indents.back() < colno) { tokenq.push(yy::parser::token::INDENT); indents.push_back(colno); }
  else if (indents.back() == colno) { }
  else while(indents.back() > colno) {
      indents.pop_back();
      tokenq.push(yy::parser::token::DEDENT);
      tokenq.push(yy::parser::token::NEWLINE);
    }
  /* std::cerr << "info: " << lineno << "|" <<  colno << "|" << yyleng << "\n"; */
  /* std::cerr << "[" << yytext << "]\n"; */
  /* std::cerr << "newline -- indents[" << indents.size() << "]: "; */
  /* foreach(indents, it) std::cerr << *it << ", "; */
  /* std::cerr << std::endl; */

  tok(NEWLINE);
}
[ \t]  { }
.      { return *yytext; }
