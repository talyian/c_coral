#depends on core.Makefile
PARSERFILES=obj/parsing/generated/lexer.o obj/parsing/generated/parser.o obj/parsing/parse.o

parser: bin/coralp bin/coralt bin/test-coral-parse

# binaries for parsing
bin/coralp: ${COREFILES} ${PARSERFILES} obj/parsing/__main_parse.o
	${LINK} -o $@ $^

bin/coralt: ${COREFILES} ${PARSERFILES} obj/parsing/__main_token.o
	${LINK} -o $@ $^

bin/test-coral-parse: ${COREFILES} ${PARSERFILES} \
  obj/tests/parsing/test_parser.o obj/tests/parsing/__main__.o
	${LINK} -o $@ $^

# flex/bison based rules
src/parsing/generated/parser.hh: src/parsing/generated/parser.cc
	@

src/parsing/generated/parser.cc: src/parsing/parser.yy
	@mkdir -p $(shell dirname $@)
	bison --defines=$(shell dirname $@)/parser.hh -o $@ $<

src/parsing/generated/lexer.cc: src/parsing/lexer.l src/parsing/generated/parser.hh
	@mkdir -p $(shell dirname $@)
	flex -o $@ $<
