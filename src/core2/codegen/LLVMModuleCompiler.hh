#include "llvm-c/Core.h"

#include "../core/expr.hh"
#include "LLVMFunctionCompiler.hh"

namespace coral {
  namespace codegen {

	class LLVMModuleCompiler : public ast::ExprVisitor {
	public :
	  LLVMContextRef llvmContext;
	  LLVMModuleRef llvmModule;
	  LLVMBuilderRef llvmBuilder;
	  ast::Module * astModule;

	  // output values
	  LLVMValueRef out = 0;
	  LLVMBasicBlockRef outBlock = 0;

	  LLVMModuleCompiler(ast::Module * m) {
		llvmContext = LLVMContextCreate();
		llvmModule = LLVMModuleCreateWithNameInContext("module", llvmContext);
		llvmBuilder = LLVMCreateBuilderInContext(llvmContext);
		m->accept(this);
		char *  outstr = LLVMPrintModuleToString(llvmModule);
		printf("\n%s\n", outstr);
		free(outstr);
	  }
	  void visit(ast::Module * m) {
		for (auto && line : m->body->lines) if (line) line->accept(this);
	  }
	  void visit(ast::Comment * c) { }
	  void visit(ast::Func * f) {
		LLVMFunctionCompiler fcc(
		  llvmContext,
		  llvmModule,
		  llvmBuilder,
		  f);
		fcc.visit(f);
	  }
	};
  }
}
