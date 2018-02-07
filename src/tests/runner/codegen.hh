#include "core/prettyprinter.hh"
#include "codegen/LLVMModuleCompiler.hh"
#include "codegen/LLVMFunctionCompiler.hh"
#include "codegen/LLVMJit.hh"
#include "analyzers/NameResolver.cc"
#include "analyzers/ReturnInserter.hh"

namespace coral {
  namespace tests {

	class CodegenTests : public TestSuite {
	public:
	  const char * getName() { return "Codegen Tests"; }

	  void Run(const char * path) {
		total += 1;
		auto parser =  coralParseModule(path);
		auto module = (ast::Module *)_coralModule(parser);

		PrettyPrinter::print(module);
		analyzers::NameResolver resolver(module);
		analyzers::ReturnInserter returner(module);
		PrettyPrinter::print(module);

		codegen::LLVMModuleCompiler compiler(module);
		codegen::LLVMRunJIT(compiler.llvmModule);
		compiler.llvmModule = 0;
		coralDestroyModule(parser);
		success += 1;
	  }
	};

	TestSuite * run_codegen_tests() {
	  auto T = new CodegenTests();
	  T->Run("tests/cases/simple/hello_world.coral");
	  return T;
	}
  }
}
