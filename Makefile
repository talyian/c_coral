CLANG=clang++-5.0 -std=c++11
WCLANG=clang++.exe
WCONFIG=llvm-config.exe
CONFIG ?= llvm-config-5.0
TESTFILE ?= samples/hello_world.coral

default: bin/coral
	@true

ir: bin/coral
	@bin/coral ir < ${TESTFILE}

run: bin/coral
	@bin/coral ir < ${TESTFILE} | lli-5.0

docker:
	docker build -t coral -f dockerenv/Dockerfile .
	docker run -u $(shell id -u):$(shell id -g) -v ${PWD}:/work --rm -it coral

bin/coral: obj/compiler.o obj/parser.o obj/lexer.o obj/ast.o obj/main.o
	${CLANG} -o bin/coral \
	obj/parser.o obj/lexer.o obj/compiler.o obj/ast.o obj/main.o \
	$(shell ${CONFIG} --libs)

bin/lexer: obj/lexer.o src/lexer_main.cpp
	${CLANG} -c -o obj/lexer_main.o src/lexer_main.cpp
	${CLANG} -o bin/lexer obj/lexer.o obj/lexer_main.o

obj/parser.cc: src/parser.yy src/ast.h
	bison -d -o obj/parser.cc src/parser.yy
obj/parser.o: obj/parser.cc
	${CLANG} -c -o obj/parser.o obj/parser.cc

obj/lexer.cc: src/lexer.l src/ast.h
	flex -o obj/lexer.cc src/lexer.l 
obj/lexer.o: obj/lexer.cc
	${CLANG} -c -o obj/lexer.o obj/lexer.cc

obj/compiler.o: src/compiler.cpp obj/parser.o src/ast.h
	${CLANG} -c -o obj/compiler.o $(shell \
	    ${CONFIG} --cxxflags | \
	    sed s/-Wl,-fuse-ld=gold// | \
	    sed s/-Wno-maybe-uninitialized//) src/compiler.cpp

obj/main.o: src/main.cpp
	${CLANG} -c -o obj/main.o src/main.cpp

obj/ast.o: src/ast.cpp src/ast.h
	${CLANG} -c -o obj/ast.o src/ast.cpp

bin/coral.exe: obj/lexer.cc obj/parser.cc src/ast.h src/ast.cpp src/compiler.cpp
	${WCLANG} -c src/compiler.cpp
	${WCLANG} -c obj/lexer.cc
	${WCLANG} -c obj/parser.cc
	${WCLANG} -c src/ast.cpp



