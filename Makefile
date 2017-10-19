# CLANG=clang++-5.0 -std=c++11 -fsanitize=address -g3
CLANG=clang++-5.0 -std=c++11 -g3
WCLANG=clang++.exe
WCONFIG=llvm-config.exe
CONFIG=llvm-config-5.0
CXXCONFIG=$(shell ${CONFIG} --cxxflags) -Wno-unused-command-line-argument -Wno-unknown-warning-option
TESTFILE ?= samples/basic/hello.coral
COMPILE=${CLANG} -c -o $@ $<

default: bin/coral
	bin/coral jit ${TESTFILE}

watch:
	/bin/bash watch.sh

clean:
	git clean -xf bin obj

docker:
	docker build -t coral -f dockerenv/Dockerfile .
	docker run -e "TERM=${TERM}" -u $(shell id -u):$(shell id -g) -v ${PWD}:/work --rm -it coral

bin/coral: obj/codegen.o obj/parser.o obj/lexer.o obj/ast.o obj/main.o obj/type.o obj/mainfuncPass.o
	${CLANG} -o $@ $+ $(shell ${CONFIG} --libs) -lpcre2-8 -rdynamic

obj/type.o: obj/type.cc obj/type.hh
	${COMPILE}

obj/ast.o: obj/ast.cc obj/ast.hh obj/type.hh
	${COMPILE}

obj/codegen.o: obj/codegen.cc obj/parser.hh obj/ast.hh obj/type.hh obj/codegen.hh
	${COMPILE} ${CXXCONFIG}

obj/lexer.o: obj/lexer.cc obj/ast.hh obj/type.hh obj/parser.hh
	${COMPILE}

obj/main.o: obj/main.cc obj/ast.hh obj/type.hh obj/parser.hh obj/lexer.hh obj/treeprinter.hh obj/mainfuncPass.hh obj/inferTypePass.hh obj/typeScope.hh
	${COMPILE}

obj/parser.o: obj/parser.cc obj/ast.hh obj/type.hh obj/parser.hh
	${COMPILE}

obj/lexer.cc : src/lexer.l
	flex -o $@ $<

obj/parser.hh: src/parser.yy obj/ast.hh
	bison -d -o $@ $<

obj/parser.cc: src/parser.yy obj/ast.hh
	bison -d -o $@ $<

obj/%.o : obj/%.cc
	${COMPILE}

obj/%.hh : src/%.hh
	ln -sf ../$< $@

obj/%.cc : src/%.cc
	ln -sf ../$< $@
