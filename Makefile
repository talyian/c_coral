CC=clang++-5.0
SHELL:=/bin/bash
COMPILE=${CC} -Wall -std=c++11 -g -c -o $@ $<
MM=${CC} -Wall -std=c++11 -MM $<
LINK=${CC}

.PHONY: build test clean

default: bin/coral-core bin/coral-parse bin/coral-token bin/test-coral-codegen

## sub-projects
.PHONY: core parsing

core: bin/coral-core

parsing: bin/coral-parse

test: bin/test-coral-parse
	$<
scope: bin/test-coral-treecompare
	$<
codegen-test: bin/test-coral-codegen
	$<

# Core includes code that other submodules are allowed to depend on
COREFILES=obj/core/expr.o obj/core/type.o obj/core/treeprinter.o
bin/coral-core: ${COREFILES} obj/core/__main__.o
	${LINK} -o $@ $^

# Parsing includes all code involved in turning text into an AST
PARSERFILES=obj/parsing/generated/lexer.o obj/parsing/generated/parser.o obj/parsing/parse.o
bin/coral-parse: ${COREFILES} ${PARSERFILES} obj/parsing/__main_parse.o
	${LINK} -o $@ $^
bin/coral-token: ${COREFILES} ${PARSERFILES} obj/parsing/__main_token.o
	${LINK} -o $@ $^
bin/test-coral-parse: ${COREFILES} ${PARSERFILES} \
  obj/tests/parsing/test_parser.o obj/tests/parsing/__main__.o
	${LINK} -o $@ $^

# Codegen includes all the parts involved in compiling the coral AST
bin/test-coral-codegen: ${COREFILES} obj/codegen/codegen.o obj/codegen/codegenExpr.o obj/codegen/__main__.o
	${LINK} -o $@ $^ $(shell llvm-config-5.0 --libs)

# Aux is all coral logic that isn't needed in Core/Parsing/Codegen
bin/test-coral-infer: ${COREFILES} ${PARSERFILES} obj/infer.o
	${LINK} -o $@ $^
bin/test-coral-treecompare: ${COREFILES} ${PARSERFILES} obj/core/treecompare.o
	${LINK} -o $@ $^

# Main includes the primary facade for Coral



src/parsing/generated/parser.hh: src/parsing/generated/parser.cc
	@true

src/parsing/generated/parser.cc: src/parsing/parser.yy
	@mkdir -p $(shell dirname $@)
	bison -d -o $@ $<

src/parsing/generated/lexer.cc: src/parsing/lexer.l src/parsing/generated/parser.hh
	@mkdir -p $(shell dirname $@)
	flex -o $@ $<

clean:
	mkdir -p bin obj src/parsing/generated
	rm -rf bin/* obj/* src/parsing/generated/*

docker:
	docker build -t coral dockerenv
	docker run --rm -it -v ${PWD}:/work coral bash

## SECTION auto-dependency generation
DEPFILES=$(shell find obj -name '*.o.d')

-include ${DEPFILES}

.PRECIOUS: obj/%.o.d

## Whenever a cc file is updated, its deps are updated
obj/%.o.d: src/%.cc
	@mkdir -p $(shell dirname $@)
	set -e; \
	set -o pipefail; \
	OUT=$$(${MM}); \
	echo -n $$(dirname $@)/ > $@ && echo "$${OUT}" >> $@ && \
	echo $$'\t' '$(value COMPILE)' >> $@

## If we build a .o, we need its .d
## If the .cc was updated, we re-generate the .d and then rebuild
obj/%.o: obj/%.o.d
	CC=${CC} ${MAKE} --no-print-directory -r $@

# There's probably a better way to do this
# we're saying everything in /codegen requires llvm-flags for compiling
obj/codegen/%.o: obj/codegen/%.o.d
	echo '*codegen ***************************************'
	CC="${CC} $(shell llvm-config-5.0 --cxxflags)"  ${MAKE} --no-print-directory -r $@

obj/codegen/%.o.d: src/codegen/%.cc
	@mkdir -p $(shell dirname $@)
	@echo ---------------------------------------- making $@
	@set -e; \
	set -o pipefail; \
	 OUT=$$(${MM} $$(llvm-config-5.0 --cxxflags)); \
	 echo -n $$(dirname $@)/ > $@; \
	 echo "$${OUT}" >> $@ && \
	 echo $$'\t' '$(value COMPILE) $(shell llvm-config-5.0 --cxxflags)' >> $@
