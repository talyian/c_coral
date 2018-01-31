default: main
	./$<

flexLexer.o : bisonParser.tab.hh
flexLexer.cc : flexLexer.l
	flex -o $@ $<

bisonParser.tab.hh : bisonParser.tab.cc
bisonParser.tab.cc : bisonParser.yy
	bison -d -o $@ $<

%.o : %.cc
	clang++-5.0 -c -o $@ $<

main: flexLexer.o bisonParser.tab.o
	clang++-5.0 -o $@ $^
