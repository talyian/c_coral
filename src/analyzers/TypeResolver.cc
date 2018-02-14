#include "utils/ansicolor.hh"
#include "core/expr.hh"
#include "core/prettyprinter.hh"
#include "TypeResolver.hh"

#include <iostream>
#include <iomanip>


using namespace coral;

namespace frobnob {
  using std::unique_ptr;
  using std::ostream;

  class TypeConstraint;

  class TypeNode {
  public:
    // the expr that this node comes from
    ast::BaseExpr * expr;
    // a pretty name for the node
    std::string name;
  };

  class TypeConstraint {
  public:
    virtual std::ostream& print(std::ostream &out) { return out << "???"; }
    virtual TypeConstraint * ReplaceVariable(TypeNode * var, TypeConstraint * tt) { return this; }
    virtual bool is_equal(TypeConstraint & other) { return this == &other; }
  };

  std::ostream & operator << (std::ostream &out, TypeConstraint & tt) { return tt.print(out); }

  class Type : public TypeConstraint {
  public:
    std::string name;
    Type(std::string name) : name(name) { }
    virtual std::ostream& print(std::ostream &out) { return out << name; }
    bool is_equal(TypeConstraint & other) {
      auto o = dynamic_cast<Type *>(&other);
      return o && o->name == this->name; }
  };

  class Variable : public TypeConstraint {
  public:
    TypeNode * var;
    Variable(TypeNode * n) : var(n) { }
    virtual std::ostream& print(std::ostream &out) {
      return out << COL_RGB(5, 3, 4) << var->name << COL_CLEAR; }
    virtual TypeConstraint * ReplaceVariable(TypeNode * var, TypeConstraint * tt) {
      return (this->var == var) ? tt : this;
    }
    bool is_equal(TypeConstraint & other) {
      auto o = dynamic_cast<Variable *>(&other);
      return o && o->var->name == this->var->name; }
  };

  class And : public TypeConstraint {
  public:
    unique_ptr<TypeConstraint> lhs, rhs;
    And(TypeConstraint * lhs, TypeConstraint * rhs) : lhs(lhs), rhs(rhs) { }
  };

  class Or : public TypeConstraint {
  public:
    unique_ptr<TypeConstraint> lhs, rhs;
    Or(TypeConstraint * lhs, TypeConstraint * rhs) : lhs(lhs), rhs(rhs) { }
    virtual std::ostream& print(std::ostream &out) {
      return out << "Union(" << *lhs << ", " << *rhs << ")";
    }
    virtual TypeConstraint * ReplaceVariable(TypeNode * var, TypeConstraint * tt) {
      auto lvarnode = dynamic_cast<Variable *>(lhs.get());
      if (lvarnode && lvarnode->var == var) lhs.reset(tt);
      auto rvarnode = dynamic_cast<Variable *>(rhs.get());
      if (rvarnode && rvarnode->var == var) rhs.reset(tt);
      return this->Simplify();
    }
    TypeConstraint * Simplify() { return lhs->is_equal(*rhs) ? lhs.get() : this; }
  };

  class Call : public TypeConstraint {
  public:
    TypeConstraint * callee;
    std::vector<TypeConstraint *> operands;
    Call(TypeConstraint * callee, std::vector<TypeConstraint *> operands)
      : callee(callee), operands(operands) { }
    virtual std::ostream& print(std::ostream &out) {
      out << "Call(" << *callee << ", ";
      for(auto &&op: operands) {
        if (op != operands.front()) out << ", ";
        out << *op;
      }
      return out << ")";
    }
    virtual TypeConstraint * ReplaceVariable(TypeNode * var, TypeConstraint * tt) {
      for(auto & operand: operands) {
        auto varnode = dynamic_cast<Variable *>(operand);
        if (varnode && varnode->var == var) {
          operand = tt;
        }
      }
      return this;
    }
  };
  class TypeSolver : public ast::ExprVisitor {
  public:
    std::string visitorName() { return "TypeSolver"; }
    TypeSolver(ast::BaseExpr * m) {
      m->accept(this);
      std::cerr << COL_RGB(5, 2, 2)
                << " -------------------- "
                << "Type Solver"
                << " -------------------- "
                << COL_CLEAR << "\n";
      ShowConstraints();
      std::cerr << "-------------------------------------------------------\n";
      SolveConstraints();
      ShowConstraints();
      // std::cerr << "-------------------------------------------------------\n";
      ApplyConstraints();


      // for(auto &&name : nodekeys) { std::cerr << COL_RGB(5, 3, 4) << "   Tvar " << name << COL_CLEAR << "\n"; }
    }

