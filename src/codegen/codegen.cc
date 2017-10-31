#include <cstdio>
#include <map>
#include <algorithm>
#include <regex>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>

#include "../core/expr.hh"
#include "codegen.hh"

using std::string;
using std::cerr;
using std::endl;
using std::vector;

#define TSTR(x) LLVMPrintTypeToString(x)
#define ISPTR(x) (LLVMGetTypeKind(x) == LLVMPointerTypeKind)
#define ISARR(x) (LLVMGetTypeKind(x) == LLVMArrayTypeKind)

namespace coral {

  LLVMValueRef FUNC_MULTI = (LLVMValueRef)-1;
  // Define takes a scope, a name, and a LLVMValueRef that
  // is a pointer to the the variable location!
  void ModuleBuilder::define_func(LLVMValueRef scope, string name, LLVMValueRef value) {
	// cerr << "defining " << (scope ? "local " : "global ")
	//      << name
	//      << (value == FUNC_MULTI ? " [multi]" : "") <<"\n";
	names[scope][name] = value;
	names[scope][name + "=func"] = LLVMConstInt(LLVMInt32Type(), 0, false);
  }
  void ModuleBuilder::define(LLVMValueRef scope, string name, LLVMValueRef value) {
	// cerr << "define: " << name << ": " << TSTR(LLVMTypeOf(value)) << endl;
	// cerr << "defining " << name << "\n";
	names[scope][name] = value;
	names[scope][name + "=func"] = 0;
  }

  LLVMValueRef ModuleBuilder::loadWithType(LLVMValueRef scope, string name, LLVMTypeRef hint)  {
	return 0;
  }

  LLVMValueRef ModuleBuilder::load(LLVMBuilderRef __unused__, LLVMValueRef scope, string name) {
	// cerr << this <<  " loading: " << name << "\n";
	auto loc = names[scope][name];
	if (!loc && scope) {
	  loc = names[0][name];
	  if (loc) scope = 0;
	}
	if (names[scope][name + "=func"])
	  return loc;

	if (!loc) {
	  cerr << "scope :     " << scope << endl;
	  cerr << "not found : " << name << endl;
	  for(auto it = names.begin(); it != names.end(); it++) {
		cerr << "    " << it->first << endl;
		for (auto j = it->second.begin(); j != it->second.end(); j++) {
		  cerr << "        " << j->first << endl;
		}
	  }
	}
	// cerr << "loading " << name << " from " << LLVMPrintValueToString(loc) << endl;
	return LLVMBuildLoad(builder, loc, "");
  }
  LLVMTypeRef FixArgument(LLVMTypeRef t) {
	// pointer-to-arrays-of-T become pointers-to-T
	if (ISPTR(t) && ISARR(LLVMGetElementType(t))) {
	  return LLVMPointerType(LLVMGetElementType(LLVMGetElementType(t)), 0);
	}
	// cerr << "+ " << TSTR(t) << endl;
	// cerr << "| " << ISARR(t) << endl;
	// cerr << "| " << ISPTR(t) << endl;
	return t;
  }
  LLVMValueRef CastArgument(LLVMBuilderRef builder, LLVMValueRef arg, LLVMTypeRef paramtype) {
	auto argtype = LLVMTypeOf(arg);
	// cerr << "Casting " << TSTR(LLVMTypeOf(arg)) << " TO " << TSTR(paramtype) << endl;
	if (ISPTR(paramtype) && ISPTR(argtype) &&
		ISARR(LLVMGetElementType(argtype)) &&
		LLVMGetElementType(paramtype) == LLVMGetElementType(LLVMGetElementType(argtype))) {
	  return LLVMBuildBitCast(builder, arg, paramtype, "");
	}
	if ( LLVMGetTypeKind(LLVMTypeOf(arg)) == LLVMIntegerTypeKind &&
		 LLVMGetTypeKind(paramtype) == LLVMIntegerTypeKind) {
	  if (LLVMGetIntTypeWidth(LLVMTypeOf(arg)) > LLVMGetIntTypeWidth(paramtype))
		return LLVMBuildTrunc(builder, arg, paramtype, "");
	  else if (LLVMGetIntTypeWidth(LLVMTypeOf(arg)) < LLVMGetIntTypeWidth(paramtype))
		return LLVMBuildZExt(builder, arg, paramtype, "");
	}
	return arg;
  }

  GetLLVMType::GetLLVMType(ModuleBuilder *mb, Type * t) : mb(mb) {
	if (!t) { cerr << "trying to get LLVM type for null type" << endl; }
	t->accept(this); }

