/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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
#include "BPatch_eventLock.h"

class process;
class instPoint;
class miniTrampHandle;
class BPatch_thread;
class BPatch_image;
class BPatch_function;
class BPatch_memoryAccess;
class BPatchSnippetHandle;
class BPatch_basicBlockLoop;
class BPatch_process;

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

#if defined (IBM_BPATCH_COMPAT)
#define BPatch_locBasicBlockLoopEntry BPatch_locLoopEntry
#define BPatch_locBasicBlockLoopExit BPatch_locLoopExit
#endif
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
    BPatch_locLoopEntry,	
    BPatch_locLoopExit,
    BPatch_locLoopStartIter,
    BPatch_locLoopEndIter,
    BPatch_locVarInitStart,		// not yet used
    BPatch_locVarInitEnd,		// not yet used
    BPatch_locStatement		// not yet used
} BPatch_procedureLocation;


/* VG (09/07/01) Created */

typedef enum BPatch_opCode {
  BPatch_opLoad,
  BPatch_opStore,
  BPatch_opPrefetch
} BPatch_opCode;

/* VG(09/17/01) Added memory access pointer */

/* VG(11/06/01) Moved constructor to implementation file because it
   needs to link instPoint back pointer (and we don't want to include
   that here) */

class BPatch_point;
#if defined( __XLC__ ) || defined(__xlC__)
BPatch_point* createInstructionInstPoint(process*proc,void*address,
                                         BPatch_point** alternative,
                                         BPatch_function* bpf = NULL);
#endif

typedef void (*BPatchDynamicCallSiteCallback)(BPatch_point *at_point, 
                                              BPatch_function *called_function);
typedef struct {
  BPatchDynamicCallSiteCallback cb;
  int handle;
} callback_record;

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_point

class BPATCH_DLL_EXPORT BPatch_point : public BPatch_eventLock {
    friend class BPatch_process;
    friend class BPatch_image;
    friend class BPatch_function;
    friend class BPatch_basicBlockLoop;
    friend class BPatch_flowGraph; // Access to setLoop
    friend class BPatch_asyncEventHandler;
    friend class process;
    friend class BPatch_edge;
#if !defined (__XLC__) && !defined(__xlC__)
    friend BPatch_point* createInstructionInstPoint(BPatch_process*proc,
                                                    void*address,
                                                    BPatch_point** alternative,
                                                    BPatch_function* bpf = NULL);
#endif
    friend BPatch_point* createInstPointForMemAccess(BPatch_process *_proc,
						     void *addr,
						     BPatch_memoryAccess* ma,
						     BPatch_point** alternative);
    BPatch_process *proc;
    const BPatch_function	*func;
    BPatch_basicBlockLoop *loop;
    instPoint	*point;

    BPatch_procedureLocation pointType;
    BPatch_memoryAccess *memacc;

    BPatch_point(BPatch_process *_proc, BPatch_function *_func, 
                 instPoint *_point, BPatch_procedureLocation _pointType, 
                 BPatch_memoryAccess* _ma = NULL);
    void setLoop(BPatch_basicBlockLoop *l);

    // We often create a point with the arbitrary point type,
    // and later need to override it to a specific type (e.g., loop entry)
    void overrideType(BPatch_procedureLocation loc) { pointType = loc; }

    //  dynamic_call_site_flag:
    //    0:  is not dynamic call site
    //    1:  is dynamic call site
    //    2:  dynamic status unknown (initial value)
    int dynamic_call_site_flag;

    BPatch_Vector<miniTrampHandle *> dynamicMonitoringCalls; // This should be BPatchSnippetHandle

    instPoint * getPoint() {return point;}
public:
    //~BPatch_point() { delete memacc; };

    //  This function should go away when paradyn lives on top of dyninst
    //  DO NOT USE
    instPoint * PDSEP_instPoint() {return point;}


    // Get the loop ID
    API_EXPORT(Int, (),
	       
    BPatch_basicBlockLoop *, getLoop, ());

