#include "tokens.hh"
#include "lexer.hh"

#include <cstdio>

void showFile(const char * filename) {
  auto lexer = lexerCreate(filename);
  coral::parser::semantic_type lval;
  lexer->lval = &lval;
  int tok = 0;
  int length;
  char * s;
  while((tok = lexerRead(lexer, &s, &length, 0)))
	printf("%s\n", coral::Token::show(tok, s).c_str());
  lexerDestroy(lexer);
}

int main(int argc, const char ** argv) {
    if (argc > 1) { showFile(argv[1]); return 0; }
	else showFile("tests/cases/simple/hello_world.coral");
}