  void GetLLVMType::visit(FuncType * f) {
	std::vector<LLVMTypeRef> llvm_params(f->args.size());
	std::transform(
	  f->args.begin(), f->args.end(),
	  llvm_params.begin(),
	  [this] (Type * p) { return LTYPE(p); });
	out = LLVMFunctionType(LTYPE(f->ret), &llvm_params[0], f->args.size(), true);
  }
  void GetLLVMType::visit(Type * f) { out = LLVMVoidType(); }
  void GetLLVMType::visit(VoidType * f) { out = LLVMVoidType(); }
  void GetLLVMType::visit(PtrType * f) { out = LLVMPointerType(LTYPE(f->inner), 0); }
  void GetLLVMType::visit(ArrType * f) { out = LLVMArrayType(LTYPE(f->inner), f->len); }
  void GetLLVMType::visit(IntType * f) { out = LLVMIntTypeInContext(mb->context, f->bits); }
  void GetLLVMType::visit(FloatType * f) { out =
	  f->bits == 32 ? LLVMFloatTypeInContext(mb->context) :
	  f->bits == 64 ? LLVMDoubleTypeInContext(mb->context) :
	  0;
	if (!out) std::cerr << "Invalid Float Type " << f->toString() << std::endl;
  }


  void ModuleBuilder::init(Module * m) {
	context = LLVMContextCreate();
	module = LLVMModuleCreateWithNameInContext(m->name.c_str(), context);
	builder = LLVMCreateBuilderInContext(context);
	for(auto i = m->lines.begin(), e=m->lines.end(); i != e; i++)
	  (*i)->accept(this);
  }
  ModuleBuilder::ModuleBuilder(Module * m) : Visitor("codegen "){ init(m); }
  ModuleBuilder::ModuleBuilder(Module * m, std::map<LLVMValueRef, std::map<std::string, LLVMValueRef>> _n) : Visitor("codegen ")
  {
	names = _n;
	init(m);
  }

  ModuleBuilder::~ModuleBuilder() {
	// cerr << "freeing builder \n";
	LLVMDisposeBuilder(builder);
  }

  std::string ModuleBuilder::finalize() {
	std::unique_ptr<char, decltype(std::free) *> ptr(
	  LLVMPrintModuleToString(module),
	  std::free);
	return std::string(ptr.get());
  }

  void ModuleBuilder::visit(Return * c) {
	auto ret_type = LLVMGetReturnType(LLVMGetElementType(LLVMTypeOf(function)));
	auto cvalue = CastArgument(builder, VAL(c->value), ret_type);
	LLVMBuildRet(builder, cvalue);
  }


  void ModuleBuilder::visit(Call * c) { VAL(c); }
  void ModuleBuilder::visit(BinOp * c){ VAL(c); }
  void ModuleBuilder::visit(Var * c){ VAL(c); }
  void ModuleBuilder::visit(String * c){ VAL(c); }
  void ModuleBuilder::visit(Long * c){ VAL(c); }
  void ModuleBuilder::visit(VoidExpr * c){ }
  void ModuleBuilder::visit(Double * c){ VAL(c); }
  void ModuleBuilder::visit(AddrOf * c){ VAL(c); }
  void ModuleBuilder::visit(DeclTypeEnum * c) { }
  void ModuleBuilder::visit(BlockExpr * c) {
	for(auto i = c->lines.begin(), e = c->lines.end(); i != e; i++) {
	  (*i)->accept(this);
	}
  }

  void ModuleBuilder::visit(If * c) {
	auto bb = LLVMGetInsertBlock(builder);
	auto fn = LLVMGetBasicBlockParent(bb);
	auto ifbb = LLVMAppendBasicBlock(fn, "ifbb");
	auto elbb = LLVMAppendBasicBlock(fn, "elbb");
	auto nextbb = LLVMAppendBasicBlock(fn, "nextbb");
	LLVMBuildCondBr(builder, VAL(c->cond), ifbb, elbb);
	LLVMPositionBuilderAtEnd(builder, ifbb);
	c->ifbody->accept(this);
	LLVMBuildBr(builder, nextbb);
	LLVMPositionBuilderAtEnd(builder, elbb);
	c->elsebody->accept(this);
	LLVMBuildBr(builder, nextbb);
	LLVMPositionBuilderAtEnd(builder, nextbb);
  }

  void ModuleBuilder::visit(Let * a) {
	// LLVMValueRef func = function;
	// auto val = VAL(a->value);
	// if (!val) { cerr << "let-assign value empty (" << a->var->toString() << endl; return ;}
	// auto valtype = LLVMTypeOf(val);
	// auto dectype = LTYPE(a->var->type);
	// if (dectype == 0) dectype = valtype;
	// if (dectype == 0) cerr << "null let-assignment type" << endl;
	// // cerr << "let-assign! " << TSTR(dectype) << " := " << TSTR(valtype) << endl;
	// auto var = LLVMBuildAlloca(builder, dectype, "");
	// define(func, a->var->toString(), var);
	// LLVMBuildStore(builder, CastArgument(builder, val, dectype), var);
  }

