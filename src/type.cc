#include "type.hh"
#include <string>
#include <algorithm>

void Type::accept(TypeVisitor * t) { t->visit(this); }
void FuncType::accept(TypeVisitor * t) { t->visit(this); }
void IntType::accept(TypeVisitor * t) { t->visit(this); }
void PtrType::accept(TypeVisitor * t) { t->visit(this); }
void ArrType::accept(TypeVisitor * t) { t->visit(this); }
void VoidType::accept(TypeVisitor * t) { t->visit(this); }
void FloatType::accept(TypeVisitor * t) { t->visit(this); }

std::string PtrType::toString() { return "Ptr[" + inner->toString() + "]"; }
std::string VoidType::toString()  { return "void"; }
std::string IntType::toString() { return "int" + std::to_string(bits); }
std::string FloatType::toString() { return "float" + std::to_string(bits); }
std::string FuncType::toString()  {
  auto s = std::string("Fn[");
  for(auto a = args.begin(); a != args.end(); a++)
    s += (a == args.begin() ? "" : ", ") + (*a)->toString();
  if (variadic) s += "...";
  return s + "->" + ret->toString() + "]";
}

std::ostream& operator << (std::ostream& os, Type * tptr) { os << tptr->toString(); return os; }


Type * BuildType(std::string name) {
  // std::cerr << "buildling type " << name << std::endl;
  if (name == "Bool") return new IntType(1);
  if (name == "Int1") return new IntType(1);
  if (name == "Int8") return new IntType(8);
  if (name == "Int32") return new IntType(32);
  if (name == "Int64") return new IntType(64);
  if (name == "...") return new VariadicMarker();
  if (name == "Void") return new VoidType();
  std::cerr << "Unknown type: " << name << std::endl;
  return 0;
}
Type * BuildType(std::string name, std::vector<Type *> typeparams) {
  // std::cerr << "buildling type " << name << std::endl;
  if (name == "Ptr") return new PtrType(typeparams[0]);
  if (name == "Fn") {
    auto returnType = typeparams[typeparams.size()-1];
    auto params = std::vector<Type *>();
    for(int i=0, j=0; i<typeparams.size() - 1; i++)
      if (typeparams[i]->toString() != "...") params.push_back(typeparams[i]);
      else delete typeparams[i];
    return new FuncType(returnType, params, params.size() + 1 < typeparams.size());
  }
  std::cerr << "Unknown type: " << name << std::endl;
  return 0;
}
