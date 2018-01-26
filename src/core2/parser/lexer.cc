// Include lexer.h before everything else, but make LexerT a transparent pointer.
// this way, we can access the struct in the flex file.
struct Lexer;
#define LexerT Lexer *
#include "lexer.hh"

// this must be included before flex.hh for easy path reasons
#include "tokens.hh"

// bison-bridge assumes YYSTYPE is defined before flex is included
#define YYSTYPE int
#define YY_EXTRA_TYPE Lexer *
#include "../build/parser/flexLexer.hh"

#include <cstdio>
#include <vector>

struct Lexer {
  yyscan_t scanner;
  int row = 0;
  int col = 0;
  int pos = 0;
  std::vector<int> indents { 0 };
  std::vector<int> tokenQueue;
  FILE * fp = 0;
};

LexerT lexerCreate(char* filename) {
  auto lexer = new Lexer();
  yylex_init_extra(lexer, &lexer->scanner);
  if (filename) {
	lexer->fp = fopen(filename, "r");
	yyset_in(lexer->fp, lexer->scanner);
  }
  return lexer;
}

void lexerDestroy(LexerT lexer) {
  yylex_destroy(lexer->scanner);
  fclose(lexer->fp);
  delete lexer;
}

/// in which we build a loop around yylex to track line numbers and indent/dedents
int lexerRead(LexerT lexer, char ** text, int * length, Position* position) {
  YYSTYPE lval;
  auto scan = lexer->scanner;
  int val = 0;
  while(true) {
	if (!lexer->tokenQueue.empty()) {
	  int val = lexer->tokenQueue.front();
	  lexer->tokenQueue.erase(lexer->tokenQueue.begin());
	  return val;
	} else {
	  val = yylex(&lval, scan);
	  if (text) *text = yyget_text(scan);
	  if (length) *length = yyget_leng(scan);
	  if (position) {
		position->start.col = yyget_column(scan);
		position->start.row = yyget_lineno(scan);
	  }
	  if (val == coral::Token::NEWLINE) {
		auto newIndent = *length - 1;
		auto curIndent = lexer->indents.empty() ? 0 : lexer->indents.back();
		if (curIndent < newIndent) {
		  lexer->indents.push_back(newIndent);
		  lexer->tokenQueue.push_back(coral::Token::NEWLINE);
		  lexer->tokenQueue.push_back(coral::Token::INDENT);
		} else if (curIndent > newIndent) {
		  while(curIndent > newIndent) {
			lexer->indents.pop_back();
			curIndent = lexer->indents.empty() ? 0 : lexer->indents.back();
			lexer->tokenQueue.push_back(coral::Token::NEWLINE);
			lexer->tokenQueue.push_back(coral::Token::DEDENT);
		  }
		} else {
		  return val;
		}
	  } else {
		return val;
	  }
	}
  }
}
