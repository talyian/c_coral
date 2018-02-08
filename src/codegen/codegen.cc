#include "../core/expr.hh"
#include "../core/prettyprinter.hh"
#include "LLVMModuleCompiler.hh"
#include "LLVMJit.hh"
#include <cstdio>

#include "analyzers/NameResolver.cc"
#include "analyzers/ReturnInserter.hh"
#include "parser/parser.hh"

namespace coral {
  void Compile(const char * path) {
	FILE * f = fopen(path, "r");
	if (!f) { std::cout << "Couldn't Open " << path << "\n"; return; }
	fclose(f);
	auto parser =  coralParseModule(path);
	auto module = (ast::Module *)_coralModule(parser);
	analyzers::NameResolver resolver(module);
	analyzers::ReturnInserter returner(module);
	codegen::LLVMModuleCompiler compiler(module);
	codegen::JIT jit(compiler.llvmModule);
	jit.compileObjectFile("bin/module.o", "i686-pc-win32");
	compiler.llvmModule = 0;
	coralDestroyModule(parser);
  }

  void Run(const char * path) {
	FILE * f = fopen(path, "r");
	if (!f) { std::cout << "Couldn't Open " << path << "\n"; return; }
	fclose(f);
	auto parser =  coralParseModule(path);
	auto module = (ast::Module *)_coralModule(parser);
	analyzers::NameResolver resolver(module);
	analyzers::ReturnInserter returner(module);
	PrettyPrinter::print(module);
	codegen::LLVMModuleCompiler compiler(module);
	std::cout << compiler.GetIR() << "\n";
	codegen::LLVMRunJIT(compiler.llvmModule);
	compiler.llvmModule = 0;
	coralDestroyModule(parser);
  }
}
