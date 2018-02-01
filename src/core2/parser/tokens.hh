#pragma once

#include <string>
#include "lexer-internal.hh"
#include "bisonParser.tab.hh"

using TOK = coral::parser::token;

namespace coral {
  namespace Token {
	static int GetOperator(std::string s) {
	  return s == "=" ? TOK::OP_EQ :
		TOK::OP;
	}

	static int GetKeyword(std::string s) {
	  return s == "func" ? TOK::FUNC :
		s == "let" ? TOK::LET :
		s == "for" ? TOK::FOR :
		s == "in" ? TOK::IN :
		s == "if" ? TOK::IF :
		s == "elif" ? TOK::ELIF :
		s == "else" ? TOK::ELSE :
		s == "return" ? TOK::RETURN :
		0;
	}
	static std::string show(int token, char * text) {
	  return token < 256 ? std::string(1, (char)token) :
		token == TOK::NEWLINE ? "NEWLINE" :
		token == TOK::INDENT ? "INDENT" :
		token == TOK::DEDENT ? "DEDENT" :
		token == TOK::IDENTIFIER ? std::string("[") + text  + "]" :
		text;
	}
  }
}//   };
// }
