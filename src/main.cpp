#include <string>
#include <stack>
#include <cstdio>
#include <unistd.h>
#include "../src/ast.h"
#include "../obj/parser.hh"

int yylex(yy::parser::semantic_type * a, yy::location * loc);

extern std::stack<int> indents;

int main(int argc, char ** argv) {
  auto command = std::string(argc >= 2 ? argv[1] : "ir");
  auto inputstream = argc >= 3 ? fileno(fopen(argv[2], "r")) : 0;

  if (inputstream) dup2(inputstream, 0);

  if (command == "ir") {
    yy::parser P; P.parse();
  }
  else if (command == "lex") {
    int t;
    yy::parser::semantic_type tok;
    yy::location loc;
    while((t = yylex(&tok, &loc))) {
      printf("+%.*s", -3 + 4 * indents.size(), "                     ");
      if (t < 255) printf("[%c]\n", t);
      else switch(t) {
	case yy::parser::token::EXTERN: printf("EXTERN\n"); break;
	case yy::parser::token::FUNC: printf("FUNC\n"); break;	  
	case yy::parser::token::NEWLINE: printf("---\n"); break;
	case yy::parser::token::STRING: printf("string %s\n", tok.as<std::string>().c_str()); break;
	case yy::parser::token::IDENTIFIER: printf("id %s\n", tok.as<std::string>().c_str()); break;
	case yy::parser::token::INTEGER: printf("num %d\n", tok.as<int64_t>()); break;		  
	default:
	  printf("[%d]\n", t);
      }
    }
  }
  return 0;
}
