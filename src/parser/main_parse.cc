#include "parser.hh"
#include "core/prettyprinter.hh"
#include <cstdio>

void showFile(const char * filename) {
  FILE * f = fopen(filename, "r");
  if (f) {
	fclose(f);
	auto parser = coralParseModule(filename);
	auto module = _coralModule(parser);
	if (module) {
	  coral::PrettyPrinter pp;
	  ((coral::ast::Module *)module)->accept(&pp);
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
