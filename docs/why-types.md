# Type System

## What do types do #1? Safety

Types prevent us from writing illogical code. For example, `strlen(5)`. You could imagine a type of C-like typeless language that allows `strlen(5)` as a valid program, and just segfaults once it interprets the value `5` as a pointer address (or even worse, on a 64-bit system, the value 5 plus 4 random bytes of whatever else is on the stack at the time as a pointer address.) Type systems in languages vary in their feature sets -- for example, in some languages, names cannot be bound to two different types in the same context.

    // in C, a variable name cannot have two different types in the same context.
    int x = 5;
    char x = '5';  <-- gcc: conflicting types for "x"

whereas in other languages, names don't have types and only values have types

    // in OCaml,
    > let x = 5;
    val x : int = 5
    > let x = "5";
    val x : string = 5

## What do types do #2? Help with method dispatch

In Python, `len([1,2,3])` and `len("foobar")` call two separate functions - one for a list and one for a string. In Java, "Integer.ToString()" and "DateTime.ToString()" are similarly separate functions. Even though one is a dynamically typed language and one is a statically typed language, both use their type system to help with dynamically dispatching a `ToString` call to the right implementation based on the type of the parameter.

## What do types do #3? Help with optimization

The compiler can carry around type information to optimize the machine code it generates. For example, `(a + b)` compiles down a single opcode if a/b are integers or floats but a string-concatenation call when a/b are `string`s (in Rust's case, the situation is slighty more complicated). As long as the types `a` and `b` can be statically proven, the optimized code can be generated. In a dynamic system, you'd have to first look up the types of `a` and `b` at runtime and conditionally run the addition code.

## The middle isn't excluded

So we often have the option of nailling down a type at compile time to generate type-specific code, or deferring the check at runtime to allow for more generic code. This doesn't mean that all languages must purely be static or dynamic -- often there's a mix of stuff that happens at compile-time, just-in-time during execution, or even in a defered process such as a tracing JIT that swaps out hotspots with optimized code. 