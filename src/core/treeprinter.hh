#pragma once

#include "expr.hh"
#include <iostream>
#include <string>

using std::string;
using std::cerr;
using std::endl;

namespace coral {
  /// TreePrinter should return a round-trippable source string from a module
  /// module == parse(printTree(module))
  class TreePrinter : public Visitor {
  public:
    Module * module;
    std::ostream & out;
    // current indentation level.
    int indent = 0;
    // if 1, inline mode (don't indent or newline)
    int line_mode = 0;
    // stupid hack for render elifs from nested ifs
    int showElif = 0;
    // stupid hack to detect where to show parens
    BinOp * curop = 0;

    TreePrinter(Module * m, std::ostream & c) : Visitor("treeprint "), module(m), out(c) { }
    void print();
    std::string IND();
    std::string END();

#define VISIT_DEF(EXPR) void visit(EXPR * e);
    void visit(BaseDef * d);
    EXPR_NODE_LIST(VISIT_DEF)
#undef VISIT_DEF
  };
}
