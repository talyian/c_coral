#pragma once

#include "../typegraph.hh"
#include <iostream>
#include <iomanip>

class TestCase {
public:
  std::string name = "Basic Test";
  int count = 0;
  int success = 0;

  void Assert(bool cond, std::string message) {
  success += cond;
  count++;
  if (message.size())
    printf(
    "\033[1;39m%-60s%s\n",
      message.c_str(),
      cond ? "\033[32mOK\033[0m" : "\033[31mERROR\033[0m");
  }

  void summary() {
  printf("----------------------------------------------------------------------\n");
  printf("%s: %d/%d (%d%%) passed\n",
           name.c_str(), success, count, success * 100 / (count ? count : 1));
  }
  virtual ~TestCase() { }
};
