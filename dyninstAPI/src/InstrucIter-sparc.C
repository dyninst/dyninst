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

//some more function used to identify the properties of the instruction
/** is the instruction used to return from the functions
 * @param i the instruction value 
 */
bool InstrucIter::isAReturnInstruction()
{
  const instruction i = getInstruction();

  if(((*i).resti.op == 0x2) && ((*i).resti.op3 == 0x38) &&
     ((*i).resti.rd == 0) && ((*i).resti.i == 0x1) &&
     (((*i).resti.rs1 == 0xf) || ((*i).resti.rs1 == 0x1f)) &&
     (((*i).resti.simm13 == 8) || ((*i).resti.simm13 == 12)))
      return true;
  return false;
}

/** is the instruction an indirect jump instruction 
  * @param i the instruction value 
  */
bool InstrucIter::isAIndirectJumpInstruction()
{
  const instruction i = getInstruction();

  if(((*i).resti.op == 0x2) && ((*i).resti.op3 == 0x38) &&
     ((*i).resti.rd == 0) && ((*i).resti.rs1 != 0xf) && 
     ((*i).resti.rs1 != 0x1f))
  {
      if((!(*i).resti.i && ((*i).restix.rs2 == 0)) ||
	   ((*i).resti.i && ((*i).resti.simm13 == 0)))
		return true;
  }
  return false;
}

/** is the instruction a conditional branch instruction 
  * @param i the instruction value 
  */ 
bool InstrucIter::isACondBranchInstruction()
{
  const instruction i = getInstruction();

  if(((*i).branch.op == 0) &&
     ((*i).branch.op2 == 2 || (*i).branch.op2 == 6) &&
     ((*i).branch.cond != 0) && ((*i).branch.cond != 8))
    return true;
  return false;
}
/** is the instruction an unconditional branch instruction 
  * @param i the instruction value 
  */
bool InstrucIter::isAJumpInstruction()
{
  const instruction i = getInstruction();

  if(((*i).branch.op == 0) &&
     ((*i).branch.op2 == 2 || (*i).branch.op2 == 6) &&
     ((*i).branch.cond == 8))
    return true;
  return false;
}
/** is the instruction a call instruction 
  * @param i the instruction value 
  */
bool InstrucIter::isACallInstruction()
{
  const instruction i = getInstruction();

  if((*i).call.op == 0x1)
    return true;
  return false;
}

bool InstrucIter::isAnneal()
{
  const instruction i = getInstruction();

  if((*i).branch.anneal)
    return true;
  return false;
}

void initOpCodeInfo()
{
  // none needed on SPARC
}

// TODO: maybe figure out a closed form for these functions
const unsigned int fpBytes[][2] = { { 4, 4 }, { 4, 8 }, { 16, 16 }, { 8, 8 } };
const unsigned int intBytes[] = { 4, 1, 2, 8 };
const unsigned int fishyBytes[] = { 0, 1, 8, 4 };

#define btst(x, bit) ((x) & (1<<(bit)))

#define MK_LDi0(bytes, in, rs1, rs2) (new BPatch_memoryAccess(&(*in).raw, instruction::size(), true, false, (bytes), 0, (rs1), (rs2)))
#define MK_STi0(bytes, in, rs1, rs2) (new BPatch_memoryAccess(&(*in).raw, instruction::size(), false, true, (bytes), 0, (rs1), (rs2)))
#define MK_LDi1(bytes, in, rs1, simm13) (new BPatch_memoryAccess(&(*in).raw, instruction::size(), true, false, (bytes), (simm13), (rs1), -1))
#define MK_STi1(bytes, in, rs1, simm13) (new BPatch_memoryAccess(&(*in).raw, instruction::size(), false, true, (bytes), (simm13), (rs1), -1))
#define MK_PFi0(in, rs1, rs2, f) (new BPatch_memoryAccess(&(*in).raw, instruction::size(), false, false, true, 0, (rs1), (rs2), \
                                           0, -1, -1, (f)))
#define MK_PFi1(in, rs1, simm13, f) (new BPatch_memoryAccess(&(*in).raw, instruction::size(), false, false, true, (simm13), (rs1), -1, \
                                              0, -1, -1, (f)))

#define MK_LD(bytes, in) ((*in).rest.i ? MK_LDi1(bytes, in, (*in).resti.rs1, (*in).resti.simm13) : \
                                      MK_LDi0(bytes, in, (*in).rest.rs1, (*in).rest.rs2))
#define MK_ST(bytes, in) ((*in).rest.i ? MK_STi1(bytes, in, (*in).resti.rs1, (*in).resti.simm13) : \
                                      MK_STi0(bytes, in, (*in).rest.rs1, (*in).rest.rs2))
#define MK_MA(bytes, in, load, store) \
          ((*in).rest.i ? \
              new BPatch_memoryAccess(&(*in).raw, instruction::size(), (load), (store), (bytes), (*in).resti.simm13, (*in).resti.rs1, -1) \
              : \
              new BPatch_memoryAccess(&(*in).raw, instruction::size(), (load), (store), (bytes), 0, (*in).rest.rs1, (*in).rest.rs2))
