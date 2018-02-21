LINKCMD=mkdir -p $(dir $@); clang++-5.0 -o $@ $^
CC=clang++-5.0 -std=c++11 -Wall -Wextra -I.
LLVM_CC=${CC} -Wno-unknown-warning-option -Wno-unused-command-line-argument $(shell llvm-config-5.0 --cxxflags)
BUILD=obj
BIN=bin
#TESTFILE?=tests/cases/features/pcre.coral
TESTFILE?=tests/cases/wip.coral
.SUFFIXES:

run: ${BIN}/coral-jit
	$< ${TESTFILE}
debug: ${BIN}/coral-test
	@gdb -ex 'run ${TESTFILE}' -ex quit $<
mdebug: ${BIN}/coral-jit
	valgrind --leak-check=full $< ${TESTFILE}
parse: ${BIN}/coral-parse
	$< ${TESTFILE}
test: ${BIN}/coral-test
	$< ${TESTARGS}
types: ${BIN}/coral-types
	$<
clean:
	@rm -r ${BIN} ${BUILD} parser/flexLexer.cc parser/bisonParser.tab.cc parser/bisonParser.tab.hh

sdl: tests/cases/wip.2.cc
	clang++-5.0 -Wall -std=c++11 -o sdl tests/cases/wip.2.cc -lSDL2
	./sdl

COREFILES=${BUILD}/core/expr.o  ${BUILD}/core/prettyprinter.o ${BUILD}/utils/ansicolor.o
PARSERFILES=${BUILD}/parser/lexer.o ${BUILD}/parser/bisonParser.tab.o  ${BUILD}/parser/flexLexer.o ${BUILD}/parser/parser.o
ANALYZERS=${BUILD}/analyzers/NameResolver.o ${BUILD}/analyzers/ReturnInserter.o ${BUILD}/analyzers/TypeResolver.o ${BUILD}/analyzers/InferenceObjectModel.o ${BUILD}/analyzers/InferenceEnvironment.o
CODEGEN=${BUILD}/codegen/codegen.o ${BUILD}/codegen/LLVMFunctionCompiler.o  ${BUILD}/codegen/LLVMModuleCompiler.o ${BUILD}/codegen/LLVMJit.o
${BIN}/coral-core: ${COREFILES} ${BUILD}/core/main_core.o
	${LINKCMD}
${BIN}/coral-token: ${COREFILES} ${PARSERFILES} ${BUILD}/parser/main_tokens.o
	${LINKCMD}
${BIN}/coral-parse: ${COREFILES} ${PARSERFILES} ${ANALYZERS} ${BUILD}/parser/main_parse.o
	${LINKCMD}
${BIN}/coral-jit: ${COREFILES} ${PARSERFILES} ${ANALYZERS} ${CODEGEN} ${BUILD}/codegen/main_jit.o
	${LINKCMD} $(shell llvm-config-5.0 --libs) -rdynamic -lpcre2-8 -ldl -lSDL2
${BIN}/coralc: ${COREFILES} ${PARSERFILES} ${ANALYZERS} ${CODEGEN} ${BUILD}/codegen/main_compile.o
	${LINKCMD} $(shell llvm-config-5.0 --libs) -rdynamic -lpcre2-8 -ldl
${BIN}/coral-test: ${COREFILES} ${PARSERFILES} ${ANALYZERS} ${CODEGEN} ${BUILD}/tests/runner/main_test.o ${BUILD}/tests/runner/codegen.o ${BUILD}/tests/runner/parser.o
	${LINKCMD} $(shell llvm-config-5.0 --libs) -ldl -rdynamic -lpcre2-8
${BIN}/coral-types: ${COREFILES} ${BUILD}/core/type.o
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
	${CC} -MM -o $@.d -MT '$@ $@.d' $<
	${CC} -c -g -o $@ $<
${BUILD}/tests/runner/%.o: tests/runner/%.cc
	@mkdir -p $(dir $@)
	${LLVM_CC} -MM -o $@.d -MT '$@ $@.d' $<
	${LLVM_CC} -c -g -o $@ $<
${BUILD}/codegen/%.o: codegen/%.cc
	@mkdir -p $(dir $@)
	${LLVM_CC} -MM -o $@.d -MT '$@ $@.d' $<
	${LLVM_CC} -c -g -o $@ $<

-include ${BUILD}/**/*.d
