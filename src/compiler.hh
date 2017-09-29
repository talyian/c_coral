#include "ast.hh"
#include <memory>
#include <string>

class ModuleCompiler {
  class impl;
  struct implDeleter { void operator () (impl *p); };
  std::unique_ptr<impl, implDeleter> pimpl;
public:
  ModuleCompiler(Module * m);
  std::string getIR();
};
