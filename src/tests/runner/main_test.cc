#include "tests/runner/parser.hh"
#include "tests/runner/codegen.hh"
#include "tests/runner/base.hh"

#include <signal.h>
#include <execinfo.h> // backtrace
void handleSIGSEGV(int sig) {
  const int SIZE = 20;
  void * bt[20];
  backtrace((void **)&bt, SIZE);
  printf("segv handler\n");
}

int main() {
  // qsignal(SIGSEGV, handleSIGSEGV);
  coral::tests::TestSuite T;
  T.add_suite(coral::tests::run_parser_tests());
  T.add_suite(coral::tests::run_codegen_tests());
  T.show(0);
  return T.getTotal() - T.getSuccess();
}
