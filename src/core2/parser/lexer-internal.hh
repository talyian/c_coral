/// Private interface for Coral Lexer

// The public Lexer interface. We forward-declare the Lexer struct
// so that LexerT is a transparent struct pointer.
#ifdef CORAL_LEXER_HH
#error Do not include "lexer.hh" before "lexer-internal.hh"
#endif

struct Lexer;
#define LexerT Lexer *

// If we need to include the flex source,
// this flag states that we have a real type for yyscan_t
#define YY_TYPEDEF_YY_SCANNER_T
#define yyscan_t void *

#include <vector>
#include <cstdio>
struct Lexer {
  yyscan_t scanner;
  int row = 0;
  int col = 0;
  int pos = 0;
  std::vector<int> indents { 0 };
  std::vector<int> tokenQueue;
  FILE * fp = 0;
};


typedef struct ParserParamStruct {
  Lexer * lexer = 0;
} * ParserParam;

int zzlex(int * yylval, ParserParam scanner);

# include "lexer.hh"
