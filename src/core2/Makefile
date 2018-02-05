LINKCMD=mkdir -p $(dir $@); clang++-5.0 -o $@ $^
CC=clang++-5.0 -std=c++11 -Wall -Wextra -I.
BUILD=obj
BIN=bin
.SUFFIXES:

run: ${BIN}/main-coral-jit
	$<
debug: ${BIN}/main-coral-jit
	@gdb -ex run -ex quit $<
mdebug: ${BIN}/main-coral-jit
	valgrind --leak-check=full $<

clean:
	@rm -r ${BIN} ${BUILD} parser/flexLexer.cc parser/bisonParser.tab.cc parser/bisonParser.tab.hh

COREFILES=${BUILD}/core/expr.o  ${BUILD}/core/prettyprinter.o
PARSERFILES=${BUILD}/parser/lexer.o ${BUILD}/parser/bisonParser.tab.o  ${BUILD}/parser/flexLexer.o ${BUILD}/parser/parser.o
ANALYZERS=${BUILD}/analyzers/NameResolver.o ${BUILD}/analyzers/ReturnInserter.o
${BIN}/main-coral-core: ${COREFILES} ${BUILD}/core/main_core.o
	${LINKCMD}
${BIN}/main-coral-parse: ${COREFILES} ${PARSERFILES} ${BUILD}/parser/main_parse.o
	${LINKCMD}
${BIN}/main-coral-jit: ${COREFILES} ${PARSERFILES} ${ANALYZERS} ${BUILD}/codegen/codegen.o ${BUILD}/codegen/LLVMFunctionCompiler.o ${BUILD}/codegen/main_jit.o
	clang++-5.0 $(shell llvm-config-5.0 --libs) -o $@ $^
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
	${CC} -MM -o $@.d -MT '$@ $@.d' $<
	${CC} -c -g -o $@ $<
${BUILD}/codegen/%.o: codegen/%.cc
	@mkdir -p $(dir $@)
	${CC} $(shell llvm-config-5.0 --cxxflags) -MM -o $@.d -MT '$@ $@.d' $<
	${CC} $(shell llvm-config-5.0 --cxxflags) -c -g -o $@ $<

-include ${BUILD}/**/*.d
