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

/* -*- Mode: C; indent-tabs-mode: true; tab-width: 4 -*- */

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

/* A few utility functions. */
int extractInstructionSlot( Address addr ) {
	return (addr % 16);
	} /* end extractInstructionSlot */

bool addressIsValidInsnAddr( Address addr ) {
	switch( extractInstructionSlot( addr ) ) {
		case 0: case 1: case 2:
			return true;
		default:
			return false;
		}
	} /* end addressIsValidInsnAddr() */

bool InstrucIter::isAReturnInstruction()
{
    instruction insn = getInstruction();
    switch( insn.getType() ) {
    case instruction::RETURN:
        return true;
        break;
	
    default:
        break;
    } /* end instruction-type switch */
    return false;
}

bool InstrucIter::isAIndirectJumpInstruction()
{
    instruction insn = getInstruction();
    switch( insn.getType() ) {
    case instruction::INDIRECT_BRANCH:
        if( insn.getPredicate() == 0 ) { return true; }
        break;
        
    default:
        break;
    } /* end instruction-type switch */
    return false;
}

bool InstrucIter::isACondBranchInstruction()
{
    instruction insn = getInstruction();
    switch( insn.getType() ) {
    case instruction::DIRECT_BRANCH:
        /* Not sure if this second case is intended. */
    case instruction::INDIRECT_BRANCH: {
        if( insn.getPredicate() != 0 ) { return true; }
        break; } 
        
    default:
        break;
    } /* end instruction-type switch */
    return false;
}

/* We take this to mean a dirct conditional branch which always executes. */
bool InstrucIter::isAJumpInstruction()
{
    instruction insn = getInstruction();

    switch( insn.getType() ) {
    case instruction::DIRECT_BRANCH:
        if( insn.getPredicate() == 0 ) { return true; }
        break;
        
    default:
        break;
    } /* end instruction-type switch */
    return false;
}

bool InstrucIter::isACallInstruction()
{
    return (getInstruction().getType() == instruction::DIRECT_CALL ||
            getInstruction().getType() == instruction::INDIRECT_CALL);
}

bool InstrucIter::isAnneal()
{
    assert( 0 );
    return false;
}

Address InstrucIter::getBranchTargetAddress()
{
    instruction nsn = getInstruction();
    Address rawTargetAddress = insn.getTargetAddress() + current;
    Address targetAddress = rawTargetAddress - (rawTargetAddress % 0x10);
    // /* DEBUG */ fprintf( stderr, "Instruction at 0x%lx targets 0x%lx\n", currentAddress, targetAddress );
    return targetAddress;
}

void initOpCodeInfo() {
    /* I don't need this for anything. */
} /* end initOpCodeInfo() */

BPatch_memoryAccess* InstrucIter::isLoadOrStore()
{
    instruction insn = getInstruction();
    instruction::insnType type = insn.getType();
    
    BPatch_memoryAccess * bpma = NULL;
    
    insn_tmpl tmpl = { insn.getMachineCode() };
    uint8_t size = 0x1 << (tmpl.M1.x6 & 0x3);
    
    switch( type ) {
    case instruction::INTEGER_16_LOAD:
        size = 16;
    case instruction::INTEGER_LOAD:
    case instruction::FP_LOAD:
        bpma = new BPatch_memoryAccess( &insn, sizeof( instruction ), true, false, size, 0, tmpl.M1.r3, -1 );
        assert( bpma != NULL );
        break;
        
    case instruction::INTEGER_16_STORE:
        size = 16;
    case instruction::INTEGER_STORE:
    case instruction::FP_STORE:
        bpma = new BPatch_memoryAccess( &insn, sizeof( instruction ), false, true, size, 0, tmpl.M1.r3, -1 );
        assert( bpma != NULL );
        break;
        
    case instruction::FP_PAIR_LOAD:
    case instruction::INTEGER_PAIR_LOAD:
        /* The load pair instructions encode sizes a little differently. */
        size = (tmpl.M1.x6 & 0x1) ? 16 : 8;
        bpma = new BPatch_memoryAccess( &insn, sizeof( instruction ), true, false, size, 0, tmpl.M1.r3, -1 );
        assert( bpma != NULL );
        break;
        
    case instruction::PREFETCH:
			bpma = new BPatch_memoryAccess( &insn, sizeof( instruction), false, false, 0, tmpl.M1.r3, -1, 0, 0, -1, -1, 0, -1, false, 0 );
			assert( bpma != NULL );
			break;
                        
    default:
        return BPatch_memoryAccess::none;
    }
    
    return bpma;
}

