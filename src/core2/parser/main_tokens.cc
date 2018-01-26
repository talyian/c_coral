#include "lexer.hh"
#include "tokens.hh"
#include <cstdio>

void tokens() {
  auto lexer = lexerCreate("../../tests/shootout/knucleotide.coral");
  int tok = 0;
  int length;
  char * s;
  while((tok = lexerRead(lexer, &s, &length, 0))) {
	switch(tok) {
	case coral::Token::INDENT: printf("INDENT\n"); break;
	case coral::Token::DEDENT: printf("DEDENT\n"); break;
	case coral::Token::NEWLINE: printf("NEWLINE\n"); break;
	case coral::Token::STRING:
	case coral::Token::IDENTIFIER:
	  printf("%5d: %s\n", tok, s); break;
	default:
	  printf("OP:    %s\n", s);
	  break;
	}
  }
  lexerDestroy(lexer);
}

int main() { tokens(); }
