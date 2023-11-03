/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

#include <string>
#include <vector>
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_Set.h"
#include "BPatch_enums.h"
#include "dyntypes.h"

class instPoint;
class BPatch_thread;
class BPatch_image;
class BPatch_function;
class BPatch_memoryAccess;
class BPatchSnippetHandle;
class BPatch_basicBlock;
class BPatch_basicBlockLoop;
class BPatch_point;
class BPatch_process;
class BPatch_frame;
class BPatch_edge;
class BPatch_snippet;
class BPatch_addressSpace;
class AddressSpace;
class BPatch_register;

#include "Instruction.h"

namespace Dyninst {
   namespace PatchAPI {
      class Instance;
      class Point;
      typedef boost::shared_ptr<Instance> InstancePtr;
      BPATCH_DLL_EXPORT Point *convert(const BPatch_point *, BPatch_callWhen);
   }
}

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

/* VG(09/17/01) Added memory access pointer */

/* VG(11/06/01) Moved constructor to implementation file because it
   needs to link instPoint back pointer (and we don't want to include
   that here) */

class BPATCH_DLL_EXPORT BPatch_point {
    friend class BPatch_process;
    friend class BPatch_binaryEdit;
    friend class BPatch_addressSpace;
    friend class BPatch_image;
    friend class BPatch_function;
    friend class BPatch_basicBlock;
    friend class BPatch_basicBlockLoop;
    friend class BPatch_flowGraph; // Access to setLoop
    friend class BPatch_asyncEventHandler;
    friend class BPatch_edge;
    friend class BPatch_snippet;
    friend Dyninst::PatchAPI::Point *Dyninst::PatchAPI::convert(const BPatch_point *, BPatch_callWhen);

private:
    
    BPatch_addressSpace *addSpace;
    AddressSpace *lladdSpace;
    BPatch_function	*func;
    BPatch_basicBlockLoop *loop;

    // We have a disconnect between how BPatch represents a point
    // (e.g., an instruction) and how the internals represent a point
    // (e.g., pre-instruction). We handle this here with a secondary
    // instPoint that is defined to be the "after" equivalent. 
    instPoint	*point;
    instPoint   *secondaryPoint;

    BPatch_procedureLocation pointType;
    BPatch_memoryAccess *memacc;
    // Instruction constructor...
    BPatch_point(BPatch_addressSpace *_addSpace, BPatch_function *_func, 
                 instPoint *_point,  instPoint *_secondary,
                 BPatch_procedureLocation _pointType,
                 AddressSpace *as);

    // Edge constructor...
    BPatch_point(BPatch_addressSpace *_addSpace, BPatch_function *_func,
                 BPatch_edge *_edge, instPoint *_point, AddressSpace *as);


    void setLoop(BPatch_basicBlockLoop *l);

    // We often create a point with the arbitrary point type,
    // and later need to override it to a specific type (e.g., loop entry)
    void overrideType(BPatch_procedureLocation loc) { pointType = loc; }

    //  a snippet used in monitoring of dynamic calls
    //  maybe we want BPatchSnippetHandle here
    Dyninst::PatchAPI::InstancePtr dynamic_point_monitor_func;

    instPoint *getPoint() const {return point;}
    instPoint *getPoint(BPatch_callWhen when) const;

    // If we're edge inst
    BPatch_edge *edge_;

    void recordSnippet(BPatch_callWhen, BPatch_snippetOrder,
                       BPatchSnippetHandle*);
    bool removeSnippet(BPatchSnippetHandle* handle);

    void attachMemAcc(BPatch_memoryAccess *memacc);

    AddressSpace *getAS();

private:
    BPatch_Vector<BPatchSnippetHandle *> preSnippets;
    BPatch_Vector<BPatchSnippetHandle *> postSnippets;
    BPatch_Vector<BPatchSnippetHandle *> allSnippets;

public:
    //~BPatch_point() { delete memacc; };

    // Internal functions, DO NOT USE.
    // Hack to get edge information. DO NOT USE.
    BPatch_edge *edge() const { return edge_; }
    bool isReturnInstruction();
    static BPatch_procedureLocation convertInstPointType_t(int intType);
    instPoint *llpoint() { return point; } 
    Dyninst::Address getCallFallThroughAddr();
    bool patchPostCallArea();
    // End internal functions

    //Added for DynC...
	       
    BPatch_addressSpace * getAddressSpace();
    

    // Get the loop ID
	       
    BPatch_basicBlockLoop * getLoop();

    //  BPatch_point::getPointType
    //  
    BPatch_procedureLocation getPointType();

    //  BPatch_point::getFunction
    //  Returns function to which this point belongs


    BPatch_function * getFunction();

    //  BPatch_point::getCalledFunction
    //  Returns a BPatch_function representing the function being called at this point.
    //  If this point is not a call site, returns NULL. 


    BPatch_function * getCalledFunction();

    std::string getCalledFunctionName();

    //  BPatch_point::getBlock
    //  Returns block to which this point belongs if such a block exists
    //  For example, function entry points do not have blocks associated with them.


    BPatch_basicBlock * getBlock();


    //  BPatch_point::getAddress
    //  Returns the address of this point in the mutatee


    void * getAddress();

    //  BPatch_point::getMemoryAccess
    //  


    const BPatch_memoryAccess * getMemoryAccess();

    Dyninst::InstructionAPI::Instruction getInsnAtPoint();


    //  BPatch_point::getCurrentSnippets
    //  
    // to get all current snippets at this point


    const BPatch_Vector<BPatchSnippetHandle *> getCurrentSnippets();

    //  BPatch_point::getCurrentSnippets
    //  
    // to get all current snippets as defined by when at this point

    const BPatch_Vector<BPatchSnippetHandle *> getCurrentSnippets(BPatch_callWhen when);

    //  BPatch_point::getLiveRegisters
    //  
    //  Get Live registers at this point


    bool getLiveRegisters(std::vector<BPatch_register> &liveRegs);

  
    //  BPatch_point::isDynamic
    //  
    //  isDynamic() returns true if this is a dynamic call site
    //  (eg a call site where a func call is made via func ptr)


    bool isDynamic();

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


    void * monitorCalls(BPatch_function *f = NULL);

    //  BPatch_point::stopMonitoring
    //  If this point  as a dynamic call site, was being monitored, turns off monitoring
    //  <handle> is the handle returned my monitorCalls()


    bool stopMonitoring();

    //  BPatch_point::getDisplacedInstructions
    //  Returns the instructions to be relocated when instrumentation is inserted
    //  at this point.  Returns the number of bytes taken up by these instructions.


    int getDisplacedInstructions(int maxSize, void *insns);

    //  BPatch_point::usesTrap_NP
    //  Returns true if this point is or would be instrumented with a trap, rather
    //  than a jump to the base tramp, false otherwise.  On platforms that do not
    //   use traps (everything other than x86), it always returns false;


    bool usesTrap_NP();
};

#endif /* _BPatch_point_h_ */
