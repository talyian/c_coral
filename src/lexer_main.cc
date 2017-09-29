#include <cstdio>
#include "ast.hh"
#include "parser.hh"
#include "lexer.hh"

void lex_stream(FILE * input, void (*f)(int, char *)) {
    yyscan_t scanner;
    int t;
    YYSTYPE lval;
    YYLTYPE lloc;
    YY_EXTRA_TYPE extra;
    yylex_init_extra(extra, &scanner);
    yyset_in(input, scanner);
    while((t = yylex(&lval, &lloc, scanner))) {
	f(t, yyget_text(scanner));
    }
    yylex_destroy(scanner);
}

Module * parse_stream(FILE * input) {
    yyscan_t scanner;
    Module * module;
    yylex_init(&scanner);
    yyset_in(input, scanner);
    yy::parser p(module, scanner);
    p.parse();
    yylex_destroy(scanner);    
    return module;
}

void handle_token(int t, char * text) {
    printf("[%4d] ", t);	
    switch(t) {
    case yy::parser::token::NEWLINE:
    case yy::parser::token::INDENT:
    case yy::parser::token::DEDENT:
	printf("\n");
	break;
    default:
	printf("[%s]\n", text);	    
    }
}

int main(int argc, char ** argv) {
    if (argc < 2) {
	printf("required argument missing\n");
	return 1;
    }
    for(int i=0; i<10; i++) {
    FILE * input = fopen(argv[1], "r");
    if (!input) {
	printf("File not Found[%s]\n", argv[1]);
	return 2;
    }
    Module * module = parse_stream(input);
    printf(
	"----------------------------------------\n%s\n",
	module->toString().c_str());
    delete module;
    fclose(input);
    }
    
}
