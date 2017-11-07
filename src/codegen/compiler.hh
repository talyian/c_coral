#include "ast.hh"

class CoralModule {
  void * scanner;
  void init(FILE * in, const char * src);
public:
  Module * module;
  CoralModule(FILE * in);
  CoralModule(const char * in);
  std::string getIR();
};

class CoralCompiler {
public:
  std::vector<CoralModule> modules;
  CoralCompiler();
  CoralCompiler(std::vector<CoralModule> modules);
  void load(CoralModule module);
  void load(std::vector<CoralModule> modules);
  void run();
  void showIR();
};

ostream& operator << (ostream & os, CoralModule t);
