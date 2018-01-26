
%option noyywrap
%option bison-bridge

%%

\w+ { printf("IDENTIFIER: %s", yytext); }
. { }

%%
