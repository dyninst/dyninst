/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

#include "Absloc.h"
#include "Graph.h"
#include "Edge.h"
#include "Node.h"
#include <assert.h>

#include "Register.h"
#include "InstructionAST.h"
#include "Instruction.h"
#include "Expression.h"

#include "dataflowAPI/h/stackanalysis.h"
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
Absloc::Ptr RegisterLoc::pc_;

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
// Make sure we have a canonical PC, since < on a shared pointer is only
// pointer uniqueness and nothing else...
        if (rP->isPC())
            pc_ = rP;

    }
    
    return allRegLocs_[*container];
}

Absloc::Ptr RegisterLoc::makePC() {
    if (pc_) return pc_;
    
    // Make the register...
    RegisterAST rPC = RegisterAST::makePC();
    RegisterAST::Ptr rpPC = RegisterAST::Ptr(new RegisterAST(rPC));
    pc_ = RegisterLoc::Ptr(new RegisterLoc(rpPC));
    return pc_;
}

const int StackLoc::STACK_GLOBAL = MININT;
const int StackLoc::STACK_REGION_DEFAULT = 0;

StackLoc::StackMap StackLoc::stackLocs_;

std::string StackLoc::format() const {
    std::stringstream ret;

    if (slot_ == STACK_GLOBAL) {
        return std::string("STACK");
    }
    
    ret << "STACK_" << slot_ << "_" << region_;

    return ret.str(); 
}

void StackLoc::getStackLocs(AbslocSet &locs) {
    for (StackMap::iterator iter = stackLocs_.begin();
         iter != stackLocs_.end(); iter++) {
        locs.insert((*iter).second);
    }
}

Absloc::Ptr StackLoc::getStackLoc(int slot, int region) {
    if (stackLocs_.find(std::make_pair(slot, region)) == stackLocs_.end()) {
        StackLoc::Ptr sP = StackLoc::Ptr(new StackLoc(slot, region));
        
        stackLocs_[std::make_pair(slot, region)] = sP;
    }
    
    return stackLocs_[std::make_pair(slot, region)];
}

Absloc::Ptr StackLoc::getStackLoc() {
    // Look up by name and return    
    return getStackLoc(STACK_GLOBAL, STACK_REGION_DEFAULT);
}

StackLoc::AbslocSet StackLoc::getAliases() const{
    AbslocSet ret;

    // If we're a specific stack slot, return the generic
    // stack slot(s)
    if (slot_ == STACK_GLOBAL) {
        for (StackMap::const_iterator iter = stackLocs_.begin();
             iter != stackLocs_.end(); iter++) {
            if ((*iter).first.first != STACK_GLOBAL)
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

std::string MemLoc::format() const { 
    if (addr_ == MEM_GLOBAL) {
        return "Mem_UNKNOWN_";
    }
    std::stringstream ret;

    ret << "Mem_" << std::hex << addr_;
    return ret.str();
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

std::string ImmLoc::format() const { 
    return "Immediate";
}

Absloc::Ptr ImmLoc::getImmLoc() {
    if (immLoc_) return immLoc_;

    immLoc_ = ImmLoc::Ptr(new ImmLoc());

    return immLoc_;
}

bool RegisterLoc::isSP() const { 
    return ((reg_->getID() == r_ESP) || (reg_->getID() == r_RSP)); 
}

bool RegisterLoc::isPC() const {
    return ((reg_->getID() == r_EIP) || (reg_->getID() == r_RIP));
}

bool RegisterLoc::isFlag() const {
    return ((reg_->getID() >= r_OF) && (reg_->getID() <= r_RF));
}

bool RegisterLoc::isSP(RegisterAST::Ptr reg) { 
    RegisterAST::Ptr reg_ = dynamic_pointer_cast<RegisterAST>(RegisterAST::promote(reg));
    return ((reg_->getID() == r_ESP) ||
            (reg_->getID() == r_RSP)); 
}

bool RegisterLoc::isPC(RegisterAST::Ptr reg) {
    RegisterAST::Ptr reg_ = dynamic_pointer_cast<RegisterAST>(RegisterAST::promote(reg));
    return ((reg_->getID() == r_EIP) || 
            (reg_->getID() == r_RIP));
}

bool RegisterLoc::isFlag(RegisterAST::Ptr reg) {
    RegisterAST::Ptr reg_ = dynamic_pointer_cast<RegisterAST>(RegisterAST::promote(reg));
    return ((reg_->getID() >= r_OF) && 
            (reg_->getID() <= r_RF));
}
