#include "base.hh"

namespace coral {
  namespace tests {
	class CodegenTests : public TestSuite {
	public:
	  const char * getName() { return "Codegen Tests"; }
	  void Run(const char * path);
	  void RunFactorial();
	  void RunFactorialWhile();
	  void RunCollatz();
	};

	TestSuite * run_codegen_tests();
  }
}
