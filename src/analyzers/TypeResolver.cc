#include "utils/ansicolor.hh"
#include "core/expr.hh"
#include "core/prettyprinter.hh"
#include "analyzers/TypeResolver.hh"

#include <iostream>
#include <iomanip>
#include <set>
#include <algorithm>

#include "analyzers/typegraph/TypeGraph.hh"

coral::analyzers::TypeResolver::TypeResolver(ast::Module * m): module(m) {
  m->accept(this);
  gg.Show("types");
  gg.Step();
  gg.Step();
  gg.Step();
  gg.Show("types");
}
