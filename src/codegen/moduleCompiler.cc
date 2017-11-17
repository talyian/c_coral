#include "moduleCompiler.hh"

namespace coral {

  void ToLLVMType::visit(FuncType * f) {
	std::vector<LLVMTypeRef> params(f->args.size());
	std::transform(
	  f->args.begin(), f->args.end(),
	  params.begin(), [this] (BaseType * t) { return TOLLVMTYPE(t); });
	if (!f->ret)
	  std::cerr << "functype no return type " << f->ret << "\n";
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

  ModuleCompiler::ModuleCompiler(coral::Module * m, std::map<Expr *, LLVMValueRef> savedValues)
	: Visitor("modulec "), savedValues(savedValues)
  {
	context = LLVMContextCreate();
	llvmModule = LLVMModuleCreateWithNameInContext(m->name.c_str(), context);
	llvmBuilder = LLVMCreateBuilderInContext(context);
	for(auto & line : m->lines)
	  line->accept(this);
  }
  ModuleCompiler::ModuleCompiler(coral::Module * m) : ModuleCompiler(m, std::map<Expr *, LLVMValueRef>()) {

  }

  std::string ModuleCompiler::getIR() {
	return std::string(LLVMPrintModuleToString(llvmModule));
  }

  void ModuleCompiler::visit(Extern * f) {
	auto type = f->type;
	if (!f->type)
	  return;
	LLVMValueRef func = LLVMAddFunction(llvmModule, f->name.c_str(), TOLLVMTYPE(type));
	savedValues[f] = func;
  }

  // TODO: global values
  void ModuleCompiler::visit(Let * f) { }

  void ModuleCompiler::visit(Struct * e) {
	std::vector<LLVMTypeRef> fields(e->fields.size());
	std::transform(
	  e->fields.begin(), e->fields.end(), fields.begin(),
	  [this] (Expr * f) {
		if (EXPRTYPE(f) == LetKind) {
		  auto let = (Let *)f;
		  return TOLLVMTYPE(let->var->type);
		}
	  });
	auto valueType = LLVMStructTypeInContext(
	  context,
	  &fields[0],
	  fields.size(),
	  true);
	// savedTypes[e] = valueType;
  }

  void ModuleCompiler::visit(FuncDef * f) {
	auto ftype = new coral::FuncType(
	  f->rettype,
	  vmap<BaseDef *, BaseType *>(f->args, [] (BaseDef * d) { return ((Def *)d)->type; }),
	  f->multi);

	llvmFunction = LLVMAddFunction(llvmModule, f->name.c_str(), TOLLVMTYPE(ftype));
	LLVMPositionBuilderAtEnd(
	  llvmBuilder,
	  LLVMAppendBasicBlockInContext(context, llvmFunction, "entry"));

	savedValues[f] = llvmFunction;
	int i = 0;
	for(auto &d : f->args) {
	  savedValues[d] = LLVMGetParam(llvmFunction, i++);
	  // auto a = (Def *)d;
	  // std::cerr << "ARG " << (i-1) << " " << a->name << " : " << a << "\n";
	}
	ExpressionCompiler ec(this);
	f->body->accept(&ec);
  }
}
