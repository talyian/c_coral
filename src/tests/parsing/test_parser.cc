#include "../../parsing/lexer.hh"
#include "../../core/treeprinter.hh"

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

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
    cout << setw(30) << name << ": \e[1;32mOK\e[0m " << s.length() << endl;
    return 1;
  } else if (s.compare(0, 12, "\"parse error") == 0) {
    cout << setw(30) << name << ": \e[1;31mParse Error\e[0m " << endl;
    return 0;
  } else {

    cout << setw(30)  << name << ": \e[1;31mError\e[m\n";
    const char * z1 = s.c_str();
    const char * z2 = s2.c_str();

    int i = 0, j = 0;
    while(*z1 && *z2 && (*z1 == *z2)) {
      if (*z1 == '\n') j = i;
      i++; z1++; z2++;
    }
    cout << i << ", " << j <<  "v-----------------------------\n";
    cout << z1 + (j - i) << endl;
    cout << "------------------------------\n";
    cout << z2 + (j - i) << endl;
    cout << "^-----------------------------\n";
    return 0;
  }
}

int total = 0;
int passed = 0;
int temp = 0;

void skipFile(string name) {
  cout << setw(30)  << name << ": \e[1;33mSkip\e[0m\n";
}

#define ASSERT_EQ(a, b) (temp = check_eq(name, a, b), total++, passed += temp, temp)

#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
// forks and runs a function with a given parameter
// returns its stdout as a string
template <typename T>
string fork_run (void (*run_func)(T a), T args) {
  int stdout_pipes[2];
  int stderr_pipes[2];
  if (pipe2(stdout_pipes, O_NONBLOCK) == -1) { perror("pipe"); exit(1); }
  if (pipe2(stderr_pipes, O_NONBLOCK) == -1) { perror("pipe"); exit(1); }
  pid_t pid = fork();
  if (pid == -1) { perror("fork"); exit(2); }
  else if (pid == 0) {
    dup2(stdout_pipes[1], 1);
    dup2(stderr_pipes[1], 2);
    close(0);
    close(stdout_pipes[1]);
    close(stdout_pipes[0]);
    close(stderr_pipes[1]);
    close(stderr_pipes[0]);
    run_func(args);
    exit(0);
  } else {
    char buf[1024 * 32];
    int status;
    waitpid(pid, &status, 0);
    buf[0] = 0;
    int len = read(stdout_pipes[0], buf, 1024 * 32 - 1);
    int len2 = read(stderr_pipes[0], buf + len, 1023 - len);
    if (len2 < 0) len2 = 0;
    buf[len + len2] = 0;
    return string(buf);
  }
}

int checkString(string name, string src, string expected) {
  static std::streambuf* p = std::cerr.rdbuf();
  std::cerr.rdbuf(nullptr);
  auto m = parse(0, src.c_str());
  auto s = moduleToString(m);
  std::cerr.rdbuf(p);
  return ASSERT_EQ(s, expected);
}

int checkString(string name, string src) {
  // auto s = fork_run<const char *>(
  //   [] (const char * cs) { cout << moduleToString(parse(0, cs)); },
  //   src.c_str());
  auto m = parse(0, src.c_str());
  auto s = moduleToString(m);
  return ASSERT_EQ(s, src);
}

int checkFile(string path) {
  ifstream filestream("tests/" + path);
  stringstream filebuf;
  filebuf << filestream.rdbuf();
  auto s = fork_run<const char *>(
    [] (const char * cs) { cout << moduleToString(parse(0, cs)); },
    filebuf.str().c_str());
  // auto s = moduleToString(parse(0, filebuf.str().c_str()));
  auto s2 = moduleToString(parse(0, s.c_str()));
  string name = path;
  return ASSERT_EQ(s, s2);
}

void runParsingTests() {
  cout << "----------[ Parsing Tests ]----------\n";
  // checkString("let", "let x = 1\n");
  // checkString("tuple-destructuring-1", "let (a, b) = x\n");
  // checkString("tuple-destructuring-2", "let a, b = x\n", "\"parse error 2\"\n");
  // checkString("tuple-destructuring-3", "let a = x, y\n", "\"parse error 2\"\n");
  // checkString("tuple-destructuring-4", "let a = (x, y)\n");

  // checkFile("core/enums.2.coral");
  // checkFile("core/enums.coral");
  // checkFile("core/fizzbuzz.coral");
  // checkFile("core/hello_world.coral");
  // checkFile("core/if.coral");
  // checkFile("core/newlines.coral");
  // checkFile("core/precedence.coral");
  // checkFile("core/returns.coral");
  // checkFile("core/scope.coral");
  // checkFile("core/string.coral");
  // checkFile("core/tuple.coral");
  // checkFile("shootout/fasta.coral");
  // checkFile("shootout/knucleotide.coral");
  // checkFile("shootout/pidigits.coral");
  // checkFile("shootout/regexredux.coral");
  checkFile("libs/syncio.coral");
  cout << "----------[ "<< passed <<" / "<< total <<" ]----------\n";
}
