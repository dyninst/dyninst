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

#include "BPatch_memoryAccess_NP.h"
#include "BPatch_Set.h"


/* Prints the op code */
void InstrucIter::printOpCode()
{
  const instruction i = getInstruction();
  printf("OpCode num %d",i.xlform.op);
}

/* Returns the value of the RT Register */
unsigned InstrucIter::getRTValue(void)
{
  const instruction i = getInstruction();
  return i.xform.rt;  
}

/* Returns the value of the RA Register */
unsigned InstrucIter::getRAValue(void)
{
  const instruction i = getInstruction();
  return i.xform.ra;
}


/* This function returns true if the instruction affects the RD Register */
bool InstrucIter::isA_RT_WriteInstruction()
{
  const instruction i = getInstruction();
  if (
      /* XO Form */
      /*add, addc, adde, addme, addze, divw, divwu, mullw, neg, subf, subfc, subfe, 
	subfme, subfze, mulhw, mulhwu*/
      (i.xoform.op == 31 && ( i.xoform.xo == 266 || i.xoform.xo == 10 || i.xoform.xo == 138 ||
			      i.xoform.xo == 28  || i.xoform.xo == 60 || i.xoform.xo == 491 ||
			      i.xoform.xo == 40 || i.xoform.xo == 8 || i.xoform.xo == 136 || 
			      i.xoform.xo == 232 || i.xoform.xo == 200 || i.xoform.xo == 75 ||  
			      i.xoform.xo == 459 || i.xoform.xo == 235 || i.xoform.xo == 104 ||
			      i.xoform.xo == 360 || i.xoform.xo == 331 || i.xoform.xo == 363 ||
			      i.xoform.xo == 264 || i.xoform.xo == 107 || i.xoform.xo == 488 ||
			      i.xoform.xo == 8 || i.xoform.xo == 136 || i.xoform.xo == 232 ||
			      i.xoform.xo == 200 || i.xoform.xo == 11 )) ||

       /* macchw, macchws, macchwsu, macchwu, machhw, machhws, machhwsu, machhwu, maclhw, maclhwsu,
	  maclhwu, maclhws, nmacchw, nmacchws, nmachhw, nmachhws, nmaclhw, nmaclhws */
       (i.xoform.op == 4 && ( i.xoform.xo == 172 || i.xoform.xo == 236 || i.xoform.xo == 204 ||
			      i.xoform.xo == 140  || i.xoform.xo == 44 || i.xoform.xo == 108 ||
			      i.xoform.xo == 76  || i.xoform.xo == 12  || i.xoform.xo == 428 ||
			      i.xoform.xo == 492 || i.xoform.xo == 460 || i.xoform.xo == 396 || 
			      i.xoform.xo == 494 || i.xoform.xo == 174 || i.xoform.xo == 238 || 
			      i.xoform.xo == 46 || i.xoform.xo == 110 || i.xoform.xo == 430)) ||
	
	/* D Form */
	(i.dform.op == 7 || i.dform.op == 8 || i.dform.op == 9   ) || /* mulli,subtc */
	(i.dform.op >= 12 && i.dform.op  <= 15) || /* addic, addic., addi, addis */
	(i.dform.op >= 32 && i.dform.op <=35) || /* lws, lwzu, lbz, lbzu */
	(i.dform.op >= 40 && i.dform.op <=43) || /* lhz, lhzu, lha, lhau */
	
	/* X Form */
	/*lbzux, lbzx, lhaux, lhax, lhbrx, lhzux, lhzx,lwarx,lwbrx, dcread
	  lwzux, lwzx, mfcr, mfdcr, mfmsr, mfspr, mftb, tlbre, tlbsx */
	(i.xform.op == 31 && ( i.xform.xo == 119 || i.xform.xo == 87 ||
			       i.xform.xo == 375  || i.xform.xo == 343 || i.xform.xo == 790 ||
			       i.xform.xo == 311  || i.xform.xo == 279 || i.xform.xo == 20 ||
			       i.xform.xo == 534  || i.xform.xo == 55 || i.xform.xo == 23 ||
			       i.xform.xo == 19  || i.xform.xo == 323 || i.xform.xo == 83 ||
			       i.xform.xo == 339  || i.xform.xo == 371 || i.xform.xo == 946 ||
			       i.xform.xo == 531  || i.xform.xo == 595 || i.xform.xo == 627 ||
			       i.xform.xo == 659  || i.xform.xo == 818 ||
			       i.xform.xo == 914 || i.xform.xo == 486 )) ||
	 
	 /* mulchw, mulchwu, mulhhw, mulhhwu, mullhw, mullhwu, 
	  */
      (i.xform.op == 4 && ( i.xform.xo == 168  || i.xform.xo == 136 || i.xform.xo == 40 ||
			    i.xform.xo == 8  || i.xform.xo == 424 || i.xform.xo == 392)))
    
    {
      return true;
    }
  else
    {
      return false;
    }
}  

