#include <iostream>
#include <iomanip>
#include "utils/ansicolor.hh"

std::ostream& operator<< (std::ostream &out, fill &&f) {
  auto w = out.width();
  out << std::setw(0);
  for(int i=0; i< w; i++) out << f.s;
  return out;
}
