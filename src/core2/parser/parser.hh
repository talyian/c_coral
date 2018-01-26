// C api for parser
#pragma once

#ifndef Module
#define Module void *
#endif

extern "C" Module coralParseModule(char * infile);
extern "C" void coralJsonModule(Module m, char * outfile);
extern "C" void coralDestroyModule(Module m);
