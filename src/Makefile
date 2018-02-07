LINKCMD=mkdir -p $(dir $@); clang++-5.0 -o $@ $^
CC=clang++-5.0 -std=c++11 -Wall -Wextra -I.
BUILD=obj
BIN=bin
.SUFFIXES:

pp: ${BIN}/coral-parse
	$< tests/cases/features/ifstatements.coral
run: ${BIN}/coral-jit
	$<
debug: ${BIN}/coral-jit
	@gdb -ex run -ex quit $<
mdebug: ${BIN}/coral-jit
	valgrind --leak-check=full $<
test: ${BIN}/coral-test
	$<

clean:
	@rm -r ${BIN} ${BUILD} parser/flexLexer.cc parser/bisonParser.tab.cc parser/bisonParser.tab.hh

COREFILES=${BUILD}/core/expr.o  ${BUILD}/core/prettyprinter.o
PARSERFILES=${BUILD}/parser/lexer.o ${BUILD}/parser/bisonParser.tab.o  ${BUILD}/parser/flexLexer.o ${BUILD}/parser/parser.o
ANALYZERS=${BUILD}/analyzers/NameResolver.o ${BUILD}/analyzers/ReturnInserter.o
${BIN}/coral-core: ${COREFILES} ${BUILD}/core/main_core.o
	${LINKCMD}
${BIN}/coral-parse: ${COREFILES} ${PARSERFILES} ${BUILD}/parser/main_parse.o
	${LINKCMD}
${BIN}/coral-token: ${COREFILES} ${PARSERFILES} ${BUILD}/parser/main_tokens.o
	${LINKCMD}
${BIN}/coral-jit: ${COREFILES} ${PARSERFILES} ${ANALYZERS} ${BUILD}/codegen/codegen.o ${BUILD}/codegen/LLVMFunctionCompiler.o ${BUILD}/codegen/main_jit.o
	${LINKCMD} $(shell llvm-config-5.0 --libs)
${BIN}/coral-test: ${COREFILES} ${PARSERFILES} ${ANALYZERS} ${BUILD}/tests/actions.o
	${LINKCMD} $(shell llvm-config-5.0 --libs)

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
	${CC} -Wno-unknown-warning-option -Wno-unused-command-line-argument $(shell llvm-config-5.0 --cxxflags) -MM -o $@.d -MT '$@ $@.d' $<
	${CC} -Wno-unknown-warning-option -Wno-unused-command-line-argument $(shell llvm-config-5.0 --cxxflags) -c -g -o $@ $<

-include ${BUILD}/**/*.d
