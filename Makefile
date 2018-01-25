CC=     clang++-5.0
SHELL:= /bin/bash
COMPILE=${CC} -Wall -std=c++11 -Isrc -g -c -o $@ $<
MM=     ${CC} -Wall -std=c++11 -Isrc -MM $<
LINK=   ${CC}

.SUFFIXES:

.PHONY: build test clean

default: bin/coral-core bin/coral-parse bin/coral-token bin/coral-codegen

test: bin/test-coral-codegen bin/test-coral-parse 
	bin/test-coral-parse && bin/test-coral-codegen

clean:
	mkdir -p bin obj src/parsing/generated
	rm -rf bin/* obj/* src/parsing/generated/*

docker:
	docker build -t coral dockerenv
	docker run --rm -it -v ${PWD}:/work coral bash

# Core includes code that other submodules are allowed to depend on
include makefiles/core.Makefile

# Parsing includes all code involved in turning text into an AST
include makefiles/parser.Makefile

# Codegen includes all the parts involved in compiling the coral AST
CODEGENFILES=${COREFILES} obj/codegen/jitEngine.o obj/codegen/moduleCompiler.o
bin/test-coral-codegen: ${PARSERFILES} ${CODEGENFILES} obj/tests/codegen/test_codegen.o
	${LINK} -o $@ $^ $(shell llvm-config-5.0 --libs)
bin/coral-codegen: ${PARSERFILES} ${CODEGENFILES} obj/codegen/__main__.o
	${LINK} -o $@ $^ $(shell llvm-config-5.0 --libs)
bin/coral-jit: ${PARSERFILES} ${CODEGENFILES} obj/codegen/__mainjit__.o
	${LINK} -o $@ $^ $(shell llvm-config-5.0 --libs)

# Aux is all coral logic that isn't needed in Core/Parsing/Codegen

# Main includes the primary facade for Coral


include makefiles/autocc.Makefile

DEPFILES=$(shell find obj -name '*.o.d')

include ${DEPFILES}

obj/%.o: src/%.cc
	@mkdir -p $(shell dirname $@)
	@${MM} | sed 's&.*.o:&$@ $@.d:&' > $@.d
	${COMPILE}
.PRECIOUS: obj/%.o.d
