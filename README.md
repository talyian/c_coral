# Coral Language

[![Build Status](https://travis-ci.org/talyian/coral.svg?branch=coral)](https://travis-ci.org/talyian/coral)

A toy LLVM-based language. 
A Kaleidoscope tutorial project that gained sentience.
A Playground to experiment with language features.

## Getting Started

Easiest way is to build the docker container that has the dependencies (llvm/clang/flex/bison) installed. (`make docker` will build the container and then dump you inside the container in a shell)

```
$ docker build -t coral -f dockerenv/Dockerfile .
$ docker run --rm coral bash
[coral]/work/src$ coral-test
... Runs tests ...
[coral]/work/src$ coral-jit tests/cases/simple/hello_world.coral
Hello, World!
```

## Dependencies

If you'd prefer to build Coral without using docker, make sure you have `make`, `llvm` 5, `clang` 5, `flex`, `bison` installed at a minimum.
