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

      void checkStructMethodInference() {
        auto module = new ast::Module({
            new ast::Tuple(
              "Person", std::vector<ast::Def *> {
                new ast::Def("age", new type::Type("Int32"), 0)}),
            new ast::Func({"Person", "getType"},
              new type::Type(""),
              {},
              new ast::Block({ new ast::StringLiteral("\"Person\"") })),
            new ast::Func({"Person", "getAge"},
              new type::Type(""),
              {},
              new ast::Block({
                  new ast::Member(new ast::Var("self"), "Person") })),
            new ast::Let(new ast::Var("p"),
                         new ast::Call(new ast::Var("Person"), {new ast::IntLiteral("33")}))
          });
        analyzers::NameResolver { module };
        analyzers::TypeResolver tr { module };
        // tr.gg.show();
        // PrettyPrinter::print(module);
        // std::cerr << module << "\n";
      }
	};
	ParserTests * run_parser_tests();
  }
}
