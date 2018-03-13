#pragma once

#include <vector>
#include <iostream>
#include <cstring>
#include <memory>

namespace coral {
  namespace tests {
	class TestSuite {
	  std::vector<std::unique_ptr<TestSuite>> subsuites;
	public:
	  int success = 0, total = 0;
	  int getTotal() {
		int t = total;
		for(auto && suite : subsuites) t += suite->getTotal();
		return t; }
	  int getSuccess() {
		int t = success;
		for(auto && suite : subsuites) t += suite->getSuccess();
		return t; }
	  int getFailure() { return getTotal() - getSuccess(); }

	  virtual const char * getName() { return "Tests"; }
	  void add_suite(TestSuite * tt) { subsuites.push_back(std::unique_ptr<TestSuite>(tt)); }
	  void show_header() { if (0) std::cout << "-------------------------------------------------------\n"; }
	  void show() { show(10); }
	  void show(int depth) {
		if (depth > 0)
		  for(auto && subsuite : subsuites)
			subsuite->show(depth - 1);
		int success = getSuccess();
		int total = getTotal();
		if (total) {
		  std::cout << "-[" << getName() << "]-";
		  for(int i=strlen(getName()); i< 20; i++) std::cout << '-';
		  std::cout << success << " / " << total
					<< " (" << (success * 100 / total) << "%)"
					<< "--------------------\n";
		}
	  }

      std::string currentTest;
      int failCounter = 0;
      void BeginTest(std::string testName) {
        currentTest = testName;
        total++;
        failCounter = 0;
      }
      void EndTest() {
        if (!failCounter) {
          printf("%-60s OK\n", currentTest.c_str());
          success++;
        } else {
          printf("%-60s ERROR\n", currentTest.c_str());
          failCounter = 0;
        }
      }
      void Assert(bool val) { if (!val) failCounter++; }

	};
  }
}
