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

#include "BPatch_snippet.h"

// #define DEBUG_FINDTARGET 1
// #define DEBUG_FINDTARGET_2 1

/* From inst-mips.C & arch-mips.C: */
extern char *reg_names_book[];
extern uint32_t get_word(const Object &elf, Address addr);
extern uint64_t get_dword(const Object &elf, Address addr);
extern void dis(void *actual_, void *addr_, int ninsns,
	        const char *pre, FILE *stream);

typedef struct {
    bool isConditional;
    bool isIndirect;
    bool isCall;
} BranchInfo;

/** returns true if an instruction is a branch, false if not.  Also returns
  * information about the branch: is it condition, is it indirect, and
  * the target address.
  * @param i the instruction value
  */
static bool getBranchInfo(instruction i, BranchInfo &bi)
{
    bi.isConditional = false;
    bi.isIndirect    = false;
    bi.isCall        = false;

    switch (i.decode.op) {
      case COP0op:
      case COP1op:
      case COP2op:
      case COP1Xop:
	if (i.itype.rs == 8) { /* BC */
	    /* XXX Actually, are all branch conditions (i.itype.rt values)
	       allowed?*/
	    bi.isConditional = true;
	    return true;
	}
	break;
      case BEQop:
      case BEQLop:
      case BNEop:
      case BNELop:
	/* Branch if equal or not equal on comparison of a register with
	   itself is an unconditional branch (or a branch never). */
	if (i.itype.rs != i.itype.rt) {
	    bi.isConditional = true;
	    return true;
	}
	return true;
      case BGTZop:
      case BGTZLop:
      case BLEZop:
      case BLEZLop:
	bi.isConditional = true;
	return true;
      case Jop:
	return true;
      case JALop:
	bi.isCall = true;
	return true;
      case REGIMMop:
	switch (i.regimm.opr) {
	  case BGEZopr:
	  case BGEZLopr:
	  case BLTZopr:
	  case BLTZLopr:
	    bi.isConditional = true;
	    return true;
	  case BGEZALopr:
	  case BGEZALLopr:
	  case BLTZALopr:
	  case BLTZALLopr:
	    bi.isConditional = true;
	    bi.isCall = true;
	    return true;
	};
	break;
      case SPECIALop:
	switch (i.rtype.ops) {
	  case JALRops:
	    if (i.rtype.rt == 0 && i.rtype.sa == 0) {
		bi.isIndirect = true;
		bi.isCall = true;
		return true;
	    } else {
		/* Invalid -- throw assert for now. */
		assert(0);
	    }
	  case JRops:
	    if (i.rtype.rt == 0 && i.rtype.rd == 0 && i.rtype.sa == 0) {
		bi.isIndirect = true;
		return true;
	    } else {
		/* Invalid. */
		assert(0);
	    }
	};
	break;
    };
    return false;
}

bool modifies_greg(instruction i, int &regnum)
{
    int cop;

    switch (i.decode.op) {
      case ADDIop:
      case ADDIUop:
      case ANDIop:
      case DADDIop:
      case DADDIUop:
      case LBop:
      case LBUop:
      case LDop:
      case LDLop:
      case LDRop:
      case LHop:
      case LHUop:
      case LLop:
      case LLDop:
      case LUIop:
      case LWop:
      case LWLop:
      case LWRop:
      case LWUop:
      case ORIop:
      case SLTIop:
      case SLTIUop:
      case XORIop:
	regnum = i.itype.rt;
	return true;
      case COP0op:
      case COP1op:
      case COP2op:
	if (i.rtype.sa == 0 && i.rtype.ops == 0) {
	    cop = i.rtype.op & 0x3;
	    if ((cop == 0 && i.rtype.rs == 1) ||
		((cop == 1 || cop == 2) && i.rtype.rs == 2) ||
		((cop == 0 || cop == 1 || cop == 2) && i.rtype.rs == 0)) {
		regnum = i.rtype.rt;
		return true;
	    }
	}
	break;
      case SPECIALop:
	switch (i.rtype.ops) {
	  case ADDops:
	  case ADDUops:
	  case ANDops:
	  case DADDops:
	  case DADDUops:
	  case DSLLVops:
	  case DSRAVops:
	  case DSRLVops:
	  case DSUBops:
	  case DSUBUops:
	  case NORops:
	  case ORops:
	  case SLLVops:
	  case SLTops:
	  case SLTUops:
	  case SRAVops:
	  case SUBops:
	  case SUBUops:
	  case XORops:
	    if (i.rtype.sa == 0) {
		regnum = i.rtype.rd;
		return true;
	    }
	    break;
	  case DSLLops:
	  case DSLL32ops:
	  case DSRAops:
	  case DSRA32ops:
	  case DSRLops:
	  case DSRL32ops:
	  case SLLops:
	  case SRAops:
	  case SRLops:
	  case SRLVops:
	    if (i.rtype.rs == 0) {
		regnum = i.rtype.rd;
		return true;
	    }
	    break;
	  case MFHIops:
	  case MFLOops:
	    if (i.rtype.rs == 0 && i.rtype.rt == 0 && i.rtype.sa == 0) {
		regnum = i.rtype.rd;
		return true;
	    }
	    break;
	  case JALRops:
	    if (i.rtype.rt == 0 && i.rtype.sa == 0) {
		regnum = i.rtype.rd;
		return true;
	    }
	    break;
	}
    }

    return false;
}

