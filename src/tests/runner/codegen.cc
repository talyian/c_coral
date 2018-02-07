#include "core/prettyprinter.hh"
#include "codegen/LLVMModuleCompiler.hh"
#include "codegen/LLVMFunctionCompiler.hh"
#include "codegen/LLVMJit.hh"
#include "analyzers/NameResolver.cc"
#include "analyzers/ReturnInserter.hh"
#include "parser/parser.hh"

#include "codegen.hh"

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
		parser =  coralParseModule(file);
		auto module = (ast::Module *)_coralModule(parser);
		analyzers::NameResolver resolver(module);
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

	void CodegenTests::Run(const char * path) {
	  total += 1;
	  TestFunction<int(*)()>("main", path).Call();
	  success += 1;
	}

	void CodegenTests::RunFactorial() {
	  total++;
	  auto factorial = TestFunction<int(*)(int)>("factorial", "tests/cases/simple/factorial.coral");
	  success += factorial.Call(10) == 3628800 ? 1 : 0;
	  return;
	}

	void CodegenTests::RunCollatz() {
	  total += 1;
	  auto collatz = TestFunction<int(*)(int, int)>("collatz", "tests/cases/simple/collatz.coral");
	  success += collatz.Call(27, 0) == 111 ? 1 : 0;
	}

	TestSuite * run_codegen_tests() {
	  auto T = new CodegenTests();
	  T->show_header();
	  T->Run("tests/cases/simple/hello_world.coral");
	  T->RunFactorial();
	  T->RunCollatz();
	  T->show(0);
	  return T;
	}
  }
}
