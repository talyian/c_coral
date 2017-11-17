#include "../core/expr.hh"
#include <vector>
#include <string>

void jit_modules(std::vector<coral::Module *> modules);

std::string ir_modules(std::vector<coral::Module *> modules);

namespace coral {
  class CompilationUnit {
	bool compiled = false;
  public:
	std::vector<coral::Module *> modules;
	std::vector<void *> llvmModules;
	CompilationUnit(std::vector<coral::Module *> modules) : modules(modules) { }
	void generate_llvm_modules();
	void run_jit();
	std::string get_source();
	std::string get_ir();
  };
}