//some more function used to identify the properties of the instruction
/** is the instruction used to return from the functions
  * @param i the instruction value 
  */
bool isAReturnInstruction(const instruction i){
    return isReturnInsn(i);
}

/** is the instruction an indirect jump instruction 
  * @param i the instruction value 
  */
bool isAIndirectJumpInstruction(const instruction i){
    return isInsnType(i, JRmask, JRmatch) && !isReturnInsn(i);
}

/** is the instruction a conditional branch instruction 
  * @param i the instruction value 
  */ 
bool isACondBranchInstruction(const instruction i){
    BranchInfo bi;

    if (getBranchInfo(i, bi)) {
	if (!bi.isCall && bi.isConditional)
    	    return true;
    }

    return false;
}

/** is the instruction an unconditional branch instruction 
  * @param i the instruction value 
  */
bool isAJumpInstruction(const instruction i){
    BranchInfo bi;

    if (getBranchInfo(i, bi)) {
    	if (!bi.isCall && !bi.isIndirect && !bi.isConditional) {
	    return true;
	}
    }

    return false;
}

/** is the instruction a call instruction 
  * @param i the instruction value 
  */
bool isACallInstruction(const instruction i){
    if (isCallInsn(i)) {
	return true;
    } else if (i.decode.op == REGIMMop) {
	if (i.regimm.opr == BGEZALopr ||
	    i.regimm.opr == BGEZALLopr ||
	    i.regimm.opr == BLTZALopr ||
	    i.regimm.opr == BLTZALLopr) {
fprintf(stderr, "Conditional branch and link found!!!\n");
	    return true;
	}
    }
    return false;
}

/** function which returns the target address of control transfer instructions
  * @param i the instruction value 
  * @param pos the address of the instruction
  */
Address getBranchTargetAddress(const instruction i,Address pos){
    /* XXX Can we assume that the instruction is a branch or jump? */
    Address delaySlot = pos + INSN_SIZE;
    if (isInsnType(i, Jmask, Jmatch) ||
	isInsnType(i, JALmask, JALmatch)) {
	Address hi = delaySlot & REGION_NUM_MASK;
	Address lo = (i.jtype.imm26 << 2) & REGION_OFF_MASK;
	return hi | lo;
    } else {
	/* All others are branches. */
	return delaySlot + (i.itype.simm16 << 2);
    }
}

/* find branches starting at the given address
 * @param ah the starting address for the search
 */
