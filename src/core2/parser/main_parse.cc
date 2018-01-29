#include "parser.hh"

int main() {
  // auto m = coralParseModule("tests/cases/simple/hello_world.coral");
  auto m = coralParseModule("tests/cases/shootout/knucleotide.coral");
  coralDestroyModule(m);
}
