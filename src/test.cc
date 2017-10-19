
#include <iostream>
#include <unistd.h>
#include <cstring>

using namespace std;

typedef void TestResult;

TestResult test_empty() {
  cout << "test";
}

TestResult test_hello_world () {
  cout << "foo";
}

TestResult test_fizzbuzz() {
  cout << "bar";
}

void _test_run(TestResult (*test)(), const char * name, const char * expected) {
  int pipes[2];
  if (pipe(pipes) == -1) { perror("pipe"); exit(1); }

  pid_t pid = fork();
  if (pid == -1) { perror("fork"); exit(2); }
  else if (pid == 0) {
    dup2(pipes[1], 1);
    close(pipes[1]);
    close(pipes[0]);
    test();
    exit(0);
  } else {
    char buf[1024];
    buf[read(pipes[0], buf, 300)] = 0;
    if (strcmp(expected, buf) == 0) { cout << name << " \e]0m;OK"; }
    else {
      cout << name << ": Mismatch!\n";
      cout << "Expected: [" << expected << "]\n";
      cout << "Actual:   [" << buf << "]\n";
    }
  }
}

#define TestRun(TESTNAME, expected) _test_run(test_##TESTNAME, #TESTNAME, expected)

int main() {
  cout << "---------- [Starting Test Run] ----------\n";
  TestRun(empty, "test");
  TestRun(hello_world, "Hello, World!");
  TestRun(fizzbuzz, "1 2 fizz 4 buzz fizz 7 8 fizz buzz 11 fizz 13 14 fizzbuzz 16");
}
