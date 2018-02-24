#include "analyzers/typegraph/constraint.hh"

template<class R, class T>
TypeEqualityRight<R, T> * makeTQR(R r, T * o) { return new TypeEqualityRight<R, T>(r, o); }
