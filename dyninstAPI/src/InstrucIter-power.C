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

#include "BPatch_memoryAccess_NP.h"
#include "BPatch_Set.h"

//some more function used to identify the properties of the instruction
/** is the instruction used to return from the functions
  * @param i the instruction value 
  */
bool InstrucIter::isAReturnInstruction()
{
  const instruction i = getInstruction();
	if((i.xlform.op == BCLRop) &&
	   (i.xlform.xo == BCLRxop) && 
	   (i.xlform.bt & 0x10) && (i.xlform.bt & 0x4))
		return true;
	return false;
}

/** is the instruction an indirect jump instruction 
  * @param i the instruction value 
  */
bool InstrucIter::isAIndirectJumpInstruction(InstrucIter ah)
{
  const instruction i = getInstruction();
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
bool InstrucIter::isACondBranchInstruction()
{
  const instruction i = getInstruction();
	if((i.bform.op == BCop) && !i.bform.lk &&
	   !((i.bform.bo & 0x10) && (i.bform.bo & 0x4)))
		return true;
	return false;
}
/** is the instruction an unconditional branch instruction 
  * @param i the instruction value 
  */
bool InstrucIter::isAJumpInstruction()
{
  const instruction i = getInstruction();
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
bool InstrucIter::isACallInstruction()
{
  const instruction i = getInstruction();
	if(i.iform.lk && 
	   ((i.iform.op == Bop) || (i.bform.op == BCop) ||
	    ((i.xlform.op == BCLRop) && 
	     ((i.xlform.xo == 16) || (i.xlform.xo == 528))))){
		return true;
	}
	return false;
}
bool InstrucIter::isAnneal(){
  return true;
}

#define MK_LD1(bytes, imm, ra) (new BPatch_memoryAccess(true, false, (bytes), (imm), (ra), -1))
#define MK_SD1(bytes, imm, ra) (new BPatch_memoryAccess(false, true, (bytes), (imm), (ra), -1))

#define MK_LX1(bytes, ra, rb) (new BPatch_memoryAccess(true, false, (bytes), 0, (ra), (rb)))
#define MK_SX1(bytes, ra, rb) (new BPatch_memoryAccess(false, true, (bytes), 0, (ra), (rb)))

#define MK_LD(bytes, i) (MK_LD1((bytes), i.dform.d_or_si, (signed)i.dform.ra))
#define MK_SD(bytes, i) (MK_SD1((bytes), i.dform.d_or_si, (signed)i.dform.ra))

// VG(11/20/01): X-forms ignore ra if 0, but not rb...
#define MK_LX(bytes, i) (MK_LX1((bytes), (i.xform.ra ? (signed)i.xform.ra : -1), i.xform.rb))
#define MK_SX(bytes, i) (MK_SX1((bytes), (i.xform.ra ? (signed)i.xform.ra : -1), i.xform.rb))

#define MK_LDS(bytes, i) (MK_LD1((bytes), (i.dsform.d << 2), (signed)i.dsform.ra))
#define MK_SDS(bytes, i) (MK_SD1((bytes), (i.dsform.d << 2), (signed)i.dsform.ra))

#define MK_LI(bytes, i) (MK_LX1((bytes), i.xform.ra, -1))
#define MK_SI(bytes, i) (MK_SX1((bytes), i.xform.ra, -1))

struct opCodeInfo {
  unsigned int bytes; //: 4;
  unsigned int direc; //: 1; // 0 == load, 1 == store
public:
  opCodeInfo(unsigned b, unsigned d) : bytes(b), direc(d) {}
  opCodeInfo() : bytes(0), direc(0) {}
};

opCodeInfo *xopCodes[1024];

void initOpCodeInfo()
{
  xopCodes[LWARXxop]	= new opCodeInfo(4, 0);
  xopCodes[LDXxop] 	= new opCodeInfo(8, 0);
  xopCodes[LXxop]	= new opCodeInfo(4, 0);
  xopCodes[LDUXxop]	= new opCodeInfo(8, 0);
  xopCodes[LUXxop] 	= new opCodeInfo(4, 0);
  xopCodes[LDARXxop]	= new opCodeInfo(8, 0);
  xopCodes[LBZXxop]	= new opCodeInfo(1, 0);
  xopCodes[LBZUXxop]	= new opCodeInfo(1, 0);

  xopCodes[STDXxop]	= new opCodeInfo(8, 1);
  xopCodes[STWCXxop]	= new opCodeInfo(4, 1);
  xopCodes[STXxop]	= new opCodeInfo(4, 1);
  xopCodes[STDUXxop]	= new opCodeInfo(8, 1);
  xopCodes[STUXxop]	= new opCodeInfo(4, 1);
  xopCodes[STDCXxop]	= new opCodeInfo(8, 1);
  xopCodes[STBXxop]	= new opCodeInfo(1, 1);
  xopCodes[STBUXxop]	= new opCodeInfo(1, 1);

  xopCodes[LHZXxop]	= new opCodeInfo(2, 0);
  xopCodes[LHZUXxop]	= new opCodeInfo(2, 0);
  xopCodes[LHAXxop]	= new opCodeInfo(2, 0);
  xopCodes[LWAXxop]	= new opCodeInfo(4, 0);
  xopCodes[LWAUXxop]	= new opCodeInfo(4, 0);
  xopCodes[LHAUXxop]	= new opCodeInfo(2, 0);

  xopCodes[STHXxop]	= new opCodeInfo(2, 1);
  xopCodes[STHUXxop]	= new opCodeInfo(2, 1);

  xopCodes[LSXxop]	= new opCodeInfo(0, 0); // count field only available at runtime
  xopCodes[LWBRXxop]	= new opCodeInfo(4, 0);
  xopCodes[LFSXxop]	= new opCodeInfo(4, 0);
  xopCodes[LFSUXxop]	= new opCodeInfo(4, 0);
  xopCodes[LSIxop]	= new opCodeInfo(0, 0); // count field needs to be computed
  xopCodes[LFDXxop]	= new opCodeInfo(8, 0);
  xopCodes[LFDUXxop]	= new opCodeInfo(8, 0);

  xopCodes[STSXxop]	= new opCodeInfo(0, 1); // count field only available at runtime
  xopCodes[STBRXxop]	= new opCodeInfo(4, 1);
  xopCodes[STFSXxop]	= new opCodeInfo(4, 1);
  xopCodes[STFSUXxop]	= new opCodeInfo(4, 1);
  xopCodes[STSIxop]	= new opCodeInfo(0, 1); // bytes field needs to be computed
  xopCodes[STFDXxop]	= new opCodeInfo(8, 1);
  xopCodes[STFDUXxop]	= new opCodeInfo(8, 1);

  xopCodes[LHBRXxop]	= new opCodeInfo(2, 0);

  xopCodes[STHBRXxop]	= new opCodeInfo(2, 1);

  xopCodes[STFIWXxop]	= new opCodeInfo(4, 1);

  //fprintf(stderr, "POWER opcode info initialized.\n");
}


/* return NULL if instruction is not load/store, else returns
   a class that contains info about the load/store */

// VG (09/11/01): I tried to optimize this somewhat. Not sure how much...
// It is a hard-coded binary search that assumes all opcodes have the same
// probability to appear. Probably not realistic, but I don't have statistics
// for Power... Due to the interleaving of load and store opcodes on Power,
// it seems cheaper to have only one function to decide if it is load or store
// Alternatively one could write this based on bits in opcode. E.g. if bit 3
// is 0 then the instruction is most likely a load, else it is likely a store.
// I also assume that no invalid instructions occur: e.g.: LU with RA=0, etc.

#define logIS_A(x) 
//#define logIS_A(x) logLine((x))

BPatch_memoryAccess* InstrucIter::isLoadOrStore()
{
  const instruction i = getInstruction();

  int op = i.dform.op;
  int  b;

  if(op > LXop) { // try D-forms
    if(op < STHop)
      if(op < STop) {
	logIS_A("IS_A: l-lbzu");
	b = op < LBZop ? 4 : 1;
	return MK_LD(b, i);
      } else if (op < LHZop) {
	logIS_A("IS_A: st-stbu");
	b = op < STBop ? 4 : 1;
	return MK_SD(b, i);
      } else {
	logIS_A("IS_A: lhz-lhau");
	return MK_LD(2, i);
      }
    else if(op < STFSop)
      if(op < LFSop)
	if(op < LMop) {
	  logIS_A("IS_A: sth-sthu");
	  return MK_SD(2, i);
	}
	else if(op < STMop) {
	  logIS_A("IS_A: lm");
	  return MK_LD((32 - i.dform.rt)*4, i);
	}
	else {
	  logIS_A("IS_A: stm");
	  return MK_SD((32 - i.dform.rt)*4, i);
	}
      else {
	logIS_A("IS_A: lfs-lfdu");
	b = op < LFDop ? 4 : 8;
	return MK_LD(b, i);
      }
    else if(op <= STFDUop) {
      logIS_A("IS_A: stfs-stfdu");
      b = op < STFDop ? 4 : 8;
      return MK_SD(b, i);
    }
    else if(op == LDop) {
      logIS_A("IS_A: ld-lwa");
      b = i.dsform.xo < 2 ? 8 : 4;
      assert(i.dsform.xo < 3);
      return MK_LDS(b, i);
    }
    else if(op == STDop) {
      logIS_A("IS_A: std-stdu");
      assert(i.dsform.xo < 2);
      return MK_SDS(8, i);
    }
    else
      return BPatch_memoryAccess::none;
  }
  else if(op == LXop) { // X-forms
    unsigned int xop = i.xform.xo;
    //char buf[100];

    //snprintf(buf, 100, "XOP:: %d\n", xop);
    //logIS_A(buf);

    opCodeInfo *oci = xopCodes[xop];

    if(oci->bytes > 0)
      return oci->direc ? MK_SX(oci->bytes, i) : MK_LX(oci->bytes, i);
    else if(xop == LSIxop || xop == STSIxop) {
      b = i.xform.rb == 0 ? 32 : i.xform.rb; 
      return oci->direc ? MK_SI(b, i) : MK_LI(b, i);
    }
    else if(xop == LSXxop || xop == STSXxop) {
      return new BPatch_memoryAccess(oci->direc == 0, oci->direc == 1,
                                     0, i.xform.ra, i.xform.rb,
                                     0, POWER_XER2531, -1); // 9999 == XER_25:31
    }
  }
  return BPatch_memoryAccess::none;
}

/** function which returns the offset of control transfer instructions
  * @param i the instruction value 
  */
Address InstrucIter::getBranchTargetAddress(Address pos)
{
  const instruction i = getInstruction();
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


void InstrucIter::getMultipleJumpTargets(BPatch_Set<Address>& result)
{
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
	if(!maxSwitch){
		result += (initialAddress + sizeof(instruction));
		return;
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
}

bool InstrucIter::delayInstructionSupported()
{
	return false;
}

bool InstrucIter::hasMore()
{
	if((currentAddress < (baseAddress + range )) &&
	   (currentAddress >= baseAddress))
		return true;
	return false;
}

bool InstrucIter::hasPrev()
{
    if((currentAddress < (baseAddress + range )) &&
       (currentAddress > baseAddress))
	return true;
    return false;
}

Address InstrucIter::prevAddress()
{
	Address ret = currentAddress-sizeof(instruction);
	return ret;
}

Address InstrucIter::nextAddress()
{
	Address ret = currentAddress + sizeof(instruction);
	return ret;
}

void InstrucIter::setCurrentAddress(Address addr)
{
	currentAddress = addr;
}

instruction InstrucIter::getInstruction()
{
	instruction ret;
	ret.raw = addressImage->get_instruction(currentAddress);
	return ret;
}

instruction InstrucIter::getNextInstruction()
{
	instruction ret;
	ret.raw = addressImage->get_instruction(currentAddress+sizeof(instruction));
	return ret;
}

instruction InstrucIter::getPrevInstruction()
{
	instruction ret;
	ret.raw = addressImage->get_instruction(currentAddress-sizeof(instruction));
	return ret;
}

Address InstrucIter::operator++()
{
	currentAddress += sizeof(instruction);
	return currentAddress;
}

Address InstrucIter::operator--()
{
	currentAddress -= sizeof(instruction);
	return currentAddress;
}

Address InstrucIter::operator++(int)
{
	Address ret = currentAddress;
	currentAddress += sizeof(instruction);
	return ret;
}

Address InstrucIter::operator--(int)
{
	Address ret = currentAddress;
	currentAddress -= sizeof(instruction);
	return ret;
}

Address InstrucIter::operator*(){
	return currentAddress;
}