#define MK_PF(in) ((*in).rest.i ? MK_PFi1(in, (*in).resti.rs1, (*in).resti.simm13, (*in).resti.rd) : \
                               MK_PFi0(in, (*in).rest.rs1, (*in).rest.rs2, (*in).rest.rd))


// VG(09/20/01): SPARC V9 decoding after the architecture manual.
BPatch_memoryAccess* InstrucIter::isLoadOrStore()
{
  const instruction i = getInstruction();

  if((*i).rest.op != 0x3) // all memory opcodes have op bits 11
    return BPatch_memoryAccess::none;

  unsigned int op3 = (*i).rest.op3;

  if(btst(op3, 4)) { // bit 4 set means alternate space
    if((*i).rest.i) {
      logLine("SPARC: Alternate space instruction using ASI register currently unhandled...");
      return BPatch_memoryAccess::none;
    }
    else if((*i).rest.unused != ASI_PRIMARY) { // unused is actually imm_asi
      logLine("SPARC: Alternate space instruction using ASI != PRIMARY currently unhandled...");
      return BPatch_memoryAccess::none;
    }
    // else it is handled below assuming that endianness is big
  }

  if(btst(op3, 5)) { // bit 5 set means dig more
    if(btst(op3, 3)) { // bit 3 set means PREFETCH or CAS(X)
      // Actually CAS(X) is not implemented, it a synthetic
      // instruction that should be coded as CAS(X)A on ASI_P.
      assert(btst(op3,2)); // Catch reserved opcodes
      if(btst(op3, 0)) { // PREFETCH
        assert(!btst(op3, 1)); // Catch reserved opcode
        return MK_PF(i);
      }
      else { // CAS(X)A
        // XXX: the manual seems to have a bug not listed in the errata:
        // it claims that CASA uses the *word* in r[rs1] as address. IMHO
        // the address should always be a doubleword on V9...
        unsigned int b = btst(op3, 1) ? 8 : 4;
        // VG(12/08/01): CAS(X)A uses rs2 as value not address...
        return new BPatch_memoryAccess(&(*i).raw, instruction::size(), true, 
                                       true, b, 0, (*i).resti.rs1, -1);
      }
    }
    else { // bit 3 zero (1u0xyz) means fp memory op
      bool isStore = btst(op3, 2); // bit 2 gives L/S
      // bits 0-1 encode #bytes except for state register ops,
      // where the number of bits is given by bit 0 from rd
      unsigned int b = fpBytes[op3 & 0x3][(*i).rest.rd & 0x1];
      return isStore ? MK_ST(b, i) : MK_LD(b, i);
    }
  }
  else { // bit 5 zero means interger memory op
    // bit 2 almost gives L/S, except LDSTUB (load-store) and SWAP
    // also look like a store. (SWAP is deprecated on V9)
    bool isStore = btst(op3, 2);
    // bit 3 gives signed/unsigned for LOADS, but we ignore that;
    // for stores, it has no clear meaning: there are 5 pure store
    // opcodes, two more that are also loads, and one is reserved.
    // (see p. 269 in manual)
    if(isStore && btst(op3, 3)) { // fishy
      bool isLoad = btst(op3, 0); // SWAP & LDSTUB are both load and store
      unsigned int b = fishyBytes[op3 & 0x3];
      assert(b); // Catch reserved opcode
      return MK_MA(b, i, isLoad, isStore);
    }
    // else simple store, therefore continue
    unsigned int b = intBytes[op3 & 0x3]; // bits 0-1 encode #bytes
    return isStore ? MK_ST(b, i) : MK_LD(b, i);
  }
  return BPatch_memoryAccess::none;
}

BPatch_instruction *InstrucIter::getBPInstruction() {

  BPatch_memoryAccess *ma = isLoadOrStore();
  BPatch_instruction *in;

  if (ma != BPatch_memoryAccess::none)
    return ma;

  const instruction i = getInstruction();
  in = new BPatch_instruction(&(*i).raw, instruction::size());

  return in;
}

/** function which returns the offset of control transfer instructions
  * @param i the instruction value 
  */
Address InstrucIter::getBranchTargetOffset()
{
  const instruction i = getInstruction();

  return i.getOffset();
}

Address InstrucIter::getBranchTargetAddress() {
    const instruction i = getInstruction();
    return i.getTarget(current);
}

