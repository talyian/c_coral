#pragma once
/// Private interface for Coral Lexer

// The public Lexer interface. We forward-declare the Lexer struct
// so that LexerT is a transparent struct pointer.
#ifdef CORAL_LEXER_HH
#error Do not include "lexer.hh" before "lexer-internal.hh"
#endif

class Lexer;
#define LexerT Lexer *
#include "lexer.hh"

// If the flex code hasn't been included yet,
// this flag states that we've defined yyscan_t already
#define YY_TYPEDEF_YY_SCANNER_T
#define yyscan_t void *

#include <vector>
#include <cstdio>
class Lexer {
  FILE * fp = 0;
  yyscan_t scanner;
  std::vector<int> indents { 0 };
  std::vector<int> tokenQueue;
public:
  Position pos;
  char * text;
  int length;
  Lexer();
  Lexer(const char * filename);
  ~Lexer();
  int Read();
};


typedef struct ParserParamStruct {
  Lexer * lexer = 0;
} * ParserParam;

typedef union {
  char * str;
} YYSTYPE;

// yylex is the original lexer function generated from flexLexer.l;
// coral_lex is our enriched lexer function (handling indents, etc) that
// encapsulates class Lexer::Read()
int coral_lex(YYSTYPE * yylval, ParserParam scanner);

#define YYTOKENTYPE coral::Token::TokenValues
