
%option noyywrap
%option bison-bridge
%option reentrant

%%

\"(\\\"|[^\"])*\" { return coral::Token::STRING; }

[a-zA-Z][a-zA-Z0-9]* { return coral::Token::IDENTIFIER; }

[ ]       { }
\n[ \t]*  { yytext[0] = ' '; return coral::Token::NEWLINE; }
.         { return yytext[0]; }

%%
