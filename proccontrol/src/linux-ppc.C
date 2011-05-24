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

#include <cassert>
#include "linux.h"

#include "common/h/arch-power.h"
using namespace NS_power;

bool linux_thread::getSegmentBase(Dyninst::MachRegister, Dyninst::MachRegisterVal &) {
    assert(!"This is not implemented on this architecture");
    return false;
}

bool linux_process::plat_convertToBreakpointAddress(psaddr_t &addr) {
    bool result = true;
    if( getAddressWidth() == 8 ) {
        psaddr_t tmpAddr = addr;
        psaddr_t resultAddr = 0;

        mem_response::ptr resp = mem_response::createMemResponse((char *) &resultAddr, sizeof(psaddr_t));
        result = readMem((Dyninst::Address) tmpAddr, resp, triggerThread());

        do {
            if( !result ) break;

            result = int_process::waitForAsyncEvent(resp);
            if( !result || resp->hasError() ) {
                result = false;
                break;
            }

            addr = resultAddr;
        }while(0);
    }

    return result;
}

static bool atomicLoad(const instruction &insn) {
    return (    (XFORM_OP(insn) == LXop)
             && (XFORM_XO(insn) == LWARXxop) );
}

static bool atomicStore(const instruction &insn) {
    return (    (XFORM_OP(insn) == STXop) 
             && (XFORM_XO(insn) == STWCXxop) );
}

bool linux_thread::plat_needsEmulatedSingleStep(vector<Address> &addrResult) {
    assert( singleStep() );

    pthrd_printf("Checking for atomic instruction sequence before single step\n");

    /* 
     * We need an emulated single step to single step an atomic
     * instruction sequence. The sequence looks something like this:
     *
     * lwarx
     * ...
     * stwcx.
     * <breapoint>
     *
     * We need to set the breakpoint at the instruction immediately after
     * stwcx.
     */
    reg_response::ptr pcResponse = reg_response::createRegResponse();
    bool result = getRegister(MachRegister::getPC(llproc()->getTargetArch()), pcResponse);
    if( !result ) {
        perr_printf("Failed to read PC address to check for emulated single step condition\n");
        return false;
    }

    result = llproc()->waitForAsyncEvent(pcResponse);
    if( !result ) {
        pthrd_printf("Error waiting for async events\n");
        return false;
    }
    assert(pcResponse->isReady());
    if( pcResponse->hasError() ) {
        pthrd_printf("Async error getting PC register\n");
        return false;
    }

    Address pc = (Address)pcResponse->getResult();

    // Check if the next instruction is a lwarx
    // If it is, scan forward until the terminating stwcx.
    bool foundEnd = false;
    bool sequenceStarted = false;
    int maxSequenceCount = 24; // arbitrary 
    int currentCount = 0;
    do {
        // Read the current instruction
        unsigned int rawInsn;
        pthrd_printf("Reading instruction at 0x%lx\n", pc);
        mem_response::ptr memresult = mem_response::createMemResponse((char *)&rawInsn, sizeof(unsigned int));
        result = llproc()->readMem(pc, memresult);
        if( !result ) {
            pthrd_printf("Error reading from memory 0x%lx\n", pc);
            return false;
        }

        llproc()->waitForAsyncEvent(memresult);

        if( memresult->hasError() ) {
            pthrd_printf("Error reading from memory 0x%lx\n", pc);
            return false;
        }

        // Decode the current instruction
        instruction insn(rawInsn);
        if( atomicLoad(insn) ) {
            sequenceStarted = true;
            pthrd_printf("Found the start of an atomic instruction sequence at 0x%lx\n", pc);
        }else{
            if( !sequenceStarted ) break;
        }

        if( atomicStore(insn) && sequenceStarted ) {
            foundEnd = true;
        }

        // For control flow instructions, assume target is outside atomic instruction sequence
        // and place breakpoint there as well
        Address cfTarget = insn.getTarget(pc);
        if( cfTarget != 0 && sequenceStarted && !foundEnd ) {
            addrResult.push_back(cfTarget);
        }

        currentCount++;
        pc += 4;
    }while( !foundEnd && currentCount < maxSequenceCount );

    // The breakpoint should be set at the instruction following the sequence
    if( foundEnd ) {
        addrResult.push_back(pc);
        pthrd_printf("Atomic instruction sequence ends at 0x%lx\n", pc);
    }else if( sequenceStarted || addrResult.size() ) {
        addrResult.clear();
        pthrd_printf("Failed to find end of atomic instruction sequence\n");
        return false;
    }else{
        pthrd_printf("No atomic instruction sequence found, safe to single step\n");
    }

    return true;
}

bool linux_thread::plat_needsPCSaveBeforeSingleStep() {
    return true;
}
