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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "common/h/Types.h"
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"

#include "arch.h"
#include "util.h"
#include "process.h"
#include "symtab.h"
#include "instPoint.h"
#include "InstrucIter.h"

#include "BPatch_Set.h"

#if !defined(arch_ia64)
// IA64 has a bundle-oriented version defined in InstrucIter-ia64.C

void InstrucIter::initializeInsn() {
    if (proc_) {
        instPtr = proc_->getPtrToOrigInstruction(current);
    }
    else {
        assert(img_); 
        if (!img_->isValidAddress(current)) {
            fprintf(stderr, "Error: addr 0x%x is not valid!\n",
                    img_);
        }
        else instPtr = img_->getPtrToOrigInstruction(current);
    }            
    if (!instPtr) {
        fprintf(stderr, "ERROR: no pointer for address 0x%x\n",
                current);
    }
    assert(instPtr);
    insn.setInstruction((codeBuf_t *)instPtr, current);
}
#endif

InstrucIter::InstrucIter(int_function* func) :
    proc_(func->proc()),
    img_(NULL),
    base(func->getAddress()),
    range(func->getSize()),
    current(base) {
    initializeInsn();
}

InstrucIter::InstrucIter(Address addr, int_function* func) :
    proc_(func->proc()),
    img_(NULL),
    base(addr),
    range(func->getSize()),
    current(base) {
    initializeInsn();
}


InstrucIter::InstrucIter(int_basicBlock* b) :
    proc_(b->proc()),
    img_(NULL),
    base(b->firstInsnAddr()),
    range(b->getSize()),
    current(base) {
    initializeInsn();
}

InstrucIter::InstrucIter(image_basicBlock *b) :
    proc_(NULL),
    img_(b->func()->img()),
    base(b->firstInsnOffset()),
    range(b->getSize()),
    current(base) {
    initializeInsn();
}


InstrucIter::InstrucIter( CONST_EXPORT BPatch_basicBlock* bpBasicBlock) :
    proc_(bpBasicBlock->flowGraph->getBProcess()->lowlevel_process()),
    img_(NULL),
    base(bpBasicBlock->startAddress),
    range(bpBasicBlock->size()),
    current(base) {
    initializeInsn();
}

//instPtr =addressImage->getPtrToInstruction(bpBasicBlock->startAddress);
//insn.getNextInstruction( instPtr ); 


/** copy constructor
 * @param ii InstrucIter to copy
 */
InstrucIter::InstrucIter(const InstrucIter& ii) :
    proc_(ii.proc_),
    img_(ii.img_),
    base(ii.base),
    range(ii.range),
    current(ii.current),
    prevInsns(ii.prevInsns)
{
    initializeInsn();
}

// For somewhere in a process (maybe)
InstrucIter::InstrucIter( Address addr, process *proc) :
    proc_(proc),
    img_(NULL),
    current(addr)
{
   // Need to set base and range. Any ideas? Basic block? 
    codeRange *cr = proc_->findCodeRangeByAddress(addr);
    if (cr->is_function()) {
        base = cr->is_function()->getAddress();
        // This can and will trigger full analysis...
        range = cr->is_function()->getSize();
    }
    else {
        // There's all sorts of reasons to iterate over the middle of nowhere;
        // we may be grabbing the PLT or new code. On the other hand, nobody knows
        // what the range is.
        base = addr;
        // We set range so that base+range = (Address) -1; that is, so that base+range will always
        // be bigger than current.
        range = ((Address) -1) - base;
    }
    initializeInsn();
}

// And truly generic
InstrucIter::InstrucIter( Address addr, unsigned size, process *proc) :
    proc_(proc),
    img_(NULL),
    base(addr),
    range(size),
    current(addr)
{
    initializeInsn();
}

// Used in parsing -- relative addrs
InstrucIter::InstrucIter(Address start, image_func *func) :
    proc_(NULL),
    img_(func->img()),
    base(func->getOffset()),
    range(func->get_size_cr()), // Probably in the middle of
    // parsing, so calling getSize is
    // a bad idea as it may
    // trigger... parsing.
    current(start) {
    initializeInsn();
}

