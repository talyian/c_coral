#include "lexer.hh"

#include "../../build/parser/flex.hh"

#include <cstdio>

LexerT lexerCreate(FILE * input) {
  return 0;
}

void lexerDestroy(LexerT lexer) {

}

int lexerRead(LexerT lexer, char ** text, int * length, Position* position) {
  return 0;
}
