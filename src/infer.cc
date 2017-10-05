#include "type.hh"

class Scope {
public:
  map<string, Type *> data;
  void add(string name, Type * t) { data[name] = t; }
  void show() {
    for(auto it = data.begin(); it != data.end(); it++) {
      cout << it->first << ": " << it->second << endl;
    }
  }
};

vector<Type *> vec() { return vector<Type*>(); }
vector<Type *> vec(Type * p) { auto v = vec(); v.push_back(p); return v; }
vector<Type *> vec(Type * p0, Type * p1) {
  auto v = vec();
  v.push_back(p0);
  v.push_back(p1);
  return v;
}  
vector<Type *> vec(Type * p0, Type * p1, Type * p2) {
  auto v = vec();
  v.push_back(p0);
  v.push_back(p1);
  v.push_back(p2);  
  return v;
}  





int main() {
  Scope s;
  s.add("a", new IntType(32));
  s.add("frob", new UnknownType());
  s.add("printf", new FuncType(new VoidType(), vec(new PtrType(new PtrType(new IntType(8)))), true));  
  cout << "\x1B[2J\x1B[H";
  cout << "Inference \n";
  s.show();
}
