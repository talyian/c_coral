#include "core/expr.hh"
#include "parser/parser.hh"
#include <iostream>

class ParserTests {
public:
  int success, total;
  ParserTests() : success(0), total(0) { }
  void finish() {
	std::cout << "\nTests Succeeded: "
			  << success << " / " << total
			  << " (" << success * 100 / total << "%)\n";
  }
  void parse_and_print(const char * name, const char * path) {
	std::cout << "-------------------- ["
			  << name
			  << "] --------------------"
			  << "\n";
	auto parser = coralParseModule(path);
	auto module = _coralModule(parser);
	total++;
	if (module) success++;
	coralPrintAST(parser);
	coralDestroyModule(parser);
  }
};

int main() {
  ParserTests T;
  T.parse_and_print("If Statement", "tests/cases/features/ifstatements.coral");
  T.parse_and_print("Hello World", "tests/cases/simple/hello_world.coral");
  T.parse_and_print("Factorial", "tests/cases/simple/factorial.coral");
  T.parse_and_print("Collatz Function", "tests/cases/simple/collatz.coral");
  T.parse_and_print("Fasta", "tests/cases/shootout/fasta.coral");
  T.parse_and_print("Knucleotide", "tests/cases/shootout/knucleotide.coral");
  T.parse_and_print("Pidigits", "tests/cases/shootout/pidigits.coral");
  T.parse_and_print("Regex Redux", "tests/cases/shootout/regexredux.coral");
  T.finish();
  return T.total - T.success;
}
