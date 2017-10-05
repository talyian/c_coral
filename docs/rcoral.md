# Transformation Between Coral and R-Coral

R-Coral is roughly feature-comparable to C. It's used as the end product for the Coral compiler

for each module:
  Coral Source -> Coral AST -> RCoral AST -> LLVM bitcode

## No Type Inference

In R-Coral, all values are statically typed at declaration. 

## Module Main Function

Coral source files are evaluated in source order. In Rcoral, global constants and function definitions are defined first. All function calls are put inside a ".init" function. Whenever a module is loaded, its `.init` is called. the runtime inserts a "main" function that initializes `argc, argv` etc but for now it is not accessible from coral

```coral
let a = 1
let b = 2
let c = strlen "foo"
```

```rcoral
let a : Int32 = 1
let b : Int32 = 2
let c : Int32 = undefined

func .init : Int32 ():
  set c = strlen "foo"
```
