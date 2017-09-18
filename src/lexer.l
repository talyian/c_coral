%option never-interactive
%option noyywrap

%{
#include <string>
#include "../src/ast.h"  
#include "../obj/parser.hh"
  
#define YYSTYPE yy::parser::semantic_type
#define tok(TOKEN_NAME) return yy::parser::token::TOKEN_NAME
%}

%option bison-bridge

int [0-9]+
num [0-9]+[.][0-9]+
str \"([^\\\"]|\\\\|\\\"|\\n)*\"
id [a-zA-Z][a-zA-Z0-9]+

%%

%{
%}

extern { tok(EXTERN); }
{int}  { tok(NUMBER); }
{num}  { tok(NUMBER); }
{str}  { yylval->build(std::string(yytext, yyleng)); tok(STRING); }
{id}   { yylval->build(std::string(yytext, yyleng)); tok(IDENTIFIER); }
\n     { tok(NEWLINE); }
[ \t]  { }
.      { return *yytext; }
%%

int main() { 

   yy::parser P;
   P.parse();

   /* printf("----------------------------------------\n"); */
   /* yy::parser::semantic_type s; */
   /* int t; */
   /* while((t = yylex(&s))) { */
   /*    printf("[%d]\n", t); */
   /* } */
   return 0;
}
