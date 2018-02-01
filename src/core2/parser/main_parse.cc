#include "parser.hh"
#include <cstdio>

void showFile(const char * filename) {
  FILE * f = fopen(filename, "r");
  if (f) {
	fclose(f);
	coralDestroyModule(coralParseModule(filename));
  }
}

int main(int argc, const char ** argv) {
  showFile("tests/cases/shootout/fasta.coral");
  showFile("tests/cases/shootout/knucleotide.coral");
  showFile("tests/cases/shootout/regexredux.coral");
  showFile("tests/cases/shootout/pidigits.coral");
}
