#include "ast.hh"
#include "typeScope.hh"
#include <iostream>
#include <string>
#include <vector>

using std::vector; using std::string; using std::cerr; using std::endl;


Module * inferTypes(Module * m);

/*
"
Inferring type goes in multiple steps:

extern 'C' printf : Fn[Str, ..., Void]


let a = printf 'hi'
func f(b):
  printf b

in the first pass,
printf -> Fn[Str, ..., Void]
a -> call (callee=(unknown typeof printf), args=string)
f -> funcdef(
   args=(unknown typeof b)
"
*/

class TypeInferer : public Visitor {
  string name;
  Type * value, * in_value;
  Scope scope;
  TypeInferer() { name = ""; value = 0; in_value = 0; }
public:
  TypeInferer(Module * module) {
    in_value = 0;
    value = 0;
    module->accept(this);
    scope.show();
    cerr << "----------------------------------------\n";
  }
  void visit(Module * m) {
    foreach(m->lines, it) {
      value = 0;
      (*it)->accept(this);
    }
  }

  void visit(DeclTypeEnum * e) {
    cout << e->name << endl;
    scope.add(e->name, new Type());
    foreach(e->body, case_iter) {
      in_value = new UserType(e->name);
      (*case_iter)->accept(this);
    }
    value = 0;
  }

  void visit(EnumCase * c) {
    cout << "visiting " << c->name << " : " << in_value << endl;
    scope.add(c->name, in_value);
  }

  void visit(MatchExpr * e) {
    foreach(e->cases, it) {
      (*it)->accept(this);
    }
  }

  void visit(MatchCaseTagsExpr * e) {
    e->body->accept(this);
  }

  void visit(Extern * e) {
    scope.add(e->name, e->type);
  }

  void visit(FuncDef * f) {
    // push a new scope with the function's arguments in it
    TypeInferer func_scope(*this);
    vector<Type *> argtypes;
    foreach(f->args, def) {
      argtypes.push_back((*def)->type);
      func_scope.scope.add((*def)->name, (*def)->type);
    }
    // A function's type is the type of its body in the context of its argument types
    f->body->accept(&func_scope);
    // TODO: fix the issue when a local argument shadows a parameter
    for(int i=0; i<argtypes.size(); i++) {
      argtypes[i] = func_scope.scope.get(f->args[i]->name);
    }
    if (!f->rettype) f->rettype = func_scope.value;
    auto ftype = new FuncType(f->rettype, argtypes, false);
    scope.add(f->name, ftype);
  }

  void visit(Var * c) {
    value = scope.get(c->value);
    if (value && value->toString() == "Unknown" && in_value) {
      value = in_value;
      scope.set(c->value, value);
    }
  }
  void visit(Long * c) { value = new IntType(64); }
  void visit(Let * l) {
    value = l->var->type;
    if (!value) l->value->accept(this);
    scope.add(l->var->name, value);
  }

  void visit(String * s) { value = new PtrType(new IntType(8)); }
  void visit(Call * c) {
    auto functype = scope.get(getName(c->callee));
    Type * known_ret_value = 0;
    vector<Type *> params;
    if (getTypeName(functype) == "Func") {
      auto f = (FuncType *)functype;
      if (f->ret) {
	known_ret_value = f->ret;
      }
      foreach(f->args, t) params.push_back((*t));
    }

    vector<Type*> argtypes;
    int i = 0;
    foreach(c->arguments, arg) {
      value = 0;
      in_value = i < params.size() ? params[i] : 0;
      (*arg)->accept(this);
      argtypes.push_back(value);
      // cout << c->callee << "----------" << endl;
      // cout << "argtype " << i << " " << value << endl;
      // cout << "paramtype " << i << " "
      // 	   << (params.size() > i ? params[i]->toString() : "(short)") << endl;
      i++;
    }
    // cout << this->in_value << " = call: " << c->callee << '(';
    // foreach(argtypes, t) { cout << (t == argtypes.begin() ? "" : ", ") << *t; }
    // cout << ')' << endl;
    value = known_ret_value;
  }

  void visit(Cast * c) {
    value = c->to_type;
  }
  void visit(BlockExpr * e) {
    foreach(e->lines, iter) {
      // TODO: non-terminating expressions should be of type void
      value = 0;
      (*iter)->accept(this);
    }
  }

  void visit(BinOp * op) {
    op->lhs->accept(this);
    auto ltype = value;
    op->rhs->accept(this);
    auto rtype = value;

    if (getTypeName(ltype) == "Unknown") {
      cout << "unknown left type \n";
    } else if (getTypeName(rtype) == "Unknown") {
      cout << "unknown right type \n";
    } else if (!typeEquals(ltype, rtype)) {
      cout << "mismatched types: " << ltype << ", " << rtype << "\n";
    }
    // TODO: this isn't exactly kosher
    value = ltype;
  }
};

Module * inferTypes(Module * m) {
  TypeInferer visitor(m);
  return m;
}

void testInference() {
  // [Var] Rule
  // type(Scope("x", new IntType(8)), new Var("x")) -> new IntType(8)
  // [Call] Rule
  // type(Scope("x", new IntType(8), "f", new FuncType(new IntType(32), vec(new IntType(32)))),
  //      new Call("f", new Var("x"))) -> new IntType(32)
}
