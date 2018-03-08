#include <memory>

#ifndef __cpp_lib_make_unique
namespace std {
  template <typename T>
  std::unique_ptr<T> make_unique(T * rawptr) { return std::unique_ptr<T>(rawptr); }
}
#endif
