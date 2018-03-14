#include <cstdio>
#include "codegen/codegen.hh"
#include "utils/opts.hh"

#ifdef __linux__
#include <signal.h>
#include <execinfo.h>
#include <cxxabi.h>
void segvhandler(int sig) {
  printf("%d = OOPS\n", sig);
  void * array[30];
  size_t size = backtrace(array, 30);
  char * funcname = new char[256];
  size_t funcsize = 256;
  char ** symbollist = backtrace_symbols(array, size);
  for(size_t i =0; i<size; i++) {
    auto mangled = symbollist[i];
    char * begin = 0, *end = 0, *offset = 0;
    for(char * p = mangled; *p; p++) {
      if (*p == '(') begin = p;
      else if (*p == '+' && begin) offset = p;
      else if (*p == ')' && begin) end = p;
    }
    *begin++ = 0;
    *offset++ = 0;
    *end++ = 0;
    int status;
    char * ret  =abi::__cxa_demangle(begin, funcname, &funcsize, &status);
    if (!status) {
      funcname = ret;
      printf("[%s] : %s + %s + %s\n", mangled, ret, offset, end);
    } else {
      printf("[%s] : %s + %s + %s\n", mangled, begin, offset, end);
    }
  }
  exit(1);
}
#endif

int main(int argc, char ** argv) {
  #if __linux__
  signal(SIGSEGV, handler);
  #endif

  coral::opt::initOpts();
  if (argc > 1) { coral::Run(argv[1]); return 0; }
  coral::Run("tests/cases/simple/hello_world.coral");
}
