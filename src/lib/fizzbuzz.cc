extern "C" const char * fizzbuzz(int n) {
  string s;
  for(int i=1; i<n; i++) {
    if (i % 15 == 0) s += "fizzbuzz ";
    else if (i % 5 == 0) s += "buzz ";
    else if (i % 3 == 0) s += "fizz ";
    else s += std::to_string(i) + " ";
  }
  return (new string(s))->c_str();
}
