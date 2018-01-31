#pragma once
/// Private interface for Coral Lexer

// The public Lexer interface. We forward-declare the Lexer struct
// so that LexerT is a transparent struct pointer.
#ifdef CORAL_LEXER_HH
#error Do not include "lexer.hh" before "lexer-internal.hh"
#endif

#include "../core/expr.hh"

class Lexer;
#define LexerHandle Lexer *
#include "lexer.hh"

struct ParserParamStruct;
typedef ParserParamStruct * ParserParam;

#include "bisonParser.tab.hh"

#define YYSTYPE coral::parser::semantic_type

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
  coral::parser::semantic_type * lval;
  Position pos;
  char * text;
  int length;
  bool debug = false;
  Lexer();
  Lexer(const char * filename);
  ~Lexer();
  int Read();
};


struct ParserParamStruct {
  Lexer * lexer = 0;
  coral::ast::Module * module = 0;
};



// yylex is the original lexer function generated from flexLexer.l;
// corallex is our enriched lexer function (handling indents, etc) that
// encapsulates class Lexer::Read()
int corallex(coral::parser::semantic_type * yylval, ParserParam scanner);

// #define YYTOKENTYPE coral::Token::TokenValues
