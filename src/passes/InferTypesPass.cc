#include "../core/type.hh"
#include "../core/expr.hh"
#include "../core/treeprinter.hh"
#include "../parsing/lexer.hh"

#include <map>
#include <string>
#include <sstream>

using namespace std;
using namespace coral;

Type * coral::ExprNotes::getBestType() { return types.empty() ? new UnknownType() : types[0]; }
void coral::ExprNotes::add(std::string msg) { messages.push_back(msg); }
void coral::ExprNotes::isType(std::string msg) { messages.push_back(" :: " + msg); }
void coral::ExprNotes::isType(Type * t) { messages.push_back(" :: " + t->toString()); }
void coral::ExprNotes::isTypeOf(std::string msg) { messages.push_back(" :: typeof(" + msg + ")"); }
void coral::ExprNotes::isTypeOf(Expr * e) { messages.push_back(" :: typeof(" + e->toString() + ")"); }
void coral::ExprNotes::returns(std::string msg) { messages.push_back(" :: returns(" + msg + ")"); }
void coral::ExprNotes::merge(coral::ExprNotes & e) {
  foreach(e.messages, it) messages.push_back(*it);
}
void coral::ExprNotes::mergeReturn(coral::ExprNotes & e) {
  std::string s;
  foreach(e.messages, it) messages.push_back(" :: Func[..., " + (*it) + "]");
}

class ScopeInfo {
public:
  Expr * expr;
  Type * type = new UnknownType();
  ScopeInfo(Expr * e) : expr(e) { }
  void setType(Type * t) { type = t; }
};

class Scope {
  map<Expr *, unique_ptr<ScopeInfo>> info;
  map<std::string, Expr *> names;
  Scope * parent;
public:
  Scope(Scope * parent) : parent(parent) { }
  Scope() : parent(0) { }

  void show(std::ostream & out) {
	out << "----------------------------------------\n";
	for(auto & it : info) {
	  auto expr = it.first;
	  auto ptr = it.second.get();
	  out << "show " << expr->toString()
		  << " :: " << ptr->type->toString()
		  << "\n";
	}
  }

  ScopeInfo * getByName(string name) {
	auto e = names.find(name);
	if (e == names.end()) {
	  if (parent == 0) {
		cerr << "name not found: " << name << "\n";
		return 0;
	  }
	  return parent->getByName(name);
	}
	auto f = info.find(e->second);
	if (f == info.end()) {
	  cerr << "Name found but expr missing: " << name << "\n";
	  if (parent == 0) return 0;
	  return parent->getByName(name);
	}
	return f->second.get();
  }

  ScopeInfo * get(Expr * e) {
	auto x = info.find(e);
	if (x == info.end()) {
	  if (parent != 0)
		return parent->get(e);
	  else
		return 0;
	}
	else return x->second.get();

  }

  void name(Expr * e, string name) {
	names[name] = e;
  }

  ScopeInfo * put(Expr * e) {
	info[e] = unique_ptr<ScopeInfo>(new ScopeInfo(e));
	return info[e].get();
  }

  Scope * nested() {
	return new Scope(this);
  }
};


class ExprToType : public Visitor {
public:
  Scope * scope;
  Type * out = 0;
  Type * in = new UnknownType();
  ExprToType(Expr * e, Scope * s, Type * in) : Visitor("exprt "), scope(s), in(in) { if (e) e->accept(this); }
  ExprToType(Expr * e, Scope * s) : Visitor("exprt "), scope(s) { if (e) e->accept(this); }

  void visit(Extern * e) {
	out = e->type;
	scope->put(e)->setType(e->type);
	scope->name(e, e->name);
  }

  void visit(Var * e) {
	if (!scope) {
	  cerr << "no scope!??\n";
	}
	else {
	  auto scopeinfo = scope->getByName(e->value);
	  if (!scopeinfo) {
		cerr << "not found: " << e->value << "\n";
	  }
	  else {
		out = scopeinfo->type;
		if (getTypeKind(out) == UnknownTypeKind && getTypeKind(in) != UnknownTypeKind) {
		  // cerr << "writing argument type " << e->value << " :: " << in->toString() << "\n";
		  out = in;
		  scopeinfo->setType(in);
		}
	  }
	}
  }

  void visit(Long * e) {
	out = new IntType(32);
  }

  void visit(String * e) {
	out = new PtrType(new IntType(8));
  }

