#include <string>
#include <stack>
#include <cstdio>
#include <unistd.h>
#include "ast.hh"
#include "parser.hh"

int yylex(yy::parser::semantic_type * a, yy::location * loc);

extern std::stack<int> indents;

extern void handle_module(Module &m);

class ModuleBuilder : public Visitor {
public:
  ModuleBuilder(Module * m);
  char * finalize();
};

int main(int argc, char ** argv) {
  if (argc == 1) {
    argc = 3;
    argv = new char *[3];
    argv[0] = (char *)"coral";
    argv[1] = (char *)"ir";
    argv[2] = (char *)"samples/functions.basic.coral";
  }
  
  auto command = std::string(argc >= 2 ? argv[1] : "ir");
  auto inputstream = argc >= 3 ? fileno(fopen(argv[2], "r")) : 0;

  if (inputstream) dup2(inputstream, 0);

  Module * module = 0;
  if (command == "ir") {
    std::cerr << "ir" << std::endl;
    yy::parser P(module);
    P.parse();
    ModuleBuilder builder(module);
    printf("%s", builder.finalize());
  }
  else if (command == "parse") {
    yy::parser P(module);
    P.parse();
    std::cout << module->toString() << std::endl;
  }
  else if (command == "lex") {
    int t;
    yy::parser::semantic_type tok;
    yy::location loc;
    while((t = yylex(&tok, &loc))) {
      printf("%3d|%.*s", loc.begin.line, -3 + 4 * (int)indents.size(), "                     ");
      if (t < 255) printf("[%c]\n", t);
      else switch(t) {
	case yy::parser::token::EXTERN: printf("EXTERN\n"); break;
	case yy::parser::token::FUNC: printf("FUNC\n"); break;	  
	case yy::parser::token::NEWLINE: printf("---\n"); break;
	case yy::parser::token::STRING: printf("string %s\n", tok.as<std::string>().c_str()); break;
	case yy::parser::token::IDENTIFIER: printf("id %s\n", tok.as<std::string>().c_str()); break;
	case yy::parser::token::INTEGER: printf("num %ld\n", tok.as<int64_t>()); break;
	default:
	  printf("[%d]\n", t);
      }
    }
  } else {
    fprintf(stderr, "unknown command: %s\n", command.c_str());
  }
  return 0;
}
