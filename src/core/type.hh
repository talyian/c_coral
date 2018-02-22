#pragma once
#include <map>
#include <memory>
#include <vector>
#include <cstdio>

namespace coral {
  namespace type {
	class Type {
	public:
	  std::string name;
	  std::vector<Type> params;
	  Type(std::string name) : name(name) { }
	  Type(std::string name, std::vector<Type> params) : name(name), params(params) { }
	  bool operator != (Type const & other) { return name != other.name; }
	  bool operator == (Type const & other) { return name == other.name; }
	  Type returnType();
	};
	std::ostream & operator << (std::ostream &os, Type & tt);
  }
}
