#include <cstdio>
#include "codegen.hh"
#include "utils/opts.hh"

int main(int argc, char ** argv) {
  coral::opt::initOpts();
  if (argc > 1) { coral::Run(argv[1]); return 0; }
  coral::Run("tests/cases/simple/hello_world.coral");
}
