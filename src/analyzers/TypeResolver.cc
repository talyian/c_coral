#include "utils/ansicolor.hh"
#include "utils/opts.hh"
#include "core/expr.hh"
#include "core/prettyprinter.hh"
#include "analyzers/TypeResolver.hh"

#include <iostream>
#include <iomanip>
#include <set>
#include <algorithm>

#include "analyzers/typegraph/TypeGraph.hh"
#include "analyzers/TypeResultWriter.hh"

coral::analyzers::TypeResolver::TypeResolver(ast::Module * m): module(m) {
  m->accept(this);
  if (coral::opt::ShowTypeSolution) gg.Show("module");
  gg.Step();
  if (coral::opt::ShowTypeSolution) gg.Show("module");
  std::map<ast::BaseExpr *, ::Type *> translation;
  for(auto &pair : gg.expr_terms)
    translation[pair.first] = gg.GetTypeConstraintForTerm(pair.second);
  TypeResultWriter::write(translation);
}
