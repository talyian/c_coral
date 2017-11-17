#include "../core/type.hh"
#include "../core/expr.hh"
#include "../core/treeprinter.hh"
#include "../parsing/lexer.hh"

#include <map>
#include <string>
#include <sstream>

using namespace std;
using namespace coral;

void coral::ExprNotes::isType(Type * t) { messages.push_back(" :: " + (t ? t->toString() : "unknown")); }

class ScopeInfo {
public:
  Expr * expr;
  Type * type = new UnknownType();
  ScopeInfo(Expr * e) : expr(e) { }
  void setType(Type * t) { type = t; }
};

class Scope {
public:
  map<Expr *, unique_ptr<ScopeInfo>> info;
  map<std::string, Expr *> names;
  map<std::string, Scope *> children;
  Scope * parent;

  Scope(Scope * parent) : parent(parent) { }
  Scope() : parent(0) { }

  void show(std::ostream & out) {  show(out, ""); }
  void show(std::ostream & out, string prefix) {
	for(auto & it : names) {
	  auto name = it.first;
	  // auto expr = it.second;
	  auto ptr = info[it.second].get();
	  out << "\033[1;36m[show] " << prefix << name
		  << " :: " << ptr->type->toString()
		  << "\n\033[0m";
	}
	for(auto & it : children) {
	  auto name = it.first;
	  auto child = it.second;
	  child->show(out, prefix + name + ".");
	}
  }

  string getName (Expr * e) {
	for(auto pair : names) {
	  if (pair.second == e)
		return pair.first;
	}
	return "";
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

  Scope * nested(string name) {
	auto child = new Scope(this);
	this->children[name] = child;
	return child;
  }
};


class ExprToType : public Visitor {
public:
  Scope * scope;
  Type * out = 0;
  Type * in = new UnknownType();
  ExprToType(Expr * e, Scope * s, Type * in) : Visitor("inferexpr "), scope(s), in(in) { if (e) e->accept(this); }
  ExprToType(Expr * e, Scope * s) : Visitor("exprt "), scope(s) { if (e) e->accept(this); }

  void visit(Set * e) {
	e->var->accept(this);
	e->value->accept(this);
	out = new VoidType();
  }
  void visit(Return * e) { e->value->accept(this); }
  void visit(VoidExpr * e) { out = new VoidType(); }
  void visit(Extern * e) {
	// std::cerr << "EXTERN " << e->name << "\n";
	out = e->type;
	scope->put(e)->setType(e->type);
	scope->name(e, e->name);
  }

  void visit(Let * e) {
	if (e->value) e->value->accept(this);
	// cerr << "let :: " << e->var->toString() << endl;
	if (e->var->kind == TupleDefKind) {
	  // TODO
	  cerr << "TODO\n";
	} else {
	  auto var = e->var;
	  auto name = var->name;
	  out = var->type;
	  if (getTypeKind(out) == UnknownTypeKind) {
		e->value->accept(this);
		if (out) var->type = out;
	  }
	  scope->put(e)->setType(out);
	  scope->name(e, name);
	}
  }

  void visit(Var * e) {
	auto scopeinfo = scope->getByName(e->name);
	if (!scopeinfo) {
	  cerr << "not found: " << e->name << "\n";
	}
	else {
	  auto scopeInfo = scope->getByName(e->name);
	  e->ref = scope->getByName(e->name)->expr;
	  // cerr << "[Var] :" << e << " " << e->name << " :: " << e->ref << "\n";
	  // std::cerr << "REF " << e->name << " : " << (e->ref ? e->ref->toString() : "(null)") << "\n\n";
	  out = scopeinfo->type;
	  if (getTypeKind(out) == UnknownTypeKind && getTypeKind(in) != UnknownTypeKind) {
		cerr << "writing argument type " << e->name << " :: " << in->toString() << "\n";
		out = in;
		scopeinfo->setType(in);
	  }
	}
  }

  void visit(If * e) {
	e->cond->accept(this);
	e->ifbody->accept(this);
	auto type1 = out;
	e->elsebody->accept(this);
	auto type2 = out;
	if (!type2) out = type1;
	// TODO: check that type1 and type2 are compatible
  }

  void visit(BinOp * e) {
	e->lhs->accept(this);
	e->rhs->accept(this);
  }
  void visit(Long * e) {
	out = new IntType(32);
  }

  void visit(String * e) {
	out = new PtrType(new IntType(8));
  }

  void visit(Member * e) {
	// this is going to be quite similar to the Call logic.
	auto base = e->base;
	auto member = e->memberName;
	cerr << "member : " << base << ".." << member << endl;
	base->accept(this);
	auto basetype = out;
	cerr << "basetype : " << basetype << endl;
	// TODO: mess
	if (getTypeKind(basetype) == UserTypeKind) {
	  auto userType = (UserType *)basetype;
	  auto c = scope->children.find(userType->name);
	  if (c != scope->children.end()) {
		auto info = c->second->getByName(member);
		out = info->type;
		return;
	  } else {
		cerr << "not found " << userType->name << " :: " << member << "\n";
	  }
	  return;
	}
	auto info = scope->get(base);
	if (info) {
	  cerr << "scopeInfo: " << info << endl;
	} else {
	  cerr << "no info\n";
	}
  }

