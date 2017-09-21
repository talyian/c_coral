# Coral Language

A toy LLVM-based language. A bigger Kaleidoscope. Created with <strike>love</strike> by [Jimmy](https://www.heyjimbo.com).

## Getting Started

Easiest way is to build the docker container that has the dependencies (llvm/clang/flex/bison) installed. (`make docker` will build the container and then dump you inside the container in a shell)

```
$ make docker
<builds docker image "coral">
<launches docker image>
[coral]/work$ make
<compiles bin/compiler>
<compiles hello_world.coral to IR>
<runs using lli-5.0>
```

## Dependencies

If you'd prefer to build Coral without using docker, make sure you have `make`, `llvm` 5.0, `clang` 5.0, `flex`, and `bison` installed.
