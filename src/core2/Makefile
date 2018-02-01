LINKCMD=mkdir -p $(dir $@); clang++-5.0 -o $@ $^
BUILD=obj
BIN=bin
.SUFFIXES:

run: ${BIN}/main-coral-parse
	$<
debug: ${BIN}/main-coral-parse
	@gdb -ex run -ex quit $<
mdebug: ${BIN}/main-coral-parse
	valgrind --leak-check=full $<

clean:
	@rm -r ${BIN} ${BUILD} parser/flexLexer.cc parser/bisonParser.tab.cc parser/bisonParser.tab.hh

${BIN}/main-coral-core: ${BUILD}/core/expr.o  ${BUILD}/core/prettyprinter.o ${BUILD}/core/main_core.o
	${LINKCMD}
${BIN}/main-coral-token: ${BUILD}/core/expr.o  ${BUILD}/core/prettyprinter.o ${BUILD}/parser/lexer.o ${BUILD}/parser/main_tokens.o
	${LINKCMD}
${BIN}/main-coral-parse: ${BUILD}/core/expr.o  ${BUILD}/core/prettyprinter.o ${BUILD}/parser/lexer.o ${BUILD}/parser/bisonParser.tab.o  ${BUILD}/parser/flexLexer.o ${BUILD}/parser/parser.o ${BUILD}/parser/main_parse.o
	${LINKCMD}

# # # Bison # # #
obj/parser/lexer.o: parser/bisonParser.tab.hh
parser/flexLexer.cc: parser/flexLexer.l
	@mkdir -p $(dir $@)
	flex -o $@ $<
parser/bisonParser.tab.hh: parser/bisonParser.tab.cc
parser/bisonParser.tab.cc: parser/bisonParser.yy
	bison -d -o $@ $<

# # # Autodeps # # #
${BUILD}/%.o: %.cc
	@mkdir -p $(dir $@)
	clang++-5.0 -std=c++11 -Wall -Wextra -MM -o $@.d -MT '$@ $@.d' $<
	clang++-5.0 -std=c++11 -Wall -Wextra -c -g -o $@ $<

-include ${BUILD}/**/*.d