BPatch_instruction *InstrucIter::getBPInstruction() {

    BPatch_memoryAccess *ma = isLoadOrStore();
    BPatch_instruction *in;
    
    if (ma != BPatch_memoryAccess::none)
        return ma;
    
    instruction i = getInstruction();
    /* Work around compiler idiocy.  FIXME: ignoring long instructions. */
    uint64_t raw = i.getMachineCode();
    in = new BPatch_instruction( & raw, sizeof( raw ) );
    
    return in;
}

void InstrucIter::getMultipleJumpTargets( BPatch_Set<Address> & targetAddresses ) {
    /* FIXME: we also see a pattern (in libc) in which the address calculated from
       the gp is the address of the table, not of the location to look it up.  (One
       less indirection, in other words.)  TODO: find out if this is just what
       gcc does for libraries, or if we have to do actual dataflow analysis to determine
       which case is happening. */

    /* The IA-64 SCRAG defines a pattern similar to the power's.  At some constant offset
       from the GP, there's a jump table whose 64-bit entries are offsets from the base
       address of the table to the target.  We assume that the nearest previous
       addition involving r1 is calculating the jump table's address; if this proves to be
       ill-founded, we'll have to trace registers backwards, starting with the branch
       register used in the indirect jump. */
    
    Address originalAddress = current;

    Address gpAddress = 0;
    if (img_) { 
        gpAddress = img_->getObject().getTOCoffset();
    }
    else {
        pdvector<mapped_object *> m_objs = proc_->mappedObjects();
        for (unsigned i = 0; i < m_objs.size(); i++) {
            void *ptr = m_objs[i]->getPtrToOrigInstruction(current);
            if (ptr) {
                gpAddress = m_objs[i]->parse_img()->getObject().getTOCoffset();
                break;
            }
        }
    }


    
    /* We assume that gcc will always generate an addl-form.  (Otherwise,
       our checks will have to be somewhat more general.) */
    Address jumpTableOffset = 0;
    do {
        /* Rewind one instruction. */
        if( ! hasPrev() ) { return; } (*this)--;
        
        /* Acquire it. */
        instruction insn = getInstruction();
        
        /* Is it an integer or memory operation? */
        instruction::unitType unitType = insn.getUnitType();
        if( unitType != instruction::I && unitType != instruction::M ) { continue; }
        
		/* If so, is it an addl? */
        insn_tmpl tmpl = { insn.getMachineCode() };
        if( GET_OPCODE( &tmpl ) != 0x9 ) { continue; }
        
        /* If it's an addl, is its destination register r1? */
        if( tmpl.A5.r3 != 0x1 ) { continue; }
        
        /* Finally, extract the constant jumpTableOffset. */
        jumpTableOffset = GET_A5_IMM( &tmpl );
        
        /* We've found the jumpTableOffset, stop looking. */
        // /* DEBUG */ fprintf( stderr, "ip: 0x%lx jumpTableOffset = %ld\n", current, jumpTableOffset );
        break;
    } while( true );
    
    /* Calculate the jump table's address. */
    Address jumpTableAddressAddress = gpAddress + jumpTableOffset;
    // /* DEBUG */ fprintf( stderr, "jumpTableAddressAddress = 0x%lx\n", jumpTableAddressAddress );
    
    /* Assume that the nearest previous immediate-register compare
       is range-checking the jump-table offset.  Extract that
       constant, n.  (Otherwise, we're kind of screwed: how big 
       is the jump table?) */
    uint64_t maxTableLength = 0;
    do {
        /* Rewind one instruction. */
        if( ! hasPrev() ) { return; } (*this)--;
        
        /* Acquire it. */
        instruction insn = getInstruction();
        
        /* Is it an integer or memory operation? */
        instruction::unitType unitType = insn.getUnitType();
        if( unitType != instruction::I && unitType != instruction::M ) { continue; }
        
        /* If so, is it a cmp.ltu? */
        insn_tmpl tmpl = { insn.getMachineCode() };
        if( GET_OPCODE( &tmpl ) != 0xD ) { continue; }
        
        /* Extract the immediate. */
        maxTableLength = GET_A8_COUNT( &tmpl );
        
        /* We've found a cmp.ltu; stop looking. */
        // /* DEBUG */ fprintf( stderr, "maxTableLength = %ld\n", maxTableLength );
        break;
    } while( true );
    
    /* Do the indirection. */
    Address jumpTableAddress = 0;

    if (img_) {
        void *ptr = img_->getPtrToData(jumpTableAddressAddress);
        if (!ptr) return;
        jumpTableAddress = *((Address *)ptr);
    }
    else {
        void *ptr = proc_->getPtrToOrigInstruction(jumpTableAddressAddress);
        if (!ptr) return;
        jumpTableAddress = *((Address *)ptr);
    }
        

    // /* DEBUG */ fprintf( stderr, "jumpTableAddress = 0x%lx\n", jumpTableAddress );
    
    /* Check for Intel compiler signature.
       
    Deviating from the Intel IA64 SCRAG, icc generated jump table entries
    contain offsets from a label inside the originating function.  Find
    that label and store it in baseJumpAddress.
    */
    bool isGCC = true;
    uint64_t baseJumpAddress = jumpTableAddress;
    do {
        /* Rewind one instruction. */
        if( ! hasPrev() ) { break; } (*this)--;
        
        /* Acquire it. */
        instruction insn = getInstruction();
        
        /* Is it an integer operation? */
        instruction::unitType unitType = insn.getUnitType();
        if( unitType != instruction::I ) { continue; }
        
        /* If so, is it a mov from ip? */
        insn_tmpl tmpl = { insn.getMachineCode() };
        if( tmpl.I25.opcode != 0x0 ||
            tmpl.I25.x6 != 0x30 ||
            tmpl.I25.x3 != 0x0 ) continue;
        
        /* We've found the base jump address; stop looking. */
        baseJumpAddress = current & ~0xF;
        isGCC = false;
        break;
    } while( true );
    // /* DEBUG */ fprintf( stderr, "baseJumpAddress = 0x%lx\n", baseJumpAddress );
    
    /* Read n entries from the jump table, summing with jumpTableAddress
       to add to the set targetAddresses. */

#if 0
    if( !addressProc->readTextSpace( (void *)jumpTableAddress, sizeof( uint64_t ) * maxTableLength, jumpTable ) ) {
        bperr( "Could not read address of jump table (0x%lx) in mutatee.\n", jumpTableAddress );
        assert( 0 );
    }
#endif
    
    for( unsigned int i = 0; i < maxTableLength; i++ ) {
        uint64_t finalAddr = baseJumpAddress;

        // Get the jump table addr
        if (img_) {
            void *ptr = img_->getPtrToData(jumpTableAddress + (sizeof(uint64_t) * i));
            if (!ptr) continue;
            finalAddr += *((uint64_t *)ptr);
        }
        else {
            void *ptr = proc_->getPtrToOrigInstruction(jumpTableAddress + (sizeof(uint64_t) * i));
            if (!ptr) continue;
            finalAddr += *((uint64_t *)ptr);
        }

        
        /* Deviating from the Intel IA64 SCRAG, GCC generated jump
           table entries contain offsets from the jump table entry
           address.  Not the base of the jump table itself.
        */
        if (isGCC) finalAddr += i * 8;

        targetAddresses.insert(finalAddr);
    } /* end jump table iteration */
    
    setCurrentAddress( originalAddress );
} /* end getMultipleJumpTargets() */

