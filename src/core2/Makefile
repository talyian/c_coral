LINKCMD=clang++-5.0 -o $@ $^
.SUFFIXES:

run: build/main-coral-parse
	@$<
clean:
	@rm -r build parser/bisonParser.tab.cc parser/bisonParser.tab.hh

build/main-coral-core: build/core/expr.o  build/core/prettyprinter.o build/core/main.o
	${LINKCMD}
build/main-coral-token: build/core/expr.o  build/core/prettyprinter.o build/parser/lexer.o build/parser/main_tokens.o
	${LINKCMD}
build/main-coral-parse: build/core/expr.o  build/core/prettyprinter.o build/parser/lexer.o build/parser/bisonParser.tab.o build/parser/parser.o build/parser/main.o
	${LINKCMD}

# # # Bison # # #
build/parser/lexer.o: build/parser/flexLexer.hh
build/parser/flexLexer.hh: parser/flexLexer.l
	@mkdir -p $(dir $@)
	flex -o $@ $<
parser/bisonParser.tab.hh: parser/bisonParser.tab.cc
parser/bisonParser.tab.cc: parser/bisonParser.yy
	bison -d -o $@ $<

# # # Autodeps # # #
build/%.o: %.cc
	@mkdir -p $(dir $@)
	clang++-5.0 -std=c++11 -Wall -Wextra -MM -o $@.d -MT '$@ $@.d' $<
	clang++-5.0 -std=c++11 -Wall -Wextra -c -g -o $@ $<

-include build/**/*.d