    void update(ast::BaseExpr * e) {
      // when an expression e is updated
      // all its dependencies have to be updated too
    }

    std::map<std::string, TypeNode *> nodes;
    std::vector<std::string> nodekeys;
    TypeNode * out;
    TypeNode * GetVariable(ast::BaseExpr * expr, std::string name) {
      for(auto && pair: nodes) {
        if (pair.second->expr == expr) {
          return pair.second;
        }
      }
      return 0;
    }

    TypeNode * AddVariable(std::string origname, ast::BaseExpr * expr) {
      auto name = origname;
      for(int i=0; i<9999999; i++) {
        if (nodes.find(name) == nodes.end()) {
          nodekeys.push_back(name);
          nodes[name] = new TypeNode();
          nodes[name]->expr = expr;
          nodes[name]->name = name;
          break;
        }
        name = origname + ":" + std::to_string(i);
      }
      return nodes[name];
    }

    // std::vector<std::pair<TypeNode *, TypeConstraint *>> constraints;
    std::map<TypeNode *, TypeConstraint *> constraints;
    void AddConstraint(TypeNode * term, TypeConstraint * rule) {
      // constraints.push_back(std::make_pair(term, rule));
      constraints.insert(std::make_pair(term, rule));
    }

    void ShowConstraints() {
      for(auto &&pair: constraints) {
        auto term = pair.first;
        auto rule = pair.second;
        std::cerr << COL_RGB(5, 3, 4) << std::setw(20) << term->name
                  << COL_RGB(5, 5, 5) << " == "
                  << *rule
                  << COL_CLEAR << "\n";
      }
    }

    void ApplyConstraints() {
      for(auto && pair : constraints) {
        auto expr = pair.first->expr;
        auto type = dynamic_cast<Type *>(pair.second);
        if (type) {
          auto func = dynamic_cast<ast::Func *>(expr);
          if (func) {
            func->type->params.back() = coral::type::Type(type->name);
            continue;
          }
          auto def = dynamic_cast<ast::Def *>(expr);
          if (def) {
            def->type.reset(new coral::type::Type(type->name));
            continue;
          }
        }
      }
    }
    void SolveConstraints() { SolveConstraints(10); }
    void SolveConstraints(int n) {
      // subsitute terminal rules
      std::vector<std::pair<TypeNode *, TypeConstraint *>> pairs;
      for(auto && pair : constraints) {
        if (dynamic_cast<Type *>(pair.second)) {
          pairs.push_back(pair);
          // constraints.erase(constraints.find(pair.first));
        }
      }
      for(auto && to_replace: pairs)
        for(auto && other: constraints)
          other.second = other.second->ReplaceVariable(to_replace.first, to_replace.second);

      // Call-Return-Type rule
      for(auto && pair : constraints) {
        Call * rule;
        if ((rule = dynamic_cast<Call *>(pair.second))) {
          auto c = dynamic_cast<Type *>(rule->callee);
          if (c) {
            if (c->name == "'op:<'") {
              constraints[pair.first] = new Type("Int1");
              Unify(rule->operands[0], rule->operands[1]);
              continue;
            }
            if (c->name == "'op:+'") {
              constraints[pair.first] = rule->operands[0];
              continue;
            }
            if (c->name == "'op:-'") {
              constraints[pair.first] = rule->operands[0];
              // if (rule->operands[0].equals(rule->operands[1])) {
              //   constraints[pair.first] = rule->operands[0];
              // }
              // rule->operands[0], rule->operands[1]);
              continue;
            }
            if (c->name == "'op:*'") {
              constraints[pair.first] = rule->operands[0];
              continue;
            }
            std::cerr << COL_LIGHT_RED << c->name << COL_CLEAR << "\n";
          }
        }
      }
      if (n) SolveConstraints(n - 1);
    }

