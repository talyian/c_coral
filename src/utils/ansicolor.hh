#pragma once
#define COL_BLACK "\033[30;1m"
#define COL_RED "\033[31m"
#define COL_GREEN "\033[32m"
#define COL_YELLOW "\033[33m"
#define COL_BLUE "\033[34m"
#define COL_MAGENTA "\033[35m"
#define COL_CYAN "\033[36m"
#define COL_WHITE "\033[37m"
#define COL_CLEAR "\033[0m"

#define COL_LIGHT_BLACK "\033[30;1m"
#define COL_LIGHT_RED "\033[31;1m"
#define COL_LIGHT_GREEN "\033[32;1m"
#define COL_LIGHT_YELLOW "\033[33;1m"
#define COL_LIGHT_BLUE "\033[34;1m"
#define COL_LIGHT_MAGENTA "\033[35;1m"
#define COL_LIGHT_CYAN "\033[36;1m"
#define COL_LIGHT_WHITE "\033[37;1m"
#define COL_LIGHT_CLEAR "\033[0;1m"

#include <iostream>
#include <iomanip>
template <int R, int G, int B>
class COL_216_m { public:  static const int val = (R * 6 + G) * 6 + B + 16; };
template <int R, int G, int B>
std::ostream& operator<< (std::ostream& out, COL_216_m<R, G, B>&& m) {
  return (out << "\033[38;5;" << m.val << "m");
}
#define COL_RGB(r, g, b) COL_216_m<r, g, b>()

class fill { public: char s; fill(char s) : s(s) {} };
std::ostream& operator<< (std::ostream &out, fill &&f);
