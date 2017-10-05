#include "type.hh"
#include <string>
#include <algorithm>

ostream& operator << (ostream & os, Type * t) {
  if (t) {
    string s(t->toString());
    if (s == "Unknown")
      os << "\e[35m" << s << "\e[0m";
    else
      os <<  s;
  }
  else { os << "(nulltype)"; }
  return os; }

#define ACCEPT_VISITOR(TYPE) void TYPE##Type::accept(TypeVisitor * v) { v->visit(this); }
TYPE_LOOP(ACCEPT_VISITOR)
#undef ACCEPT_VISITOR

#define VISITDEF(TYPE) void TypeName::visit(TYPE##Type * t) { out = #TYPE; }
TYPE_LOOP(VISITDEF)
#undef VISITDEF

std::string getTypeName(Type * t) {
  TypeName tn;
  if (!t) return "(nulltype)";
  t->accept(&tn);
  return tn.out;
}

bool typeEquals(Type * a, Type * b) { return false; }

string VoidType::toString() { return "Void"; }
string IntType::toString() { return "int" + to_string(bits); }
string FloatType::toString() { return "float"; }
string FuncType::toString() {
    string val = "Func[";
    for(auto it = args.begin(); it != args.end(); it++) {
      if (*it) 
	val += (*it)->toString() + ", ";
    }
    if (variadic) val += "... , ";
    if (ret) val += ret->toString() + "]";
    else val += "??]";
    return val;
  }
string UnknownType::toString() {
  return info;
}
string PtrType::toString() {
    if (inner) 
      return "Ptr[" + inner->toString() + "]";
    return "Ptr[???]";
  }

string ArrType::toString() {
  if (inner) 
    return "Arr[" + inner->toString() + "]";
  return "Arr[???]";
}


Type * BuildType(std::string name) {
  // std::cerr << "buildling type " << name << std::endl;
  if (name == "Bool") return new IntType(1);
  if (name == "Int1") return new IntType(1);
  if (name == "Int8") return new IntType(8);
  if (name == "Int32") return new IntType(32);
  if (name == "Int64") return new IntType(64);
  if (name == "...") return new VariadicTypeInfo();
  if (name == "Void") return new VoidType();
  // std::cerr << "Unknown type: " << name << std::endl;
  return new UnknownType(name);
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
