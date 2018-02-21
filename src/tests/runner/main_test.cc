#include "tests/runner/parser.hh"
#include "tests/runner/codegen.hh"
#include "tests/runner/base.hh"

#include <array>

int main(int argc, const char ** argv) {
  coral::tests::TestSuite T;
  if (argc < 2 || std::string("parser") == argv[1])
    T.add_suite(coral::tests::run_parser_tests());
  if (argc < 2 || std::string("codegen") == argv[1])
  T.add_suite(coral::tests::run_codegen_tests());
  T.show(0);
  return T.getTotal() - T.getSuccess();
}
