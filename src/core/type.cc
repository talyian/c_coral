#include "type.hh"

namespace coral {
  Type::Type(std::string s) : name(s) { }
  Type::Type(std::string s, std::vector<Type *> params) : name(s), params(params) { }
  std::string Type::to_string() {
	std::string s = name;
	if (params.size()) {
	  s += "[";
	  s += join(", ", params, typeToString);
	  s += "]";
	}
	return name; }
  std::string typeToString(Type * t) { return t ? t->to_string() : "(nulltype)"; }

  Type * BuildType(std::string name) { return new Type(name); }
  Type * BuildType(std::string name, std::vector<Type *> params) { return new Type(name, params); }
//   TypeKind getTypeKind(BaseType * t) { return TypeKindVisitor(t).out;  }

//   std::string getTypeName(BaseType * t) { if (!t) return "(nulltype)"; return TypeNameVisitor(t).out; }

//   std::ostream& operator << (std::ostream& os, BaseType * t) {
//     os << (!t ? "(nulltype)" : t->toString());
//     return os;
//   }

//   std::string BaseType::toString() { return TypeNameVisitor(this).out; }


// #define ACCEPT(TYPE) void TYPE##Type::accept(TypeVisitor * t) { t->visit(this); }
//   TYPE_LOOP(ACCEPT);
// #undef ACCEPT

//   std::string TupleType::toString() {
// 	auto x = inner;
// 	if (x.empty()) return "Void";
// 	return "Tuple[" + join<BaseType *>(", ", x, [] (BaseType * a) { return (a ? a : new UnknownType())->toString(); }) + "]";
//   }

//   std::string FuncType::toString() {
// 	auto x = args;
// 	x.push_back(ret);
// 	return "Func[" + join<BaseType *>(", ", x, [] (BaseType * a) { return (a ? a : new UnknownType())->toString(); }) + "]";
//   }

//   Type * BuildType(std::string name) {
//     // std::cerr << "buildling type " << name << std::endl;
//     if (name == "Bool") return new IntType(1);
//     if (name == "Int1") return new IntType(1);
//     if (name == "Int8") return new IntType(8);
//     if (name == "Int32") return new IntType(32);
//     if (name == "Int64") return new IntType(64);
//     if (name == "...") return new VariadicTypeInfo();
//     if (name == "Void") return new VoidType();
// 	return new UserType(name, std::vector<BaseType *>{ });
//   }
//   Type * BuildType(std::string name, std::vector<Type *> typeparams) {
//     // std::cerr << "buildling type " << name << std::endl;
//     if (name == "Ptr") return new PtrType(typeparams[0]);
//     if (name == "Fn" || name == "Func") {
//       auto returnType = typeparams[typeparams.size()-1];
//       auto params = std::vector<Type *>();
//       for(int i=0; i<typeparams.size() - 1; i++)
// 	if (typeparams[i]->toString() != "...") params.push_back(typeparams[i]);
// 	else delete typeparams[i];
//       return new FuncType(returnType, params, params.size() + 1 < typeparams.size());
//     }
// 	return new UserType(name, typeparams);
//     std::cerr << "Unknown type: " << name << std::endl;
//     return 0;
//   }
}
