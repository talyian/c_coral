#include <cstdio>

enum CoralTokenType {
  IDENTIFIER,
  STRING,
  INTEGER,
  FLOAT,
  SYMBOL,
  FOR,
  IN,
  IMPL,
  CLASS,
  MATCH,
  TYPE,
  ADDR_OF,
  LET,
  SET,
  AS,
  EXTERN,
  FUNC,
  NEWLINE,
  INDENT,
  DEDENT,
  IF,
  ELSE,
  ELIF,
  RETURN,
  PASS,
  t_EOF,
  t_NULL,
};

struct CoralToken {
  CoralTokenType type;
};

struct CoralTokenStream { };

struct CoralLexer { };

struct CoralLexer * coralCreateLexer(FILE * f);

CoralTokenType coralLex(struct CoralLexer * lexer, void * data, char * text);

void coralDestroyLexer(struct CoralLexer * lexer);
