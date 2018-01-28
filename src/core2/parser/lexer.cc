#include "lexer-internal.hh"

// this must be included before flex.hh for easy path reasons
#include "tokens.hh"

// bison-bridge assumes YYSTYPE is defined before flex is included
#define YYSTYPE int
#define YY_EXTRA_TYPE Lexer *
#include "../build/parser/flexLexer.hh"

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

// Used by the Bison parser
int zzlex(YYSTYPE * val, ParserParam pp) {
  printf("[ZZLEX] \n");
  auto lexer = pp->lexer;
  int length;
  char * text;

  if (!lexer) return 0;
  auto ret_val = lexerRead(lexer, &text, &length, 0);
  std::string tok = coral::Token::show(ret_val, text);
  printf("lexerread: [%d] %s \n", ret_val, tok.c_str());
  return ret_val;
}

// in which we build a loop around yylex to track line numbers and indent/dedents
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

	  if (val != coral::Token::NEWLINE) {
		return val;
	  } else {
		auto newIndent = yyget_leng(scan) - 1;
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
	  }
	}
  }
}
