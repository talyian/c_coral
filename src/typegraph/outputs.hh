#include "constraints.hh"

// these are all the output constraints we allow
// in addition to the standard "Type" node.

namespace typegraph {

  // An Index is used to to write out a Field index for a member access
  // struct foo = {a:x, b:y} ;; The codegen backend needs to know that foo.a
  // is at index 0
  class Index : public Constraint {

  };
  // A FuncTerm points to a term which has a function AST node in it.
  // This is used when the type checker is used
  // to resolve an overloaded function
  class FuncTerm : public Constraint {

  };

  // A MethodCall solution

}
