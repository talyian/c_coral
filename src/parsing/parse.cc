#include "../core/treeprinter.hh"
#include "../core/expr.hh"
#include "../parsing/lexer.hh"

#include <vector>
#include <queue>
#include <sstream>

// passing src to parser for error display
std::string yy_src = "";

// lexer internals - TODO encapsulate this
extern std::queue<int> tokenq;
extern std::vector<int> indents;
extern int lineno, colno, paren_depth;

Module * parse(FILE * in, const char * src) {
  if (in) {
    char * buf = 0;
    fseek(in, 0, SEEK_END);
    int len = ftell(in);
    buf = new char[len+1];
    fseek(in, 0, 0);
    buf[fread(buf, 1, len, in)] = 0;
    src = buf;
    in = 0;
  } else if (!src) {
    std::cerr << "failed to initialize input\n";
    return new Module(std::vector<Expr *> { new String("\"parse error 1\"") });
  }
  yy_src = src;
  void * scanner = 0;
  Module * module = 0;
  yylex_init(&scanner);

  // TODO: make this reentrant;
  while(!tokenq.empty()) tokenq.pop();
  while(!indents.empty()) indents.pop_back();
  indents.push_back(0);
  lineno = 0;
  colno = 0;
  paren_depth = 0;

  if (in) yyset_in(in, scanner);
  else if (src) yy_scan_string(src, scanner);

  yy::parser coralp(module, scanner);
  if (coralp.parse()) module = 0;
  yylex_destroy(scanner);
  if (module) return module;
  return new Module(std::vector<Expr *> { new String("\"parse error 2\"") });
}

void yy::parser::error(const yy::location &loc, const std::string& m) {
  std::stringstream ss(yy_src);
  std::string line;
  int i = 0;
  for(; i < (int)loc.begin.line - 1; i++) {
    std::getline(ss, line, '\n');
  }
  for(; i<(int)loc.begin.line + 2; i++) {
    std::getline(ss, line, '\n');
    std::cerr << line << std::endl;
	if (i == (int)loc.begin.line)
	  std::cerr << std::string(loc.begin.column, '.') << "^\n";
  }
  std::cerr << '[' << loc.begin << '-' << loc.end << "]: " << m << std::endl;
}
