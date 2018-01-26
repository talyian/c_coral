#include <cstdio>

struct ModuleT {
  FILE * fp = 0;
};

#define Module ModuleT *

#include "parser.hh"

Module coralParseModule(char * infile) {
  auto m = new ModuleT();
  if (infile) m->fp = fopen(infile, "r");
  return 0;
}

void coralJsonModule(Module m, char * outfile) {

}

void coralDestroyModule(Module m) {

}
