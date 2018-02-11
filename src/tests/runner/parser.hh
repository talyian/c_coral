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
      std::string currentTest;
      int failCounter = 0;
	  ParserTests() : TestSuite() { }
	  void parse_and_print(const char * name, const char * path);
      void checkTypeInference();
	  const char * getName() { return "Parser Tests"; }
      void BeginTest(std::string testName) {
        currentTest = testName;
        total++;
        failCounter = 0;
      }
      void EndTest() {
        if (!failCounter) {
          printf("%-60s OK\n", currentTest.c_str());
          success++;
        } else {
          printf("%-60s ERROR\n", currentTest.c_str());
          failCounter = 0;
        }
      }
      void Assert(bool val) { if (!val) failCounter++; }
	};
	ParserTests * run_parser_tests();
  }
}
