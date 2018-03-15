#include <cstdio>
#include "codegen/codegen.hh"
#include "utils/opts.hh"
// #ifdef __linux__
// #include "utils/segvhandler.hh"
// #endif

int main(int argc, char ** argv) {
  // #ifdef __linux__
  // signal(SIGSEGV, segvhandler);
  // #endif

  coral::opt::initOpts();
  if (argc > 1) { coral::Run(argv[1]); return 0; }
  coral::Run("tests/cases/simple/hello_world.coral");
}
