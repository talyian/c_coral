#include <string>
#include <stack>
#include <cstdio>
#include <unistd.h>
#include <vector>

#include "expr.hh"
#include "lexer.hh"
#include "compiler.hh"

int main(int argc, char **argv) {
  std::string command(argv[1]);
  if (command == "lex") {
    void * scanner;
    YYSTYPE lval;
    YYLTYPE loc;
    yylex_init(&scanner);
    yyset_in(fopen(argv[2], "r"), scanner);
    int t = 0;
    while((t = yylex(&lval, &loc, scanner))) {
      string text(yyget_text(scanner));
      replace(text.begin(), text.end(), '\n', ' ');
      switch(t) {
      case yy::parser::token::INDENT:
	printf("       >>>>\n");
	break;
      case yy::parser::token::DEDENT:
	printf("       <<<<\n");
	break;
      case yy::parser::token::NEWLINE:
	printf("       ...\n");
	break;
      default:
	printf( "[%4d] %s\n", t, text.c_str());
      }
    }
    yylex_destroy(scanner);
  }
  else if (command == "parse") {
    cout << CoralModule(fopen(argv[2], "r")) << endl;
  }
  if (command == "ir") {
    CoralCompiler cc;
    cc.load(CoralModule(fopen(argv[2], "r")));
    cc.showIR();
  } else if (command == "jit") {
    CoralCompiler cc;
    cc.load(CoralModule(fopen(argv[2], "r")));
    cc.run();
  }
}
