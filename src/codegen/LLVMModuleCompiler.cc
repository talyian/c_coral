#include "llvm-c/Core.h"
#include "../core/expr.hh"
#include "LLVMFunctionCompiler.hh"
#include "LLVMModuleCompiler.hh"

coral::codegen::LLVMModuleCompiler::LLVMModuleCompiler(ast::Module * m) {
		llvmContext = LLVMContextCreate();
		llvmModule = LLVMModuleCreateWithNameInContext("module", llvmContext);
		llvmBuilder = LLVMCreateBuilderInContext(llvmContext);

		LLVMAddFunction(
		  llvmModule, "pcre2_compile_8",
		  LLVMFunctionType(
			LLVMPointerType(LLVMInt8Type(), 0),
			0,
			0,
			false));

		LLVMAddFunction(
		  llvmModule, "pcre2_match_8",
		  LLVMFunctionType(
			LLVMInt32Type(),
			0,
			0,
			false));

		LLVMAddFunction(
		  llvmModule, "pcre2_match_data_create_8",
		  LLVMFunctionType(
			LLVMPointerType(LLVMInt8Type(), 0),
			0,
			0,
			false));

		LLVMAddFunction(
		  llvmModule, "pcre2_get_ovector_pointer_8",
		  LLVMFunctionType(
			LLVMArrayType(LLVMInt32Type(), 2),
			0, 0, false));

		LLVMAddFunction(
		  llvmModule, "pcre2_get_ovector_count_8",
		  LLVMFunctionType(
			LLVMInt32Type(),
			0, 0, false));

		m->accept(this);
	  }
