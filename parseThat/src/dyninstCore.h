#ifndef __DYNINSTCORE_H_
#define __DYNINSTCORE_H_

#include "BPatch_function.h"

int launch_monitor(FILE *);
int launch_mutator();

struct dynHandle;

dynHandle *mutatorInit(void);
bool parseInit(dynHandle *);

bool dynStartTransaction(dynHandle *);
bool dynEndTransaction(dynHandle *);
bool instrumentFunctionEntry(dynHandle *, BPatch_function *func);
bool instrumentFunctionExit(dynHandle *, BPatch_function *func);
bool instrumentBasicBlocks(dynHandle *, BPatch_function *func);

#include "dyninstCompat.h"

#endif
