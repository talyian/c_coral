# Coral Language

[https://travis-ci.org/talyian/coral.svg?branch=coral]

A toy LLVM-based language. A Kaleidoscope tutorial project that gained sentience.

## Getting Started

Easiest way is to build the docker container that has the dependencies (llvm/clang/flex/bison) installed. (`make docker` will build the container and then dump you inside the container in a shell)

```
$ make docker
<builds docker image "coral">
   <launches docker image>
[coral]/work$ make test
   <compiles and runs tests>
[coral]/work$ make bin/coral
[coral]/work$ bin/coral jit your_source_file.coral
```

## Dependencies

If you'd prefer to build Coral without using docker, make sure you have `make`, `llvm` 5, `clang` 5, `flex`, and `bison` installed.
