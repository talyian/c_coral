%option noyywrap
%option bison-bridge
%option reentrant
%option yylineno

%{
// the path for this is kind of annoying so
// we just rely on the relative include from the parent
#include "tokens.hh"
%}

%%

\"(\\\"|[^\"])*\" { return coral::Token::STRING; }

[a-zA-Z][a-zA-Z0-9]* { return coral::Token::GetKeyword(yytext) ?: coral::Token::IDENTIFIER; }

[0-9][0-9]*L? { return coral::Token::INTEGER; }

[ ]       { }
\n[ \t]*  { yytext[0] = ' '; return coral::Token::NEWLINE; }
.         { return yytext[0]; }

%%
