#include "lexer-internal.hh"

// this must be included before flex.hh for easy path reasons
#include "tokens.hh"

// bison-bridge assumes YYSTYPE is defined before flex is included
#define YYSTYPE int
#define YY_EXTRA_TYPE Lexer *
#include "../build/parser/flexLexer.hh"

LexerT lexerCreate(const char * filename) { return new Lexer(filename); }

void lexerDestroy(LexerT lexer) {
  delete lexer;
}

int lexerRead(LexerT lexer, char ** text, int * length, Position* position) {
  int val = lexer->Read();
  if (text) *text = lexer->text;
  if (length) *length = lexer->length;
  if (position) *position = lexer->pos;
  return val;
}

Lexer::Lexer(const char * filename) {
  yylex_init_extra(this, &scanner);
  if (!filename) return;
  fp = fopen(filename, "r");
  yyset_in(fp, scanner);
}

Lexer::Lexer() {
  yylex_init_extra(this, &scanner);
}

Lexer::~Lexer() {
  if (fp) fclose(fp);
  yylex_destroy(scanner);
}

// in which we build a loop around yylex to track line numbers and indent/dedents
int Lexer::Read() {
  YYSTYPE lval;
  int val = 0;
  while(true) {
	if (!tokenQueue.empty()) {
	  int val = tokenQueue.front();
	  tokenQueue.erase(tokenQueue.begin());
	  return val;
	} else {
	  val = yylex(&lval, scanner);
	  text = yyget_text(scanner);
	  length = yyget_leng(scanner);
	  pos.start.col = yyget_column(scanner);
	  pos.start.row = yyget_lineno(scanner);

	  if (val != coral::Token::NEWLINE) {
		return val;
	  } else {
		auto newIndent = (int) yyget_leng(scanner) - 1;
		auto curIndent = indents.empty() ? 0 : indents.back();
		if (curIndent < newIndent) {
		  indents.push_back(newIndent);
		  tokenQueue.push_back(coral::Token::NEWLINE);
		  tokenQueue.push_back(coral::Token::INDENT);
		} else if (curIndent > newIndent) {
		  while(curIndent > newIndent) {
			indents.pop_back();
			curIndent = indents.empty() ? 0 : indents.back();
			tokenQueue.push_back(coral::Token::NEWLINE);
			tokenQueue.push_back(coral::Token::DEDENT);
		  }
		} else {
		  return val;
		}
	  }
	}
  }
}

// Used by the Bison parser
int zzlex(YYSTYPE * val, ParserParam pp) {
  auto lexer = pp->lexer;
  int length;
  char * text;

  if (!lexer) return 0;
  auto ret_val = lexerRead(lexer, &text, &length, 0);
  std::string tok = coral::Token::show(ret_val, text);
  printf("ZZLEX: [%4d] %s \n", ret_val, tok.c_str());
  return ret_val;
}