/* This function returns true if the instruction affects the FRD Register */
bool InstrucIter::isA_FRT_WriteInstruction()
{
  const instruction i = getInstruction();
  if (
      /* A Form */
      (i.aform.op == 63 && ( i.aform.xo == 21 || i.aform.xo == 18 || i.aform.xo == 25 ||
			      i.aform.xo == 29  || i.aform.xo == 28 || i.aform.xo == 31 ||
			      i.aform.xo == 30 || i.aform.xo == 22 || i.aform.xo == 14 || 
			      i.aform.xo == 20 )) ||  

      (i.aform.op == 59 && ( i.aform.xo == 18 || i.aform.xo == 29 || i.aform.xo == 25 ||
			     i.aform.xo == 28  || i.aform.xo == 30 || i.aform.xo == 31 ||
			     i.aform.xo == 20 )) ||  
      
	/* D Form */
      (i.dform.op == 56 || i.dform.op == 57 ) || 
      (i.dform.op >= 48 && i.dform.op  <= 51) ||

	/* X Form */
      (i.xform.op == 63 && ( i.xform.xo == 583 || i.xform.xo == 264 ||
			     i.xform.xo == 15  || i.xform.xo == 72 || i.xform.xo == 136 ||
			     i.xform.xo == 40 || i.xform.xo == 12 )) ||

      (i.xform.op == 31 && ( i.xform.xo == 631  || i.xform.xo == 599 || i.xform.xo == 823 ||
			     i.xform.xo == 791  || i.xform.xo == 567 || i.xform.xo == 535 )))
			     
    {
      return true;
    }
  else
    {
      return false;
    }
}  

/* This function returns true if the instruction affects the FRA Register */
bool InstrucIter::isA_FRA_WriteInstruction()
{
  const instruction i = getInstruction();
  if (
      /* A Form */
      (i.aform.op == 63 && ( i.aform.xo == 21 || i.aform.xo == 18 || i.aform.xo == 25 ||
			      i.aform.xo == 29  || i.aform.xo == 28 || i.aform.xo == 31 ||
			      i.aform.xo == 30 || i.aform.xo == 20 )) ||  

      (i.aform.op == 59 && ( i.aform.xo == 18 || i.aform.xo == 29 || i.aform.xo == 25 ||
			     i.aform.xo == 28  || i.aform.xo == 30 || i.aform.xo == 31 ||
			     i.aform.xo == 20 )) ||  
      
	/* X Form */
      (i.xform.op == 63 && (i.xform.xo == 32 || i.xform.xo == 0 )))

    {
      return true;
    }
  else
    {
      return false;
    }
}  



