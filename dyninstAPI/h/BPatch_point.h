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

#define BPatch_entry BPatch_locEntry
#define BPatch_exit BPatch_locExit
#define BPatch_subroutine BPatch_locSubroutine
#define BPatch_longJump	BPatch_locLongJump
#define BPatch_allLocations BPatch_locAllLocations
/* #define BPatch_instruction BPatch_locInstruction */
#define BPatch_arbitrary BPatch_locInstruction

class BPATCH_DLL_EXPORT BPatch_point {
    friend class BPatch_process;
    friend class BPatch_binaryEdit;
    friend class BPatch_addressSpace;
    friend class BPatch_image;
    friend class BPatch_function;
    friend class BPatch_basicBlock;
    friend class BPatch_basicBlockLoop;
    friend class BPatch_flowGraph;
    friend class BPatch_asyncEventHandler;
    friend class BPatch_edge;
    friend class BPatch_snippet;
    friend Dyninst::PatchAPI::Point *Dyninst::PatchAPI::convert(const BPatch_point *, BPatch_callWhen);

private:
    
    BPatch_addressSpace *addSpace;
    AddressSpace *lladdSpace;
    BPatch_function	*func;
    BPatch_basicBlockLoop *loop;

    instPoint	*point;
    instPoint   *secondaryPoint;

    BPatch_procedureLocation pointType;
    BPatch_memoryAccess *memacc;

    BPatch_point(BPatch_addressSpace *_addSpace, BPatch_function *_func, 
                 instPoint *_point,  instPoint *_secondary,
                 BPatch_procedureLocation _pointType,
                 AddressSpace *as);


    BPatch_point(BPatch_addressSpace *_addSpace, BPatch_function *_func,
                 BPatch_edge *_edge, instPoint *_point, AddressSpace *as);


    void setLoop(BPatch_basicBlockLoop *l);

    void overrideType(BPatch_procedureLocation loc) { pointType = loc; }

    Dyninst::PatchAPI::InstancePtr dynamic_point_monitor_func;

    instPoint *getPoint() const {return point;}
    instPoint *getPoint(BPatch_callWhen when) const;

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

    BPatch_edge *edge() const { return edge_; }
    bool isReturnInstruction();
    static BPatch_procedureLocation convertInstPointType_t(int intType);
    instPoint *llpoint() { return point; } 
    Dyninst::Address getCallFallThroughAddr();
    bool patchPostCallArea();

    BPatch_addressSpace * getAddressSpace();
    

    BPatch_basicBlockLoop * getLoop();

    BPatch_procedureLocation getPointType();

    BPatch_function * getFunction();

    BPatch_function * getCalledFunction();

    std::string getCalledFunctionName();

    BPatch_basicBlock * getBlock();

    void * getAddress();

    const BPatch_memoryAccess * getMemoryAccess();

    Dyninst::InstructionAPI::Instruction getInsnAtPoint();

    const BPatch_Vector<BPatchSnippetHandle *> getCurrentSnippets();

    const BPatch_Vector<BPatchSnippetHandle *> getCurrentSnippets(BPatch_callWhen when);

    bool getLiveRegisters(std::vector<BPatch_register> &liveRegs);

  
    bool isDynamic();

    void * monitorCalls(BPatch_function *f = NULL);

    bool stopMonitoring();

    int getDisplacedInstructions(int maxSize, void *insns);

    bool usesTrap_NP();
};

#endif /* _BPatch_point_h_ */
