%option noyywrap
%option bison-bridge
%option reentrant
%option yylineno

%%

\"(\\\"|[^\"])*\" { yylval->str.buf = yytext; yylval->str.len = yyleng; return STRING; }
[a-zA-Z][a-zA-Z0-9]* { yylval->str.buf = yytext; yylval->str.len = yyleng; return coral::Token::GetKeyword(yytext) ?: IDENTIFIER; }
[0-9][0-9]*L? { yylval->str.buf = yytext; yylval->str.len = yyleng; return INTEGER; }
[ ]       { }
\n[ \t]*  { yytext[0] = ' '; return NEWLINE; }
.         { return yytext[0]; }

%%
