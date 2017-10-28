/*
  Stuff used across the code base
 */
#pragma once

#include <string>
#include <vector>

#define foreach(_COLLECTION, _IT) \
  for(auto _IT = _COLLECTION.begin(); _IT != _COLLECTION.end(); _IT++)

template <typename T>
std::string join(const char * sep, std::vector<T> items, std::string (*f)(T t)) {
  std::string s = "";
  foreach(items, it) {
    if (it != items.begin()) s += sep;
    s += f(*it);
  }
  return s;
}
