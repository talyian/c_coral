// C api for Coral Parser
#pragma once

#ifndef ParserModule
#define ParserModule void *
#endif

extern "C" ParserModule coralParseModule(const char * infile);
extern "C" void coralJsonModule(ParserModule m, const char * outfile);
extern "C" void coralDestroyModule(ParserModule m);
