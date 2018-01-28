#include "lexer.hh"
#include "tokens.hh"
#include <cstdio>

void tokens() {
  auto lexer = lexerCreate("tests/cases/shootout/knucleotide.coral");
  int tok = 0;
  int length;
  char * s;
  while((tok = lexerRead(lexer, &s, &length, 0)))
	printf("%s\n", coral::Token::show(tok, s).c_str());
  lexerDestroy(lexer);
}

int main() { tokens(); }