/* This function returns true if the instruction affects the RA Register */
bool InstrucIter::isA_RA_WriteInstruction()
{
  const instruction i = getInstruction();
  //if ( i.xform.op == 31 )
  //  printf("la la %d \n", i.xform.xo);


  if (
      /* X Form */
      /*nor, or, orc, slw, sraw, srawi, srw, stbux, and, andc, cntlzw, eqv, extsb, extsh,
       lhzux, lwzux, lbzux, lhaux, nand, sthux, stwux, xor */
      ( i.xform.op == 31 && ( i.xform.xo == 124 ||  i.xform.xo == 247 || i.xform.xo == 28 ||
			      i.xform.xo == 444  || i.xform.xo == 412 || i.xform.xo == 24 ||
			      i.xform.xo == 60  || i.xform.xo == 257 || i.xform.xo == 284 ||
			      i.xform.xo == 954  || i.xform.xo == 922  || i.xform.xo == 311  ||
			      i.xform.xo == 55   || i.xform.xo == 119  || i.xform.xo == 375  ||
			      i.xform.xo == 476   || i.xform.xo == 439   || i.xform.xo == 183  ||
			      i.xform.xo == 316  || i.xform.xo == 664   || i.xform.xo == 536   ||  
			      i.xform.xo == 118   || i.xform.xo == 502   || i.xform.xo == 630  ||
			      i.xform.xo == 29   || i.xform.xo == 541   || i.xform.xo == 242  ||
			      i.xform.xo == 537   || i.xform.xo == 24   || i.xform.xo == 153  ||
			      i.xform.xo == 217   || i.xform.xo == 184   || i.xform.xo == 248  ||
			      i.xform.xo == 216   || i.xform.xo == 152   || i.xform.xo == 952  ||
			      i.xform.xo == 920   || i.xform.xo == 792   || i.xform.xo == 824  ||
			      i.xform.xo == 665   || i.xform.xo == 921   || i.xform.xo == 729  ||
			      i.xform.xo == 631   || i.xform.xo == 599   || i.xform.xo == 823  ||
			      i.xform.xo == 791   || i.xform.xo == 567   || i.xform.xo == 535  ||
			      i.xform.xo == 696   || i.xform.xo == 760   || i.xform.xo == 728  ||
			      i.xform.xo == 792  || i.xform.xo == 824 || i.xform.xo == 536 )) ||
      /* XO Form */
      (i.xoform.op == 31 && ( i.xoform.xo == 8 || i.xoform.xo == 136 || i.xoform.xo == 232 ||
			      i.xoform.xo == 200)) ||
      
      /* D Form */
      /* andi., andis., lbzu, lhau, lhzu, lwzu, ori, oris, stbu, sthu, stwu, xori, xoris */ 
      (i.dform.op == 28 || i.dform.op == 29 || i.dform.op == 35 || i.dform.op == 43 ||
       i.dform.op == 41 || i.dform.op == 33 || i.dform.op == 24 || i.dform.op == 25 ||
       i.dform.op == 39 || i.dform.op == 45 || i.dform.op == 37 || i.dform.op == 26 ||
       i.dform.op == 27 || i.dform.op == 8 || i.dform.op == 12 || i.dform.op == 13) ||
      
      /* M Form */
      /* rlwimi, rlwinm, rlwnm */
      (i.mform.op == 20 || i.mform.op == 21 || i.mform.op == 22 ||  i.mform.op == 23))
    {
      //printf("la %d \n", i.xform.xo);
      return true;
    }
  else
    {
      return false;
    }
  
}



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
		if(!ah.hasPrev())
			return false;
                --ah;
		if(!ah.hasPrev())
			return false;
		instruction j = ah.getInstruction();
		if((j.xfxform.op == 31) && (j.xfxform.xo == 467) &&
		   (j.xfxform.spr == 0x100))
			return true;
		++ah;
                j = ah.getInstruction();
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

    //cerr << currentAddress << endl;
    const instruction i = getInstruction();
    //const instruction i = getInstruction();
#define CALLmatch 0x48000001 /* bl */
        
    // Only look for 'bl' instructions for now, although a branch
    // could be a call function, and it doesn't need to set the link
    // register if it is the last function call
    //cerr << currentAddreOPmask << endl;

    return (isInsnType(i, OPmask | AALKmask, CALLmatch));
    
/*
  const instruction i = getInstruction();
  if(i.iform.lk && 
	   ((i.iform.op == Bop) || (i.bform.op == BCop) ||
       ((i.xlform.op == BCLRop) && 
       ((i.xlform.xo == 16) || (i.xlform.xo == 528))))){
       return true;
       }
       return false;

*/
}
bool InstrucIter::isADynamicCallInstruction()
{
// I'm going to break this up a little so that I can comment
    // it better. 
    
    const instruction i = getInstruction();

  if (i.xlform.op == BCLRop && i.xlform.xo == BCLRxop)
    {
      if (i.xlform.lk)
	// Type one: Branch-to-LR, save PC in LR
	// Standard function pointer call
	return true;
      else
	// Type two: Branch-to-LR, don't save PC
	// Haven't seen one of these around
	// It would be a return instruction, probably
	{
	  return false;
	}
    }
  if (i.xlform.op == BCLRop && i.xlform.xo == BCCTRxop)
    {
      if (i.xlform.lk)
	// Type three: Branch-to-CR, save PC
	{
	  return true;
	}
      else
	// Type four: Branch-to-CR, don't save PC
	// Used for global linkage code.
	// We handle those elsewhere.
	{
	  return true;
	}
    }
  return false;


}


bool InstrucIter::isAnneal(){
  return true;
}

#define MK_LD1(bytes, i, imm, ra) (new BPatch_memoryAccess(&i.raw, sizeof(instruction), true, false, (bytes), (imm), (ra), -1))
#define MK_SD1(bytes, i, imm, ra) (new BPatch_memoryAccess(&i.raw, sizeof(instruction), false, true, (bytes), (imm), (ra), -1))

#define MK_LX1(bytes, i, ra, rb) (new BPatch_memoryAccess(&i.raw, sizeof(instruction), true, false, (bytes), 0, (ra), (rb)))
#define MK_SX1(bytes, i, ra, rb) (new BPatch_memoryAccess(&i.raw, sizeof(instruction), false, true, (bytes), 0, (ra), (rb)))

#define MK_LD(bytes, i) (MK_LD1((bytes), i, i.dform.d_or_si, (signed)i.dform.ra))
#define MK_SD(bytes, i) (MK_SD1((bytes), i, i.dform.d_or_si, (signed)i.dform.ra))

