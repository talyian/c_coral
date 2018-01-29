#pragma once

#include <string>
#include "lexer-internal.hh"
#include "bisonParser.tab.hh"

namespace coral {
  namespace Token {
	static int GetKeyword(std::string s) {
	  return s == "func" ? FUNC :
		s == "let" ? LET :
		s == "for" ? FOR :
		s == "in" ?  IN :
		0;
	}
  }
}

static std::string show(int token, char * yytext) {
  return token < 256 ? std::string(1, (char)token) :
	token == NEWLINE ? "NEWLINE" :
	token == INDENT ? "INDENT" :
	token == DEDENT ? "DEDENT" :
	token == IDENTIFIER ? std::string("[") + yytext  + "]" :
	yytext;
}
//   };
// }
