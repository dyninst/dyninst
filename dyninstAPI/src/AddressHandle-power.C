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
	if((i.xlform.op == BCLRop) &&
	   (i.xlform.xo == BCLRxop) && 
	   (i.xlform.bt & 0x10) && (i.xlform.bt & 0x4))
		return true;
	return false;
}

/** is the instruction an indirect jump instruction 
  * @param i the instruction value 
  */
bool isLocalIndirectJump(const instruction i,AddressHandle ah){
	if((i.xlform.op == BCLRop) && (i.xlform.xo == BCCTRxop) &&
	   !i.xlform.lk && (i.xlform.bt & 0x10) && (i.xlform.bt & 0x4))
		return true;

	if((i.xlform.op == BCLRop) && (i.xlform.xo == BCLRxop) &&
	   (i.xlform.bt & 0x10) && (i.xlform.bt & 0x4)){
		--ah;--ah;
		if(!ah.hasMore())
			return false;
		instruction j = ah.getInstruction();
		if((j.xfxform.op == 31) && (j.xfxform.xo == 467) &&
		   (j.xfxform.spr == 0x100))
			return true;
	}
	return false;
}

/** is the instruction a conditional branch instruction 
  * @param i the instruction value 
  */ 
bool isLocalCondBranch(const instruction i){
	if((i.bform.op == BCop) && !i.bform.lk &&
	   !((i.bform.bo & 0x10) && (i.bform.bo & 0x4)))
		return true;
	return false;
}
/** is the instruction an unconditional branch instruction 
  * @param i the instruction value 
  */
bool isLocalJump(const instruction i){
	if((i.iform.op == Bop) && !i.iform.lk)
		return true;
	if((i.bform.op == BCop) && !i.bform.lk &&
	   (i.bform.bo & 0x10) && (i.bform.bo & 0x4))
		return true;
	return false;
}
/** is the instruction a call instruction 
  * @param i the instruction value 
  */
bool isLocalCall(const instruction i){
	cout << "CALL called\n";
	if(i.iform.lk && 
	   ((i.iform.op == Bop) || (i.bform.op == BCop) ||
	    ((i.xlform.op == BCLRop) && 
	     ((i.xlform.xo == 16) || (i.xlform.xo == 528))))){
		cout << "TIKIR : found CALL\n";
		return true;
	}
	return false;
}
/** function which returns the offset of control transfer instructions
  * @param i the instruction value 
  */
Address getBranchTargetAddress(const instruction i,Address pos){
	Address ret = 0;
	if((i.iform.op == Bop) || (i.bform.op == BCop)){
		int disp = 0;
		if(i.iform.op == Bop)
			disp = i.iform.li;
		else if(i.bform.op == BCop)
			disp = i.bform.bd;
		disp <<= 2;
		if(i.iform.aa)
			ret = (Address)disp;
		else
			ret = (Address)(pos+disp);
	}
	return (Address)ret;
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

	(*this)--;
	Address initialAddress = currentAddress;
	Address TOC_address = (addressImage->getObject()).getTOCoffset();

	instruction check;
	Address jumpStartAddress = 0;
	while(hasMore()){
		check = getInstruction();
		if((check.dform.op == Lop) && (check.dform.ra == 2)){
			jumpStartAddress = 
				(Address)(TOC_address + check.dform.d_or_si);
			break;
		}
		(*this)--;
	}
	(*this)--;
	Address adjustEntry = 0;
	check = getInstruction();
	if((check.dform.op == Lop))
		adjustEntry = check.dform.d_or_si;

	Address tableStartAddress = 0;
	while(hasMore()){
		instruction check = getInstruction();
		if((check.dform.op == Lop) && (check.dform.ra == 2)){
			tableStartAddress = 
				(Address)(TOC_address + check.dform.d_or_si);
			break;
		}
		(*this)--;
	}

	setCurrentAddress(initialAddress);
	int maxSwitch = 0;
	while(hasMore()){
		instruction check = getInstruction();
		if((check.bform.op == BCop) && 
	           !check.bform.aa && !check.bform.lk){
			(*this)--;
			check = getInstruction();
			if(10 != check.dform.op)
				break;
			maxSwitch = check.dform.d_or_si + 1;
			break;
		}
		(*this)--;
	}

	Address jumpStart = 
		(Address)addressImage->get_instruction(jumpStartAddress);
	Address tableStart = 
		(Address)addressImage->get_instruction(tableStartAddress);

	for(int i=0;i<maxSwitch;i++){
		Address tableEntry = adjustEntry + tableStart + (i * sizeof(instruction));
		int jumpOffset = (int)addressImage->get_instruction(tableEntry);
		result += (Address)(jumpStart+jumpOffset);
	}
	if(!maxSwitch)
		result += (initialAddress + sizeof(instruction));
}

bool AddressHandle::delayInstructionSupported(){
	return false;
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
	return range / sizeof(instruction);
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
