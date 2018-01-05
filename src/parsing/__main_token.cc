#include "../core/treeprinter.hh"
#include "lexer.hh"
#include <cstdio>

int main(int argc, char ** argv) {
  Lexer lexer(argc > 1 ? fopen(argv[1], "r") : 0);
  while(lexer.lex()) {
	printf("[%d] %s\n", lexer.tokenType, lexer.text);
  }
}
