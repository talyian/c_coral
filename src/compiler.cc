#include "ast.hh"
#include "parser.hh"
#include "codegen.hh"

#include <vector>
#include <iostream>

using std::vector;
using std::ifstream;

void Compile(vector<ifstream *> input) {
  Module * m;
  for(auto i = input.begin(), e = input.end(); i != e; i++) {
    yy::parser parser(m);
    parser.parse();
    ModuleBuilder mbuilder(m);
    mbuilder.finalize();
  }
}

void GenerateIR(Module & m) {

}

void GenerateObj(Module & m) {
  
}
