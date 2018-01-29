#include "tokens.hh"
#include "lexer.hh"

#include <cstdio>

void tokens() {
  auto lexer = lexerCreate("tests/cases/simple/hello_world.coral");
  int tok = 0;
  int length;
  char * s;
  while((tok = lexerRead(lexer, &s, &length, 0)))
	printf("%s\n", show(tok, s).c_str());
  lexerDestroy(lexer);
}

int main() { tokens(); }
