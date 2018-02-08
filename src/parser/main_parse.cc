#include "parser.hh"
#include "core/prettyprinter.hh"
#include "analyzers/ReturnInserter.hh"
#include <cstdio>

void showFile(const char * filename) {
  FILE * f = fopen(filename, "r");
  if (f) {
	fclose(f);
	auto parser = coralParseModule(filename);
	auto module = (coral::ast::Module *)_coralModule(parser);
	if (module) {
	  coral::analyzers::ReturnInserter ri(module);
	  coral::PrettyPrinter::print(module);
	}
	coralDestroyModule(parser);
  }
  printf("\n");
}

int main(int argc, const char ** argv) {
  if (argc > 1) { showFile(argv[1]); return 0; }
  showFile("tests/cases/simple/collatz.coral"); return 0;
  showFile("tests/cases/shootout/fasta.coral");
  showFile("tests/cases/shootout/knucleotide.coral");
  showFile("tests/cases/shootout/regexredux.coral");
  showFile("tests/cases/shootout/pidigits.coral");
}
