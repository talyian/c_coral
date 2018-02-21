/* High level functionality for Coral */
#include <vector>
#include <memory>

#include "analyzers/NameResolver.hh"
#include "analyzers/TypeResolver.hh"
#include "analyzers/ReturnInserter.hh"
#include "parser/parser.hh"
#include "codegen/LLVMModuleCompiler.hh"
#include "../core/expr.hh"
#include "../core/prettyprinter.hh"

namespace coral {
  void Run(const char * path);
  void Compile(const char * path);


  class CodeProcessingUnit {
  public:
    ast::Module * module;
    void * parser;
    // analyzers::NameResolver * nameResolver;
    // analyzers::TypeResolver * typeResolver;
    // analyzers::ReturnInserter * returner;
    codegen::LLVMModuleCompiler * compiler;
    LLVMModuleRef llvmModule;
    CodeProcessingUnit(const char * path) ;
    void showSource();
    void showIR();
    void runJIT();
    ~CodeProcessingUnit() {
      if (parser) coralDestroyModule(parser);
      if (compiler) delete compiler;
    }
  };
}
