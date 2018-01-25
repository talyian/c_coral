COREFILES=obj/core/expr.o obj/core/type.o obj/core/treeprinter.o

bin/coral-core: ${COREFILES} obj/core/__main__.o
	${LINK} -o $@ $^
