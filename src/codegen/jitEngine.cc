
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
