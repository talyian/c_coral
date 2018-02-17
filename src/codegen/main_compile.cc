#include <cstdio>
#include "codegen.hh"

void Compile(const char * path) {
  coral::CodeProcessingUnit cc(path);
  cc.showIR();
}

int main(int argc, const char ** argv) {
  auto file = argc > 1 ? argv[1] : "tests/cases/simple/hello_world.coral";
  Compile(file);
}
