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

  if((i.mem_jmp.opcode == OP_MEM_BRANCH) && 
     (i.mem_jmp.ext == MD_RET) &&
     (i.mem_jmp.ra == 31))
    return true;
  return false;
}

/** is the instruction an indirect jump instruction 
  * @param i the instruction value 
  */
bool InstrucIter::isAIndirectJumpInstruction()
{
  const instruction i = getInstruction();

  if((i.mem_jmp.opcode == OP_MEM_BRANCH) && 
     (i.mem_jmp.ext == MD_JMP) &&
     (i.mem_jmp.ra == 31))
    return true;
  return false;
}

/** is the instruction a conditional branch instruction 
  * @param i the instruction value 
  */ 
bool InstrucIter::isACondBranchInstruction()
{
  const instruction i = getInstruction();

  if((i.branch.opcode == OP_BR) || 
     (i.branch.opcode == OP_BSR))
    return false;
  if((i.branch.opcode & ~0xf) == 0x30)
    return true;
  return false;
}

/** is the instruction an unconditional branch instruction 
  * @param i the instruction value 
  */
bool InstrucIter::isAJumpInstruction()
{
  const instruction i = getInstruction();

  if((i.branch.opcode == OP_BR) &&
     (i.branch.ra == 31))
    return true;
  return false;
}

/** is the instruction a call instruction 
  * @param i the instruction value 
  */
bool InstrucIter::isACallInstruction()
{
  const instruction i = getInstruction();

  if((i.branch.opcode == OP_BSR) ||
     ((i.mem_jmp.opcode == OP_MEM_BRANCH) && 
      (i.mem_jmp.ext == MD_JSR)))
    return true;
  return false;
}

bool InstrucIter::isAnneal()
{
  return true;
}

/** function which returns the offset of control transfer instructions
  * @param i the instruction value 
  */
Address InstrucIter::getBranchTargetAddress(Address pos)
{
  const instruction i = getInstruction();

  pos += sizeof(instruction);
  int offset = i.branch.disp << 2;
  return (Address)(pos + offset);
}

void initOpCodeInfo()
{
}

/* NOT yet implemented. */
BPatch_memoryAccess* InstrucIter::isLoadOrStore()
{
  return BPatch_memoryAccess::none;
}

BPatch_instruction *InstrucIter::getBPInstruction() {

  BPatch_memoryAccess *ma = isLoadOrStore();
  BPatch_instruction *in;

  if (ma != BPatch_memoryAccess::none)
    return ma;

  const instruction i = getInstruction();
  in = new BPatch_instruction(&i.raw, sizeof(instruction));

  return in;
}

