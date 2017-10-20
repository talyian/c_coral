#include <iostream>
#include <iomanip>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include "ast.hh"
#include "parser.hh"
#include "lexer.hh"
#include "compiler.hh"

typedef void TestRun;
void _test_run(TestRun (*test)(), const char * name, const char * expected) {
  int pipes[2];
  if (pipe2(pipes, O_NONBLOCK) == -1) { perror("pipe"); exit(1); }

  pid_t pid = fork();
  if (pid == -1) { perror("fork"); exit(2); }
  else if (pid == 0) {
    dup2(pipes[1], 1);
    close(0);
    close(pipes[1]);
    close(pipes[0]);
    test();
    exit(0);
  } else {
    char buf[1024];
    int status;
    waitpid(pid, &status, 0);
    buf[0] = 0;
    buf[read(pipes[0], buf, 1023)] = 0;
    if (strcmp(expected, buf) == 0) { cout << left << setw(20) << name << " \e[1;32mOK\e[0m" << endl; }
    else {
      cout << setw(20) << name << "\e[1;31m Err \e[0m\n";
      cout << "Expected: [" << expected << "]\n";
      cout << "Actual:   [" << buf << "]\n";
      if (status) {
	cout << "Child exited with code " << status << "\n";
      }
    }
  }
}

#define runTest(TESTNAME) _test_run(run_##TESTNAME, #TESTNAME, expected_##TESTNAME)
#define DefTest(TESTNAME, expected) const char * expected_##TESTNAME=expected; auto run_##TESTNAME = [] ()
using std::vector;
using std::cout;

DefTest(empty, "test") {
  cout << "test";
};

DefTest(hello_world, "Hello, World!") {
  CoralCompiler cc;
  cc.load(CoralModule(fopen("tests/hello_world.coral", "r")));
  cc.run();
};

DefTest(fizzbuzz, "1 2 fizz 4 buzz fizz 7 8 fizz buzz 11 fizz 13 14 fizzbuzz 16") {
  CoralModule cm(fopen("tests/fizzbuzz.coral", "r"));
  CoralCompiler cc; cc.load(cm); cc.run();
};

int main() {
  cout << "---------- [Starting Test Run] ----------\n";
  runTest(empty);
  runTest(hello_world);
  runTest(fizzbuzz);
}