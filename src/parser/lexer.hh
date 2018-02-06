/// Public Interface for the Coral lexer

#pragma once
#define CORAL_LEXER_HH

#include <cstdio>

#ifndef LexerHandle
typedef void * LexerHandle;
#endif

struct Position {
  struct { int pos, row, col; } start;
  struct { int pos, row, col; } end;
};

extern "C" LexerHandle lexerCreate(const char * input);
extern "C" void lexerDestroy(LexerHandle lexer);
extern "C" int lexerRead(LexerHandle lexer, char ** text, int * length, Position* position);