void InstrucIter::getMultipleJumpTargets(BPatch_Set<Address>& result){
    Address oldCurrent = current;
    while(hasMore()){
        instruction check = getInstruction();
        if(((*check).sethi.op == 0x0) && 
           ((*check).sethi.op2 == 0x4) &&
           ((*check).sethi.rd != 0x0))
            {
                register signed offset = (*check).sethi.imm22 << 10;
                check = getNextInstruction();
                if(((*check).resti.op == 0x2) &&
                   ((*check).resti.op3 == 0x2) &&
                   ((*check).resti.i == 0x1)){
                    register signed lowData = (*check).resti.simm13 & 0x3ff;
                    offset |= lowData;

                    while(true) { // Probably should calculate max table size
                        // as on other platforms.
                        void *targetPtr = NULL;
                        
                        if (img_) {
                            if (img_->isCode(offset))
                                targetPtr = img_->getPtrToOrigInstruction(offset);
                        }
                        else {
                            // Process
                            targetPtr = proc_->getPtrToOrigInstruction(offset);
                        }

                        if (targetPtr == NULL) break;

                        // This is a horrid way to catch the end of the table;
                        // however, I don't know enough about SPARC to fix it.

                        Address target = *((Address *)targetPtr);
                        bool valid = true;

                        if (img_) {
                            if (!img_->isCode(target))
                                valid = false;
                            // I've seen this as well, when they pad
                            // jump tables.
                            if (target == img_->codeOffset())
                                valid = false;
                        }
                        else {
                            if (!proc_->getPtrToOrigInstruction(target))
                                valid = false;
                        }

                        instruction check(target);
                        if (check.valid()) {
                            // Apparently, we've hit the end of the... something.
                            valid = false;
                        }
                        if (target == 0) {
                            // What?
                            valid = false;
                        }
                        if (!valid) break;

                        result += target;
                        offset += instruction::size();
                    }

                    setCurrentAddress(oldCurrent);
                    return;
                }
            }
        (*this)--;
    }
    setCurrentAddress(oldCurrent);
}
bool InstrucIter::delayInstructionSupported(){
  return true;
}

Address InstrucIter::peekPrev() {
    // What about delay slots?
    Address ret = current-instruction::size();
    return ret;
}

Address InstrucIter::peekNext() {
    // Delay slot?
    Address ret = current + instruction::size();
    return ret;
}

void InstrucIter::setCurrentAddress(Address addr){
  current = addr;
}

instruction InstrucIter::getInstruction(){
    return insn;
}

instruction InstrucIter::getNextInstruction(){
    instruction ret;
    if (img_)
        (*ret) = *((instructUnion *)img_->getPtrToOrigInstruction(peekNext()));
    else {
        (*ret) = *((instructUnion *)proc_->getPtrToOrigInstruction(peekNext()));
    }
    return ret;
}
instruction InstrucIter::getPrevInstruction(){
    instruction ret;
    if (img_)
        (*ret) = *((instructUnion *)img_->getPtrToOrigInstruction(peekPrev()));
    else {
        (*ret) = *((instructUnion *)proc_->getPtrToOrigInstruction(peekPrev()));
    }
    return ret;
}

Address InstrucIter::operator++(){
  current += instruction::size();
  initializeInsn();
  return current;
}
Address InstrucIter::operator--(){
  current -= instruction::size();
  initializeInsn();
  return current;
}
Address InstrucIter::operator++(int){
  Address ret = current;
  current += instruction::size();
  initializeInsn();
  return ret;
}
Address InstrucIter::operator--(int){
  Address ret = current;
  current -= instruction::size();
  initializeInsn();
  return ret;
}
Address InstrucIter::operator*(){
  return current;
}

// Check to see if we make a stack frame; in Sparc terms,
// execute a save instruction
bool InstrucIter::isStackFramePreamble(int &) {
    while (!isAReturnInstruction() &&
           !isACondBranchInstruction() &&
           !isACallInstruction() &&
           !isADynamicCallInstruction() &&
           !isAJumpInstruction() &&
           insn.valid()) {
        if (insn.isInsnType(SAVEmask, SAVEmatch)) 
            return true;
        (*this)++;
    }
    return false;
}

bool InstrucIter::isADynamicCallInstruction() {
    instruction i = getInstruction();
    return i.isInsnType(CALLImask, CALLImatch);
}

void InstrucIter::getAndSkipDSandAgg(instruction &ds,
                                     bool &validDS,
                                     instruction &agg,
                                     bool &validAgg) {
    validDS = false;
    validAgg = false;
    instruction insn = getInstruction();
    if (!insn.isDCTI())
        return;
    // Get the next two instructions, by address
    // since we don't know where we are in the bbl.
    
    void *dsPtr;
    void *aggPtr;

    if (proc_) {
        dsPtr = proc_->getPtrToOrigInstruction(current + instruction::size());
        aggPtr = proc_->getPtrToOrigInstruction(current + 2*instruction::size());
    }
    else {
        assert(img_); 
        if (!img_->isValidAddress(current + instruction::size())) {
            fprintf(stderr, "Error: addr 0x%x is not valid!\n",
                    img_);
        }
        else {
            dsPtr = img_->getPtrToOrigInstruction(current+instruction::size());
            aggPtr = img_->getPtrToOrigInstruction(current+2*instruction::size());
        }            
    }
    assert(dsPtr);
    ds.setInstruction((codeBuf_t *)dsPtr);
    validDS = true;
    assert(aggPtr);
    agg.setInstruction((codeBuf_t *)aggPtr);
    if (!agg.valid()) {
        if ((*agg).raw != 0x0)
            validAgg = true;
    }
}