bool InstrucIter::delayInstructionSupported()
{
    return false;
}

bool InstrucIter::hasMore()
{
    if( (current >= base) &&
        addressIsValidInsnAddr( current ) &&
        current < (base + range) ) {
        return true;
    } else {
        return false;
    }
}

bool InstrucIter::hasPrev()
{
	if( (current > base) &&
		addressIsValidInsnAddr( current ) &&
		current < (base + range) ) {
		return true;
	} else {
		return false;
	}
}

Address InstrucIter::peekPrev()
{
    int instructionSlot = extractInstructionSlot( current ); 
    switch( instructionSlot ) {
    case 0: {
        // This is complicated. If we're going onto a bundle
        // with a long insn, skip 2.
        Address temp = current - 16;
        void *tmpPtr;
        if (proc_) {
            tmpPtr = proc_->getPtrToOrigInstruction(temp);
        }
        else {
            assert(img_); 
            if (!img_->isValidAddress(temp)) {
                assert(0);
            }
            else tmpPtr = img_->getPtrToOrigInstruction(temp);
        }            
        
        ia64_bundle_t *rawBundle = (ia64_bundle_t *)tmpPtr;
        IA64_bundle tmpBundle = IA64_bundle(*rawBundle);
        
        if (tmpBundle.hasLongInstruction())
            return current - 16 + 1;
        else
            return current - 16 + 2;
    }
    case 1: return current - 1;
    case 2: 
        if (bundle.hasLongInstruction()) {
            return current - 2;
        }
        else
            return current - 1;
    default: return 0;
    }
}

