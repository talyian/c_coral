// C api for parser
#pragma once

#ifndef Module
#define Module void *
#endif

extern "C" Module coralParseModule(const char * infile);
extern "C" void coralJsonModule(Module m, const char * outfile);
extern "C" void coralDestroyModule(Module m);
