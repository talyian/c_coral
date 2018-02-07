#include <cstdio>

void Run(const char * path);

int main() {
  printf("Coral Codegen\n");
  // Run("tests/cases/simple/hello_world.coral");
  // Run("tests/cases/simple/collatz.coral");
  Run("tests/cases/features/pcre.coral");
}
