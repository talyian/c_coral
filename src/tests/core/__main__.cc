
#include "../../core/treeprinter.hh"

#include <iostream>

using namespace std;
using namespace coral;

int main() {
  cout << "----------[ Core Tests ]----------\n";
  TreePrinter(new Module(vector<Expr *>()), cout).print();
}
