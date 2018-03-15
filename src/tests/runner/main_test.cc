#include "tests/runner/parser.hh"
#include "tests/runner/codegen.hh"
#include "tests/runner/base.hh"
#include "utils/opts.hh"
#ifdef __linux__
#include "utils/segvhandler.hh"
#endif

int main(int argc, const char ** argv) {
  #ifdef __linux__
  signal(SIGSEGV, segvhandler);
  #endif

  coral::opt::initOpts();
  coral::tests::TestSuite T;
  if (argc < 2 || std::string("parser") == argv[1])
    T.add_suite(coral::tests::run_parser_tests());
  if (argc < 2 || std::string("codegen") == argv[1])
    T.add_suite(coral::tests::run_codegen_tests());
  T.show(0);
  return T.getTotal() - T.getSuccess();
}
