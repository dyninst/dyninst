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
#include "AddressHandle.h"

#include "BPatch_Set.h"

//some more function used to identify the properties of the instruction
/** is the instruction used to return from the functions
  * @param i the instruction value 
  */
bool isReturn(const instruction i){

	if((i.resti.op == 0x2) && (i.resti.op3 == 0x38) &&
	   (i.resti.rd == 0) && (i.resti.i == 0x1) &&
	   ((i.resti.rs1 == 0xf) || (i.resti.rs1 == 0x1f)) &&
	   ((i.resti.simm13 == 8) || (i.resti.simm13 == 12)))
		return true;
	return false;
}

/** is the instruction an indirect jump instruction 
  * @param i the instruction value 
  */
bool isLocalIndirectJump(const instruction i){

	if((i.resti.op == 0x2) && (i.resti.op3 == 0x38) &&
	   (i.resti.rd == 0) && (i.resti.rs1 != 0xf) && 
	   (i.resti.rs1 != 0x1f))
		return true;
	return false;
}

/** is the instruction a conditional branch instruction 
  * @param i the instruction value 
  */ 
bool isLocalCondBranch(const instruction i){
	if((i.branch.op == 0) &&
	   (i.branch.op2 == 2 || i.branch.op2 == 6) &&
	   (i.branch.cond != 0) && (i.branch.cond != 8))
		return true;
	return false;
}
/** is the instruction an unconditional branch instruction 
  * @param i the instruction value 
  */
bool isLocalJump(const instruction i){
	if((i.branch.op == 0) &&
	   (i.branch.op2 == 2 || i.branch.op2 == 6) &&
	   (i.branch.cond == 8))
		return true;
	return false;
}
/** is the instruction a call instruction 
  * @param i the instruction value 
  */
bool isLocalCall(const instruction i){
	if(i.call.op == 0x1)
		return true;
	return false;
}
/** function which returns the offset of control transfer instructions
  * @param i the instruction value 
  */
Address getBranchTargetAddress(const instruction i,Address pos){
	int ret;
	if(i.branch.op == 0)
		ret = i.branch.disp22;
	else if(i.call.op == 0x1)
		ret = i.call.disp30;
	ret <<= 2;
	return (Address)(ret+pos);
}

//Address Handle used by flowGraph which wraps the instructions
//and supply enough operation to iterate over the instrcution sequence.

AddressHandle::AddressHandle(image* fImage,
			     Address bAddress,
			     unsigned fSize)
	: addressImage(fImage),baseAddress(bAddress),
	  range(fSize),currentAddress(bAddress) {}

AddressHandle::AddressHandle(Address cAddress,image* fImage,
			     Address bAddress,
			     unsigned fSize)
	: addressImage(fImage),baseAddress(bAddress),
	  range(fSize),currentAddress(cAddress) {}

AddressHandle::AddressHandle(const AddressHandle& ah){
	addressImage = ah.addressImage;
	baseAddress = ah.baseAddress;
	currentAddress = ah.currentAddress;
	range = ah.range;
}
void AddressHandle::getMultipleJumpTargets(BPatch_Set<Address>& result){
	while(hasMore()){
		instruction check = getInstruction();
		if((check.sethi.op == 0x0) && 
		   (check.sethi.op2 == 0x4) &&
		   (check.sethi.rd != 0x0))
		{
			register signed offset = check.sethi.imm22 << 10;
			check = getNextInstruction();
			if((check.resti.op == 0x2) &&
			   (check.resti.op3 == 0x2) &&
			   (check.resti.i == 0x1)){
				register signed lowData = check.resti.simm13 & 0x3ff;
				offset |= lowData;
				setCurrentAddress((Address)offset);
				for(;;){
					check = getInstruction();
					if(IS_VALID_INSN(check))
						break;
					result += check.raw;
					(*this)++;
				}
				return;
			}
		}
		(*this)--;
	}
}
bool AddressHandle::delayInstructionSupported(){
	return true;
}

bool AddressHandle::hasMore(){
	if((currentAddress < (baseAddress + range )) &&
	   (currentAddress >= baseAddress))
		return true;
	return false;
}
bool AddressHandle::hasPrev(){
    if((currentAddress < (baseAddress + range )) &&
       (currentAddress > baseAddress))
	return true;
    return false;
}
Address AddressHandle::prevAddress(){
	Address ret = currentAddress-sizeof(instruction);
	return ret;
}
Address AddressHandle::prevAddressOf(Address addr){
	Address ret = addr - sizeof(instruction);
	return ret;
}
Address AddressHandle::nextAddress(){
	Address ret = currentAddress + sizeof(instruction);
	return ret;
}
Address AddressHandle::nextAddressOf(Address addr){
	Address ret = addr + sizeof(instruction);
	return ret;
}
void AddressHandle::setCurrentAddress(Address addr){
	currentAddress = addr;
}
unsigned AddressHandle::getInstructionCount(){
	return range / sizeof(Word);
}
instruction AddressHandle::getInstruction(){
	instruction ret;
	ret.raw = addressImage->get_instruction(currentAddress);
	return ret;
}
instruction AddressHandle::getNextInstruction(){
	instruction ret;
	ret.raw = addressImage->get_instruction(currentAddress+sizeof(instruction));
	return ret;
}
instruction AddressHandle::getPrevInstruction(){
	instruction ret;
	ret.raw = addressImage->get_instruction(currentAddress-sizeof(instruction));
	return ret;
}
Address AddressHandle::operator++(){
	currentAddress += sizeof(instruction);
	return currentAddress;
}
Address AddressHandle::operator--(){
	currentAddress -= sizeof(instruction);
	return currentAddress;
}
Address AddressHandle::operator++(int){
	Address ret = currentAddress;
	currentAddress += sizeof(instruction);
	return ret;
}
Address AddressHandle::operator--(int){
	Address ret = currentAddress;
	currentAddress -= sizeof(instruction);
	return ret;
}
Address AddressHandle::operator*(){
	return currentAddress;
}
