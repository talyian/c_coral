#pragma once
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
	  void parse_and_print(const char * name, const char * path);
      void checkTypeInference();
	  const char * getName() { return "Parser Tests"; }
	};
	ParserTests * run_parser_tests();
  }
}
