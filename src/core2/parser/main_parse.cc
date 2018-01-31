#include "parser.hh"
#include <cstdio>

int main(int argc, const char ** argv) {
  const char * filename = argc > 1 ? argv[1] : "tests/cases/simple/factorial.coral";
  FILE * f = fopen(filename, "r");
  if (f) fclose(f); else return 1;
  auto m = coralParseModule(filename);
  coralDestroyModule(m);
}
