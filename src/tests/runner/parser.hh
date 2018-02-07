#include "core/expr.hh"
#include "parser/parser.hh"
#include "tests/runner/base.hh"

#include <iostream>
#include <sstream>
namespace coral {
  namespace tests {
	class ParserTests : public TestSuite {
	public:
	  ParserTests() : TestSuite() { }
	  void parse_and_print(const char * name, const char * path) {
		std::cout
		  << "-------------------- ["
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
	  const char * getName() { return "Parser Tests"; }
	};

	ParserTests * run_parser_tests() {
	  auto T = new ParserTests();
	  T->parse_and_print("If Statement", "tests/cases/features/ifstatements.coral");
	  T->parse_and_print("Hello World", "tests/cases/simple/hello_world.coral");
	  T->parse_and_print("Factorial", "tests/cases/simple/factorial.coral");
	  T->parse_and_print("Collatz Function", "tests/cases/simple/collatz.coral");
	  T->parse_and_print("Fasta", "tests/cases/shootout/fasta.coral");
	  T->parse_and_print("Knucleotide", "tests/cases/shootout/knucleotide.coral");
	  T->parse_and_print("Pidigits", "tests/cases/shootout/pidigits.coral");
	  T->parse_and_print("Regex Redux", "tests/cases/shootout/regexredux.coral");
	  return T;
	}

  }
}
