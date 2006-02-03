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

#if DYNINST_VER > 4
#include "BPatch_process.h"
#else
#define BPatch_process BPatch_thread
#endif


struct dynHandle {
    BPatch *bpatch;
    BPatch_process *proc;
    BPatch_image *image;
};

dynHandle *mutatorInit(void);
bool dynStartTransaction(dynHandle *);
bool dynEndTransaction(dynHandle *);
bool dynContinueExecution(dynHandle *);
bool dynIsTerminated(dynHandle *);
BPatch_exitType dynTerminationStatus(dynHandle *);
int dynGetExitCode(dynHandle *);
int dynGetExitSignal(dynHandle *);
void dynTerminateExecution(dynHandle *);

#endif