  void visit(Call * e) {
	// if the calleetype is a functype,
	// the type of the call is the calleetype->ret;
	auto calleeType = ExprToType(e->callee.get(), scope).out;
	if (calleeType && getTypeKind(calleeType) == FuncTypeKind) {
	  auto ftype = (FuncType *)calleeType;
	  // cerr << "Known Callee: " << calleeType << "\n";
	  if (ftype->args.size() != e->arguments.size()) {
		// TODO: handle variadics or raise error
		// cerr << "TODO: VARIADIC!\n";
		for(unsigned long i=0; i<e->arguments.size(); i++) {
		  auto argExpr = e->arguments[i].get();
		  argExpr->accept(this);
		}
	  } else for(unsigned long i=0; i<ftype->args.size(); i++)  {
		  auto argExpr = e->arguments[i].get();
		  in = ftype->args[i];
		  argExpr->accept(this);
		  // cerr << "\t\t\t" << i << ": " << argExpr->toString() << "\n";
		// auto argtype = ExprToType(e->arguments[i].get(), scope, ftype->args[i]).out;
        // auto argtype = ExprToType(e->arguments[i].get(), scope, ftype->args[i]).out;
      }
	  out = ftype->ret;
	} else {
	  cerr << "Unknown Callee Type: " << calleeType << "\n";
	  out = new UnknownType();
	}
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
	auto funcScopeInfo = scope->put(e);
	scope->name(e, e->name);
	ExprToType nested(0, scope->nested());
	vector<Type *>args;
	for(auto &arg : e->args) {
		ExprToType x(0, scope);
		x.visitDef(arg);
		args.push_back(x.out);
		nested.scope->put(arg)->setType(x.out);
		if (arg->kind == DefKind) {
		  auto name = ((Def *)arg)->name;
		  nested.scope->name(arg, name);
		} else {
		  std::cerr << "TODO: multiarg\n";
		}
		// nested.scope->saveDef(arg);
	}

	int i = 0;
	for(auto &arg : e->args) {
	  args[i++] = nested.scope->get(arg)->type;
	}
	auto ftype = new FuncType(e->rettype, args, false);
	out = ftype;
	funcScopeInfo->setType(ftype);

	e->body->accept(&nested);
	auto rettype = e->rettype;
	if (rettype == 0 || getTypeKind(rettype) == UnknownTypeKind) {
	  rettype = nested.out;
	  // HMM. I wonder if this is kosher.
	  e->rettype = rettype;
	  ftype->ret = rettype;
	}
	// nested.scope->show(cerr);
	funcScopeInfo->setType(ftype);
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

  void visit(Struct * s) {
	ExprToType nested(0, scope->nested(s->name));
	for(auto & f : s->fields) {
	  f->accept(&nested);
	}
	for(auto & m : s->methods) {
	  m->accept(&nested);
	}
	// for(auto & it : nested.scope->names) {
	//   auto name = it.first;
	//   auto expr = it.second;
	//   scope->put(expr)->setType(nested.scope->get(expr)->type);
	//   scope->name(expr, s->name + "." + name);
	// }
	scope->name(s, s->name);
	scope->put(s)->setType(new UserType(s->name));
	scope->show(cerr);
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

  InferTypesPass(Expr * e, Scope * scope) : Visitor("infer "), out(e), scope(scope) {
	if (e) e->accept(this);
  }

  void visit(Module * m) {
	foreach(m->lines, it) { (*it)->accept(this); }
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

  void visit(BlockExpr * e) { e->notes.isType(new UnknownType());  }

  void visit(Call * c) { c->notes.isType(ExprToType(c, scope).out); }

  void visit(Struct * s) { s->notes.isType(ExprToType(s, scope).out); }

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

  module = parse(0, R"CORAL(
type io:
  extern "C" printf : Func[Ptr[Int8],..., Void]
  extern "C" open : Func[Ptr[Int8], Int32, Ptr[Int8]]

type FdByteReader:
  let fd: Int32
  func read(n):
    # TODO buffer input
    let buf = Array[Int8].create n
    let m = platform.read(fd, buf, intNative n)
    ByteString.new(buf, m)

io.printf "Hello "

io.printf("%s!\n", "World")

io.open("foobar.coral", 0)

)CORAL");
  TreePrinter(module, cout).print();
  module = (Module *)InferTypesPass(module).out;
  cout << " [ Inferred ] ----------------------------------------\n";
  TreePrinter(module, cout).print();
}
