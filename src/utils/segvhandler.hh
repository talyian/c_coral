#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <unistd.h>

void segvhandler(int) {
  void * array[30];
  size_t size = backtrace(array, 30);
  char * funcname = new char[256];
  size_t funcsize = 256;
  char ** symbollist = backtrace_symbols(array, size);
  for(size_t i =0; i<size; i++) {
    auto mangled = symbollist[i];
    // char * begin_name = 0, *end = 0, *lbracket = 0, *rbracket = 0;
    // for(char * p = mangled; *p; p++) {
    //   if (*p == '(') begin_name = p;
    //   else if (*p == ')' && begin_name) end = p;
    //   else if (*p == '[' && end) lbracket = p;
    //   else if (*p == ']' && lbracket) rbracket = p;
    // }
    // *begin_name++ = 0;
    // *end = 0;
    // lbracket++;
    // *rbracket = 0;
    // char command[1024];
    // snprintf(command, 1024, "/usr/bin/addr2line -e %s %s", mangled, lbracket);
    // if (pit_fork()) {

    // } else {

    // }
    // char funcname[1024];
    // size_t funcsize = 1024;
    // int status = 0;
    // abi::__cxa_demangle(begin_name, funcname, &funcsize, &status);
    // printf("[%s] %s\n", mangled, funcname);
    // continue;
    char * begin = 0, *offset = 0, *end = 0;
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
      printf("[\033[38;5;98m%s\033[0m]\n%s +%s%s\n", mangled, ret, offset, end);
    } else {
      printf("[%s] : %s + %s + %s\n", mangled, begin, offset, end);
    }
  }
  exit(1);
}