void AddressHandle::getMultipleJumpTargets(BPatch_Set<Address>& result){
    /*
     * We need to look for series of instructions:
     * 1. Calculate some offset into a table
     * 2. lw R1, X(R2), or ld R1, X(R2)
     * 3. jr R1 (this should be ah ah-1)
     */
    AddressHandle ah = *this;

    Address origAddr = *ah;

    assert(ah.hasPrev() && isInsnType(ah.getPrevInstruction(),JRmask,JRmatch));

    ah--;

#ifdef DEBUG_FINDTARGET_2
	printf("** Checking indirect jump at 0x%p.\n",
		(void *)origAddr);
#endif

    /* Get register used in JR instruction. */
    instruction i = ah.getInstruction();

    int jumpReg = i.rtype.rs;

#ifdef DEBUG_FINDTARGET_2
    printf("jumpReg = %s ($%d)\n", reg_names_book[jumpReg], jumpReg);
#endif

    int jumpSrcReg    = -1;
#ifdef DEBUG_FINDTARGET
    int jumpSrcOffset = 0;
#endif

    /* Get table load. */
    while (ah.hasPrev()) {
    	ah--;

	i = ah.getInstruction();

	if ((isInsnType(i, LWmask, LWmatch) || 
	     isInsnType(i, LDmask, LDmatch)) &&
	    i.itype.rt == jumpReg) {
	    jumpSrcReg = i.itype.rs;
#ifdef DEBUG_FINDTARGET
	    jumpSrcOffset = i.itype.simm16;
#endif
	    break;
	}
    }

    if (jumpSrcReg == -1) {
#ifdef DEBUG_FINDTARGET
	printf("** Can't find possible targets for indirect jump at 0x%p!\n",
		(void *)origAddr);
	printf("   There was no lw or ld instruction before jump.\n");
#endif
	return;
    }

#ifdef DEBUG_FINDTARGET_2
    printf("Jump source register: %s ($%d)\n",
	    reg_names_book[jumpSrcReg], jumpSrcReg);
    printf("  Jump source offset: %d\n", jumpSrcOffset);
#endif

    unsigned int regMask = 1 << jumpSrcReg;

    int baseReg = -1;
    int baseOffset;
    Address baseLoadAddr;

    int tmpReg;

    BPatch_Set<int> stackLocations;

    while (ah.hasPrev()) {
	ah--;

	i = ah.getInstruction();

#ifdef DEBUG_FINDTARGET_2
	dis(&i, (void *)(*ah), 1,  "  ", stdout);
#endif

	if ((isInsnType(i, LWmask, LWmatch) || 
	     isInsnType(i, LDmask, LDmatch)) &&
	    ((regMask & (1 << i.itype.rt)) != 0) &&
	    ((i.itype.rs == REG_GP) ||
	     (i.itype.rs == REG_AT && i.itype.rt != REG_AT))) {
#ifdef DEBUG_FINDTARGET_2
	    if (i.itype.rs == REG_GP)
		printf("Load from GP");
	    else if (i.itype.rs == REG_AT)
		printf("Load from AT");
	    else
		printf("Load from ??");
	    printf(" at 0x%p\n", *ah);
#endif
	    baseReg      = i.itype.rt;
	    baseOffset   = i.itype.simm16;
	    baseLoadAddr = *ah;

	    if (i.itype.rs == REG_GP)
		break;
	} else if ((isInsnType(i, LWmask, LWmatch) || 
		    isInsnType(i, LDmask, LDmatch)) &&
		   ((regMask & (1 << i.itype.rt)) != 0) &&
		   (i.itype.rs == REG_SP)) {
#ifdef DEBUG_FINDTARGET_2
	    printf("0x%08p: Removing %s ($%d) from set,\n",
		    *ah, reg_names_book[i.itype.rt], i.itype.rt);
	    printf("  and adding stack location %d\n", (int)i.itype.simm16);
#endif
	    stackLocations.insert(i.itype.simm16);
	    regMask &= ~(1 << (i.itype.rt));
	} else if ((isInsnType(i, SWmask, SWmatch) || 
		    isInsnType(i, SDmask, SDmatch)) &&
		   (i.itype.rs == REG_SP) &&
		   (stackLocations.contains(i.itype.simm16))) {
#ifdef DEBUG_FINDTARGET_2
	    printf("0x%08p: Adding %s ($%d) to set,\n",
		    *ah, reg_names_book[i.itype.rt], i.itype.rt);
	    printf("  and removing stack location %d\n", (int)i.itype.simm16);
#endif
	    stackLocations.remove(i.itype.simm16);
	    regMask |= 1 << i.rtype.rt;
	} else if ((isInsnType(i, ADDmask, ADDmatch) ||
		    isInsnType(i, ADDUmask, ADDUmatch) ||
	    	    isInsnType(i, DADDmask, DADDmatch) ||
		    isInsnType(i, DADDUmask, DADDUmatch)) &&
		   ((regMask & (1 << i.rtype.rd)) != 0)) {
#ifdef DEBUG_FINDTARGET_2
	    printf("0x%08p: Adding %s ($%d) and %s ($%d) to set.\n",
		    *ah,
		    reg_names_book[i.itype.rs], i.itype.rs,
		    reg_names_book[i.itype.rt], i.rtype.rt);
#endif
	    regMask |= (1 << i.rtype.rs) | (1 << i.rtype.rt);
	} else if (modifies_greg(i, tmpReg) &&
		   !isInsnType(i, ADDIUmask, ADDIUmatch) &&
		   !isInsnType(i, DADDIUmask, DADDIUmatch)) {
#ifdef DEBUG_FINDTARGET_2
	    printf("0x%08p: Removing %s ($%d) from set.\n",
		    *ah, reg_names_book[tmpReg], tmpReg);
#endif
	    regMask &= ~(1 << tmpReg);
	}
    }

    if (baseReg == -1) {
#ifdef DEBUG_FINDTARGET
	printf("** Can't find possible targets for indirect jump at 0x%p!\n",
		(void *)origAddr);
	printf("   Couldn't find load relative to GP or AT.\n");
#endif
#ifdef DEBUG_FINDTARGET_2
	exit(1);
#endif
	return;
    }

#ifdef DEBUG_FINDTARGET_2
    printf("        Base register is: %s ($%d)\n", 
	    reg_names_book[baseReg], baseReg);
#endif

    ah.setCurrentAddress(baseLoadAddr);

    int offset = 0;

    while (ah.hasMore() && (*ah) < origAddr) {
	ah++;

	i = ah.getInstruction();

	if ((isInsnType(i, ADDIUmask, ADDIUmatch) || 
	     isInsnType(i, DADDIUmask, DADDIUmatch)) &&
	    i.itype.rs == baseReg) {
	    offset += i.itype.simm16;
	}
    }

#ifdef DEBUG_FINDTARGET_2
    printf("               Offset is: %d\n", offset);
#endif

    const Object &elf = ah.getImage()->getObject();
    bool is_elf64 = elf.is_elf64();

    Address addr = elf.get_gp_value() + baseOffset;

    if (is_elf64) addr = get_dword(elf, addr);
    else addr = get_word(elf, addr);

    addr += offset;
    addr += elf.get_base_addr();

#ifdef DEBUG_FINDTARGET_2
    printf("Address of table is: 0x%p\n", (void *)addr);
#endif

    for (;;) {
	Address target;
	if (is_elf64) {
	    target = get_dword(elf, addr);
	    addr += 8;
	} else {
	    target = get_word(elf, addr);
	    addr += 4;
	}
	if (!ah.containsAddress(target)) {
#ifdef DEBUG_FINDTARGET_2
	    printf("Not an entry: 0x%p\n", (void *)target);
#endif
	    break;
	}
#ifdef DEBUG_FINDTARGET_2
	printf("Entry: 0x%p\n", (void *)target);
#endif
	result += target;
    }

#if DEBUG_FINDTARGET
    if (result.size() == 0) {
	printf("** Can't find possible targets for indirect jump at 0x%p!\n",
		(void *)origAddr);
#ifdef DEBUG_FINDTARGET_2
	exit(1);
#endif
    } else if (result.size() == 1) {
	printf("** Found only one entry in table for indirect jump at 0x%p!\n",
		(void *)origAddr);
#ifdef DEBUG_FINDTARGET_2
	exit(1);
#endif
    }
#endif
}

//Address Handle used by flowGraph which wraps the instructions
//and supply enough operation to iterate over the instrcution sequence.

AddressHandle::AddressHandle(process* fProcess,
			     Address bAddress,
			     unsigned fSize)
	: addressProc(fProcess),
	  addressImage(fProcess->getImage()),baseAddress(bAddress),
	  range(fSize),currentAddress(bAddress) {}

AddressHandle::AddressHandle(const AddressHandle& ah){
	addressImage = ah.addressImage;
	addressProc = ah.addressProc;
	baseAddress = ah.baseAddress;
	currentAddress = ah.currentAddress;
	range = ah.range;
}

AddressHandle::~AddressHandle(){}

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