  void visit(Call * e) {
	// if the calleetype is a functype,
	// the type of the call is the calleetype->ret;
	e->callee->accept(this);
	auto calleeType = out;
	if (calleeType && getTypeKind(calleeType) == FuncTypeKind) {
	  auto ftype = (FuncType *)calleeType;
	  if (ftype->args.size() != e->arguments.size()) {
		// TODO: handle variadics or raise error
	  } else for(int i=0; i<ftype->args.size(); i++)  {
		  auto argExpr = e->arguments[i].get();
		  in = ftype->args[i];
		  argExpr->accept(this);
		  // cerr << "\t\t\t" << i << ": " << ->toString() << "\n";
		// auto argtype = ExprToType(e->arguments[i].get(), scope, ftype->args[i]).out;
        // auto argtype = ExprToType(e->arguments[i].get(), scope, ftype->args[i]).out;
      }
	  out = ftype->ret;
	} else
	  out = new UnknownType();
  }

  void visit(BlockExpr * e) {
	for(auto & line : e->lines)
	  line->accept(this);
  }

  void visit(Tuple * e) {
	vector<Type *> types;
	for(auto &d : e->items) types.push_back(ExprToType(d, scope).out);
	out = new TupleType(types);
  }

  void visit(FuncDef * e) {
	ExprToType nested(0, scope->nested());

	vector<Type *>args;
	for(auto &arg : e->args) {
		ExprToType x(0, scope);
		x.visitDef(arg);
		args.push_back(x.out);
		nested.scope->put(arg)->setType(x.out);
		if (arg->kind == DefKind)
		  nested.scope->name(arg, ((Def *)arg)->name);

		// nested.scope->saveDef(arg);
	}

	e->body->accept(&nested);

	auto rettype = e->rettype;
	if (rettype == 0 || getTypeKind(rettype) == UnknownTypeKind) {
	  rettype = nested.out;
	}
	// nested.scope->show(cerr);
	int i = 0;
	for(auto &arg : e->args) {
	  args[i++] = nested.scope->get(arg)->type;
	}

	out = new FuncType(rettype, args, false);
	scope->put(e)->setType(out);
	scope->name(e, e->name);
	// scope->saveType(e->name, out);
  }

  void visitDef(BaseDef * e) {
	if (e->kind == TupleDefKind)
	  visitDef((TupleDef *)e);
	else
	  visitDef((Def *)e);
  }

  void visitDef(Def * e) {
	out = e->type;
  }

  void visitDef(TupleDef * e) {
	vector<Type *> types;
	for(auto &d : e->items) {
		ExprToType x(0, scope);
		x.visitDef(d);
		types.push_back(x.out);
	}
	out = new TupleType(types);
  }

};

class InferTypesPass : public Visitor {
public:
  Expr * expr;
  Expr * out;
  Scope * scope;
  InferTypesPass(Expr * e) : Visitor("infer "), out(e) {
	scope = new Scope();
	if (e) e->accept(this);
  }

  void visit(Module * m) {
	foreach(m->lines, it) (*it)->accept(this);
	out = m;
  }

  void visit(Extern * e) { e->notes.isType(ExprToType(e, scope).out); }

  void visit(FuncDef * e) {
	e->notes.isType(ExprToType(e, scope).out);
	return;
	vector<Type *> args;
	for(auto &def : e->args){
	  ExprToType e(0, scope);
	  e.visitDef(def);
	  args.push_back(e.out);
	}
	auto ftype = new FuncType(ExprToType(e->body, scope).out, args, false);
	e->notes.isType(ftype);
  }

  void visit(BlockExpr * e) {
	e->notes.isType(new UnknownType());
  }

  void visit(Call * c) {
	c->notes.isType(ExprToType(c, scope).out);
  }
};


void massert(string name, bool cond, string msg) {
  printf("%-20s: ", name.c_str());
  if (cond)
	cout << "\033[1;32m" << "OK" << "\033[0m ";
  else
	cout << "\033[1;31m" << "ERROR" << "\033[0m ";
  cout << msg << "\n";
}

void massert(bool cond) { return massert("(test)", cond, ""); }

void testInferTypes() {
  auto module = parse(0, R"-CORAL-(
extern "C" strlen : Fn[String, Int32]

let a = 1
let b = "test"

func f(x):
  x

func g(y, z):
  y * 2 + z

func h(s):
  strlen(s)

func y(f, x):
  f (f x)

func z1 : Int32():
  1

func z2 : Int32(x: Int64):
  1
)-CORAL-");

  module = parse(0, R"RRR(
extern "C" f : Func[Int8, Int16]
func e : Void (a : Int32):
  ()
func g(a, b):
  f a
func h(a):
  func i(b):
     g(a, b)
  i 3

g(3, 4)
)RRR");

  TreePrinter(module, cout).print();
  module = (Module *)InferTypesPass(module).out;
  cout << "\n\n\n";
  TreePrinter(module, cout).print();
}
