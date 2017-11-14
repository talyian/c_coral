#include "moduleCompiler.hh"

namespace coral {

  void ToLLVMType::visit(FuncType * f) {
	std::vector<LLVMTypeRef> params(f->args.size());
	std::transform(
	  f->args.begin(), f->args.end(),
	  params.begin(), [this] (BaseType * t) { return TOLLVMTYPE(t); });
	// std::cerr << "fret " << f->ret << "\n";
	// std::cerr << "farg " << f->args.size() << "\n";
	out = LLVMFunctionType(
	  TOLLVMTYPE(f->ret),
	  &params[0],
	  params.size(),
	  f->variadic);
  }

  void ToLLVMType::visit(coral::VoidType * f) { out = LLVMVoidTypeInContext(context); }
  void ToLLVMType::visit(coral::IntType * f) { out = LLVMInt64TypeInContext(context); }
  void ToLLVMType::visit(coral::PtrType * f) { out = LLVMPointerType(TOLLVMTYPE(f->inner), 0); }

  ModuleCompiler::ModuleCompiler(coral::Module * m)  : Visitor("modulec ") {
	context = LLVMContextCreate();
	llvmModule = LLVMModuleCreateWithNameInContext(m->name.c_str(), context);
	llvmBuilder = LLVMCreateBuilderInContext(context);
	for(auto & line : m->lines)
	  line->accept(this);
  }

  std::string ModuleCompiler::getIR() {
	return std::string(LLVMPrintModuleToString(llvmModule));
  }

  void ModuleCompiler::visit(Extern * f) {
	auto type = f->type;
	if (!f->type)
	  return;
	LLVMValueRef func = LLVMAddFunction(llvmModule, f->name.c_str(), TOLLVMTYPE(type));
  }
  void ModuleCompiler::visit(Let * f) { }

  void ModuleCompiler::visit(FuncDef * f) {
	auto ftype = new coral::FuncType(
	  f->rettype,
	  vmap<BaseDef *, BaseType *>(f->args, [] (BaseDef * d) { return ((Def *)d)->type; }),
	  f->multi);

	llvmFunction = LLVMAddFunction(llvmModule, f->name.c_str(), TOLLVMTYPE(ftype));

	LLVMPositionBuilderAtEnd(
	  llvmBuilder,
	  LLVMAppendBasicBlockInContext(context, llvmFunction, "entry"));

	ExpressionCompiler ec(this);
	f->body->accept(&ec);
  }
}
