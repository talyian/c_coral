#include "utils/opts.hh"
#include <cstdlib>
#include <string>
#include <cstring>

int coral::opt::ShowTypeSolution = 0;
int coral::opt::ShowIR = 0;
int coral::opt::ShowInitialParseTree = 0;
int coral::opt::ShowFinalParseTree = 0;

namespace coral {
  namespace opt {
    bool on(char * s) { return s && strlen(s) && strcmp(s, "0") != 0; }
    void initOpts() {
      if (on(std::getenv("SHOWIR"))) ShowIR = 1;
      if (on(std::getenv("SHOWTYPE"))) ShowTypeSolution = 1;
      if (on(std::getenv("SHOWTREE"))) ShowFinalParseTree = 1;
      if (on(std::getenv("VERBOSE"))) {
        ShowTypeSolution = ShowIR = ShowFinalParseTree = 1;
      }
    }
  }
}