// VG(11/20/01): X-forms ignore ra if 0, but not rb...
#define MK_LX(bytes, i) (MK_LX1((bytes), i, (i.xform.ra ? (signed)i.xform.ra : -1), i.xform.rb))
#define MK_SX(bytes, i) (MK_SX1((bytes), i, (i.xform.ra ? (signed)i.xform.ra : -1), i.xform.rb))

#define MK_LDS(bytes, i) (MK_LD1((bytes), i, (i.dsform.d << 2), (signed)i.dsform.ra))
#define MK_SDS(bytes, i) (MK_SD1((bytes), i, (i.dsform.d << 2), (signed)i.dsform.ra))

#define MK_LI(bytes, i) (MK_LX1((bytes), i, i.xform.ra, -1))
#define MK_SI(bytes, i) (MK_SX1((bytes), i, i.xform.ra, -1))

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

  //bperr( "POWER opcode info initialized.\n");
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
      cerr << "****" << i.dsform.xo << endl; 
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
      return new BPatch_memoryAccess(&i.raw, sizeof(instruction), 
				     oci->direc == 0, oci->direc == 1,
                                     0, i.xform.ra, i.xform.rb,
                                     0, POWER_XER2531, -1); // 9999 == XER_25:31
    }
  }
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
    Address initialAddress = currentAddress;
	Address TOC_address = (addressImage->getObject()).getTOCoffset();

	instruction check;

	// If there are no prior instructions then we can't be looking at a
	// jump through a jump table.
	if( !hasPrev() ) 
    {
		result += (initialAddress + sizeof(instruction));
		return;
	}
    
    // Check if the previous instruction is a move to CTR or LR;
	// if it is, then this is the pattern we're familiar with.  The
	// register being moved into CTR or LR has the address to jump to.
	(*this)--;
	int jumpAddrReg;
	check = getInstruction();
	if (check.xfxform.op == STXop && check.xfxform.xo == MTSPRxop &&
	    (check.xfxform.spr == 0x100 || check.xfxform.spr == 0x120)) {
		jumpAddrReg = check.xfxform.rt;
	} else {
		result += (initialAddress + sizeof(instruction));
		return;
	}

    // In the pattern we've seen, if the instruction previous to this is
	// an add with a result that ends up being used as the jump address,
	// then we're adding a relative value we got from the table to a base
	// address to get the jump address; in other words, the contents of
	// the jump table are relative.
	bool tableIsRelative = false;
	if (hasPrev()) {
		(*this)--;
		check = getInstruction();
		if (check.xoform.op == CAXop && check.xoform.xo == CAXxop &&
		    check.xoform.rt == (unsigned)jumpAddrReg)
			tableIsRelative = true;
		else
			(*this)++;
	}

	Address jumpStartAddress = 0;
	Address adjustEntry = 0;
	Address tableStartAddress = 0;

	if (tableIsRelative) {
		while( hasPrev() ){
			check = getInstruction();
			if((check.dform.op == Lop) && (check.dform.ra == 2)){
				jumpStartAddress = 
				    (Address)(TOC_address + check.dform.d_or_si);
				break;
			}
			(*this)--;
		}
		(*this)--;
		check = getInstruction();
		if((check.dform.op == Lop))
			adjustEntry = check.dform.d_or_si;

		while(hasPrev()){
			instruction check = getInstruction();
			if((check.dform.op == Lop) && (check.dform.ra == 2)){
				tableStartAddress = 
				    (Address)(TOC_address + check.dform.d_or_si);
				break;
			}
			(*this)--;
		}
	} else {
		bool foundAdjustEntry = false;
		while( hasPrev() ){
			check = getInstruction();
			if(check.dform.op == CALop &&
			   check.dform.rt == (unsigned)jumpAddrReg &&
			   !foundAdjustEntry){
				foundAdjustEntry = true;
				adjustEntry = check.dform.d_or_si;
				jumpAddrReg = check.dform.ra;
			} else if(check.dform.op == Lop &&
				  check.dform.ra == 2 &&
				  check.dform.rt == (unsigned)jumpAddrReg){
				tableStartAddress = 
				    (Address)(TOC_address + check.dform.d_or_si);
				break;
			}
			(*this)--;
		}
	}


	setCurrentAddress(initialAddress);
	int maxSwitch = 0;
  
	while( hasPrev() ){
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
    
    if( currentAddress > baseAddress )
    //if((currentAddress < (baseAddress + range )) &&
    // (currentAddress > baseAddress))
	return true;

    //cerr << "hasprev" << std::hex << currentAddress 
    //   << " "  << baseAddress << " "  << range << endl;
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
