#include "llvm-c/Core.h"
#include "llvm-c/ExecutionEngine.h"
#include <iostream>

using std::cerr;
using std::string;

class ModuleExecutor {
public:
  LLVMModuleRef module;
  LLVMExecutionEngineRef engine;
  
  ModuleExecutor(LLVMModuleRef module) : module(module) {
      char * error;
      if (!LLVMCreateExecutionEngineForModule(&engine, module, &error)) { cerr << "oops"; }
      else if (error) { cerr << error; }
  }
  LLVMValueRef findFunction(string name) {
    LLVMValueRef fn;
    if (LLVMFindFunction(engine, name->c_str(), &fn)) return fn;
    return 0;
  }
}
  
class Executor {
  
}
