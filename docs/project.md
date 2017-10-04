
# Core

1. ast - syntax nodes
2. infer - type inference

# Parsing

The primary interface here is via a top level interface of
```
Module * coral::parsing::parse(FILE *)

void coral::parsing::lex_stream(FILE *, void (*f) (int token, char ** text))
```
1. lexer.l - flex lexer
2. parser.yy - bison grammar

# Codegen

```
void run_jit(Module * m);

std::string get_ir(Module * m);
```
