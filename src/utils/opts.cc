#include "utils/opts.hh"
#include <cstdlib>
#include <string>

int coral::opt::ShowTypeSolution = 0;
int coral::opt::ShowIR = 0;
int coral::opt::ShowInitialParseTree = 0;
int coral::opt::ShowFinalParseTree = 0;

namespace coral {
  namespace opt {
    void initOpts() {
      if (std::getenv("SHOWIR")) ShowIR = 1;
      if (std::getenv("SHOWTYPE")) ShowTypeSolution = 1;
      if (std::getenv("SHOWTREE")) ShowFinalParseTree = 1;
      if (std::getenv("VERBOSE")) {
        ShowTypeSolution = ShowIR = ShowFinalParseTree = 1;
      }
    }
  }
}
