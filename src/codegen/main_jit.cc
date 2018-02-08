#include <cstdio>

void Run(const char * path);

int main(int argc, char ** argv) {
  printf("Coral Codegen\n");
  if (argc > 1) { Run(argv[1]); return 0; }
  // Run("tests/cases/simple/hello_world.coral");
  // Run("tests/cases/simple/collatz.coral");
  Run("tests/cases/features/returnInsert.coral");
}
