#include "compiler.hh"
#include "codegen.hh"
#include <memory>
#include <string>

class ModuleCompiler::impl {
public:
  ModuleBuilder builder;
  impl(Module * m) : builder(m) { }
  ~impl() { }
};

void ModuleCompiler::implDeleter::operator () (impl * p) { delete p; }

ModuleCompiler::ModuleCompiler(Module * m)
  : pimpl(std::unique_ptr<impl, implDeleter>(new impl(m))) { }

std::string ModuleCompiler::getIR() {
  return pimpl->builder.finalize();
}
