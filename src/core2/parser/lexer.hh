
/// Public Interface for the Coral lexer

#pragma once

#include <cstdio>

#ifndef LexerT
typedef void * LexerT;
#endif

struct Position {
  struct { int pos, row, col; } start;
  struct { int pos, row, col; } end;
};

extern "C" LexerT lexerCreate(char* input);
extern "C" void lexerDestroy(LexerT lexer);
extern "C" int lexerRead(LexerT lexer, char ** text, int * length, Position* position);
