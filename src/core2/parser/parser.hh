// C api for Coral Parser
#pragma once

#ifndef ParserType
#define ParserType void *
#endif

#ifndef ModuleType
#define ModuleType void *
#endif

extern "C" ParserType coralParseModule(const char * infile);
extern "C" void coralJsonModule(ParserType m, const char * outfile);
extern "C" void coralDestroyModule(ParserType m);
extern "C" ModuleType _coralModule(ParserType m);
