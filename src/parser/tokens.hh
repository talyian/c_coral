#pragma once

#include <string>
#include "lexer-internal.hh"
#include "bisonParser.tab.hh"

using TOK = coral::parser::token;

namespace coral {
  namespace Token {
	static int GetOperator(std::string s) {
	  return s == "=" ? TOK::OP_EQ :
		s == "**" ? TOK::OP1 :
		s == "*" ? TOK::OP2 :
		s == "/" ? TOK::OP2 :
		s == "%" ? TOK::OP2 :
		s == "+" ? TOK::OP_ADD :
		s == "-" ? TOK::OP_SUB :
        s == "|" ? TOK::OP_PIPE :
        s == "&" ? TOK::OP_AMP :
		s == "<" ? TOK::OP4 :
		s == ">" ? TOK::OP4 :
		s == "=" ? TOK::OP4 :
		s == "!=" ? TOK::OP4 :
		s == "<=" ? TOK::OP4 :
		s == ">=" ? TOK::OP4 :
		TOK::OP;
	}

	static int GetKeyword(std::string s) {
	  return s == "func" ? TOK::FUNC :
		s == "let" ? TOK::LET :
		s == "set" ? TOK::SET :
		s == "for" ? TOK::FOR :
		s == "in" ? TOK::IN :
		s == "if" ? TOK::IF :
		s == "elif" ? TOK::ELIF :
		s == "else" ? TOK::ELSE :
		s == "return" ? TOK::RETURN :
		s == "while" ? TOK::WHILE :
		s == "continue" ? TOK::CONTINUE :
		s == "match" ? TOK::MATCH:
		s == "type" ? TOK::TYPE:
		s == "c_extern" ? TOK::C_EXTERN:
        s == "import" ? TOK::IMPORT:
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
