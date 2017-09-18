# This is a Makefile designed to run in WSL Ubuntu

CLANGCMD=clang++-5.0
CONFIGCMD=llvm-config-5.0

ir: bin/compiler
	@bin/compiler < samples/hello_world.coral

run: bin/compiler
	@bin/compiler < samples/hello_world.coral | lli-5.0
docker:
	docker build -t coral -f dockerenv/Dockerfile .
	docker run -v ${PWD}:/work --rm -it coral

bin/compiler: obj/compiler.o obj/parser.o obj/lexer.o obj/ast.o bin
	${CLANGCMD} -std=c++11 -o bin/compiler obj/parser.o obj/lexer.o obj/compiler.o obj/ast.o $(shell ${CONFIGCMD} --libs)

bin/lexer: obj/lexer.o src/lexer_main.cpp bin
	clang++-5.0 -std=c++11 -c -o obj/lexer_main.o src/lexer_main.cpp
	clang++-5.0 -std=c++11 -o bin/lexer obj/lexer.o obj/lexer_main.o

obj/parser.o: src/parser.yy src/ast.h obj
	bison -d -o obj/parser.cc src/parser.yy
	clang++-5.0 -std=c++11 -c -o obj/parser.o obj/parser.cc

obj/lexer.o: src/lexer.l src/ast.h obj
	flex -o obj/lexer.cc src/lexer.l
	clang++-5.0 -std=c++11 -c -o obj/lexer.o obj/lexer.cc

obj/compiler.o: src/compiler.cpp obj/parser.o
	${CLANGCMD} -c -o obj/compiler.o $(shell \
	    ${CONFIGCMD} --cxxflags | \
	    sed s/-Wl,-fuse-ld=gold// | \
	    sed s/-Wno-maybe-uninitialized//) src/compiler.cpp

obj/ast.o: src/ast.cpp obj
	${CLANGCMD} -std=c++11 -c -o obj/ast.o src/ast.cpp

bin/test: test.cpp bin
	${CLANGCMD} -o bin/test -std=c++11 test.cpp -lSDL2

bin/test.exe: test.cpp bin
	clang++.exe -c -o obj/test.o -Iinclude test.cpp
	clang++.exe -o bin/test.exe -Llib/SDL2/x64 obj/test.o -lSDL2 -Xlinker /subsystem:console

bin:
	mkdir -p bin

obj:
	mkdir -p obj