Address InstrucIter::peekNext()
{
    int instructionSlot = extractInstructionSlot( current );
    switch( instructionSlot ) {
    case 0: return current + 1;
    case 1: 
        if (bundle.hasLongInstruction())
            return current - 1 + 16;
        else
            return current + 1;
    case 2: return current - 2 + 16;
    default: return 0;
    }
}

void InstrucIter::setCurrentAddress(Address addr)
{
    assert( (current % 0x10) < 3 );
    current = addr;
    initializeInsn();
}

instruction InstrucIter::getInstruction()
{	
    return insn;
}

instruction InstrucIter::getNextInstruction()
{	
    (*this)++;
    return insn;
}

instruction InstrucIter::getPrevInstruction()
{	
    (*this)--;
    return insn;
}

Address InstrucIter::operator++()
{
    current = peekNext();
    initializeInsn();
    return current;
}

Address InstrucIter::operator--()
{
    current = peekPrev();
    initializeInsn();
    return current;
}

Address InstrucIter::operator++(int)
{
    Address ret = current;
    current = peekNext();
    initializeInsn();
    return ret;
}

Address InstrucIter::operator--(int)
{
    Address ret = current;
    current = peekPrev();
    initializeInsn();
    return ret;
}

Address InstrucIter::operator*()
{	
    return current;
}

// Suuuure...
bool InstrucIter::isStackFramePreamble(int &) {
    return true;
}

void InstrucIter::initializeInsn() {
    unsigned short slotno = current % 16;
    Address aligned = current - slotno;

   if (proc_) {
        instPtr = proc_->getPtrToOrigInstruction(aligned);
    }
    else {
        assert(img_); 
        if (!img_->isValidAddress(aligned)) {
            fprintf(stderr, "Error: addr 0x%x is not valid!\n",
                    img_);
        }
        else instPtr = img_->getPtrToOrigInstruction(aligned);
    }            

   ia64_bundle_t *rawBundle = (ia64_bundle_t *)instPtr;
   bundle = IA64_bundle(*rawBundle);

   if (bundle.hasLongInstruction() && 
       (slotno == 2)) {
       cerr << "Attempt to grab the middle of a long instruction!" << endl;
       assert(0);
   }

   instruction *newInsn = bundle.getInstruction(slotno);

   insn = *newInsn;
}
               
bool InstrucIter::isADynamicCallInstruction()
{
    return insn.getType() == instruction::INDIRECT_CALL;
}
