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
using namespace Dyninst::DDG;
using namespace Dyninst::InstructionAPI;

void Absloc::getAbslocs(AbslocSet &locs) {
    RegisterLoc::getRegisterLocs(locs);
    StackLoc::getStackLocs(locs);
    HeapLoc::getHeapLocs(locs);
    MemLoc::getMemLocs(locs);
}

Absloc::Ptr Absloc::getAbsloc(const InstructionAPI::Expression::Ptr exp,
                              Function *func,
                              Address addr) {
    // We want to simplify the expression as much as possible given 
    // currently known state, and then quantify it as one of the following:
    // 
    // Stack: a memory access based off the current frame pointer (FP) or
    //   stack pointer (SP). If we can determine an offset from the "top"
    //   of the stack we create a stack slot location. Otherwise we create
    //   a "stack" location that represents all stack locations.
    //
    // Heap: a memory access to a generic pointer.
    //
    // Memory: a memory access to a known address. 
    //
    // TODO: aliasing relations. Aliasing SUCKS. 

    // Since we have an Expression as input, we don't have the dereference
    // operator.

    // Here's the logic:
    // If no registers are used:
    //   If only immediates are used:
    //     Evaluate and create a MemLoc.
    //   If a dereference exists:
    //     WTF???
    // If registers are used:
    //   If the only register is the FP AND the function has a stack frame:
    //     Set FP to 0, eval, and create a specific StackLoc.
    //   If the only register is the SP:
    //     If we know the contents of SP:
    //       Eval and create a specific StackLoc
    //     Else create a generic StackLoc.
    //   If a non-stack register is used:
    //     Create a generic MemLoc.

    std::set<InstructionAST::Ptr> regUses;
    exp->getUses(regUses);

    if (regUses.empty()) {
        // Case 1: Immediate only.
        Result res = exp->eval();
        Address addr;
        if (res.defined && Absloc::convertResultToAddr(res, addr)) {
            return MemLoc::getMemLoc(addr);
        }
        else {
            // Oops...
            return MemLoc::getMemLoc();
        }
    }
    else {
        // We have register uses...
        bool isStack = false;

        for (std::set<InstructionAST::Ptr>::iterator iter = regUses.begin();
             iter != regUses.end();
             iter++) {
            if (Absloc::isStackPointer(*iter, func, addr)) {
                isStack = true;
                InstructionAST::Ptr sp = *iter;
                bindSP(sp, func, addr);
            }
            else if (Absloc::isFramePointer(*iter, func, addr)) {
                isStack = true;
                InstructionAST::Ptr fp = *iter;
                bindFP(fp, func, addr);
            }
        }

        if (isStack) {
            Result res = exp->eval();
            
            int slot;
            if (res.defined && Absloc::convertResultToSlot(res, slot)) {
                return StackLoc::getStackLoc(slot);
            }
            else {
                return StackLoc::getStackLoc();
            }
        }
        else {
            return MemLoc::getMemLoc();
        }
    }    
    assert(0);
    return MemLoc::getMemLoc();
}

// Things are a lot easier if we know it's a register...
Absloc::Ptr Absloc::getAbsloc(const InstructionAPI::RegisterAST::Ptr reg) {
    return RegisterLoc::getRegLoc(reg);
}

void Absloc::getUsedAbslocs(const InstructionAPI::Instruction insn,
                            Function *func,
                            Address addr,
                            AbslocSet &uses) {
    std::set<RegisterAST::Ptr> regReads;
    insn.getReadSet(regReads);

    // Registers are nice and easy. The next clause is for memory... now
    // that sucks.

    for (std::set<RegisterAST::Ptr>::const_iterator r = regReads.begin();
         r != regReads.end();
         r++) {
        // We have 'used' this Absloc
        Absloc::Ptr aP = Absloc::getAbsloc(*r);        
        uses.insert(Absloc::getAbsloc(*r));
        fprintf(stderr,"\t (u)(reg) %s\n", (*r)->format().c_str());
    }

    // Also handle memory writes
    if (insn.readsMemory()) {
        std::set<Expression::Ptr> memReads;
        insn.getMemoryReadOperands(memReads);
        for (std::set<Expression::Ptr>::const_iterator r = memReads.begin();
             r != memReads.end();
             r++) {
            fprintf(stderr, "\t (u)(exp) %s\n", (*r)->format().c_str());
            uses.insert(Absloc::getAbsloc(*r, func, addr));
        }
    }
}

