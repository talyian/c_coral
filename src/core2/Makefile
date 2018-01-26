run: build/main-coral-parse
	@$<

build/main-coral-core: build/core/expr.o  build/core/prettyprinter.o build/core/main.o
	clang++-5.0 -o $@ $^

build/main-coral-parse: build/core/expr.o  build/core/prettyprinter.o build/parser/lexer.o build/parser/main.o
	clang++-5.0 -o $@ $^

# # # Bison # # #
build/parser/flex.hh: parser/flex.l
	flex -o $@ $<

# # # Autodeps # # #
build/%.o: %.cc
	@mkdir -p $(dir $@)
	clang++-5.0 -MM -std=c++11 -Wall -Wextra -c -o $@.d -MT $@ $<
	clang++-5.0 -std=c++11 -Wall -Wextra -c -o $@ $<

-include build/*.d
