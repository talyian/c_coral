%option noyywrap
%option bison-bridge
%option reentrant
%option yylineno
%{
#include "lexer-internal.hh"
#include "bisonParser.tab.hh"
#include "tokens.hh"

#define STRLVAL yylval->build(std::string(yytext, yyleng))
%}
%%

#.*                      { STRLVAL; return TOK::COMMENT; }
\"(\\\"|[^\"])*\"        { STRLVAL; return TOK::STRING; }
[a-zA-Z][a-zA-Z0-9]*     { STRLVAL; return coral::Token::GetKeyword(yytext) ?: TOK::IDENTIFIER; }
[0-9][0-9]*L?            { STRLVAL; return TOK::INTEGER; }
[-+*/=><$!|^%@]+         { STRLVAL; return TOK::OP; }
[ ]                      { }
\n[ \t]*                 { yytext[0] = ' '; return TOK::NEWLINE; }
.                        { return yytext[0]; }

%%

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
		// val = yylex(lval, scanner);
      val = yylex(lval, scanner);
  	  text = yyget_text(scanner);
  	  length = yyget_leng(scanner);
  	  pos.start.col = yyget_column(scanner);
  	  pos.start.row = yyget_lineno(scanner);

  	  if (val != TOK::NEWLINE) {
  		return val;
  	  } else {
  		auto newIndent = (int) yyget_leng(scanner) - 1;
  		auto curIndent = indents.empty() ? 0 : indents.back();
  		if (curIndent < newIndent) {
  		  indents.push_back(newIndent);
  		  tokenQueue.push_back(TOK::INDENT);
  		} else if (curIndent > newIndent) {
  		  while(curIndent > newIndent) {
  			indents.pop_back();
  			curIndent = indents.empty() ? 0 : indents.back();
  			tokenQueue.push_back(TOK::DEDENT);
  			tokenQueue.push_back(TOK::NEWLINE);
  		  }
  		} else {
  		  return val;
  		}
  		return val;
  	  }
  	}
  }
}

LexerHandle lexerCreate(const char * filename) { return new Lexer(filename); }

void lexerDestroy(LexerHandle lexer) {
  delete lexer;
}

int lexerRead(LexerHandle lexer, char ** text, int * length, Position* position) {
  int val = lexer->Read();
  if (text) *text = lexer->text;
  if (length) *length = lexer->length;
  if (position) *position = lexer->pos;
  return val;
}

// Used by the Bison parser
int corallex(coral::parser::semantic_type * val, ParserParam pp) {
  auto lexer = pp->lexer;
  if (!lexer) return 0;
  lexer->lval = val;
  auto ret_val = lexer->Read();
  std::string tok = show(ret_val, lexer->text);
  if (lexer->debug) printf("lex: [%4d] %s \n", ret_val, tok.c_str());
  return ret_val;
}