void Absloc::getDefinedAbslocs(const InstructionAPI::Instruction insn,
                               Function *func,
                               Address addr,
                               AbslocSet &defs) {
    std::set<RegisterAST::Ptr> regWrites;
    insn.getWriteSet(regWrites);            

    // Registers are nice and easy. The next clause is for memory... now
    // that sucks.

    for (std::set<RegisterAST::Ptr>::const_iterator w = regWrites.begin();
         w != regWrites.end();
         w++) {
        // We have 'defined' this Absloc
        Absloc::Ptr aP = Absloc::getAbsloc(*w);
        defs.insert(Absloc::getAbsloc(*w));
        fprintf(stderr,"\t (d)(reg) %s\n", (*w)->format().c_str());
    }

    // Also handle memory writes
    if (insn.writesMemory()) {
        std::set<Expression::Ptr> memWrites;
        insn.getMemoryWriteOperands(memWrites);
        for (std::set<Expression::Ptr>::const_iterator w = memWrites.begin();
             w != memWrites.end();
             w++) {
            defs.insert(Absloc::getAbsloc(*w, func, addr));
            fprintf(stderr,"\t (d)(exp) %s\n", (*w)->format().c_str());
        }
    }
}

bool Absloc::convertResultToAddr(const InstructionAPI::Result &res, Address &addr) {
    assert(res.defined);
    switch (res.type) {
    case u8:
        addr = (Address) res.val.u8val;
        return true;
    case s8:
        addr = (Address) res.val.s8val;
        return true;
    case u16:
        addr = (Address) res.val.u16val;
        return true;
    case s16:
        addr = (Address) res.val.s16val;
        return true;
    case u32:
        addr = (Address) res.val.u32val;
        return true;
    case s32:
        addr = (Address) res.val.s32val;
        return true;
    case u48:
        addr = (Address) res.val.u48val;
        return true;
    case s48:
        addr = (Address) res.val.s48val;
        return true;
    case u64:
        addr = (Address) res.val.u64val;
        return true;
    case s64:
        addr = (Address) res.val.s64val;
        return true;
    default:
        return false;
    }
}

bool Absloc::convertResultToSlot(const InstructionAPI::Result &res, int &addr) {
    assert(res.defined);
    switch (res.type) {
    case u8:
        addr = (int) res.val.u8val;
        return true;
    case s8:
        addr = (int) res.val.s8val;
        return true;
    case u16:
        addr = (int) res.val.u16val;
        return true;
    case s16:
        addr = (int) res.val.s16val;
        return true;
    case u32:
        addr = (int) res.val.u32val;
        return true;
    case s32:
        addr = (int) res.val.s32val;
        return true;
        // I'm leaving these in because they may get used, but
        // we're definitely truncating them down.
    case u48:
        addr = (int) res.val.u48val;
        return true;
    case s48:
        addr = (int) res.val.s48val;
        return true;
    case u64:
        addr = (int) res.val.u64val;
        return true;
    case s64:
        addr = (int) res.val.s64val;
        return true;
    default:
        return false;
    }
}

bool Absloc::isFramePointer(const InstructionAPI::InstructionAST::Ptr &reg, Function *func, Address addr) {
    InstructionAPI::RegisterAST::Ptr container = boost::dynamic_pointer_cast<InstructionAPI::RegisterAST>(RegisterAST::promote(reg));

    if (!container) return false;

    if ((container->getID() != InstructionAPI::r_EBP) &&
        (container->getID() != InstructionAPI::r_RBP)) 
        return false; 
   
    StackAnalysis sA(func->lowlevel_func()->ifunc()); 
    
    const Dyninst::StackAnalysis::PresenceTree *pT = sA.presenceIntervals();
    Dyninst::StackAnalysis::StackPresence exists;

    Offset off = func->lowlevel_func()->addrToOffset(addr);

    if (!pT->find(off, exists)) return false;

    assert(!exists.isTop());

    return (exists.presence() == StackAnalysis::StackPresence::frame);
}

