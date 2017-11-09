/*
  Stuff used across the code base
 */
#pragma once

#include <string>
#include <vector>
#include <algorithm>

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

template <typename T>
T id(T val) { return val; }

template<typename T, typename U>
std::vector<U> vmap(std::vector<T> in, U (*f)(T t)) {
  std::vector<U> res(in.size());
  std::transform(in.begin(), in.end(), res.begin(), f);
  return res;
}

template<typename T>
void vec_erase(std::vector<T> &vec, T val) {
  vec.erase(std::remove(vec.begin(), vec.end(), val), vec.end());
}