void InstrucIter::getMultipleJumpTargets(BPatch_Set<Address>& result)
{
    instruction check;
    Address jumpTableLoc = 0;
    Address jumpTableOffset = 0;
    bool jumpTableIndirect;
    Address initialAddress = currentAddress - INSN_SIZE;

    //
    // Search Backwards for an LDQ LDL or LDAH LDL instruction pair.
    // Allow one instruction between the pair.  Note, we are searching
    // backwards, so look for LDL first.
    //
    (*this)--;
    while (hasMore()) {
	check = getInstruction();
	if (check.mem.opcode == OP_LDL) {
	    jumpTableOffset = check.mem.disp;
	    (*this)--;
	    (*this)--;
	    check = getInstruction();
	    if ((check.mem.opcode == OP_LDQ || check.mem.opcode == OP_LDAH) &&
		check.mem.rb == 29) {
		jumpTableIndirect = (check.mem.opcode == OP_LDQ);
		jumpTableLoc = check.mem.disp;
		break;
	    }
	}
	jumpTableOffset = 0;
	(*this)--;
    }
    if (!jumpTableLoc)
	return;

    //
    // Search for a CMPULE BEQ instruction pair.
    // Allow one instruction between the pair.
    //
    (*this)--;
    unsigned maxSwitch = 0;
    while (hasMore()){
	check = getInstruction();
	if (check.branch.opcode == OP_BEQ) {
	    (*this)--;
	    check = getInstruction();
	    if (check.oper_lit.opcode == OP_CMPULE &&
		check.oper_lit.function == FC_CMPULE &&
		check.oper_lit.one == 1) {
		maxSwitch = check.oper_lit.lit + 1;
		break;
	    }

	    (*this)--;
	    check = getInstruction();
	    if (check.oper_lit.opcode == OP_CMPULE &&
		check.oper_lit.function == FC_CMPULE &&
		check.oper_lit.one == 1) {
		maxSwitch = check.oper_lit.lit + 1;
		break;
	    }
	}
	(*this)--;
    }
    if (!maxSwitch){
	result += (initialAddress + INSN_SIZE);
	return;
    }

    //
    // Search for current $gp.
    //
    Address GOT_Value = 0;
    Address gpBase;
    while (hasMore()) {
	check = getInstruction();
	if (check.mem.opcode == OP_LDAH && check.mem.ra == 29 &&
	    (check.mem.rb == 27 || check.mem.rb == 26)) {

	    int disp = check.mem.disp * 0x10000;
	    gpBase = (check.mem.rb == 26 ? currentAddress : baseAddress);

	    (*this)++;
	    check = getInstruction();
	    if (check.mem.opcode == OP_LDA &&
		check.mem.ra == 29 && check.mem.rb == 29) {
		disp += check.mem.disp;
		GOT_Value = gpBase + disp;
	    }
	    break;
	}
	(*this)--;
    }
    if (!GOT_Value)
	return;

    Address jumpTableAddress;
    if (jumpTableIndirect) {
	jumpTableLoc += GOT_Value;
	addressProc->readTextSpace((const void*)jumpTableLoc,
				   sizeof(Address),
				   (const void*)&jumpTableAddress);
    } else {
	jumpTableAddress = (jumpTableLoc * 0x10000) + GOT_Value;
    }

    for (unsigned int i = 0; i < maxSwitch; i++) {
	int tableEntryValue;
	Address tableEntryAddress = jumpTableAddress + jumpTableOffset;

	tableEntryAddress += i * INSN_SIZE;
	addressProc->readTextSpace((const void*)tableEntryAddress,
				   INSN_SIZE,
				   (const void*)&tableEntryValue);
	result += (Address)(GOT_Value + tableEntryValue) & ~0x3;
    }
}
bool InstrucIter::delayInstructionSupported(){
	return false;
}
bool InstrucIter::hasMore(){
	if((currentAddress < (baseAddress + range )) &&
	   (currentAddress >= baseAddress))
		return true;
	return false;
}
bool InstrucIter::hasPrev(){
    if((currentAddress < (baseAddress + range )) &&
       (currentAddress > baseAddress))
	return true;
    return false;
}
Address InstrucIter::prevAddress(){
	Address ret = currentAddress-sizeof(instruction);
	return ret;
}
Address InstrucIter::nextAddress(){
	Address ret = currentAddress + sizeof(instruction);
	return ret;
}
void InstrucIter::setCurrentAddress(Address addr){
	currentAddress = addr;
}
instruction InstrucIter::getInstruction(){
	instruction ret;
	ret.raw = addressImage->get_instruction(currentAddress);
	return ret;
}
instruction InstrucIter::getNextInstruction(){
	instruction ret;
	ret.raw = addressImage->get_instruction(currentAddress+sizeof(instruction));
	return ret;
}
instruction InstrucIter::getPrevInstruction(){
	instruction ret;
	ret.raw = addressImage->get_instruction(currentAddress-sizeof(instruction));
	return ret;
}
Address InstrucIter::operator++(){
	currentAddress += sizeof(instruction);
	return currentAddress;
}
Address InstrucIter::operator--(){
	currentAddress -= sizeof(instruction);
	return currentAddress;
}
Address InstrucIter::operator++(int){
	Address ret = currentAddress;
	currentAddress += sizeof(instruction);
	return ret;
}
Address InstrucIter::operator--(int){
	Address ret = currentAddress;
	currentAddress -= sizeof(instruction);
	return ret;
}
Address InstrucIter::operator*(){
	return currentAddress;
}
