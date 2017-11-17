/*
  Unclassify Pass: convert methods to mangled global functions

  type ClassName[TemplateT1]:
      field0 : FType0
	  field1 : FType1
	  func new(n):
	      set field0 = n
	  func toString():
*/

#include "../core/expr.hh"
#include "../core/utils.hh"
#include "../core/treeprinter.hh"

class TraversingVisitor : public coral::AbstractVisitor {
public:
  coral::Expr * out = 0;
  TraversingVisitor(std::string name) { }
  void visit(coral::Expr * e) { }
  void visit(coral::Module * m) { for(auto &x : m->lines) x->accept(this); }
  void visit(coral::BlockExpr * e) { for(auto &x : e->lines) x->accept(this); }
  void visit(coral::FuncDef * e) { e->body->accept(this); }
  void visit(coral::Extern * e) { }
  void visit(coral::Let * e) { }
  void visit(coral::BinOp * e) {
	out = e->lhs;
	e->lhs->accept(this);
	e->lhs = out;
	out = e->rhs;
	e->rhs->accept(this);
	e->rhs = out;
	out = e;
  }
  void visit(coral::Call * e) {
	out = e->callee.get();
	out->accept(this);
	if (out) {
	  e->callee.release();
	  e->callee.reset(out);
	} else {
	  cerr << "null callee ???\n";
	}
	for(auto &arg : e->arguments) {
	  out = arg.get();
	  arg->accept(this);
	  arg.release();
	  arg.reset(out);
	}
	out = e;
  }
  void visit(coral::Index * e) { }
  void visit(coral::String * e) { }
  void visit(coral::Long * e) { }
  void visit(coral::VoidExpr * e) { }
  void visit(coral::BoolExpr * e) { }
  void visit(coral::Double * e) { }
  void visit(coral::Var * e) { }
  void visit(coral::If * e) { }
  void visit(coral::For * e) { }
  void visit(coral::Return * e) { }
  void visit(coral::Cast * e) { }
  void visit(coral::AddrOf * e) { }
  void visit(coral::DeclClass * e) { }
  void visit(coral::ImplType * e) { }
  void visit(coral::ImplClassFor * e) { }
  void visit(coral::DeclTypeEnum * e) { }
  void visit(coral::DeclTypeAlias * e) { }
  void visit(coral::MatchExpr * e) { }
  void visit(coral::MatchCaseTagsExpr * e) { }
  void visit(coral::Tuple * e) { }
  void visit(coral::EnumCase * e) { }
  void visit(coral::MatchEnumCaseExpr * e) { }
  void visit(coral::Member * e) { }
  void visit(coral::Struct * e) { }
  void visit(coral::Set * e) { }
  void visit(coral::MultiLet * e) { }
};

// turns "func Class$method(this): member"
// into "func Class$method(this): this.member"
// if "member" is a member of the this Struct
class ThisMemberInsert : public TraversingVisitor {
public :
  coral::Struct * target;
  ThisMemberInsert(std::string prefix, coral::Struct * s)
	: TraversingVisitor("unstruct "), target(s) {
	for(auto &m : s->methods)
	  m->accept(this);
  }
  void visit(coral::Var * e) {
	for(auto &f : target->fields) {
	  if (coral::ExprTypeVisitor(f).out == coral::LetKind) {
		auto letf = (coral::Let *)f;
		if (letf->var->kind == coral::DefKind) {
		  auto var = (coral::Def *)letf->var;
		  // TODO: this should actually happen after name resolution
		  // and use a reference match instead of name match
		  // because the name could get shadowed by a parameter or local

		  // Or rely on the results of type inference by converting
		  // free variables in the function into member references
		  if (var->name == e->name) {
			// cerr << "-------------------- " << e->name << " --------------------\n";
			out = new coral::Member(new coral::Var("this"), e->name);
			return;
		  }
		}
	  }
	}
  }
};

class UnclassifyStruct : public TraversingVisitor {
public:
  coral::Struct * target;
  std::string prefix;
  std::vector<coral::FuncDef *> out;
  UnclassifyStruct(std::string prefix, coral::Struct * s)
	: TraversingVisitor("unstruct "), prefix(prefix) {

	ThisMemberInsert(prefix, s);

	target = s;
	this->prefix = prefix + s->name + "$";

	for(auto &f : s->fields)
	  f->accept(this);
	for(auto &m : s->methods)
	  m->accept(this);
  }

  void visit(coral::FuncDef * f) {
	std::cerr << "\033[1;34m";
	f->name = prefix + f->name;
	f->args.insert(
	  f->args.begin(),
	  new coral::Def("this", new coral::UserType(target->name)));

	f->body->accept(this);

	out.push_back(f);
	coral::TreePrinter(f, std::cerr).print();
	std::cerr << "\033[0m";
  }

  void visit(coral::Let * e) { }
};

class UnclassifyPass : public coral::Visitor {
public:
  coral::Module * module;
  std::vector<coral::Expr *> transfer;
  std::string prefix;

  UnclassifyPass(coral::Module * m) : Visitor("unclass ") {
	module = m;
	if (m) m->accept(this);
  }

  void visit(coral::Module * m) {
	std::vector<coral::Expr *> lines;
	for(auto &line : m->lines) {
	  transfer.clear();
	  transfer.push_back(line);
	  line->accept(this);
	  for(auto &e : transfer)
		lines.push_back(e);
	}
	m->lines = lines;
  }
  void visit(coral::Extern * e) { }
  void visit(coral::Let * e) { }
  void visit(coral::Call * e) { }
  void visit(coral::DeclTypeAlias * e) { }
  void visit(coral::FuncDef * e) { }
  void visit(coral::Struct * s) {
	UnclassifyStruct ucs { "$", s };
	for(auto &expr : ucs.out)
	  transfer.push_back(expr);
  }

};
