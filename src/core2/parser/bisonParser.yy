
%{
#include <cstdio>

int yylex() {

}

void yyerror(char * msg) {

}

%}

%%

program : { printf("hi\n"); }

%%

int main() {
	printf("hi there\n");
	yyparse();
}