    void Unify(TypeConstraint * a, TypeConstraint * b) {
      auto var_a = dynamic_cast<Variable *>(a);
      if (var_a) {
        AddConstraint(var_a->var, b);
      } else {
        std::cerr << ":( \n";
      }
    }

    void visit(__attribute__((unused)) ast::Comment * c) { }
    void visit(ast::Module * m) { m->body->accept(this); }
    void visit(ast::Block * b) {
      for(auto && line:b->lines) if (line) line->accept(this);
    }
    void visit(ast::IfExpr * f) {
      f->cond->accept(this);
      f->ifbody->accept(this);
      auto ifv = out;
      if (f->elsebody) {
        f->elsebody->accept(this);
        auto elsev = out;
        out = AddVariable("if", f);
        AddConstraint(out, new Or(new Variable(ifv), new Variable(elsev)));
      } else {
        out = AddVariable("if", f);
        AddConstraint(out, new Variable(ifv));
      }
    }
    void visit(ast::Let * let) {
      let->var->accept(this);
      auto varnode = out;
      let->value->accept(this);
      auto valuenode = out;
      AddConstraint(varnode, new Variable(valuenode));
    }
    void visit(ast::While * w) {
      w->cond->accept(this);
      w->body->accept(this);
      out = 0;
    }
    void visit(ast::Return * r) {

    }
    void visit(ast::IntLiteral * i) {
      out = AddVariable("Int{" + i->value+"}", i);
      AddConstraint(out, new Type("Int32"));
    }

    void visit(ast::StringLiteral * i) {
      out = AddVariable("String{" + i->value+"}", i);
      AddConstraint(out, new Type("Ptr"));
    }

    void visit(ast::BinOp * o) {
      o->lhs->accept(this);
      auto lv = out;
      o->rhs->accept(this);
      auto rv = out;
      out = AddVariable("Op{" + o->op + "}", o);
      AddConstraint(
        out,
        new Call(new Type("'op:" + o->op + "'"), { new Variable(lv), new Variable(rv) }));
    }
    void visit(ast::Var * v) {
      out = GetVariable(v->expr, v->name);
      if (!out) {
        out = new TypeNode();
        out->expr = v;
        out->name = v->name;
        // std::cerr << COL_LIGHT_RED << "Warning - Not found: " << v->name << COL_CLEAR << "\n";
      }
    }
    void visit(ast::Call * f) {
      std::vector<TypeConstraint *> argnodes;
      for(auto &&arg: f->arguments) {
        arg->accept(this);
        argnodes.push_back(new Variable(out));
      }
      f->callee->accept(this);
      auto calleenode = out;
      out = AddVariable("call." + calleenode->name, f);
      AddConstraint(
        out,
        new Call(new Variable(calleenode), argnodes));
    }
    void visit(ast::Def * d) {
      out = AddVariable(d->name, d);
    }
    void visit(ast::Func * f) {
      out = AddVariable(f->name, f);
      auto funcout = out;
      for(auto &&param: f->params) {
        param->accept(this);
      }
      if (f->body) {
        f->body->accept(this);
        auto bodyVariable = out;
        auto retval = AddVariable(funcout->name + ".return", 0);
        AddConstraint(
          funcout,
          new Variable(bodyVariable));
      }
    }
  };
}

coral::analyzers::TypeResolver::TypeResolver(ast::Module * m): module(m) {
  for(auto && line: m->body->lines) {
    auto func = dynamic_cast<ast::Func *>(line.get());
    if (func)
      frobnob::TypeSolver x(func);
  }
}
