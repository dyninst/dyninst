// whereAxisMisc.h
// Ariel Tamches

#ifndef _WHERE_AXIS_MISC_H_
#define _WHERE_AXIS_MISC_H_

#ifndef PARADYN
#include <fstream.h>

// this is only for the where axis test program:
string readUntilQuote(ifstream &is);
string readUntilSpace(ifstream &is);
string readItem(ifstream &is);
#endif

#endif
