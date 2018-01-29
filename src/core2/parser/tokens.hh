#pragma once

#include <string>

namespace coral {
  class Token {
  public:

	enum {
	  STRING = 258,
	  IDENTIFIER = 259,
	  INTEGER = 260,
	  NEWLINE = 261,
	  INDENT = 262,
	  DEDENT = 263,
	  FUNC = 264,
	  LET = 265
	};

	static int GetKeyword(std::string s) {
	  return s == "func" ? Token::FUNC :
		s == "let" ? Token::LET :
		0;
	}

	static std::string show(int token, char * yytext) {
	  return token < 256 ? std::string(1, (char)token) :
		token == NEWLINE ? "NEWLINE" :
		token == INDENT ? "INDENT" :
		token == DEDENT ? "DEDENT" :
		token == IDENTIFIER ? std::string("[") + yytext  + "]" :
		yytext;
	}
  };
}
