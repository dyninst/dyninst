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

// Node class implementation

#include "Graph.h"
#include "Absloc.h"
#include "Edge.h"
#include "Node.h"
#include <assert.h>

#include "Register.h"
#include "InstructionAST.h"
#include "Instruction.h"
#include "Expression.h"

#include "dyninstAPI/src/stackanalysis.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/image-func.h"

#include "BPatch_function.h"

using namespace Dyninst;
using namespace Dyninst::DepGraphAPI;
using namespace Dyninst::InstructionAPI;
using namespace dyn_detail::boost;

void Absloc::getAbslocs(AbslocSet &locs) {
    RegisterLoc::getRegisterLocs(locs);
    StackLoc::getStackLocs(locs);
    //HeapLoc::getHeapLocs(locs);
    MemLoc::getMemLocs(locs);
    ImmLoc::getImmLocs(locs);
}

RegisterLoc::RegisterMap RegisterLoc::allRegLocs_;

void RegisterLoc::getRegisterLocs(AbslocSet &locs) {
    for (RegisterMap::iterator iter = allRegLocs_.begin(); iter != allRegLocs_.end(); iter++) {
        locs.insert((*iter).second);
    }
}

Absloc::Ptr RegisterLoc::getRegLoc(const InstructionAPI::RegisterAST::Ptr reg) {
    // Upconvert the register to its canonical container
    InstructionAPI::RegisterAST::Ptr container = dynamic_pointer_cast<InstructionAPI::RegisterAST>(RegisterAST::promote(reg));
    if (!container) {
        assert(0);
        return Absloc::Ptr();
    }

    // Look up by name and return    
    if (allRegLocs_.find(*container) == allRegLocs_.end()) {
        RegisterLoc::Ptr rP = RegisterLoc::Ptr(new RegisterLoc(container));

        allRegLocs_[*container] = rP;
    }
    
    return allRegLocs_[*container];
}

const int StackLoc::STACK_GLOBAL = MININT;

StackLoc::StackMap StackLoc::stackLocs_;

std::string StackLoc::name() const {
    if (slot_ == STACK_GLOBAL) {
        return std::string("STACK");
    }
    else {
        char buf[256];
        sprintf(buf, "STACK_%d", slot_);
        return std::string(buf);
    }
}

void StackLoc::getStackLocs(AbslocSet &locs) {
    for (StackMap::iterator iter = stackLocs_.begin();
         iter != stackLocs_.end(); iter++) {
        locs.insert((*iter).second);
    }
}
Absloc::Ptr StackLoc::getStackLoc(int slot) {
    if (stackLocs_.find(slot) == stackLocs_.end()) {
        StackLoc::Ptr sP = StackLoc::Ptr(new StackLoc(slot));
        
        stackLocs_[slot] = sP;
    }
    
    return stackLocs_[slot];
}

Absloc::Ptr StackLoc::getStackLoc() {
    // Look up by name and return    
    return getStackLoc(STACK_GLOBAL);
}

StackLoc::AbslocSet StackLoc::getAliases() const{
    AbslocSet ret;

    // If we're a specific stack slot, return the generic
    // stack slot(s)
    if (slot_ == STACK_GLOBAL) {
        for (StackMap::const_iterator iter = stackLocs_.begin();
             iter != stackLocs_.end(); iter++) {
            if ((*iter).first != STACK_GLOBAL)
                ret.insert((*iter).second);
        }
    } 
    else {
        ret.insert(getStackLoc());
    }
    
    // Global memory references might touch the stack...
    ret.insert(MemLoc::getMemLoc());

    return ret;
}

#if 0
Absloc::Ptr HeapLoc::heapLoc_;

void HeapLoc::getHeapLocs(AbslocSet &locs) {
    if (heapLoc_) locs.insert(heapLoc_);
    return;
}

Absloc::Ptr HeapLoc::getHeapLoc() {
    if (!heapLoc_) {
        heapLoc_ = HeapLoc::Ptr(new HeapLoc());
    }
    return heapLoc_;
}

HeapLoc::AbslocSet HeapLoc::getAliases() {
    AbslocSet ret;
    // Global memory reference is included...
    ret.insert(MemLoc::getMemLoc());
    return ret;
}
#endif

const Address MemLoc::MEM_GLOBAL = (Address) -1;
MemLoc::MemMap MemLoc::memLocs_;

std::string MemLoc::name() const { 
    if (addr_ == MEM_GLOBAL) {
        return "Mem_UNKNOWN_";
    }
    else {
        char buf[256];
        sprintf(buf, "Mem_0x%lx_", addr_);
        return std::string(buf);
    }
}

void MemLoc::getMemLocs(AbslocSet &locs) {
    for (MemMap::iterator iter = memLocs_.begin();
         iter != memLocs_.end(); iter++) {
        locs.insert((*iter).second);
    }
}

Absloc::Ptr MemLoc::getMemLoc(Address addr) {
    // Look up by name and return    
    if (memLocs_.find(addr) == memLocs_.end()) {
        MemLoc::Ptr mP = MemLoc::Ptr(new MemLoc(addr));
        
        memLocs_[addr] = mP;
    }
    
    return memLocs_[addr];
}

Absloc::Ptr MemLoc::getMemLoc() {
    return getMemLoc(MEM_GLOBAL);
}

MemLoc::AbslocSet MemLoc::getAliases() const {
    AbslocSet ret;
    if (addr_ != MEM_GLOBAL) {
        ret.insert(getMemLoc());
    }
    else {
        // All known memory locations
        for (MemMap::const_iterator iter = memLocs_.begin();
             iter != memLocs_.end(); iter++) {
            if ((*iter).first != MEM_GLOBAL)
                ret.insert((*iter).second);
        }
        // All stack locations
        StackLoc::getStackLocs(ret);
    }

    return ret;
}

ImmLoc::Ptr ImmLoc::immLoc_; 

std::string ImmLoc::name() const { 
    return "Immediate";
}

Absloc::Ptr ImmLoc::getImmLoc() {
    if (immLoc_) return immLoc_;

    immLoc_ = ImmLoc::Ptr(new ImmLoc());

    return immLoc_;
}
