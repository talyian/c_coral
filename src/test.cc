#include <iostream>
#include <fstream>
#include <regex>
#include <sstream>
#include <iomanip>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include "../obj/ast.hh"
#include "../obj/parser.hh"
#include "../obj/lexer.hh"
#include "compiler.hh"

int success = 0, failure = 0;

class TestCase {
public:
  string name;
  string expected;
  TestCase() { }
};

// forks and runs a function with a given parameter
// returns its stdout as a string
template <typename T>
string fork_run (void (*run_func)(T a), T args) {
  int pipes[2];
  if (pipe2(pipes, O_NONBLOCK) == -1) { perror("pipe"); exit(1); }
  pid_t pid = fork();
  if (pid == -1) { perror("fork"); exit(2); }
  else if (pid == 0) {
    dup2(pipes[1], 1);
    close(0);
    close(pipes[1]);
    close(pipes[0]);
    run_func(args);
    exit(0);
  } else {
    char buf[1024];
    int status;
    waitpid(pid, &status, 0);
    buf[0] = 0;
    buf[read(pipes[0], buf, 1023)] = 0;
    return string(buf);
  }
}

class FileTestCase : public TestCase {
public:
  string path;
  string source;
  string output;
  static regex expect_search;
  static regex s_escape;
  FileTestCase(string testName, string path) {
    name = testName;
    this->path = path;
    ifstream ff(path.c_str());
    stringstream buffer;
    buffer << ff.rdbuf();
    this->source = buffer.str();
    smatch m;
    if (regex_search(this->source, m, expect_search)) {
      string s = m[1];
      expected = regex_replace(s, s_escape, "\n");
    }
  }
  bool run() {
    output = fork_run<string>([] (string args) {
	CoralCompiler cc;
	cc.load(CoralModule(args.c_str()));
	cc.run();
    }, source);
    success += output == expected;
    failure += output != expected;
    return output == expected;
  }
  void runStandalone() {
    if (run()) printf("%-20s \e[1;32mOK\e[0m\n", name.c_str());
    else {
      printf("%-20s \e[1;31mMismatch\e[0m\n", name.c_str());
      printf("Expected: [%s]\n", expected.c_str());
      printf("Result:   [%s]\n", output.c_str());
    }
  }
};

regex FileTestCase::expect_search = regex("_expected = \"(.*)\"");
regex FileTestCase::s_escape = regex("\\\\n");

#define runTest(TESTNAME) _test_run(run_##TESTNAME, #TESTNAME, expected_##TESTNAME)
#define DefTest(TESTNAME, expected) void run_##TESTNAME()
#define runFileTest(TESTNAME, fname, _expected) do { \
    FileTestCase tc(#TESTNAME, fname); \
    if (_expected != "") tc.expected = _expected; \
    tc.runStandalone(); } while(0);
using std::vector;
using std::cout;

int main() {
  cout << "---------- [Starting Test Run] ----------\n";
  runFileTest(hello_world, "tests/hello_world.coral", "");
  runFileTest(ifstatement, "tests/if.coral", "");
  runFileTest(fizzbuzz, "tests/fizzbuzz.coral", "");
  runFileTest(returns, "tests/returns.coral", "101\n200\n201\n402\n");
  runFileTest(tuple, "tests/tuple.coral", "");
  // runFileTest(enums, "tests/enums.coral", "");
  // runFileTest(scope, "tests/scope.coral", "");
  printf("---------- [%d/%d] (%2.0f%%) ----------\n",
	 success , success + failure,
	 success * 100.0 / (success + failure));
}
