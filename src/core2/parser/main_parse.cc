#include "parser.hh"
#include <cstdio>

void showFile(const char * filename) {
  FILE * f = fopen(filename, "r");
  if (f) {
	fclose(f);
	coralDestroyModule(coralParseModule(filename));
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
