/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#ifndef _BPatch_point_h_
#define _BPatch_point_h_

#include "BPatch_dll.h"
#include "BPatch_Vector.h"

class process;
class instPoint;
class BPatch_thread;
class BPatch_image;
class BPatch_function;
class BPatch_memoryAccess;
class BPatchSnippetHandle;

/*
 * Used to specify whether a snippet is to be called before the instructions
 * at the point where it is inserted, or after.
 */
typedef enum {
    BPatch_callBefore,
    BPatch_callAfter
} BPatch_callWhen;

/*
 * Provide these definitions for backwards compatability.
 *
 */
#define BPatch_entry BPatch_locEntry
#define BPatch_exit BPatch_locExit
#define BPatch_subroutine BPatch_locSubroutine
#define BPatch_longJump	BPatch_locLongJump
#define BPatch_allLocations BPatch_locAllLocations
/* #define BPatch_instruction BPatch_locInstruction */
#define BPatch_arbitrary BPatch_locInstruction

/*
 * Used with BPatch_function::findPoint to specify which of the possible
 * instrumentation points within a procedure should be returned.
 */
typedef enum eBPatch_procedureLocation {
    BPatch_locEntry,
    BPatch_locExit,
    BPatch_locSubroutine,
    BPatch_locLongJump,
    BPatch_locAllLocations,
    BPatch_locInstruction,
#ifdef IBM_BPATCH_COMPAT
    BPatch_locUnknownLocation,
#endif
    BPatch_locSourceBlockEntry,		// not yet used
    BPatch_locSourceBlockExit,		// not yet used
    BPatch_locSourceLoopEntry,		// not yet used
    BPatch_locSourceLoopExit,		// not yet used
    BPatch_locBasicBlockEntry,		// not yet used
    BPatch_locBasicBlockExit,		// not yet used
    BPatch_locSourceLoop,		// not yet used
    BPatch_locBasicBlockLoopEntry,	// not yet used
    BPatch_locBasicBlockLoopExit,	// not yet used
    BPatch_locVarInitStart,		// not yet used
    BPatch_locVarInitEnd,		// not yet used
    BPatch_locStatement		// not yet used
} BPatch_procedureLocation;


/* VG (09/07/01) Created */

typedef enum eBPatch_opCode {
  BPatch_opLoad,
  BPatch_opStore,
  BPatch_opPrefetch
} BPatch_opCode;

/* VG(09/17/01) Added memory access pointer */

/* VG(11/06/01) Moved constructor to implementation file because it
   needs to link instPoint back pointer (and we don't want to include
   that here) */
class BPATCH_DLL_EXPORT BPatch_point {
    friend class BPatch_thread;
    friend class BPatch_image;
    friend class BPatch_function;
    friend class process;
    friend BPatch_point* createInstructionInstPoint(process*proc,void*address,
						    BPatch_point** alternative,
						    BPatch_function* bpf = NULL);
    friend BPatch_point* createInstPointForMemAccess(process *proc,
						     void *addr,
						     BPatch_memoryAccess* ma,
						     BPatch_point** alternative);
    process	*proc;
    const BPatch_function	*func;
    instPoint	*point;
    BPatch_procedureLocation pointType;
    BPatch_memoryAccess *memacc;

    BPatch_point(process *_proc, BPatch_function *_func, instPoint *_point,
		 BPatch_procedureLocation _pointType, BPatch_memoryAccess* _ma = NULL);
public:
    //~BPatch_point() { delete memacc; };

    const BPatch_procedureLocation getPointType() { return pointType; }
    const BPatch_function *getFunction() { return func; }
    BPatch_function *getCalledFunction();
    void            *getAddress();
    const BPatch_memoryAccess* getMemoryAccess() const { return memacc; }

    // to get all current snippets at this point
    const BPatch_Vector<BPatchSnippetHandle *> getCurrentSnippets();

    // to get all current snippets as defined by when at this point
    const BPatch_Vector<BPatchSnippetHandle *> 
               getCurrentSnippets(BPatch_callWhen when);

#ifdef IBM_BPATCH_COMPAT
    void *getPointAddress() { return getAddress(); }

    int	getPointLine() { return -1; }

    BPatch_function     *getContainingFunction() { return const_cast<BPatch_function*>(getFunction()); };
#endif

    int             getDisplacedInstructions(int maxSize, void *insns);

    bool	usesTrap_NP();
};

#endif /* _BPatch_point_h_ */
