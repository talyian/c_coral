#include "../core/treeprinter.hh"
#include "lexer.hh"
#include <cstdio>

int main(int argc, char ** argv) {
  FILE * f = 0;
  if (argc > 1) f = fopen(argv[1], "r");
  void * scanner = 0;
  YYLTYPE loc;
  YYSTYPE lval;
  yylex_init(&scanner);
  yyset_in(f, scanner);
  int tok; while((tok = yylex(&lval, &loc, scanner))) {
    string text(yyget_text(scanner));
    std::replace(text.begin(), text.end(), '\n', '_');
    switch(tok) {
    case yy::parser::token::NEWLINE: printf("  [ NL]\n"); break;
    case yy::parser::token::INDENT: printf("  [IND]\n"); break;
    case yy::parser::token::DEDENT: printf("  [DED]\n"); break;
    default: printf("[%d] %s\n", tok, text.c_str()); break;
    }
  }
  yylex_destroy(scanner);
}
