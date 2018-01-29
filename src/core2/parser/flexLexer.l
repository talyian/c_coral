%option noyywrap
%option bison-bridge
%option reentrant
%option yylineno
%{
#define STRLVAL yylval->str.buf = yytext; yylval->str.len = yyleng
%}
%%

#.* { }
\"(\\\"|[^\"])*\" { STRLVAL; return STRING; }
[a-zA-Z][a-zA-Z0-9]* { STRLVAL; return coral::Token::GetKeyword(yytext) ?: IDENTIFIER; }
[0-9][0-9]*L? { STRLVAL; return INTEGER; }
[ ]       { }
\n[ \t]*  { yytext[0] = ' '; return NEWLINE; }
.         { return yytext[0]; }

%%
