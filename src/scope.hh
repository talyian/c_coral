#include <string>
#include <vector>
#include <map>

class ScopeInfo {
public:
  std::string name;
  std::string globalname;
  std::string type;
  std::string declaredtype;
  std::string inferredtype;
  std::vector<std::string> typeRules;
};

class Scope {
public:
  std::map<std::string, ScopeInfo> data;
  ScopeInfo& operator [](std::string name) {
    return data[name];
  }
};
