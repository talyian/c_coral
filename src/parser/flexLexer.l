%option noyywrap
%option bison-bridge
%option reentrant
%option yylineno
%option nounistd never-interactive
%{
#include "parser/lexer-internal.hh"
#include "parser/bisonParser.tab.hh"
#include "parser/tokens.hh"

#define STRLVAL yylval->build(std::string(yytext, yyleng))
%}

ASC     [\x00-\x7f]
ASCN    [\x00-\t\v-\x7f]
U       [\x80-\xbf]
U2      [\xc2-\xdf]
U3      [\xe0-\xef]
U4      [\xf0-\xf4]

UNICODE {U2}{U}|{U3}{U}{U}|{U4}{U}{U}{U}

%%

#.*                      { STRLVAL; return TOK::COMMENT; }
\"(\\\"|[^\"])*\"        { STRLVAL; return TOK::STRING; }
\'(\\\'|[^\'])*\'        { STRLVAL; return TOK::STRING; }
([_a-zA-Z]|{UNICODE})([_a-zA-Z0-9]|{UNICODE})* { STRLVAL; return coral::Token::GetKeyword(yytext) ? coral::Token::GetKeyword(yytext): TOK::IDENTIFIER; }
[0-9][0-9]*L?               { STRLVAL; return TOK::INTEGER; }
[0-9][0-9]*[.][0-9][0-9]*   { STRLVAL; return TOK::FLOAT; }
[0-9][0-9]*([.][0-9][0-9]*)?e(-?[0-9][0-9]*) { STRLVAL; return TOK::FLOAT; }
[-+*/=><$!|^%@]+         { STRLVAL; return coral::Token::GetOperator(yytext); }
[ ]                      { }
\n[ \t]*                 { yytext[0] = ' '; return TOK::NEWLINE; }
[.][.][.]                { return TOK::ELLIPSIS; }
.                        { return yytext[0]; }
<<EOF>>                  { return TOK::ENDOFFILE; }

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

int Lexer::BasicRead() {
    int val = yylex(lval, scanner);
    text = yyget_text(scanner);
    length = yyget_leng(scanner);
    pos.start.col = yyget_column(scanner);
    pos.start.row = yyget_lineno(scanner);
    return val;
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
        auto curIndent = indents.back();
        if (newIndent > curIndent) {
            indents.push_back(newIndent);
            tokenQueue.push_back(TOK::NEWLINE);
            tokenQueue.push_back(TOK::INDENT);
        } else if (newIndent < curIndent) {
            while (newIndent < curIndent) {
                indents.pop_back();
                tokenQueue.push_back(TOK::NEWLINE);
                tokenQueue.push_back(TOK::DEDENT);
                curIndent = indents.back();
            }
            tokenQueue.push_back(TOK::NEWLINE);
        } else {
            tokenQueue.push_back(TOK::NEWLINE);
        }
        return IndentRead();
    } else if (tok == TOK::ENDOFFILE) {
        if (indents.size() <= 1) return tok;
        while(indents.size() > 1) {
            indents.pop_back();
            tokenQueue.push_back(TOK::NEWLINE);
            tokenQueue.push_back(TOK::DEDENT);
        }
        tokenQueue.push_back(TOK::NEWLINE);
        tokenQueue.push_back(TOK::ENDOFFILE);
        return IndentRead();
    }
    return tok;
}

// this function converts [NEWLINE + ELSE] into [ELSE]
// TODO:
//   blank lines
//
//   in blocks should be allowed
int Lexer::IfElseRead() {
    if (prev_tok && prev_tok != TOK::NEWLINE) { auto v = prev_tok; prev_tok = 0; return v; }
    auto _tok = IndentRead();
    if (_tok == TOK::ELSE) {
        if (prev_tok) { prev_tok = 0; return _tok; }
        else { prev_tok = _tok; return _tok; }
    } else if (_tok == TOK::NEWLINE) {
        if (prev_tok) { return _tok; }
        prev_tok = _tok;
        return IfElseRead();
    } else {
        if (prev_tok) { prev_tok = _tok; return TOK::NEWLINE; }
        return _tok;
    }
}

int Lexer::Read() {
    return IfElseRead();
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