bool Absloc::isStackPointer(const InstructionAPI::InstructionAST::Ptr &reg, Function *, Address) {
    InstructionAPI::RegisterAST::Ptr container = boost::dynamic_pointer_cast<InstructionAPI::RegisterAST>(RegisterAST::promote(reg));

    if (!container) return false;

    if ((container->getID() != InstructionAPI::r_ESP) &&
        (container->getID() != InstructionAPI::r_RSP)) 
        return false;

    return true;
}

// Determine the current value of the FP and "bind" it for later evaluation.
// We can assert here that we were passed the frame pointer (and, for correctness,
// that is currently holding the base of the frame) and then bind it to 0.
void Absloc::bindFP(InstructionAPI::InstructionAST::Ptr &reg, Function *func, Address addr) {
    assert(isFramePointer(reg, func, addr));
    
    InstructionAPI::RegisterAST::Ptr container = boost::dynamic_pointer_cast<InstructionAPI::RegisterAST>(reg);

    if (container->getID() == InstructionAPI::r_EBP) {
        InstructionAPI::Result res(InstructionAPI::s32,0);
        container->setValue(res);
    }
    else {
        InstructionAPI::Result res(InstructionAPI::s64, 0);
        container->setValue(res);
    }
}

void Absloc::bindSP(InstructionAPI::InstructionAST::Ptr &reg, Function *func, Address addr) {
    fprintf(stderr, "\t Binding stack pointer value at 0x%lx\n", addr);

    assert(isStackPointer(reg, func, addr));

    StackAnalysis sA(func->lowlevel_func()->ifunc());
    const Dyninst::StackAnalysis::HeightTree *hT = sA.heightIntervals();

    StackAnalysis::StackHeight height;

    Offset off = func->lowlevel_func()->addrToOffset(addr);

    fprintf(stderr, "\t\tAddr is 0x%lx, offset 0x%lx\n", addr, off);

    if (!hT->find(off, height)) {
        fprintf(stderr, "\t\t analysis failed, ret...\n");
        return;
    }

    // Ensure that analysis has been performed.
    assert(!height.isTop());

    if (height.isBottom()) {
        fprintf(stderr, "\t\t analysis ret bottom, ret...\n");
        return;
    }

    InstructionAPI::RegisterAST::Ptr container = boost::dynamic_pointer_cast<InstructionAPI::RegisterAST>(reg);

    if (container->getID() == InstructionAPI::r_EBP) {
        InstructionAPI::Result res(InstructionAPI::s32, height.height());
        container->setValue(res);
        fprintf(stderr, "\t\t Bound to %ld\n", height.height());
    }
    else {
        InstructionAPI::Result res(InstructionAPI::s64, height.height());
        container->setValue(res);
        fprintf(stderr, "\t\t Bound to %ld\n", height.height());
    }
    return;
}

RegisterLoc::RegisterMap RegisterLoc::allRegLocs_;

void RegisterLoc::getRegisterLocs(AbslocSet &locs) {
    for (RegisterMap::iterator iter = allRegLocs_.begin(); iter != allRegLocs_.end(); iter++) {
        locs.insert((*iter).second);
    }
}

Absloc::Ptr RegisterLoc::getRegLoc(const InstructionAPI::RegisterAST::Ptr reg) {
    // Upconvert the register to its canonical container
    InstructionAPI::RegisterAST::Ptr container = boost::dynamic_pointer_cast<InstructionAPI::RegisterAST>(RegisterAST::promote(reg));
    if (!container) {
        assert(0);
        return Absloc::Ptr();
    }

    // Look up by name and return    
    if (allRegLocs_.find(*container) == allRegLocs_.end()) {
        fprintf(stderr, "... didn't find register %s/%d in Abslocs, creating...\n", container->format().c_str(),
                container->getID());

        RegisterLoc::Ptr rP = RegisterLoc::Ptr(new RegisterLoc(container));

        allRegLocs_[*container] = rP;
    }
    
    return allRegLocs_[*container];
}

