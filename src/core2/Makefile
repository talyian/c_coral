run: build/coralcore
	@$<

build/coralcore: build/expr.o build/main.o build/prettyprinter.o
	clang++-5.0 -o $@ $^

build/%.o: %.cc
	clang++-5.0 -std=c++11 -Wall -Wextra -c -o $@ $<
	clang++-5.0 -MM -std=c++11 -Wall -Wextra -c -o $@.d -MT $@ $<
-include build/*.d
