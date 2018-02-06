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
\'(\\\'|[^\'])*\'        { STRLVAL; return TOK::STRING; }
[_a-zA-Z][_a-zA-Z0-9]*     { STRLVAL; return coral::Token::GetKeyword(yytext) ?: TOK::IDENTIFIER; }
[0-9][0-9]*L?            { STRLVAL; return TOK::INTEGER; }
[0-9][0-9]*[.][0-9][0-9]*  { STRLVAL; return TOK::INTEGER; }
[-+*/=><$!|^%@]+         { STRLVAL; return coral::Token::GetOperator(yytext); }
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

// This function tracks parentheses and if we're not in a top-level context
// then whitespace is not significant
int Lexer::InlineRead() {
	int tok = BasicRead();
	if (!strcmp(text, "(") || !strcmp(text, "[") || !strcmp(text, "{"))
		parens.push_back(text);
	if (!strcmp(text, ")") || !strcmp(text, "]") || !strcmp(text, "}"))
		parens.pop_back();
	if (!parens.empty() && tok == TOK::NEWLINE) return InlineRead();
	return tok;
}

// this function tracks newlines and issues indents and dedent tokens
int Lexer::IndentRead() {
	if (!tokenQueue.empty()) {
		int val = tokenQueue.front();
		tokenQueue.erase(tokenQueue.begin());
		return val;
	}
	int tok = InlineRead();
	if (tok == TOK::NEWLINE) {
		auto newIndent = (int) yyget_leng(scanner) - 1;
		auto curIndent = indents.empty() ? 0 : indents.back();
		if (newIndent > curIndent) {
			indents.push_back(newIndent);
			tokenQueue.push_back(TOK::NEWLINE);
			tokenQueue.push_back(TOK::INDENT);
		} else if (newIndent < curIndent) {
			indents.pop_back();
			tokenQueue.push_back(TOK::NEWLINE);
			tokenQueue.push_back(TOK::DEDENT);
			tokenQueue.push_back(TOK::NEWLINE);
		} else {
			tokenQueue.push_back(TOK::NEWLINE);
		}
		return IndentRead();
	}
	return tok;
}

int Lexer::IfElseRead() {
	static int prev_tok = -1;
	static Position prev_pos;
	static std::string prev_text;

	static int _tok = -1;
	static Position _pos;
	static std::string _text;

	prev_tok = _tok;
	prev_pos = _pos;
	prev_text = _text;

	_tok = IndentRead();
	_pos = pos;
	_text = std::string(text, length);

	if (prev_tok == -1) return IfElseRead();

	// if (prev_tok == TOK::NEWLINE && _tok == TOK::ELSE) return IfElseRead();

	text = (char *)_text.c_str();
	length = _text.size();
	pos = _pos;
	return prev_tok;
}

int Lexer::BasicRead() {
	int val = yylex(lval, scanner);
	text = yyget_text(scanner);
	length = yyget_leng(scanner);
	pos.start.col = yyget_column(scanner);
	pos.start.row = yyget_lineno(scanner);
	return val;
}

int Lexer::Read() {
	return IndentRead();
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
  std::string tok = coral::Token::show(ret_val, lexer->text);
  if (lexer->debug) printf("lex: [%4d] %s \n", ret_val, tok.c_str());
  return ret_val;
}
