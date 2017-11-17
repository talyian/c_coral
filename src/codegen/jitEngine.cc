#include "../core/expr.hh"
#include "../core/treeprinter.hh"
#include "../codegen/moduleCompiler.hh"
// #include "../passes/UnclassifyPass.cc"
#include "../passes/MainFuncPass.cc"
#include "../passes/UnclassifyPass.cc"
#include "../passes/ReturnInsertPass.cc"
#include "../passes/ResolveNamesPass.cc"
#include "../passes/InferTypesPass.cc"

#include "jitEngine.hh"

#include <llvm-c/ExecutionEngine.h>
#include <vector>
#include <iostream>
#include <sstream>

using namespace coral;

std::vector<LLVMModuleRef> compile_modules(std::vector<Module *> modules) {
  vector<LLVMModuleRef> llvmModules;

  // A map of names to expression data
  // As well as expression data to inferred type info
  Scope s;
  // A map of exressions to generated LLVM SSA nodes
  // This is used so we can link variable references.
  std::map<Expr *, LLVMValueRef> savedValues;
  for(auto & m : modules) {
	m = UnclassifyPass(m).module;
	// WARN: not sure MainFuncPass should come before InferTypes Pass
	// it changes the ordering of var aliasing, for example.
	m = MainFuncPass(m).out;
	m = (Module *)InferTypesPass(m, &s).out;
	m = ReturnInsertPass(m).out;
	ModuleCompiler mc(m, savedValues);
	savedValues = mc.savedValues;
	llvmModules.push_back(mc.llvmModule);
  }
  return llvmModules;
}

std::string ir_modules(std::vector<Module *> modules) {
  auto llvmModules = compile_modules(modules);
  return std::string(LLVMPrintModuleToString(llvmModules.back()));
}

void jit_modules(std::vector<Module *> modules) {
  LLVMLinkInMCJIT();
  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();

  auto llvmModules = compile_modules(modules);

  auto firstModule = llvmModules[0];
  char * error = 0;
  LLVMExecutionEngineRef engine;
  if (LLVMCreateExecutionEngineForModule(&engine, firstModule, &error) != 0) {
	fprintf(stderr, "failed to create execution engine\n");
  } else if (error) {
	fprintf(stderr, "error: %s\n", error);
	LLVMDisposeMessage(error);
  } else {
	for(auto &m : llvmModules)
	  if (m != firstModule)
		LLVMAddModule(engine, m);
	LLVMValueRef fn;
	if (!LLVMFindFunction(engine, "main", &fn))
	  LLVMRunFunction(engine, fn, 0, 0);
	else
	  std::cerr << "No main function found\n";
  }
}

void CompilationUnit::generate_llvm_modules() {
  for(auto &m : compile_modules(modules))
	llvmModules.push_back((void *)m);
  compiled = true;
}

void CompilationUnit::run_jit() {
  jit_modules(modules);
}

std::string CompilationUnit::get_source() {
  std::stringstream ss;
  TreePrinter(modules.back(), ss).print();
  return ss.str();
}

std::string CompilationUnit::get_ir() {
  if (!compiled) generate_llvm_modules();
  auto lastModule = llvmModules.back();
  return std::string(LLVMPrintModuleToString((LLVMModuleRef)lastModule));
}
