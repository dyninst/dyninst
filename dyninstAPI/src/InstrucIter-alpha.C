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

void InstrucIter::getMultipleJumpTargets(BPatch_Set<Address>& result){

        (*this)--;
	Address initialAddress = currentAddress;

        instruction check;
        int jumpTableOffset = 0;
        while(hasMore()){
                check = getInstruction();
		if((check.mem.opcode == OP_LDQ) && (check.mem.rb == 29)){
			jumpTableOffset = check.mem.disp;
			break;
		}
                (*this)--;
        }
	if(!jumpTableOffset)
		return;
        (*this)--;
        unsigned maxSwitch = 0;
        while(hasMore()){
                check = getInstruction();
		if(check.branch.opcode == OP_BEQ){
                        (*this)--;
                        check = getInstruction();
                	if((check.oper_lit.opcode != OP_CMPLUE) ||
		   	   (check.oper_lit.function != 0x3d) ||
		   	   (!check.oper_lit.one))
				break;
                        maxSwitch = check.oper_lit.lit + 1;
                        break;
                }
                (*this)--;
        }
        if(!maxSwitch){
                result += (initialAddress + sizeof(instruction));
		return;
	}

	currentAddress = baseAddress;
	Address GOT_Value = 0;
	while(hasMore()){
		check = getInstruction();
		if((check.mem.opcode == OP_LDAH) && 
		   (check.mem.ra == 29) &&
		   (check.mem.rb == 27))
		{
			int highDisp = check.mem.disp;
			(*this)++;
			check = getInstruction();
			if((check.mem.opcode != OP_LDA) ||
			   (check.mem.ra != 29) || 
			   (check.mem.rb != 29))
				return;	
			int lowDisp = check.mem.disp;
			GOT_Value = (Address)((long)baseAddress + (highDisp * (long)0x10000) + lowDisp);
			break;
		}
		(*this)++;
	}
	if(!GOT_Value)
		return;

        Address jumpTableAddress = 
		(Address)((long)GOT_Value + jumpTableOffset);
	Address jumpTable = 0;
	addressProc->readTextSpace((const void*)jumpTableAddress,
				   sizeof(Address),
				   (const void*)&jumpTable);

        for(unsigned int i=0;i<maxSwitch;i++){
                Address tableEntry = jumpTable + (i * sizeof(instruction));
                int jumpOffset = 0;
		addressProc->readTextSpace((const void*)tableEntry,
					   sizeof(Address),
					   (const void*)&jumpOffset);
                result += (Address)((long)GOT_Value + jumpOffset) & ~0x3;
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
