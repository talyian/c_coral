#include "type.hh"
#include <string>

void Type::accept(TypeVisitor * t) { t->visit(this); }
void FuncType::accept(TypeVisitor * t) { t->visit(this); }
void IntType::accept(TypeVisitor * t) { t->visit(this); }
void PtrType::accept(TypeVisitor * t) { t->visit(this); }
void ArrType::accept(TypeVisitor * t) { t->visit(this); }
void VoidType::accept(TypeVisitor * t) { t->visit(this); }

std::string PtrType::toString() { return "Ptr[" + inner->toString() + "]"; }
std::string VoidType::toString()  { return "void"; }
std::string IntType::toString() { return "int" + std::to_string(bits); }
std::string FuncType::toString()  {
  auto s = std::string("Fn[");
  for(auto a = args.begin(); a != args.end(); a++)
    s += (a == args.begin() ? "" : ", ") + (*a)->toString();
  if (variadic) s += "...";
  return s + "->" + ret->toString() + "]";
}

std::ostream& operator << (std::ostream& os, Type * tptr) { os << tptr->toString(); return os; }
