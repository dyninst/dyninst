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
bool isAReturnInstruction(const instruction i){
	if((i.mem_jmp.opcode == OP_MEM_BRANCH) && 
	   (i.mem_jmp.ext == MD_RET) &&
	   (i.mem_jmp.ra == 31))
	{
		return true;
	}
	return false;
}

/** is the instruction an indirect jump instruction 
  * @param i the instruction value 
  */
bool isAIndirectJumpInstruction(const instruction i){
	if((i.mem_jmp.opcode == OP_MEM_BRANCH) && 
	   (i.mem_jmp.ext == MD_JMP) &&
	   (i.mem_jmp.ra == 31))
	{
		return true;
	}
	return false;
}

/** is the instruction a conditional branch instruction 
  * @param i the instruction value 
  */ 
bool isACondBranchInstruction(const instruction i){
	if((i.branch.opcode == OP_BR) || 
	   (i.branch.opcode == OP_BSR))
		return false;
	if((i.branch.opcode & ~0xf) == 0x30)
	{
		return true;
	}
	return false;
}
/** is the instruction an unconditional branch instruction 
  * @param i the instruction value 
  */
bool isAJumpInstruction(const instruction i){
	if((i.branch.opcode == OP_BR) &&
	   (i.branch.ra == 31))
	{
		return true;
	}
	return false;
}
/** is the instruction a call instruction 
  * @param i the instruction value 
  */
bool isACallInstruction(const instruction i){
	if((i.branch.opcode == OP_BSR) ||
	   ((i.mem_jmp.opcode == OP_MEM_BRANCH) && 
	    (i.mem_jmp.ext == MD_JSR)))
		return true;
	return false;
}

bool isAnneal(const instruction i){
        return true;
}

/** function which returns the offset of control transfer instructions
  * @param i the instruction value 
  */
Address getBranchTargetAddress(const instruction i,Address pos){
	pos += sizeof(instruction);
	int offset = i.branch.disp << 2;
	return (Address)(pos + offset);
}

void initOpCodeInfo()
{
}

/* NOT yet implemented. */
MemoryAccess isLoadOrStore(const instruction i)
{
  return MemoryAccess::none;
}

//Address Handle used by flowGraph which wraps the instructions
//and supply enough operation to iterate over the instrcution sequence.

AddressHandle::AddressHandle(process* fProcess,
			     image* fImage,
			     Address bAddress,
			     unsigned fSize)
	: addressProc(fProcess),
	  addressImage(fImage),baseAddress(bAddress),
	  range(fSize),currentAddress(bAddress) {}

AddressHandle::AddressHandle(const AddressHandle& ah){
	addressImage = ah.addressImage;
	addressProc = ah.addressProc;
	baseAddress = ah.baseAddress;
	currentAddress = ah.currentAddress;
	range = ah.range;
}

AddressHandle::~AddressHandle(){}

void AddressHandle::getMultipleJumpTargets(BPatch_Set<Address>& result){

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
Address AddressHandle::nextAddress(){
	Address ret = currentAddress + sizeof(instruction);
	return ret;
}
void AddressHandle::setCurrentAddress(Address addr){
	currentAddress = addr;
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
