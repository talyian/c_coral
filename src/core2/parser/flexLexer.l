%option noyywrap
%option bison-bridge
%option reentrant
%option yylineno

%{
// the path for this is kind of annoying so
// we just rely on the relative include from the parent
// #include "token.hh"
%}

%%

\"(\\\"|[^\"])*\" { return coral::Token::STRING; }

[a-zA-Z][a-zA-Z0-9]* { return coral::Token::GetKeyword(yytext) ?: coral::Token::IDENTIFIER; }

[ ]       { }
\n[ \t]*  { yytext[0] = ' '; return coral::Token::NEWLINE; }
.         { return yytext[0]; }

%%
