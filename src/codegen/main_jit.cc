#include <cstdio>
#include "codegen.hh"

int main(int argc, char ** argv) {
  printf("Coral Codegen\n");
  if (argc > 1) { coral::Run(argv[1]); return 0; }
  coral::Run("tests/cases/simple/hello_world.coral");
}
