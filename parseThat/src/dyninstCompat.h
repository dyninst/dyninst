#ifndef __DYNINSTCOMPAT_H__
#define __DYNINSTCOMPAT_H__

/*******************************************************************************
 * This file, and the accompaning dyninstCompat.vX.C files are simply to
 * get around incompatabilities between DyninstAPI versions.  Hopefully,
 * when DyninstAPI v4 support is dropped, we can merge these files into
 * dyninstCore.[Ch]
 */

#include "BPatch.h"
#include "BPatch_thread.h"
#include "BPatch_image.h"
#include "BPatch_Vector.h"

#if defined(HAVE_BPATCH_PROCESS_H)
#include "BPatch_process.h"
#else
#define BPatch_process BPatch_thread
#endif

struct dynHandle {
  BPatch *bpatch;
  BPatch_addressSpace *addSpace;
  BPatch_process *proc;
  BPatch_image *image;
};

dynHandle *mutatorInit(void);
bool dynStartTransaction(dynHandle *);
bool dynEndTransaction(dynHandle *);

#endif
