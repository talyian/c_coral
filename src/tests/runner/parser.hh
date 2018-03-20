#pragma once
#include "core/expr.hh"
#include "core/prettyprinter.hh"
#include "parser/parser.hh"
#include "tests/runner/base.hh"
#include "analyzers/NameResolver.hh"
#include "analyzers/TypeResolver.hh"
#include <iostream>
#include <sstream>
namespace coral {
  namespace tests {
	class ParserTests : public TestSuite {
	public:
	  ParserTests() : TestSuite() { }
	  void parse_and_print(const char * name, const char * path);
      void checkTypeInferenceParam();
      void checkTypeInferenceReturn();
	  const char * getName() { return "Parser Tests"; }

      void checkStructMethodInference();
	};
	ParserTests * run_parser_tests();
  }
}
