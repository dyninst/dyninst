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
bool isAIndirectJumpInstruction(const instruction i){

	if((i.resti.op == 0x2) && (i.resti.op3 == 0x38) &&
	   (i.resti.rd == 0) && (i.resti.rs1 != 0xf) && 
	   (i.resti.rs1 != 0x1f))
		return true;
	return false;
}

/** is the instruction a conditional branch instruction 
  * @param i the instruction value 
  */ 
bool isACondBranchInstruction(const instruction i){
	if((i.branch.op == 0) &&
	   (i.branch.op2 == 2 || i.branch.op2 == 6) &&
	   (i.branch.cond != 0) && (i.branch.cond != 8))
		return true;
	return false;
}
/** is the instruction an unconditional branch instruction 
  * @param i the instruction value 
  */
bool isAJumpInstruction(const instruction i){
	if((i.branch.op == 0) &&
	   (i.branch.op2 == 2 || i.branch.op2 == 6) &&
	   (i.branch.cond == 8))
		return true;
	return false;
}
/** is the instruction a call instruction 
  * @param i the instruction value 
  */
bool isACallInstruction(const instruction i){
	if(i.call.op == 0x1)
		return true;
	return false;
}

bool isAnneal(const instruction i){
	if(i.branch.anneal)
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

#define MK_LDi0(bytes, rs1, rs2) (BPatch_memoryAccess(true, false, (bytes), 0, (rs1), (rs2)))
#define MK_STi0(bytes, rs1, rs2) (BPatch_memoryAccess(false, true, (bytes), 0, (rs1), (rs2)))
#define MK_LDi1(bytes, rs1, simm13) (BPatch_memoryAccess(true, false, (bytes), (simm13), (rs1), -1))
#define MK_STi1(bytes, rs1, simm13) (BPatch_memoryAccess(false, true, (bytes), (simm13), (rs1), -1))
#define MK_PFi0(rs1, rs2, f) (BPatch_memoryAccess(false, false, true, 0, (rs1), (rs2), \
                                           0, -1, -1, (f)))
#define MK_PFi1(rs1, simm13, f) (BPatch_memoryAccess(false, false, true, (simm13), (rs1), -1, \
                                              0, -1, -1, (f)))

#define MK_LD(bytes, in) (in.rest.i ? MK_LDi1(bytes, in.resti.rs1, in.resti.simm13) : \
                                      MK_LDi0(bytes, in.rest.rs1, in.rest.rs2))
#define MK_ST(bytes, in) (in.rest.i ? MK_STi1(bytes, in.resti.rs1, in.resti.simm13) : \
                                      MK_STi0(bytes, in.rest.rs1, in.rest.rs2))
#define MK_MA(bytes, in, load, store) \
          (in.rest.i ? \
              BPatch_memoryAccess((load), (store), (bytes), in.resti.simm13, in.resti.rs1, -1) \
              : \
              BPatch_memoryAccess((load), (store), (bytes), 0, in.rest.rs1, in.rest.rs2))
#define MK_PF(in) (in.rest.i ? MK_PFi1(in.resti.rs1, in.resti.simm13, in.resti.rd) : \
                               MK_PFi0(in.rest.rs1, in.rest.rs2, in.rest.rd))

// VG(09/20/01): SPARC V9 decoding after the architecture manual.
// One can see this a Huffman tree...
BPatch_memoryAccess isLoadOrStore(const instruction i)
{
  if(i.rest.op != 0x3) // all memory opcodes have op bits 11
    return BPatch_memoryAccess::none;

  unsigned int op3 = i.rest.op3;

  if(btst(op3, 4)) { // bit 4 set means alternate space
    if(i.rest.i) {
      logLine("SPARC: Alternate space instruction using ASI register currently unhandled...");
      return BPatch_memoryAccess::none;
    }
    else if(i.rest.unused != ASI_PRIMARY) { // unused is actually imm_asi
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
        return BPatch_memoryAccess(true, true, b, 0, i.resti.rs1, -1);
      }
    }
    else { // bit 3 zero (1u0xyz) means fp memory op
      bool isStore = btst(op3, 2); // bit 2 gives L/S
      // bits 0-1 encode #bytes except for state register ops,
      // where the number of bits is given by bit 0 from rd
      unsigned int b = fpBytes[op3 & 0x3][i.rest.rd & 0x1];
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
