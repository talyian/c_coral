#include "core/expr.hh"
#include "core/prettyprinter.hh"
#include "parser/parser.hh"
#include "analyzers/NameResolver.hh"
#include "analyzers/TypeResolver.hh"
#include "tests/runner/base.hh"

#include "parser.hh"

#include <iostream>
#include <iomanip>
#include <sstream>
namespace coral {
  namespace tests {
    void ParserTests::parse_and_print(const char * name, const char * path) {
      std::cout
        << "-------------------- ["
        << std::setw(20) << name
        << "] --------------------"
        << "\n";
      auto parser = coralParseModule(path);
      auto module = _coralModule(parser);
      total++;
      if (module) success++;
      coralDestroyModule(parser);
    }

    void ParserTests::checkTypeInferenceParam() {
      auto parser = coralParseModule("tests/cases/features/typeInference-parameter.coral");
      auto module = (ast::Module *)_coralModule(parser);
      analyzers::NameResolver nresolve(module);
      analyzers::TypeResolver tresolve(module);

      BeginTest("Parameter Type Inference");
      for(auto &&expr: module->body->lines) {
        auto func = dynamic_cast<ast::Func *>(expr.get());
        if (!func) continue;
        if (func->name == "StringParameter") {
          Assert(func->type->params[0] == coral::type::Type("Ptr"));
        } else if (func->name == "IntegerAddition") {
          Assert(func->type->params[0] == coral::type::Type("Int32"));
        }
      }
      if (this->failCounter) PrettyPrinter::print(module);
      EndTest();
      coralDestroyModule(parser);
    }
    void ParserTests::checkTypeInferenceReturn() {
      auto parser = coralParseModule("tests/cases/features/typeInference-return.coral");
      auto module = (ast::Module *)_coralModule(parser);
      analyzers::NameResolver nresolve(module);
      analyzers::TypeResolver treslve(module);
      BeginTest("Return Type Inference");

      for(auto && expr : module->body->lines) {
        auto func = dynamic_cast<ast::Func *>(expr.get());
        if (!func) continue;
        if (func->name == "returnsInt") {
          Assert(func->type->params.back() == coral::type::Type("Int32"));
        } else if (func->name == "returnsIntVar") {
          Assert(func->type->params.back() == coral::type::Type("Int32"));
        }
        else if (func->name == "returnsIntParam") {
          Assert(func->type->params.back() == coral::type::Type("Int32"));
        }
        else if (func->name == "returnsIntCall") {
          Assert(func->type->params.back() == coral::type::Type("Int32"));
        }
        else if (func->name == "returnsIntAddition") {
          Assert(func->type->params.back() == coral::type::Type("Int32"));
        }
        else if (func->name == "returnsString") {
          Assert(func->type->params.back() == coral::type::Type("Ptr"));
        }
      }
      if (this->failCounter) PrettyPrinter::print(module);
      EndTest();
      coralDestroyModule(parser);
    }

    void ParserTests::checkStructMethodInference()  {
      // type Person = {age:Int32}
      // func Person.getType(): "Person"
      // func Person.getAge(): self.age
      // let p = Person 33
      auto module = new ast::Module({
          new ast::Tuple(
            "Person", std::vector<ast::Def *> {
              new ast::Def("age", new type::Type("Int32"), 0)}),
          new ast::Func(
            {"Person", "getType"},
            new type::Type(""),
            {},
            new ast::Block({ new ast::StringLiteral("\"Person\"") })),
          new ast::Func(
            {"Person", "getAge"},
            new type::Type(""),
            {},
            new ast::Block({new ast::Member(new ast::Var("self"), "age") })),
          new ast::Let(
            new ast::Var("p"),
            new ast::Call(new ast::Var("Person"), {new ast::IntLiteral("33")}))
          });
      // PrettyPrinter::print(module);
      analyzers::NameResolver { module };
      analyzers::TypeResolver tr { module };
      // tr.gg.show();
      // PrettyPrinter::print(module);
      // std::cerr << module << "\n";
      }

	ParserTests * run_parser_tests() {
	  auto T = new ParserTests();
	  T->parse_and_print("If Statement", "tests/cases/features/ifstatements.coral");
	  T->parse_and_print("Hello World", "tests/cases/simple/hello_world.coral");
	  T->parse_and_print("Factorial", "tests/cases/simple/factorial.coral");
	  T->parse_and_print("Collatz Function", "tests/cases/simple/collatz.coral");
	  T->parse_and_print("Unicode Strings", "tests/cases/simple/unicode-strings.coral");
      T->checkTypeInferenceReturn();
      T->checkTypeInferenceParam();
      T->checkStructMethodInference();
	  T->parse_and_print("Structs", "tests/cases/simple/tuples.coral");
      // exit (1);
 	  return T;
	}

  }
}
