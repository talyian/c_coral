#include "../core/treeprinter.hh"
#include "lexer.hh"
#include <cstdio>

int main(int argc, char ** argv) {
  Module * modulep = 0;
  if (argc > 1) {
	for(int i=1; i<argc; i++) {
	  FILE * f = fopen(argv[i], "r");
	  modulep = parse(f, 0);
	  fclose(f);
	  fclose(f);
	  TreePrinter tp(modulep, std::cout);
	  tp.print();
	  delete modulep;
	}
  }
  else {
    modulep = parse(stdin, 0);
	TreePrinter tp(modulep, std::cout);
	tp.print();
	delete modulep;
  }
}