    //  BPatch_point::getPointType
    //  
    API_EXPORT(Int, (),

    const BPatch_procedureLocation,getPointType,());

    //  BPatch_point::getFunction
    //  Returns function to which this point belongs

    API_EXPORT(Int, (),

    const BPatch_function *,getFunction,());

    //  BPatch_point::getCalledFunction
    //  Returns a BPatch_function representing the function being called at this point.
    //  If this point is not a call site, returns NULL. 

    API_EXPORT(Int, (),

    BPatch_function *,getCalledFunction,());

    //  BPatch_point::getAddress
    //  Returns the address of this point in the mutatee

    API_EXPORT(Int, (),

    void *,getAddress,());

    //  BPatch_point::getMemoryAccess
    //  

    API_EXPORT(Int, (),

    const BPatch_memoryAccess *,getMemoryAccess,());

    //  BPatch_point::getCurrentSnippets
    //  
    // to get all current snippets at this point

    API_EXPORT(Int, (),

    const BPatch_Vector<BPatchSnippetHandle *>,getCurrentSnippets,());

    //  BPatch_point::getCurrentSnippets
    //  
    // to get all current snippets as defined by when at this point

    API_EXPORT(ByWhen, (when),

    const BPatch_Vector<BPatchSnippetHandle *>,getCurrentSnippets,(BPatch_callWhen when));
      
    //  BPatch_point::isDynamic
    //  
    //  isDynamic() returns true if this is a dynamic call site
    //  (eg a call site where a func call is made via func ptr)

    API_EXPORT(Int, (),

    bool,isDynamic,());

    //  BPatch_point::monitorCalls
    //  
    //  monitorCalls(BPatch_function *cbFuncInUserLibrary)
    //  Applies only to dynamic call sites (returns false if this BPatch_point
    //  is not a dynamic call site).  Inserts a call to the user-written function
    //  cbFuncInUserLibrary(), which must exist in a user defined library,
    //  at this call site.  cbFuncInUserLibrary must adhere to the prototype:
    //  void cbFuncInUserLibrary(void *callee_address, void *callSite_address).
    //
    // Returns handle on success, NULL on failure

    API_EXPORT(Int, (f),

    void *,monitorCalls,(BPatch_function *f));

    //  BPatch_point::stopMonitoring
    //  If this point, as a dynamic call site, was being monitored, turns off monitoring
    //  <handle> is the handle returned my monitorCalls()

    API_EXPORT(Int, (handle),

    bool,stopMonitoring,(void *handle));

    //  BPatch_point::registerDynamicCallCallback
    //  Specifies a user-supplied function to be called when a dynamic call is
    //  executed.
    //
    //  Returns a handle (useful for de-registering callback), NULL if error

    API_EXPORT(Int, (cb),

    void *,registerDynamicCallCallback,(BPatchDynamicCallSiteCallback cb));

    //  BPatch_point::removeDynamicCallCallback
    //  Argument is (void *) handle to previously specified callback function to be
    //  de-listed.
    //
    //  Returns true upon success, false if handle is not currently represented

    API_EXPORT(Int, (handle),

    bool,removeDynamicCallCallback,(void *handle));

    //  BPatch_point::getDisplacedInstructions
    //  Returns the instructions to be relocated when instrumentation is inserted
    //  at this point.  Returns the number of bytes taken up by these instructions.

    API_EXPORT(Int, (maxSize, insns),

    int,getDisplacedInstructions,(int maxSize, void *insns));

    //  BPatch_point::usesTrap_NP
    //  Returns true if this point is or would be instrumented with a trap, rather
    //  than a jump to the base tramp, false otherwise.  On platforms that do not
    //   use traps (everything other than x86), it always returns false;

    API_EXPORT(Int, (),

    bool,usesTrap_NP,());

#ifdef IBM_BPATCH_COMPAT
    void *getPointAddress() { return getAddress(); }
    int getPointLine() { return -1; }
    BPatch_function *getContainingFunction() { return const_cast<BPatch_function*>(getFunction()); };
#endif

};

#endif /* _BPatch_point_h_ */
