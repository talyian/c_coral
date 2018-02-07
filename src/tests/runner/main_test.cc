#include "tests/runner/parser.hh"
#include "tests/runner/codegen.hh"
#include "tests/runner/base.hh"

int main() {
  coral::tests::TestSuite T;
  T.add_suite(coral::tests::run_parser_tests());
  T.add_suite(coral::tests::run_codegen_tests());
  T.show(0);
  return T.getTotal() - T.getSuccess();
}