const int StackLoc::STACK_GLOBAL = MININT;

StackLoc::StackMap StackLoc::allStackLocs_;

std::string StackLoc::name() const {
    if (slot_ == STACK_GLOBAL) {
        return "STACK_Unknown";
    }
    else {
        char buf[256];
        sprintf(buf, "STACK_%d", slot_);
        return std::string(buf);
    }
}

void StackLoc::getStackLocs(AbslocSet &locs) {
    for (StackMap::iterator iter = allStackLocs_.begin();
         iter != allStackLocs_.end(); iter++) {
        locs.insert((*iter).second);
    }
}

Absloc::Ptr StackLoc::getStackLoc(int slot) {
    // Look up by name and return    
    if (allStackLocs_.find(slot) == allStackLocs_.end()) {
        fprintf(stderr, "... didn't find stack slot %d in Abslocs, creating...\n", slot);
        
        StackLoc::Ptr sP = StackLoc::Ptr(new StackLoc(slot));
        
        allStackLocs_[slot] = sP;
    }
    
    return allStackLocs_[slot];
}

Absloc::Ptr StackLoc::getStackLoc() {
    // Look up by name and return    
    if (allStackLocs_.find(STACK_GLOBAL) == allStackLocs_.end()) {
        fprintf(stderr, "... didn't find stack slot <UNKNOWN> in Abslocs, creating...\n");
        
        StackLoc::Ptr sP = StackLoc::Ptr(new StackLoc());
        
        allStackLocs_[STACK_GLOBAL] = sP;
    }
    
    return allStackLocs_[STACK_GLOBAL];
}

StackLoc::AbslocSet StackLoc::getAliasedAbslocs() {
    AbslocSet ret;
    // If we're a specific stack slot, return the generic
    // stack slot (if it exists)
    if (slot_ != STACK_GLOBAL) {
        ret.insert(getStackLoc());
    } 
    else {
        // What if we're the generic stack slot? Should we include
        // specific stack slots? 
        for (StackMap::iterator i = allStackLocs_.begin(); i != allStackLocs_.end(); i++) {
            if ((*i).first != STACK_GLOBAL) {
                ret.insert((*i).second);
            }
        }
    }

    // Global memory reference is included...
    ret.insert(MemLoc::getMemLoc());

    return ret;
}

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

HeapLoc::AbslocSet HeapLoc::getAliasedAbslocs() {
    AbslocSet ret;
    // Global memory reference is included...
    ret.insert(MemLoc::getMemLoc());
    return ret;
}

const Address MemLoc::MEM_GLOBAL = (Address) -1;
MemLoc::MemMap MemLoc::allMemLocs_;

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
    for (MemMap::iterator iter = allMemLocs_.begin();
         iter != allMemLocs_.end(); iter++) {
        locs.insert((*iter).second);
    }
}

Absloc::Ptr MemLoc::getMemLoc(Address addr) {
    // Look up by name and return    
    if (allMemLocs_.find(addr) == allMemLocs_.end()) {
        fprintf(stderr, "... didn't find memory addr 0x%lx in Abslocs, creating...\n", addr);
        
        MemLoc::Ptr mP = MemLoc::Ptr(new MemLoc(addr));
        
        allMemLocs_[addr] = mP;
    }
    
    return allMemLocs_[addr];
}

Absloc::Ptr MemLoc::getMemLoc() {
    // Look up by name and return    
    if (allMemLocs_.find(MEM_GLOBAL) == allMemLocs_.end()) {
        fprintf(stderr, "... didn't find memory addr <UNKNOWN> in Abslocs, creating...\n");
        
        MemLoc::Ptr mP = MemLoc::Ptr(new MemLoc());
        
        allMemLocs_[MEM_GLOBAL] = mP;
    }
    
    return allMemLocs_[MEM_GLOBAL];
}

MemLoc::AbslocSet MemLoc::getAliasedAbslocs() {
    AbslocSet ret;
    if (addr_ != MEM_GLOBAL) {
        ret.insert(getMemLoc());
    }

    return ret;
}