  void ModuleBuilder::visit(FuncDef * c) {
	std::vector<LLVMTypeRef> llvm_params(c->args.size());
	// std::transform(
	//   c->args.begin(), c->args.end(),
	//   llvm_params.begin(),
	//   [this] (Def * p) { return LTYPE(p->type); });

	// cerr << "funcdef " << c->name << " -> " << c->rettype << "\n";

	if (!c->rettype) c->rettype = new VoidType();
	auto type = LLVMFunctionType(LTYPE(c->rettype), &llvm_params[0], c->args.size(), true);
	LLVMValueRef func = LLVMAddFunction(module, c->name.c_str(), type);
	function = func;
	if (c->multi) {
	  // auto name = c->name + "$" + LLVMPrintTypeToString(LTYPE(c->args[0]->type));
	  // define_func(0, name, func);
	  // define_func(0, c->name, FUNC_MULTI);
	} else define_func(0, c->name, func);

	// cerr << c->name << " ";
	// cerr << "args(" << c->args.size() << ") ";
	// cerr << "(" << LLVMPrintTypeToString(LLVMTypeOf(func)) << ") ";
	for(auto i=0; i<(int)c->args.size(); i++) {
	  // cerr << i << " " << c->args[i]->name << ", ";
	  // TODO
	  // define_func(func, c->args[i]->name, LLVMGetParam(func, i));
	}
	// cerr << endl;
	LLVMPositionBuilderAtEnd(builder, LLVMAppendBasicBlockInContext(context, func, "entry"));
	c->body->accept(this);

	// TODO - move this into a compile pass so we can push returns into if statements etc.
	// safety - all unterminated blocks get an implicit return
	// This only works for voidfuncs
	for(auto bb = LLVMGetFirstBasicBlock(func); bb; bb = LLVMGetNextBasicBlock(bb)) {
	  if (!LLVMGetBasicBlockTerminator(bb)) {
		LLVMPositionBuilderAtEnd(builder, bb);
		LLVMBuildRetVoid(builder);
	  }
	}
  }

  void ModuleBuilder::visit(Extern * c) {
	// cerr << "Extern " << c->name << ":" << (c->type ? c->type->toString() : "null") << endl;
	auto type = c->type ? c->type : new FuncType(new VoidType(), std::vector<Type *>(), true);
	LLVMValueRef func = LLVMAddFunction(module, c->name.c_str(), LTYPE(type));
	define_func(0, c->name, func);
  }

  void ModuleBuilder::visit(Tuple * n) { VAL(n); }

  std::string ir_module(Module * m) {
	ModuleBuilder moduleB(m);
	return moduleB.finalize();
  }

  void jit_modules(std::vector<Module *> modules) {
	LLVMLinkInMCJIT();
	LLVMInitializeNativeTarget();
	LLVMInitializeNativeAsmPrinter();

	std::vector<ModuleBuilder *> builders;
	std::map<LLVMValueRef, std::map<std::string, LLVMValueRef>> all_names;
	for(auto i = modules.begin(); i < modules.end(); i++) {
	  if (*i) {
		auto m = new ModuleBuilder(*i, all_names);
		builders.push_back(m);
		all_names = m->names;
	  }
	}
	auto llvm_module = builders[0]->module;

	LLVMExecutionEngineRef engine;
	char * error = NULL;

	if (LLVMCreateExecutionEngineForModule(&engine, llvm_module, &error) != 0) {
	  fprintf(stderr, "failed to create execution engine\n");
	} else if (error) {
	  fprintf(stderr, "error: %s\n", error);
	  LLVMDisposeMessage(error);
	} else {

	  for(auto i = builders.begin() + 1; i < builders.end(); i++) {
		LLVMAddModule(engine, (*i)->module);
	  }

	  LLVMValueRef fn;
	  if (!LLVMFindFunction(engine, "main", &fn))
		LLVMRunFunction(engine, fn, 0, 0);
	  else
		cerr << "No main function found\n";
	}
  }

  void show_modules(std::vector<Module *> modules) {
	std::vector<ModuleBuilder *> builders;
	std::map<LLVMValueRef, std::map<std::string, LLVMValueRef>> all_names;
	for(auto i = modules.begin(); i < modules.end(); i++) {
	  if (*i) {
		auto m = new ModuleBuilder(*i, all_names);
		builders.push_back(m);
		all_names = m->names;
	  }
	}
	cerr << builders.back()->finalize() << endl;
	return;
  }

}
