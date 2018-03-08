#include "core/prettyprinter.hh"
#include "codegen/LLVMModuleCompiler.hh"
#include "codegen/LLVMFunctionCompiler.hh"
#include "codegen/LLVMJit.hh"
#include "analyzers/NameResolver.hh"
#include "analyzers/TypeResolver.hh"
#include "analyzers/ReturnInserter.hh"
#include "parser/parser.hh"

#include "codegen.hh"
#include <dlfcn.h>
namespace coral {
  namespace tests {
    template<typename T>
    class TestFunction {
    public:
      ParserType parser;
      codegen::LLVMModuleCompiler * compiler;
      codegen::JIT * jit;
      std::string funcName;
      T Call;

      TestFunction(const char * funcName, const char * file) {
		this->funcName = funcName;
		if (!fopen(file, "r")) return;
		parser =  coralParseModule(file);
		auto module = (ast::Module *)_coralModule(parser);
		analyzers::NameResolver nresolver(module);
		analyzers::TypeResolver tresolver(module);
		analyzers::ReturnInserter returner(module);
		compiler = new codegen::LLVMModuleCompiler(module);
		jit = new codegen::JIT(compiler->llvmModule);
		Call = jit->GetFunctionPointer<T>(funcName);
      }
      ~TestFunction() {
		compiler->llvmModule = 0;
		delete jit;
		delete compiler;
		coralDestroyModule(parser);
      }
    };

#define ASSERT(cond, name) { Assert(cond); }

    void CodegenTests::Run(const char * path) {
      BeginTest(path);
      ASSERT((TestFunction<int(*)()>("main", path).Call(), 1), path);
      EndTest();
    }

    void CodegenTests::RunFactorial() {
      auto factorial = TestFunction<int(*)(int)>("factorial", "tests/cases/simple/factorial.coral");
      BeginTest("Factorial");
      ASSERT((factorial.Call(10) == 3628800), "Factorial");
      EndTest();
    }

    void CodegenTests::RunFactorialWhile() {
      auto factorial = TestFunction<int(*)(int)>("factorial_while", "tests/cases/simple/while.coral");
      BeginTest("Factorial While");
      ASSERT((factorial.Call(9) == 362880), "Factorial While");
      EndTest();
    }

    void CodegenTests::RunCollatz() {
      auto collatz = TestFunction<int(*)(int, int)>("collatz", "tests/cases/simple/collatz.coral");
      BeginTest("Collatz");
      ASSERT((collatz.Call(27, 0) == 111), "Collatz");
      EndTest();
    }

    void CodegenTests::RunReturnInserter() {
	  const char * file = "tests/cases/features/returnInsert.coral";
      BeginTest("Return Inserter");
      ASSERT((TestFunction<int(*)()>("foo1", file).Call() == 1234), "Return Inserter 1");
      ASSERT((TestFunction<int(*)()>("foo2", file).Call() == 1234), "Return Inserter 2");
      ASSERT((TestFunction<int(*)()>("foo3", file).Call() == 1234), "Return Inserter 3");
      ASSERT((!strcmp(TestFunction<char *(*)()>("foo4", file).Call(), "1234")), "Return Inserter 4");
      ASSERT((TestFunction<int(*)()>("foo5", file).Call() == 0), "Return Inserter 5");
	  ASSERT((TestFunction<int(*)()>("foo6", file).Call() == 1234), "Return Inserter 6");
	  ASSERT((TestFunction<int(*)()>("foo7", file).Call() == 1234), "Return Inserter 7");
      EndTest();
    }

    TestSuite * run_codegen_tests() {
      auto T = new CodegenTests();
      T->show_header();
      T->Run("tests/cases/simple/hello_world.coral");
      T->RunFactorial();
	  T->RunFactorialWhile();
	  T->RunReturnInserter();
      T->RunCollatz();
      T->Run("tests/cases/simple/tuples.coral");
      T->Run("tests/cases/simple/tuples-methods.coral");
      // T->Run("tests/cases/features/pcre.coral");
      T->show(0);
      return T;
    }
  }
}
