#include <string>

namespace coral {
  class Token {
  public:
	enum TokenValues {
	  EndOfFile = 0,
	  UNKNOWN = 1000,
	  STRING,
	  IDENTIFIER,
	  NEWLINE,
	  INDENT,
	  DEDENT,
	  FUNC,
	  LET,
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
