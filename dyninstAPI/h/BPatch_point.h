/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _BPatch_point_h_
#define _BPatch_point_h_

#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_eventLock.h"
#include "BPatch_snippet.h" // snippetOrder
#include "BPatch_Set.h"

class InstrucIter;
class process;
class instPoint;
class miniTramp;
class BPatch_thread;
class BPatch_image;
class BPatch_function;
class BPatch_memoryAccess;
class BPatchSnippetHandle;
class BPatch_basicBlockLoop;
class BPatch_process;
class BPatch_frame;
class BPatch_edge;

#include "Instruction.h"



/*
 * Used to specify whether a snippet is to be called before the instructions
 * at the point where it is inserted, or after.
 */
typedef enum {
    BPatch_callBefore,
    BPatch_callAfter,
    BPatch_callUnset
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
    BPatch_locUnknownLocation,
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

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_point

class BPATCH_DLL_EXPORT BPatch_point : public BPatch_eventLock {
    friend class BPatch_process;
    friend class BPatch_binaryEdit;
    friend class BPatch_addressSpace;
    friend class BPatch_image;
    friend class BPatch_function;
    friend class BPatch_basicBlock;
    friend class BPatch_basicBlockLoop;
    friend class BPatch_flowGraph; // Access to setLoop
    friend class BPatch_asyncEventHandler;
    friend class process;
    friend class BPatch_edge;
    friend class BPatch_snippet;
    
    static BPatch_point* createInstructionInstPoint(//BPatch_process *proc,
                                                    BPatch_addressSpace *addSpace,
						    void*address,
                                                    BPatch_function* bpf = NULL);
    // Create a set of points, all that match a given op in the given instruciter.

    static BPatch_Vector<BPatch_point *> *getPoints(const BPatch_Set<BPatch_opCode> &ops,
                                                    InstrucIter &ii,
                                                    BPatch_function *bpf);

    BPatch_addressSpace *addSpace;
    AddressSpace *lladdSpace;
    BPatch_function	*func;
    BPatch_basicBlockLoop *loop;
    instPoint	*point;

    BPatch_procedureLocation pointType;
    BPatch_memoryAccess *memacc;
    // Instruction constructor...
    BPatch_point(BPatch_addressSpace *_addSpace, BPatch_function *_func, 
                 instPoint *_point, BPatch_procedureLocation _pointType,
                 AddressSpace *as);

    // Edge constructor...
    BPatch_point(BPatch_addressSpace *_addSpace, BPatch_function *_func,
                 BPatch_edge *_edge, instPoint *_point, AddressSpace *as);


    void setLoop(BPatch_basicBlockLoop *l);

    // We often create a point with the arbitrary point type,
    // and later need to override it to a specific type (e.g., loop entry)
    void overrideType(BPatch_procedureLocation loc) { pointType = loc; }

    //  dynamic_call_site_flag:
    //    0:  is not dynamic call site
    //    1:  is dynamic call site
    //    2:  dynamic status unknown (initial value)
    int dynamic_call_site_flag;

    //  a snippet used in monitoring of dynamic calls
    //  maybe we want BPatchSnippetHandle here
    miniTramp *dynamic_point_monitor_func;

    instPoint * getPoint() {return point;}

    BPatch_Vector<BPatchSnippetHandle *> preSnippets;
    BPatch_Vector<BPatchSnippetHandle *> postSnippets;
    BPatch_Vector<BPatchSnippetHandle *> allSnippets;

    // If we're edge inst
    BPatch_edge *edge_;

    void recordSnippet(BPatch_callWhen, BPatch_snippetOrder,
                       BPatchSnippetHandle*);
    bool deleteSnippet(BPatchSnippetHandle* handle);

    void attachMemAcc(BPatch_memoryAccess *memacc);

    AddressSpace *getAS();

public:
    //~BPatch_point() { delete memacc; };

    // Internal functions, DO NOT USE.
    // Hack to get edge information. DO NOT USE.
    const BPatch_edge *edge() const { return edge_; }
    bool isReturnInstruction();
    static BPatch_procedureLocation convertInstPointType_t(int intType);
    instPoint *llpoint() { return point; } 
    bool getCFTargets(BPatch_Vector<Dyninst::Address> &targets);
    Dyninst::Address getCallFallThroughAddr();
    void setResolved();
    Dyninst::Address getSavedTarget();
    // End internal functions

    //Added for DynC...
    API_EXPORT(Int, (),
	       
    BPatch_addressSpace *, getAddressSpace, ());
    

    // Get the loop ID
    API_EXPORT(Int, (),
	       
    BPatch_basicBlockLoop *, getLoop, ());

    //  BPatch_point::getPointType
    //  
    API_EXPORT(Int, (),
    BPatch_procedureLocation,getPointType,());

    //  BPatch_point::getFunction
    //  Returns function to which this point belongs

    API_EXPORT(Int, (),

    BPatch_function *,getFunction,());

    //  BPatch_point::getCalledFunction
    //  Returns a BPatch_function representing the function being called at this point.
    //  If this point is not a call site, returns NULL. 

    API_EXPORT(Int, (),

    BPatch_function *,getCalledFunction,());

    API_EXPORT(Int, (),
    std::string,getCalledFunctionName,());

    //  BPatch_point::getAddress
    //  Returns the address of this point in the mutatee

    API_EXPORT(Int, (),

    void *,getAddress,());

    //  BPatch_point::getMemoryAccess
    //  

    API_EXPORT(Int, (),

    const BPatch_memoryAccess *,getMemoryAccess,());

    API_EXPORT(Int, (),
	       Dyninst::InstructionAPI::Instruction::Ptr, getInsnAtPoint, ());


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


    //  BPatch_point::getLiveRegisters
    //  
    //  Get Live registers at this point

    API_EXPORT(Int, (liveRegs),

    bool, getLiveRegisters, (std::vector<BPatch_register> &liveRegs));

  
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

    void *,monitorCalls,(BPatch_function *f = NULL));

    //  BPatch_point::stopMonitoring
    //  If this point, as a dynamic call site, was being monitored, turns off monitoring
    //  <handle> is the handle returned my monitorCalls()

    API_EXPORT(Int, (),

    bool,stopMonitoring,());

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
