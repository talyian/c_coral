#include "lexer-internal.hh"

// this must be included before flex.hh for easy path reasons
#include "tokens.hh"

// bison-bridge assumes YYSTYPE is defined before flex is included
#define YY_EXTRA_TYPE Lexer *
#include "flexLexer.hh"
#include "bisonParser.tab.hh"

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

	  if (val != NEWLINE) {
		return val;
	  } else {
		auto newIndent = (int) yyget_leng(scanner) - 1;
		auto curIndent = indents.empty() ? 0 : indents.back();
		if (curIndent < newIndent) {
		  indents.push_back(newIndent);
		  tokenQueue.push_back(INDENT);
		} else if (curIndent > newIndent) {
		  while(curIndent > newIndent) {
			indents.pop_back();
			curIndent = indents.empty() ? 0 : indents.back();
			tokenQueue.push_back(DEDENT);
			tokenQueue.push_back(NEWLINE);
		  }
		} else {
		  return val;
		}
		return val;
	  }
	}
  }
}

// Used by the Bison parser
int coral_lex(YYSTYPE * val, ParserParam pp) {
  auto lexer = pp->lexer;
  int length;
  char * text;

  if (!lexer) return 0;
  auto ret_val = lexerRead(lexer, &text, &length, 0);
  std::string tok = show(ret_val, text);
  printf("Coral LEX: [%4d] %s \n", ret_val, tok.c_str());
  *val = lexer->lval;
  return ret_val;
}
