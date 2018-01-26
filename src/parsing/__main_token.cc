#include "../core/treeprinter.hh"
#include "lexer.hh"
#include <cstdio>

int main(int argc, char ** argv) {
  void * scanner;
  FILE * f = argc > 1 ? fopen(argv[1], "r") : 0;
  yylex_init(&scanner);
  yyset_in(f, scanner);

  YYLTYPE loc;
  YYSTYPE lval;
  int tok = 0;
  while((tok = yylex(&lval, &loc, scanner))) {
	printf("%4d\n", tok);
  }
  yylex_destroy(scanner);
  fclose(f);
}
