#include "../core/treeprinter.hh"
#include "lexer.hh"
#include <cstdio>

int main(int argc, char ** argv) {
  Module * modulep = 0;
  if (argc > 1)
    modulep = parse(fopen(argv[1], "r"), 0);
  else
    modulep = parse(
      0,
      "let x = 1 + 2\n"
      "let y = \"foobar\"\n"
      "printf(\"[%d]%s\", x, y)\n"
      );
  TreePrinter tp(modulep, std::cout);
  tp.print();
}
