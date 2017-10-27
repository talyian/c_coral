
#include "../../parsing/lexer.hh"
#include "../../core/treeprinter.hh"

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;
using namespace coral;

string moduleToString(Module * m) {
  stringstream ss;
  TreePrinter(m, ss).print();
  return ss.str();
}

template <typename T>
int check_eq(string name, T s, T s2) {
  if (s == s2) {
    cout << name << ": OK\n";
    return 1;
  } else {
    cout << name << ": Error\n";
    cout << "------------------------------\n";
    cout << s << endl;
    cout << "------------------------------\n";
    cout << s2 << endl;
    cout << "------------------------------\n";
    return 0;
  }
}

#define ASSERT_EQ(a, b) check_eq(name, a, b)

#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
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

int checkString(string name, string src) {
  auto s = fork_run<const char *>(
    [] (const char * cs) { cout << moduleToString(parse(0, cs)); },
    src.c_str());
  return ASSERT_EQ(s, src);
}

int checkFile(string path) {
  ifstream filestream(path);
  stringstream filebuf;
  filebuf << filestream.rdbuf();
  auto s = fork_run<string>(
    [] (string s) { cout << moduleToString(parse(0, s.c_str())); },
    filebuf.str());
  return checkString(path, s);
}

int main() {
  cout << "----------[ Parsing Tests ]----------\n";
  checkString("let", "let x = 1\n");
  // checkFile("tests/enums.2.coral");
  // checkFile("tests/enums.coral");
  checkFile("tests/fizzbuzz.coral");
  checkFile("tests/hello_world.coral");
  checkFile("tests/if.coral");
  // checkFile("tests/returns.coral");
  // checkFile("tests/scope.coral");
  // checkFile("tests/string.coral");
  checkFile("tests/tuple.coral");
}
