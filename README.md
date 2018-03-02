# Coral Language

[![Join the chat at https://gitter.im/talyian-coral/Lobby](https://badges.gitter.im/talyian-coral/Lobby.svg)](https://gitter.im/talyian-coral/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

[![Build Status](https://travis-ci.org/talyian/coral.svg?branch=coral)](https://travis-ci.org/talyian/coral)

A toy LLVM-based language.  
A Kaleidoscope tutorial project that gained sentience.  
A Playground to experiment with language features.  
This is not https://corallanguage.org/ ! -- I need a new name

## Show me some Code!

    printf "Hello, World!\n"

    func factorial(n):
       if n < 2:
          1
       else:
          n * factorial(n - 1)

## Getting Started

Easiest way is to build the docker container that has the dependencies (llvm/clang/flex/bison) installed.

```
$ docker build -t coral -f dockerenv/Dockerfile .
$ docker run --rm coral bash
[coral]/work/src$ coral-test
... Runs tests ...
[coral]/work/src$ coral-jit tests/cases/simple/hello_world.coral
Hello, World!
```

## Dependencies

If you'd prefer to build Coral without using docker, make sure you have `make`, `llvm` 5, `clang` 5, `flex`, `bison` installed at a minimum. Additional libraries such as `libpcre2` or `libuv` may be required to build.
