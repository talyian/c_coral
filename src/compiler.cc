#include "compiler.hh"
#include "treeprinter.hh"
#include "type.hh"
#include "ast.hh"
#include "lexer.hh"

#include "inferTypePass.hh"
#include "returnInsertionPass.hh"
#include "mainfuncPass.hh"

std::string ir_module(Module * m);
void jit_modules(std::vector<Module *> m);
void show_modules(std::vector<Module *> m);

void CoralModule::init(FILE * in, const char * src) {
  yylex_init(&scanner);
  if (in) yyset_in(in, scanner);
  if (src) yy_scan_string(src, scanner);
  yy::parser coralp(module, scanner);
  coralp.parse();
  yylex_destroy(scanner);

  module = inferTypes(module);
  module = insertReturns(module);
  module = buildMainFunction(module);
}

CoralModule::CoralModule(FILE * in) { init(in, 0); }

CoralModule::CoralModule(const char * src) { init(0, src); }

std::string CoralModule::getIR() {
  return ir_module(module);
}

ostream& operator << (ostream & os, CoralModule t) {
  TreePrinter pp(t.module, os);
  pp.print();
  return os;
}

CoralCompiler::CoralCompiler() {
  CoralModule p1(fopen("samples/prelude/extern.coral", "r"));
  modules.push_back(p1);
}
CoralCompiler::CoralCompiler(std::vector<CoralModule> modules) : modules(modules)
{
  CoralModule p1(fopen("samples/prelude/extern.coral", "r"));
  modules.insert(modules.begin(), p1);
}
void CoralCompiler::load(CoralModule module) { modules.push_back(module); }
void CoralCompiler::run() {
  std::vector<Module *> m;
  foreach(modules, it) m.push_back(it->module);
  jit_modules(m);
}

void CoralCompiler::showIR() {
  std::vector<Module *> m;
  foreach(modules, it) m.push_back(it->module);
  show_modules(m);
}
