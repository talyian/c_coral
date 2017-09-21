CXX=clang++-5.0 -std=c++11
CLANG=clang++-5.0 -std=c++11
WCLANG=clang++.exe
WCONFIG=llvm-config.exe
CONFIG ?= llvm-config-5.0
CXXCONFIG=$(shell ${CONFIG} --cxxflags | sed 's/-Wl,-fuse-ld=gold//; s/-Wno-maybe-uninitialized//;')
TESTFILE ?= samples/hello_world.coral
COMPILE=${CLANG} -c -o $@ $<

default: bin/coral
	@true

watch:
	/bin/bash watch.sh

clean:
	git clean -xf bin obj

docker:
	docker build -t coral -f dockerenv/Dockerfile .
	docker run -u $(shell id -u):$(shell id -g) -v ${PWD}:/work --rm -it coral

bin/coral: obj/compiler.o obj/parser.o obj/lexer.o obj/ast.o obj/main.o obj/type.o
	${CLANG} -o $@ $+ $(shell ${CONFIG} --libs)

obj/ast.o: obj/ast.cc obj/ast.hh obj/type.hh
	${COMPILE}

obj/compiler.o: obj/compiler.cc obj/parser.cc obj/ast.hh obj/type.hh obj/compiler.hh
	${COMPILE} ${CXXCONFIG}

obj/lexer.o: obj/lexer.cc obj/ast.hh obj/type.hh obj/parser.hh
	${COMPILE}

obj/main.o: obj/main.cc obj/ast.hh obj/type.hh obj/parser.hh
	${COMPILE}

obj/parser.o: obj/parser.cc obj/ast.hh obj/type.hh obj/parser.hh
	${COMPILE}

obj/%.hh : src/%.hh
	ln -sf ../$< $@

obj/lexer.cc : src/lexer.l
	flex -o $@ $<

obj/parser.hh obj/parser.cc: src/parser.yy obj/ast.hh
	bison -d -o $@ $<

obj/%.cc : src/%.cc
	ln -sf ../$< $@
