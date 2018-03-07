#include "parser.hh"
#include "utils/ansicolor.hh"
#include "utils/opts.hh"
#include "core/prettyprinter.hh"
#include "analyzers/ReturnInserter.hh"
#include "analyzers/ImportResolver.hh"
#include "analyzers/NameResolver.hh"
#include "analyzers/TypeResolver.hh"

#include <cstdio>

void showFile(const char * filename) {
  FILE * f = fopen(filename, "r");
  if (f) {
	fclose(f);
	auto parser = coralParseModule(filename);
	auto module = (coral::ast::Module *)_coralModule(parser);
	if (module) {
	  if (coral::opt::ShowInitialParseTree) coral::PrettyPrinter::print(module);          
          coral::analyzers::ImportResolver iri(module);
	  coral::analyzers::NameResolver nri(module);
	  coral::analyzers::TypeResolver tri(module);
	  coral::analyzers::ReturnInserter ri(module);
	  coral::PrettyPrinter::print(module);
	}
	coralDestroyModule(parser);
  } else { printf("%sNot found: %s%s\n", COL_LIGHT_RED, filename, COL_CLEAR); }
  printf("\n");
}

int main(int argc, const char ** argv) {
  coral::opt::initOpts();
  if (argc > 1) { showFile(argv[1]); return 0; }
  showFile("tests/cases/simple/collatz.coral"); return 0;
  showFile("tests/cases/shootout/fasta.coral");
  showFile("tests/cases/shootout/knucleotide.coral");
  showFile("tests/cases/shootout/regexredux.coral");
  showFile("tests/cases/shootout/pidigits.coral");
}
