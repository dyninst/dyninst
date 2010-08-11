/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
#include "instPoint.h"
#include "InstrucIter.h"
#include "mapped_object.h"

//#include "std::set.h"

#include "BPatch_instruction.h"
#include "BPatch_memoryAccess_NP.h"

#include "registerSpace.h"

/* Prints the op code */
void InstrucIter::printOpCode()
{
  const instruction i = getInstruction();
  printf("OpCode num %d", XLFORM_OP(i));
}

bool InstrucIter::isRegConstantAssignment(int * regArray, Address * regWL)
{
  const instruction i = getInstruction();
  int regNum = DFORM_RT(i);

  if (isA_RT_WriteInstruction() || isA_RA_WriteInstruction())
  {
    if (regNum >=3 && regNum <= 10)
      regWL[regNum] = current;
  }
  
  // int regNum = (*i).dform.rt;
  if(XLFORM_OP(i) == 14 && DFORM_RA(i) == 0 
     && regNum >= 3 && regNum <= 10)
  {
    regArray[regNum]  = DFORM_SI(i);
    //if(regNum == 8)
    //printf("Value for reg 8 is %d\n",regArray[regNum]);
      
    return true;
  }
  else
    return false;
}


bool InstrucIter::isClauseInstruction()
{
  const instruction i = getInstruction();
  if(XLFORM_OP(i) == 14 && DFORM_RT(i) == 3 && DFORM_RA(i) == 0)
  {
    printf("Clause value is %d\n", DFORM_SI(i));
    return true;
  }
  else
    return false;
}

signed InstrucIter::getDFormDValue()
{
  const instruction i = getInstruction();
  return DFORM_SI(i);
}

/* Returns the value of the RT Register */
unsigned InstrucIter::getRTValue(void)
{
  const instruction i = getInstruction();
  return XFORM_RT(i);
}

/* Returns the value of the RA Register */
unsigned InstrucIter::getRAValue(void)
{
  const instruction i = getInstruction();
  return XFORM_RA(i);
}

unsigned InstrucIter::getRBValue(void)
{
  const instruction i = getInstruction();
  return XFORM_RB(i);
}

unsigned InstrucIter::getFRTValue(void)
{
  const instruction i = getInstruction();
  return AFORM_FRT(i);
}

unsigned InstrucIter::getFRAValue(void)
{
  const instruction i = getInstruction();
  return AFORM_FRA(i);
}

unsigned InstrucIter::getFRBValue(void)
{
  const instruction i = getInstruction();
  return AFORM_FRB(i);
}

unsigned InstrucIter::getFRCValue(void)
{
  const instruction i = getInstruction();
  return AFORM_FRC(i);
}

/* Returns true if the instruction reads/writes the MX (SPR0) SP Register */
bool InstrucIter::isA_MX_Instruction()
{
  const instruction i = getInstruction();

  if (
      (XFORM_OP(i) == X_EXTENDEDop
       && ( XFORM_XO(i) == SREQxop || XFORM_XO(i) == SLEQxop ||
	    XFORM_XO(i) == SLLIQxop || XFORM_XO(i) == SRLIQxop ||
	    XFORM_XO(i) == SRLQxop || XFORM_XO(i) == SLLQxop ||
	    XFORM_XO(i) == SLIQxop || XFORM_XO(i) == SRQxop ||
	    XFORM_XO(i) == SLQxop || XFORM_XO(i) == SRIQxop ||
	    XFORM_XO(i) == SRAIQxop || XFORM_XO(i) == SREAxop ||
	    XFORM_XO(i) == STSXxop || XFORM_XO(i) == STSIxop ||
	    XFORM_XO(i) == LSCBXxop
	    ))
      ||
      (XOFORM_OP(i) == XO_EXTENDEDop
       && ( XOFORM_XO(i) == DIVxop || XOFORM_XO(i) == DIVSxop ||
	    XOFORM_XO(i) == MULxop )))
    return true;
  else
    return false;
}



/* This function returns true if the instruction affects the RD Register */
bool InstrucIter::isA_RT_WriteInstruction()
{
  const instruction i = getInstruction();
  if (
      /* XO Form */
      (XOFORM_OP(i) == XO_EXTENDEDop
       && ( XOFORM_XO(i) == CAXxop || XOFORM_XO(i) == Axop 
	    || XOFORM_XO(i) == AExop || XOFORM_XO(i) == NABSxop 
	    || XOFORM_XO(i) == ANDCxop || XOFORM_XO(i) == DIVWxop
	    || XOFORM_XO(i) == SUBFxop || XOFORM_XO(i) == SFxop
	    || XOFORM_XO(i) == SFExop || XOFORM_XO(i) == SFMExop 
	    || XOFORM_XO(i) == SFZExop || XOFORM_XO(i) == MULHWxop 
	    || XOFORM_XO(i) == DIVWUxop || XOFORM_XO(i) == MULSxop 
	    || XOFORM_XO(i) == NEGxop || XOFORM_XO(i) == ABSxop 
	    || XOFORM_XO(i) == DIVxop || XOFORM_XO(i) == DIVSxop
	    || XOFORM_XO(i) == DOZxop || XOFORM_XO(i) == MULxop 
	    || XOFORM_XO(i) == SFExop || XOFORM_XO(i) == SFMExop 
	    || XOFORM_XO(i) == SFZExop || XOFORM_XO(i) == MULHWUxop )) ||
	
      /* D Form */
      (DFORM_OP(i) == MULIop || DFORM_OP(i) == SFIop || DFORM_OP(i) == DOZIop   ) || 
      (DFORM_OP(i) >= SIop && DFORM_OP(i)  <= CAUop) || 
      (DFORM_OP(i) >= Lop && DFORM_OP(i) <= LBZUop) || 
      (DFORM_OP(i) >= LHZop && DFORM_OP(i) <= LHAUop) || 
	
      /* X Form */
      (XFORM_OP(i) == X_EXTENDEDop 
       && ( XFORM_XO(i) == LBZUXxop || 
            XFORM_XO(i) == LBZXxop || 
	    XFORM_XO(i) == LHAUXxop  || 
	    XFORM_XO(i) == LHAXxop || 
            XFORM_XO(i) == LHBRXxop ||
	    XFORM_XO(i) == LHZUXxop  || 
            XFORM_XO(i) == LHZXxop || 
	    XFORM_XO(i) == LWARXxop || 
            XFORM_XO(i) == LWBRXxop  || 
	    XFORM_XO(i) == LUXxop || 
            XFORM_XO(i) == LXxop ||
	    XFORM_XO(i) == MFCRxop  ||  
	    XFORM_XO(i) == MFMSRxop || 
            XFORM_XO(i) == MFSPRxop  || 
	    XFORM_XO(i) == CLCSxop  || 
            XFORM_XO(i) == MFSRxop || 
	    XFORM_XO(i) == MFSRIxop ||
	    XFORM_XO(i) == MFSRINxop  || 
            XFORM_XO(i) == RACxop ||
            XFORM_XO(i) == LWAXxop ||
            XFORM_XO(i) == LWAUXxop ||
            XFORM_XO(i) == LDXxop ||
            XFORM_XO(i) == LDUXxop ||
            XFORM_XO(i) == LDARXxop)) ||
      /* dsform */ 
      (DSFORM_OP(i) == LDop &&
       (DSFORM_XO(i) == LWAxop ||
        DSFORM_XO(i) == LDxop ||
        DSFORM_XO(i) == LDUxop)))
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
      (AFORM_OP(i) == A_FP_EXTENDEDop2 
       && ( AFORM_XO(i) == FAxop || AFORM_XO(i) == FDxop || 
	    AFORM_XO(i) == FMxop || AFORM_XO(i) == FMAxop  || 
	    AFORM_XO(i) == FMSxop || AFORM_XO(i) == FNMAxop ||
	    AFORM_XO(i) == FNMSxop || AFORM_XO(i) == FSQRTxop || 
	    AFORM_XO(i) == FSxop )) ||  

      (AFORM_OP(i) == A_FP_EXTENDEDop1
       && ( AFORM_XO(i) == FDIVSxop || AFORM_XO(i) == FMADDSxop || 
	    AFORM_XO(i) == FMULSxop || AFORM_XO(i) == FMSUBSxop  || 
	    AFORM_XO(i) == FNMSUBSxop || AFORM_XO(i) == FNMADDSxop ||
	    AFORM_XO(i) == FSUBSxop )) ||  
      /* X Form */
      (XFORM_OP(i) == X_FP_EXTENDEDop 
       && ( XFORM_XO(i) == MFFSxop || XFORM_XO(i) == FABSxop ||
	    XFORM_XO(i) == FCIRZxop  || XFORM_XO(i) == FMRxop || 
	    XFORM_XO(i) == FNABSxop || XFORM_XO(i) == FCIRxop ||
	    XFORM_XO(i) == FNEGxop || XFORM_XO(i) == FRSPxop )) ||

      (XFORM_OP(i) == X_EXTENDEDop
       && ( XFORM_XO(i) == LFDUXxop  || XFORM_XO(i) == LFDXxop || 
	    XFORM_XO(i) == LFSUXxop || XFORM_XO(i) == LFSXxop )))
  {
    return true;
  }
  else
  {
    return false;
  }
}  


/* This function returns true if the instruction reads the RT Register */
bool InstrucIter::isA_RT_ReadInstruction()
{
  const instruction i = getInstruction();
  if (
      /* D Form */
      (DFORM_OP(i) >= ORILop && DFORM_OP(i)  <= ANDIUop) || 
      (DFORM_OP(i) >= STop && DFORM_OP(i)  <= STBUop) ||
      DFORM_OP(i) == STMop ||

      /* M form */
      (MFORM_OP(i) >= RLIMIop && MFORM_OP(i) <= RLNMop) ||

      /*  XFX Form */
      (XFXFORM_OP(i) == X_EXTENDEDop && XFXFORM_XO(i) == MTCRFxop ) ||
      
      /* X Form */
      (XFXFORM_OP(i) == X_EXTENDEDop
       && ( XFORM_XO(i) == SLxop || 
            XFORM_XO(i) == CNTLZxop ||
	    XFORM_XO(i) == ANDxop  || 
            XFORM_XO(i) == MASKGxop || 
	    XFORM_XO(i) == ANDCxop || 
            XFORM_XO(i) == EXTSxop ||
	    XFORM_XO(i) == NORxop  || 
            XFORM_XO(i) == STWCXxop ||
	    XFORM_XO(i) == STXxop  || 
            XFORM_XO(i) == SLQxop || 
	    XFORM_XO(i) == SLExop ||
	    XFORM_XO(i) == STUXxop  ||
            XFORM_XO(i) == SLIQxop || 
	    XFORM_XO(i) == STBXxop ||
	    XFORM_XO(i) == SLLQxop  ||
            XFORM_XO(i) == SLEQxop || 
	    XFORM_XO(i) == STBUXxop ||
	    XFORM_XO(i) == SLLIQxop  || 
            XFORM_XO(i) == EQVxop || 
	    XFORM_XO(i) == XORxop ||
	    XFORM_XO(i) == ORCxop  || 
            XFORM_XO(i) == ORxop ||
	    XFORM_XO(i) == MTSPRxop  || 
            XFORM_XO(i) == NANDxop || 
	    XFORM_XO(i) == SRxop ||
	    XFORM_XO(i) == RRIBxop  ||
            XFORM_XO(i) == MASKIRxop || 
	    XFORM_XO(i) == STSXxop ||
	    XFORM_XO(i) == STBRXxop  ||
            XFORM_XO(i) == SRQxop || 
	    XFORM_XO(i) == SRExop ||
	    XFORM_XO(i) == SRIQxop  ||
            XFORM_XO(i) == STSIxop || 
	    XFORM_XO(i) == SRLQxop ||
	    XFORM_XO(i) == SREQxop  ||
            XFORM_XO(i) == SRLIQxop ||
	    XFORM_XO(i) == SRAxop ||
	    XFORM_XO(i) == SRAIxop  || 
            XFORM_XO(i) == SRAQxop || 
	    XFORM_XO(i) == SREAxop ||
	    XFORM_XO(i) == EXTSxop ||
            XFORM_XO(i) == SRAIQxop || 
	    XFORM_XO(i) == EXTSBxop ||
            XFORM_XO(i) == STDXxop || XFORM_XO(i) == STDUXxop ||
            XFORM_XO(i) == STDCXxop)) ||
      (DSFORM_OP(i) == STDop &&
       (DSFORM_XO(i) == STDxop ||
        DSFORM_XO(i) == STDUxop)) ||
      (MDFORM_OP(i) == RLDop))
  {
    return true;
  }
  else
  {
    return false;
  }
}

/* This function returns true if the instruction reads the FRT Register */
bool InstrucIter::isA_FRT_ReadInstruction()
{
  const instruction i = getInstruction();
  if (
      /* D Form */
      (DFORM_OP(i) == STHop || DFORM_OP(i) == STHUop ) || 
      (DFORM_OP(i) >= STFSop && DFORM_OP(i)  <= STFDUop) ||
      /* X Form */
      (XFORM_OP(i) == X_EXTENDEDop 
       && ( XFORM_XO(i) == STFSXxop || XFORM_XO(i) == STFSUXxop  || 
	    XFORM_XO(i) == STFDXxop || XFORM_XO(i) == STFDUXxop ||  
	    XFORM_XO(i) == STFIWXxop )))
  {
    return true;
  }
  else
  {
    return false;
  }
}  

  
/* This function returns true if the instruction reads the RA Register */
bool InstrucIter::isA_RA_ReadInstruction()
{
  const instruction i = getInstruction();
  if (
      /* XO Form */
      (XOFORM_OP(i) == XO_EXTENDEDop 
       && ( XOFORM_XO(i) == SFxop || XOFORM_XO(i) == Axop || XOFORM_XO(i) == MULHWUxop ||
	    XOFORM_XO(i) == SUBFxop  || XOFORM_XO(i) == MULHWxop || XOFORM_XO(i) == NEGxop ||
	    XOFORM_XO(i) == MULxop || XOFORM_XO(i) == SFExop || XOFORM_XO(i) == AExop || 
	    XOFORM_XO(i) == SFZExop || XOFORM_XO(i) == AZExop || XOFORM_XO(i) == SFMExop ||  
	    XOFORM_XO(i) == AMExop || XOFORM_XO(i) == DOZxop || XOFORM_XO(i) == CAXxop ||
	    XOFORM_XO(i) == DIVxop || XOFORM_XO(i) == ABSxop || XOFORM_XO(i) == DIVSxop ||
	    XOFORM_XO(i) == DIVWUxop || XOFORM_XO(i) == NABSxop || XOFORM_XO(i) == DIVWxop ||
            XOFORM_XO(i) == MULSxop)) ||
      /* D Form */
      (DFORM_OP(i) == TIop || DFORM_OP(i) == MULIop || 
       DFORM_OP(i) == SFIop || DFORM_OP(i) == DOZIop   ) || 
      (DFORM_OP(i) >= CMPLIop && DFORM_OP(i) < CALop) ||
      (DFORM_OP(i) >= CALop && DFORM_OP(i) <= CAUop && DFORM_RA(i)) ||
      (DFORM_OP(i) >= Lop && DFORM_OP(i) <= STFDUop) ||
      
      /* X Form */
      (XFORM_OP(i) == X_EXTENDEDop
       && ( XFORM_XO(i) == CMPxop || XFORM_XO(i) == Txop ||
	    XFORM_XO(i) == LWARXxop  || XFORM_XO(i) == LXxop || XFORM_XO(i) == CMPLxop ||
	    XFORM_XO(i) == DCBSxop  || XFORM_XO(i) == LUXxop || XFORM_XO(i) == DCBFxop ||
	    XFORM_XO(i) == LBZXxop  || XFORM_XO(i) == CLFxop || XFORM_XO(i) == LBZUXxop ||
	    XFORM_XO(i) == STWCXxop  || XFORM_XO(i) == STXxop || XFORM_XO(i) == STUXxop ||
	    XFORM_XO(i) == STBXxop  || XFORM_XO(i) == DCBTSTxop || XFORM_XO(i) == STBUXxop ||
	    XFORM_XO(i) == LSCBXxop  || XFORM_XO(i) == DCBTxop || XFORM_XO(i) == LHZXxop ||
            XFORM_XO(i) == LHZUXxop ||
	    XFORM_XO(i) == TLBIxop  || XFORM_XO(i) == DIVxop || XFORM_XO(i) == LHAXxop ||
	    XFORM_XO(i) == LHAUXxop  || XFORM_XO(i) == STHXxop || XFORM_XO(i) == STHUXxop ||
	    XFORM_XO(i) == DCBIxop || XFORM_XO(i) == CLIxop || XFORM_XO(i) == CLCSxop ||
	    XFORM_XO(i) == LSXxop  || XFORM_XO(i) == LWBRXxop || XFORM_XO(i) == LFSXxop ||
	    XFORM_XO(i) == LFSUXxop  || XFORM_XO(i) == LSIxop || XFORM_XO(i) == LFDXxop ||
	    XFORM_XO(i) == DCLSTxop  || XFORM_XO(i) == LFDUXxop || XFORM_XO(i) == STSXxop ||
	    XFORM_XO(i) == STBRXxop  || XFORM_XO(i) == STFSXxop || XFORM_XO(i) == STFSUXxop ||
	    XFORM_XO(i) == STSIxop  || XFORM_XO(i) == STFDXxop || XFORM_XO(i) == STFDUXxop ||
	    XFORM_XO(i) == LHBRXxop  || XFORM_XO(i) == RACxop ||
	    XFORM_XO(i) == STHBRXxop || XFORM_XO(i) == ICBIxop || 
	    XFORM_XO(i) == STFIWXxop || XFORM_XO(i) == DCLZxop ||
            XFORM_XO(i) == LWAXxop || XFORM_XO(i) == LWAUXxop ||
            XFORM_XO(i) == LDXxop || XFORM_XO(i) == LDUXxop ||
            XFORM_XO(i) == STDXxop || XFORM_XO(i) == STDUXxop ||
            XFORM_XO(i) == LDARXxop || XFORM_XO(i) == STDCXxop)) ||
      (DSFORM_OP(i) == LDop &&
       (DSFORM_XO(i) == LWAxop ||
        DSFORM_XO(i) == LDxop ||
        DSFORM_XO(i) == LDUxop)) ||
      (DSFORM_OP(i) == STDop &&
       (DSFORM_XO(i) == STDxop ||
        DSFORM_XO(i) == STDUxop)))
  {
    return true;
  }
  else
  {
    return false;
  }
}  


/* This function returns true if the instruction reads the RB Register */
bool InstrucIter::isA_RB_ReadInstruction()
{
  const instruction i = getInstruction();
  if (
      /* XO Form */
      (XOFORM_OP(i) == XO_EXTENDEDop
       && ( XOFORM_XO(i) == SFxop || XOFORM_XO(i) == Axop || XOFORM_XO(i) == MULHWUxop ||
	    XOFORM_XO(i) == SUBFxop  || XOFORM_XO(i) == MULHWxop || 
	    XOFORM_XO(i) == MULxop || XOFORM_XO(i) == SFExop ||
	    XOFORM_XO(i) == MULSxop || XOFORM_XO(i) == AExop || 
	    XOFORM_XO(i) == DOZxop || XOFORM_XO(i) == CAXxop ||
	    XOFORM_XO(i) == DIVxop || XOFORM_XO(i) == DIVSxop ||
	    XOFORM_XO(i) == DIVWUxop || XOFORM_XO(i) == DIVWxop)) ||
      /* M Form */
      (MFORM_OP(i) == RLMIop || MFORM_OP(i) == RLNMop) ||
     
      /* X Form */
      (XFORM_OP(i) == X_EXTENDEDop
       && ( XFORM_XO(i) == CMPxop || XFORM_XO(i) == Txop ||
	    XFORM_XO(i) == LWARXxop  || XFORM_XO(i) == LXxop || XFORM_XO(i) == SLxop ||
	    XFORM_XO(i) == ANDxop  || XFORM_XO(i) == MASKGxop || XFORM_XO(i) == CMPLxop ||
	    XFORM_XO(i) == DCBSxop  || XFORM_XO(i) == LUXxop || XFORM_XO(i) == ANDCxop ||
	    XFORM_XO(i) == DCBFxop  || XFORM_XO(i) == LBZXxop || XFORM_XO(i) == CLFxop ||
	    XFORM_XO(i) == LBZUXxop  || XFORM_XO(i) == NORxop || XFORM_XO(i) == STWCXxop ||
	    XFORM_XO(i) == STXxop  || XFORM_XO(i) == SLQxop || XFORM_XO(i) == SLExop ||
	    XFORM_XO(i) == STUXxop  || XFORM_XO(i) == STBXxop || XFORM_XO(i) == SLLQxop ||
	    XFORM_XO(i) == SLEQxop  || XFORM_XO(i) == DCBTSTxop || XFORM_XO(i) == STBUXxop ||
	    XFORM_XO(i) == LSCBXxop  || XFORM_XO(i) == DCBTxop || XFORM_XO(i) == LHZXxop ||
            XFORM_XO(i) == LHZUXxop ||
	    XFORM_XO(i) == EQVxop  || XFORM_XO(i) == TLBIxop || XFORM_XO(i) == XORxop ||
	    XFORM_XO(i) == DIVxop  || XFORM_XO(i) == LHAXxop || XFORM_XO(i) == LHAUXxop ||
	    XFORM_XO(i) == STHXxop  || XFORM_XO(i) == ORCxop || XFORM_XO(i) == STHUXxop ||
	    XFORM_XO(i) == ORxop || XFORM_XO(i) == DCBIxop || XFORM_XO(i) == NANDxop ||
	    XFORM_XO(i) == CLIxop  || XFORM_XO(i) == LSXxop || XFORM_XO(i) == LWBRXxop ||
	    XFORM_XO(i) == LFSXxop  || XFORM_XO(i) == SRxop || XFORM_XO(i) == RRIBxop ||
	    XFORM_XO(i) == MASKIRxop  || XFORM_XO(i) == LFSUXxop || XFORM_XO(i) == LFDXxop ||
	    XFORM_XO(i) == DCLSTxop  || XFORM_XO(i) == LFDUXxop || XFORM_XO(i) == STSXxop ||
	    XFORM_XO(i) == STBRXxop  || XFORM_XO(i) == STFSXxop || XFORM_XO(i) == SRQxop ||
	    XFORM_XO(i) == SRExop  || XFORM_XO(i) == SRIQxop || XFORM_XO(i) == STFDXxop ||
	    XFORM_XO(i) == SREQxop  || XFORM_XO(i) == STFDUXxop || XFORM_XO(i) == LHBRXxop ||
	    XFORM_XO(i) == SRAxop || XFORM_XO(i) == RACxop ||
	    XFORM_XO(i) == STHBRXxop ||
	    XFORM_XO(i) == SRAQxop  || XFORM_XO(i) == SREAxop ||
	    XFORM_XO(i) == ICBIxop || XFORM_XO(i) == STFIWXxop ||
	    XFORM_XO(i) == DCLZxop ||
            XFORM_XO(i) == LWAXxop || XFORM_XO(i) == LWAUXxop ||
            XFORM_XO(i) == LDXxop || XFORM_XO(i) == LDUXxop ||
            XFORM_XO(i) == STDXxop || XFORM_XO(i) == STDUXxop ||
            XFORM_XO(i) == LDARXxop || XFORM_XO(i) == STDCXxop)))
     
  {
    return true;
  }
  else
  {
    return false;
  }
}  




/* This function returns true if the instruction affects the FRA Register */
bool InstrucIter::isA_FRA_ReadInstruction()
{
  const instruction i = getInstruction();
  if (
      /* A Form */
      (AFORM_OP(i) == A_FP_EXTENDEDop2 
       && ( AFORM_XO(i) == FDxop || AFORM_XO(i) == FSxop || AFORM_XO(i) == FAxop ||
	    AFORM_XO(i) == FMxop  || AFORM_XO(i) == FMSxop || AFORM_XO(i) == FMAxop ||
	    AFORM_XO(i) == FNMSxop || AFORM_XO(i) == FNMAxop )) ||

      (AFORM_OP(i) == A_FP_EXTENDEDop1
       && ( AFORM_XO(i) == FDIVSxop || AFORM_XO(i) == FSUBSxop || 
	    AFORM_XO(i) == FADDSxop || AFORM_XO(i) == FMULSxop  || 
	    AFORM_XO(i) == FMSUBSxop || AFORM_XO(i) == FMADDSxop ||
	    AFORM_XO(i) == FNMSUBSxop || AFORM_XO(i) == FNMADDSxop )) ||  
      
      /* XL Form */
      (XLFORM_OP(i) == X_FP_EXTENDEDop && XLFORM_XO(i) == FCMPUxop ) || 
      /* X Form */
      (XFORM_OP(i) == X_FP_EXTENDEDop && XFORM_XO(i) == FCMPOxop ))

  {
    return true;
  }
  else
  {
    return false;
  }
}  

/* This function returns true if the instruction affects the FRB Register */
bool InstrucIter::isA_FRB_ReadInstruction()
{
  const instruction i = getInstruction();
  if (
      /* A Form */
      (AFORM_OP(i) == A_FP_EXTENDEDop2 
       && (  AFORM_XO(i) == FDxop || AFORM_XO(i) == FSxop || 
	     AFORM_XO(i) == FAxop ||
	     AFORM_XO(i) == FSQRTxop ||
	     AFORM_XO(i) == FMxop  || AFORM_XO(i) == FMSxop || 
	     AFORM_XO(i) == FMAxop ||
	     AFORM_XO(i) == FNMSxop || AFORM_XO(i) == FNMAxop )) ||

      (AFORM_OP(i) == A_FP_EXTENDEDop1
       && ( AFORM_XO(i) == FDIVSxop || AFORM_XO(i) == FSUBSxop || 
	    AFORM_XO(i) == FADDSxop ||
	    AFORM_XO(i) == FMULSxop  || AFORM_XO(i) == FMSUBSxop || 
	    AFORM_XO(i) == FMAxop ||
	    AFORM_XO(i) == FNMSxop || AFORM_XO(i) == FNMAxop )) ||  
      
      /* XL Form */
      (XLFORM_OP(i) == X_EXTENDEDop 
       && XLFORM_XO(i) == FCMPUxop ) || 
     

      /* X Form */
      (XFORM_OP(i) == X_EXTENDEDop 
       && ( XFORM_XO(i) == FRSPxop || XFORM_XO(i) == FCIRxop || 
	    XFORM_XO(i) == FCIRZxop || XFORM_XO(i) == FCMPOxop || 
	    XFORM_XO(i) == FNEGxop ||
	    XFORM_XO(i) == FMRxop ||
	    XFORM_XO(i) == FNABSxop || XFORM_XO(i) == FABSxop )))
  {
    return true;
  }
  else
  {
    return false;
  }
}  

/* This function returns true if the instruction reads the FRC Registers */
bool InstrucIter::isA_FRC_ReadInstruction()
{
  const instruction i = getInstruction();
  if (
      /* A Form */
      (AFORM_OP(i) == A_FP_EXTENDEDop2
       && ( AFORM_XO(i) == FMxop || AFORM_XO(i) == FMSxop || AFORM_XO(i) == FMAxop ||
	    AFORM_XO(i) == FNMSxop  || AFORM_XO(i) == FNMAxop )))
    return true;
  else
    return false;
	   
}


/* This function returns true if the instruction affects the FRA Register */
bool InstrucIter::isA_FRA_WriteInstruction()
{
  const instruction i = getInstruction();
  if (
      /* A Form */
      (AFORM_OP(i) == A_FP_EXTENDEDop2
       && ( AFORM_XO(i) == FAxop || AFORM_XO(i) == FDxop || AFORM_XO(i) == FMxop ||
	    AFORM_XO(i) == FMAxop  || AFORM_XO(i) == FMSxop || AFORM_XO(i) == FNMAxop ||
	    AFORM_XO(i) == FNMSxop || AFORM_XO(i) == FSxop )) ||  
      
      (AFORM_OP(i) == A_FP_EXTENDEDop1
       && ( AFORM_XO(i) == FDIVSxop || AFORM_XO(i) == FMADDSxop ||
	    AFORM_XO(i) == FMULSxop ||
	    AFORM_XO(i) == FMSUBSxop  || AFORM_XO(i) == FNMSUBSxop 
	    || AFORM_XO(i) == FNMADDSxop ||
	    AFORM_XO(i) == FSUBSxop )) ||  
      
      /* X Form */
      (XFORM_OP(i) == X_FP_EXTENDEDop 
       && (XFORM_XO(i) == FCMPOxop || XFORM_XO(i) == FCMPUxop )))

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
  if (
      /* X Form */
      ( XFORM_OP(i) == X_EXTENDEDop
	&& ( XFORM_XO(i) == NORxop ||  XFORM_XO(i) == STBUXxop || XFORM_XO(i) == ANDxop ||
	     XFORM_XO(i) == ORxop  || XFORM_XO(i) == ORCxop || XFORM_XO(i) == SLxop ||
	     XFORM_XO(i) == ANDCxop  || XFORM_XO(i) == EQVxop ||
	     XFORM_XO(i) == EXTSBxop  || XFORM_XO(i) == EXTSxop  || XFORM_XO(i) == LHZUXxop  ||
	     XFORM_XO(i) == LUXxop   || XFORM_XO(i) == LBZUXxop  || XFORM_XO(i) == LHAUXxop ||
	     XFORM_XO(i) == NANDxop   || XFORM_XO(i) == STHUXxop   || XFORM_XO(i) == STUXxop  ||
	     XFORM_XO(i) == XORxop  || XFORM_XO(i) == SRQxop   || XFORM_XO(i) == SRxop   ||  
	     XFORM_XO(i) == CLFxop   || XFORM_XO(i) == CLIxop   || XFORM_XO(i) == DCLSTxop  ||
	     XFORM_XO(i) == MASKGxop   || XFORM_XO(i) == MASKIRxop   || XFORM_XO(i) == MTSRIxop  ||
	     XFORM_XO(i) == RRIBxop   || XFORM_XO(i) == SLxop   || XFORM_XO(i) == SLExop  ||
	     XFORM_XO(i) == SLEQxop   || XFORM_XO(i) == SLIQxop   || XFORM_XO(i) == SLLIQxop  ||
	     XFORM_XO(i) == SLLQxop   || XFORM_XO(i) == SLQxop   || XFORM_XO(i) == SRAIQxop  ||
	     XFORM_XO(i) == SRAQxop   || XFORM_XO(i) == SRAxop   || XFORM_XO(i) == SRAIxop  ||
	     XFORM_XO(i) == SRExop   || XFORM_XO(i) == SREAxop   || XFORM_XO(i) == SREQxop  ||
	     XFORM_XO(i) == LFDUXxop   || XFORM_XO(i) == LFDXxop   ||
	     XFORM_XO(i) == LFSUXxop   || XFORM_XO(i) == LFSXxop  ||
	     XFORM_XO(i) == SRIQxop   || XFORM_XO(i) == SRLIQxop   || XFORM_XO(i) == SRLQxop  ||
	     XFORM_XO(i) == SRAxop  || XFORM_XO(i) == SRAIxop || XFORM_XO(i) == SRxop ||
             XFORM_XO(i) == LWAUXxop || XFORM_XO(i) == LDUXxop ||
             XFORM_XO(i) == STDUXxop)) ||
      
      /* D Form */
      (DFORM_OP(i) == ANDILop || DFORM_OP(i) == ANDIUop || DFORM_OP(i) == LBZUop 
       || DFORM_OP(i) == LHAUop ||DFORM_OP(i) == LHZUop || DFORM_OP(i) == LUop || 
       DFORM_OP(i) == ORILop || DFORM_OP(i) == ORIUop || DFORM_OP(i) == STBUop || 
       DFORM_OP(i) == STHUop || DFORM_OP(i) == STUop || DFORM_OP(i) == XORILop ||
       DFORM_OP(i) == XORIUop || DFORM_OP(i) == SFIop || DFORM_OP(i) == SIop) || 
      
      /* M Form */
      (MFORM_OP(i) == RLIMIop || MFORM_OP(i) == RLINMxop || MFORM_OP(i) == RLMIop 
       ||  MFORM_OP(i) == RLNMop) ||
      /* DS form */
      (DSFORM_OP(i) == LDop &&
       (DSFORM_XO(i) == LDUxop)) ||
      (DSFORM_OP(i) == STDop &&
       (DSFORM_XO(i) == STDUxop)) ||
      MDFORM_OP(i) == RLDop)
  {
    return true;
  }
  else
  {
    return false;
  }
  
}

/* This function returns true if the instruction reads from multiple RT Registers */
bool InstrucIter::isA_MRT_ReadInstruction()
{
  const instruction i = getInstruction();
  if (
      /* D Form */
      (DFORM_OP(i) == STMop) ||
      /* X Form */
      (XFORM_OP(i) == X_EXTENDEDop
       && (XFORM_XO(i) == STSIxop || XFORM_XO(i) == STSXxop)))
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool InstrucIter::isA_MRT_WriteInstruction()
{
  const instruction i = getInstruction();
  if (
      /* X Form */
      (DFORM_OP(i) == LMop) ||
      /* X Form */
      (XFORM_OP(i) == X_EXTENDEDop
       && (XFORM_XO(i) == LSIxop || XFORM_XO(i) == LSXxop)))
  {
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
  if((XLFORM_OP(i) == BCLRop) &&
     (XLFORM_XO(i) == BCLRxop) && 
     (XLFORM_BT(i) & 0x10) && (XLFORM_BT(i) & 0x4) &&
     !(XLFORM_BB(i)) 
#if defined(os_linux)   // not sure if this is safe on AIX, yet
     // Bernat, 9JUL07. A linking branch-to-link-register is
     // unlikely to be a return instruction.
     //
     // nater 16FEB09 unless the instruction hint indicates subroutine return
     //               (BH is low 2 bits of BB in XL-FORM)
     && (!(XLFORM_LK(i)) || !(XLFORM_BB(i) & 3))
#endif
     )
    return true;
  return false;
}


/** is the instruction used to return from the functions,
    dependent upon a condition register
    * @param i the instruction value 
    */
bool InstrucIter::isACondReturnInstruction()
{
  const instruction i = getInstruction();
  if((XLFORM_OP(i) == BCLRop) &&
     (XLFORM_XO(i) == BCLRxop) &&
     (XLFORM_BT(i) & 0x14) != 0x14)
    return true;
  return false;
}

/** is the instruction an indirect jump instruction 
 * @param i the instruction value 
 */
bool InstrucIter::isAIndirectJumpInstruction()
{
  const instruction i = getInstruction();
  if((XLFORM_OP(i) == BCLRop) && (XLFORM_XO(i) == BCCTRxop) &&
     !XLFORM_LK(i) && (XLFORM_BT(i) & 0x10) && (XLFORM_BT(i) & 0x4))
    return true;
  if((XLFORM_OP(i) == BCLRop) && (XLFORM_XO(i) == BCLRxop) &&
     !XLFORM_LK(i) && (XLFORM_BT(i) & 0x10) && (XLFORM_BT(i) & 0x4) &&
     (XLFORM_BB(i) == 0x1))
    return true;
#if 0
  // Disabled; a branch to link register is a return until we can prove
  // otherwise, not a indirect branch.
  if((XLFORM_OP(i) == BCLRop) && (XLFORM_XO(i) == BCLRxop) &&
     (XLFORM_BT(i) & 0x10) && (XLFORM_BT(i) & 0x4)){
    if(!ah.hasPrev())
      return false;
    --ah;
    if(!ah.hasPrev())
      return false;
    instruction j = ah.getInstruction();
    if((XFXFORM_OP(j) == STXop) && (XFXFORM_XO(j) == MTSPRxop) &&
       (XFXFORM_SPR(j) == 0x100)) // LR
      return true;
    ++ah;
    j = ah.getInstruction();
    if((XFXFORM_OP(j) == STXop) && (XFXFORM_XO(j) == MTSPRxop) &&
       (XFXFORM_SPR(j) == 0x100)) // LR
      return true;
  }
#endif
  return false;
}

/** is the instruction a conditional branch instruction 
 * @param i the instruction value 
 */ 
bool InstrucIter::isACondBranchInstruction()
{
  const instruction i = getInstruction();

  if((BFORM_OP(i) == BCop) && !BFORM_LK(i) &&
     !((BFORM_BO(i) & 0x10) && (BFORM_BO(i) & 0x4)))
    return true;
  return false;
}

bool InstrucIter::isACondBDZInstruction()
{
  const instruction i = getInstruction();

  if( BFORM_OP(i) == BCop  && !BFORM_LK(i) &&
      ( BFORM_BO(i) == 18 || BFORM_BO(i) == 19 ||
	BFORM_BO(i) == 26 || BFORM_BO(i) == 27 ) ) 
    return true;
  return false;
}

bool InstrucIter::isACondBDNInstruction()
{
  const instruction i = getInstruction();
  if( BFORM_OP(i) == BCop  && !BFORM_LK(i) &&
      ( BFORM_BO(i) == 16 || BFORM_BO(i) == 17 ||
	BFORM_BO(i) == 24 || BFORM_BO(i) == 25 ) ) 
    return true;
  return false;
}


/** is the instruction an unconditional branch instruction 
 * @param i the instruction value 
 */
bool InstrucIter::isAJumpInstruction()
{
  const instruction i = getInstruction();
  if((IFORM_OP(i) == Bop) && !IFORM_LK(i))
    return true;
  if((BFORM_OP(i) == BCop) && !BFORM_LK(i) &&
     (BFORM_BO(i) & 0x10) && (BFORM_BO(i) & 0x4))
    return true;
  return false;
}
/** is the instruction a call instruction 
 * @param i the instruction value 
 */
bool InstrucIter::isACallInstruction()
{

  //cerr << current << endl;
  const instruction i = getInstruction();
  //const instruction i = getInstruction();
#define CALLmatch 0x48000001 /* bl */
        
  // Only look for 'bl' instructions for now, although a branch
  // could be a call function, and it doesn't need to set the link
  // register if it is the last function call
  //cerr << currentAddreOPmask << endl;

  return (i.isInsnType(OPmask | LLmask, CALLmatch));
  /*
    const instruction i = getInstruction();
    if((*i).iform.lk && 
    (((*i).iform.op == Bop) || (BFORM_OP(i) == BCop) ||
    ((XLFORM_OP(i) == BCLRop) && 
    ((XLFORM_XO(i) == 16) || (XLFORM_XO(i) == 528))))){
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

  if (XLFORM_OP(i) == BCLRop && XLFORM_XO(i) == BCLRxop)
  {
    if (XLFORM_LK(i))
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
  if (XLFORM_OP(i) == BCLRop && XLFORM_XO(i) == BCCTRxop)
  {
    if (XLFORM_LK(i))
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

#define MK_LD1(bytes, i, imm, ra) (new BPatch_memoryAccess(new internal_instruction(i), current,true, false, (bytes), (imm), (ra), -1))
#define MK_SD1(bytes, i, imm, ra) (new BPatch_memoryAccess(new internal_instruction(i), current, false, true, (bytes), (imm), (ra), -1))

#define MK_LX1(bytes, i, ra, rb) (new BPatch_memoryAccess(new internal_instruction(i), current,true, false, (bytes), 0, (ra), (rb)))
#define MK_SX1(bytes, i, ra, rb) (new BPatch_memoryAccess(new internal_instruction(i), current,false, true, (bytes), 0, (ra), (rb)))

#define MK_LD(bytes, i) (MK_LD1((bytes), i, DFORM_SI(*i), (signed)DFORM_RA(*i)))
#define MK_SD(bytes, i) (MK_SD1((bytes), i, DFORM_SI(*i), (signed)DFORM_RA(*i)))

// VG(11/20/01): X-forms ignore ra if 0, but not rb...
#define MK_LX(bytes, i) (MK_LX1((bytes), i, (XFORM_RA(*i) ? (signed)XFORM_RA(*i) : -1), XFORM_RB(*i)))
#define MK_SX(bytes, i) (MK_SX1((bytes), i, (XFORM_RA(*i) ? (signed)XFORM_RA(*i) : -1), XFORM_RB(*i)))

#define MK_LDS(bytes, i) (MK_LD1((bytes), i, (DSFORM_DS(*i) << 2), (signed)DSFORM_RA(*i)))
#define MK_SDS(bytes, i) (MK_SD1((bytes), i, (DSFORM_DS(*i) << 2), (signed)DSFORM_RA(*i)))

#define MK_LI(bytes, i) (MK_LX1((bytes), i, XFORM_RA(*i), -1))
#define MK_SI(bytes, i) (MK_SX1((bytes), i, XFORM_RA(*i), -1))

struct opCodeInfo {
    static bool doneInit;
  unsigned int bytes; //: 4;
  unsigned int direc; //: 1; // 0 == load, 1 == store
public:
  opCodeInfo(unsigned b, unsigned d) : bytes(b), direc(d) {}
  opCodeInfo() : bytes(0), direc(0) {}
};

bool opCodeInfo::doneInit = false;
opCodeInfo *xopCodes[1024];

void initOpCodeInfo()
{
    opCodeInfo::doneInit = true;
  memset(xopCodes, sizeof(opCodeInfo *)*1024, 0);
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
  instruction *i = getInsnPtr();

  int op = DFORM_OP(*i);
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
	  return MK_LD((32 - DFORM_RT(*i))*4, i);
	}
	else {
	  logIS_A("IS_A: stm");
	  return MK_SD((32 - DFORM_RT(*i))*4, i);
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
      logIS_A("IS_A: ldx-lwa");
      b = DSFORM_XO(*i) < 2 ? 8 : 4;
      assert(DSFORM_XO(*i) < 3);
      return MK_LDS(b, i);
    }
    else if(op == STDop) {
      logIS_A("IS_A: std-stdu");
      assert(DSFORM_XO(*i) < 2);
      
      

      return MK_SDS(8, i);
    }
    else
      return BPatch_memoryAccess::none;
  }
  else if(op == LXop) { // X-forms
    unsigned int xop = XFORM_XO(*i);
    
    //logIS_A("XOP::%d\n", xop);
    if(!opCodeInfo::doneInit)
        initOpCodeInfo();

    opCodeInfo *oci = xopCodes[xop];

    if (oci == NULL)
      return BPatch_memoryAccess::none;
    else if(oci->bytes > 0)
      return oci->direc ? MK_SX(oci->bytes, i) : MK_LX(oci->bytes, i);
    else if(xop == LSIxop || xop == STSIxop) {
      b = XFORM_RB(*i) == 0 ? 32 : XFORM_RB(*i); 
      return oci->direc ? MK_SI(b, i) : MK_LI(b, i);
    }
    else if(xop == LSXxop || xop == STSXxop) {
      return new BPatch_memoryAccess(new internal_instruction(i), current,
				     oci->direc == 0, oci->direc == 1,
                                     (long)0, XFORM_RA(*i), XFORM_RB(*i),
                                     (long)0, POWER_XER2531, -1); // 9999 == XER_25:31
    }
  }
  free(i);
  return BPatch_memoryAccess::none;
}

/** function which returns the offset of control transfer instructions
 * @param i the instruction value 
 */
Address InstrucIter::getBranchTargetAddress(bool *isAbsolute)
{
  const instruction i = getInstruction();

  Address ret = 0;

#if defined(os_vxworks)
  if (relocationTarget(current, &ret))
      return ret;
#endif

  if((IFORM_OP(i) == Bop) || (BFORM_OP(i) == BCop)){
    int disp = 0;

    if(IFORM_OP(i) == Bop)
      disp = IFORM_LI(i);
    else if(BFORM_OP(i) == BCop)
      disp = BFORM_BD(i);
    disp <<= 2;

    if(IFORM_AA(i)) {
      ret = (Address)disp;
      if (isAbsolute) *isAbsolute = true;
    }
    else
      ret = (Address)(current+disp);
  }
  return (Address)ret;
}

bool InstrucIter::getMultipleJumpTargets(std::set<Address>& result)
{
  Address initialAddress = current;
  Address TOC_address = 0;
  unsigned TOC_register = 2;
  
  image* img = dynamic_cast<image*>(instructions_);
  process* proc = dynamic_cast<process*>(instructions_);
    
  if (img) { 
    TOC_address = img->getObject()->getTOCoffset();
  }
  else {
    pdvector<mapped_object *> m_objs = proc->mappedObjects();
    for (unsigned i = 0; i < m_objs.size(); i++) {
      void *ptr = m_objs[i]->getPtrToInstruction(current);
      if (ptr) {
	TOC_address = m_objs[i]->parse_img()->getObject()->getTOCoffset();
	TOC_address += m_objs[i]->dataBase();
	break;
      }
    }
  }

  instruction check;
    
  // If there are no prior instructions then we can't be looking at a
  // jump through a jump table.
  if( !hasPrev() ) {
    result.insert(initialAddress + instruction::size());
    setCurrentAddress(initialAddress);
    return false;
  }
    
  // Check if the previous instruction is a move to CTR or LR;
  // if it is, then this is the pattern we're familiar with.  The
  // register being moved into CTR or LR has the address to jump to.
  (*this)--;
  int jumpAddrReg;
  check = getInstruction();
  if (XFXFORM_OP(check) == STXop && XFXFORM_XO(check) == MTSPRxop &&
      (XFXFORM_SPR(check) == 0x100 || XFXFORM_SPR(check) == 0x120)) {
    jumpAddrReg = XFXFORM_RT(check);
    //fprintf(stderr, "Jump table candidate, jump target register is %d\n", jumpAddrReg);
  } else {
    result.insert(initialAddress + instruction::size());
    setCurrentAddress(initialAddress);
    return false;
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
    if (XOFORM_OP(check) == CAXop && XOFORM_XO(check) == CAXxop &&
	XOFORM_RT(check) == (unsigned)jumpAddrReg) {
      tableIsRelative = true; 
      //fprintf(stderr, "table is relative...\n");
    }
    else
      (*this)++;
  }

  setCurrentAddress(initialAddress);    
    
  Address jumpStartAddress = 0;
  Address adjustEntry = 0;
  Address tableStartAddress = 0;
  if(!TOC_address)
  {
    while( hasPrev() ){
      check = getInstruction();
      if(DFORM_OP(check) == CALop) {
         unsigned jumpAddressReg = DFORM_RA(check);
	tableStartAddress = DFORM_SI(check);
	(*this)--;
	check = getInstruction();
	if(DFORM_OP(check) == CAUop && DFORM_RT(check) == jumpAddressReg)
	{
	  tableStartAddress += (DFORM_SI(check) * 0x10000) & 0xFFFF0000;
	  //fprintf(stderr, "Found CAU and CAL at %lx, ", current);
	  //fprintf(stderr, "tableStartAddress is %lx, high half is %lx\n", tableStartAddress, DFORM_SI(check));
	  break;
	}
	else
	{
	  tableStartAddress = 0;
	}
      }
      else if(DFORM_OP(check) == CAUop) {
         unsigned jumpAddressReg = DFORM_RT(check);
	tableStartAddress = (DFORM_SI(check) * 0x10000) & 0xFFFF0000;
	(*this)--;
	check = getInstruction();
	if(DFORM_OP(check) == CALop && DFORM_RA(check) == jumpAddressReg)
	{
	  tableStartAddress += DFORM_SI(check);
	  //fprintf(stderr, "Found CAU and CAL at %x, ", current);
	  //fprintf(stderr, "tableStartAddress is %x, low half is %x\n", tableStartAddress, DFORM_SI(check));
	  break;
	}
	else
	{
	  tableStartAddress = 0;
	}
      }
      (*this)--;
    }
    jumpStartAddress = tableStartAddress;
  }
  else if (tableIsRelative) {
    while( hasPrev() ){
      check = getInstruction();
      if((DFORM_OP(check) == Lop) && (DFORM_RA(check) == TOC_register)) { // 32-bit form
	//fprintf(stderr, "Jump offset from TOC: %d\n", DFORM_SI(check));
	jumpStartAddress = 
	(Address)(TOC_address + DFORM_SI(check));
	break;
      }

      if ((DSFORM_OP(check) == LDop) && (DSFORM_RA(check) == TOC_register)) { // 64-bit form
	//fprintf(stderr, "Jump offset from TOC: %d\n", (DSFORM_DS(check) << 2));
	jumpStartAddress = 
	(Address)(TOC_address + (DSFORM_DS(check) << 2));
	break;
      }
      (*this)--;
    }
    // Anyone know what this does?
    (*this)--;
    check = getInstruction();
    if((DFORM_OP(check) == Lop)) { // 32-bit form
      adjustEntry = DFORM_SI(check);
      //fprintf(stderr, "adjustEntry is 0x%lx (%d)\n",
      //adjustEntry, adjustEntry);
    }
       
    while(hasPrev()){
      instruction check = getInstruction();
      if((DFORM_OP(check) == Lop) && (DFORM_RA(check) == TOC_register)){ // 32-bit form
	//fprintf(stderr, "Table offset from TOC: %d\n", DFORM_SI(check));
	tableStartAddress = 
	(Address)(TOC_address + DFORM_SI(check));
	//fprintf(stderr, "tableStartAddr is 0x%lx\n", tableStartAddress);
	break;
      }
      if((DSFORM_OP(check) == LDop) && (DSFORM_RA(check) == TOC_register)){ // 64-bit form
	//fprintf(stderr, "Table offset from TOC: %d\n", (DSFORM_DS(check) << 2));
	tableStartAddress = 
	(Address)(TOC_address + (DSFORM_DS(check) << 2));
	//fprintf(stderr, "tableStartAddr is 0x%lx\n", tableStartAddress);
	break;
      }
      (*this)--;
    }
  } else {
    bool foundAdjustEntry = false;
    while( hasPrev() ){
      check = getInstruction();
      if(DFORM_OP(check) == CALop &&
	 DFORM_RT(check) == (unsigned)jumpAddrReg &&
	 !foundAdjustEntry){
	foundAdjustEntry = true;
	adjustEntry = DFORM_SI(check);
	jumpAddrReg = DFORM_RA(check);
      } else if(DFORM_OP(check) == Lop &&
		DFORM_RA(check) == TOC_register &&
		DFORM_RT(check) == (unsigned)jumpAddrReg){
	tableStartAddress = 
	(Address)(TOC_address + DFORM_SI(check));
	break;
      }
      (*this)--;
    }
  }

  // We could also set this = jumpStartAddress...
  if (tableStartAddress == 0)  {
    //fprintf(stderr, "No table start addr, returning\n"); 
    setCurrentAddress(initialAddress);
    return false;
  }

  setCurrentAddress(initialAddress);
  int maxSwitch = 0;
    
  while( hasPrev() ){
    instruction check = getInstruction();
    if((BFORM_OP(check) == BCop) && 
       !BFORM_AA(check) && !BFORM_LK(check)){
      (*this)--;
      check = getInstruction();
      if(10 != DFORM_OP(check))
	break;
      maxSwitch = DFORM_SI(check) + 1;
      break;
    }
    (*this)--;
  }
//fprintf(stderr, "After checking: max switch %d\n", maxSwitch);
  if(!maxSwitch){
    result.insert(initialAddress + instruction::size());
    //fprintf(stderr, "No maximum, returning\n");
    setCurrentAddress(initialAddress);
    return false;
  }

  Address jumpStart = 0;
  Address tableStart = 0;
  bool is64 = (instructions_->getAddressWidth() == 8);

  if(TOC_address)
  {
    if (img) {
      if (tableIsRelative) {
	void *jumpStartPtr = img->getPtrToData(jumpStartAddress);
	//fprintf(stderr, "jumpStartPtr (0x%lx) = %p\n", jumpStartAddress, jumpStartPtr);
	if (jumpStartPtr)
	  jumpStart = (is64
		       ? *((Address  *)jumpStartPtr)
		       : *((uint32_t *)jumpStartPtr));
	//fprintf(stderr, "jumpStart 0x%lx, initialAddr 0x%lx\n",
	//	jumpStart, initialAddress);
	if (jumpStartPtr == NULL) {
	  setCurrentAddress(initialAddress);
	  return false;
	}
      }
      void *tableStartPtr = img->getPtrToData(tableStartAddress);
      //fprintf(stderr, "tableStartPtr (0x%lx) = %p\n", tableStartAddress, tableStartPtr);
	tableStart = *((Address *)tableStartPtr);
      if (tableStartPtr)
	tableStart = (is64
		      ? *((Address  *)tableStartPtr)
		      : *((uint32_t *)tableStartPtr));
      else {
	setCurrentAddress(initialAddress);                    
	return false;
      }
      //fprintf(stderr, "... tableStart 0x%lx\n", tableStart);
      
      // We're getting an absolute out of the TOC. Figure out
      // whether we're in code or data.
      const fileDescriptor &desc = img->desc();
      Address textStart = desc.code();
      Address dataStart = desc.data();
      
      // I think this is valid on ppc64 linux.  dataStart and codeStart can be 0.
      // assert(jumpStart < dataStart);
      
      bool tableData = false;
      
      if (proc) {
	if (tableStart > dataStart) {
	  tableData = true;
	  tableStart -= dataStart;
	  //fprintf(stderr, "Table in data, offset 0x%lx\n", tableStart);
	}
	else {
	  tableData = false;
	  tableStart -= textStart;
	  //fprintf(stderr, "Table in text, offset 0x%lx\n", tableStart);
	}
      }
      else {
	//fprintf(stderr, "*** Table neither in data nor in text, confused\n");
	//fprintf(stderr, "tableStart 0x%lx, codeOff 0x%lx, codeLen 0x%lx, dataOff 0x%lx, dataLen 0x%lx\n",
	//tableStart, img->codeOffset(), img->codeLength(),
	//img->dataOffset(), img->dataLength());
	
	// Not sure what to do, really. We don't know where it is, and I'm
	// not sure how to check.
      }
      
      for(int i=0;i<maxSwitch;i++){                    
	Address tableEntry = adjustEntry + tableStart + (i * instruction::size());
	//fprintf(stderr, "Table entry at 0x%lx\n", tableEntry);
	if (instructions_->isValidAddress(tableEntry)) {
	  int jumpOffset;
	  if (tableData) {
	  jumpOffset = *((int *)img->getPtrToData(tableEntry));
	  }
	  else
	  jumpOffset = *((int *)instructions_->getPtrToInstruction(tableEntry));
	  
	  //fprintf(stderr, "jumpOffset 0x%lx\n", jumpOffset);
	  Address res = (Address)(jumpStart + jumpOffset);

	  if (img->isCode(res))
              result.insert((Address)(jumpStart+jumpOffset));
	  //fprintf(stderr, "Entry of 0x%lx\n", (Address)(jumpStart + jumpOffset));
	}
	else {
	  //fprintf(stderr, "Address not valid!\n");
	}
      }
    }
    else {
      void *ptr;
      if (tableIsRelative) {
	ptr = instructions_->getPtrToInstruction(jumpStartAddress);
	assert(ptr);
	jumpStart = (is64
		     ? *((Address  *)ptr)
		     : *((uint32_t *)ptr));

      }
      ptr = NULL;
      ptr = instructions_->getPtrToInstruction(tableStartAddress);
      assert(ptr);
      tableStart = (is64
		    ? *((Address  *)ptr)
		    : *((uint32_t *)ptr));

      for(int i=0;i<maxSwitch;i++){
      Address tableEntry = adjustEntry + tableStart + (i * instruction::size());
      ptr = instructions_->getPtrToInstruction(tableEntry);
      assert(ptr);
      int jumpOffset = *((int *)ptr);
      result.insert((Address)(jumpStart+jumpOffset));
      }
    }    
  }
  // No TOC, so we're on Power32 Linux.  Do the ELF thing.
  else
  {
    int entriesAdded = 0;
    for(int i = 0; i < maxSwitch; i++)
    {
      void* ptr = NULL;
      Address tableEntry = tableStartAddress + i*instruction::size();
      if(instructions_->isValidAddress(tableEntry))
      {
	ptr = instructions_->getPtrToInstruction(tableEntry);
      }
      if(ptr)
      {
	int jumpOffset = *((int *)ptr);
        result.insert((Address)(jumpStartAddress+jumpOffset));
	++entriesAdded;
      }
    }
    if(!entriesAdded)
    {
      setCurrentAddress(initialAddress);
      return false;
    }
    //fprintf(stderr, "Found %d entries in jump table, returning success\n", entriesAdded);
  }

  // Sanity check entries in res
  for (std::set<Address>::iterator iter = result.begin();
       iter != result.end(); iter++) {
      if ((*iter) % 4) {
          parsing_printf("Warning: found unaligned jump table destination 0x%lx for jump at 0x%lx, disregarding table\n",
                         *iter, initialAddress);
          return false;
      }
  }

  
  setCurrentAddress(initialAddress);
  return true;
}

bool InstrucIter::delayInstructionSupported()
{
  return false;
}

Address InstrucIter::peekPrev()
{
  Address ret = current-instruction::size();
  return ret;
}

Address InstrucIter::peekNext()
{
  Address ret = current + instruction::size();
  return ret;
}

void InstrucIter::setCurrentAddress(Address addr)
{
  current = addr;
  initializeInsn();
}

instruction InstrucIter::getInstruction()
{
  return insn;
}

instruction *InstrucIter::getInsnPtr() {
  instruction *insnPtr = new instruction(insn);
  return insnPtr;
}

// Stack frame creation on Power is easy:
// "stu r1, -120(r1)"
// That being, "store the current value of r1 at *r1 and subtract 120 from r1"
bool InstrucIter::isStackFramePreamble() {
  // We check the entire block. Don't know when it ends,
  // so go until we hit a jump.
  bool foundStackPreamble = false;
  while (instPtr != NULL &&
	 !isAReturnInstruction() &&
	 !isACondBranchInstruction() &&
	 !isACallInstruction() &&
	 !isADynamicCallInstruction() &&
	 !isAJumpInstruction() &&
	 insn.valid()) {

    // We need to bit twiddle.
    if ((DFORM_OP(insn) == STUop) &&
	(DFORM_RT(insn) == 1) &&
	(DFORM_RA(insn) == 1)) {
      foundStackPreamble = true;
      break;
    }
    (*this)++;
  }

  return foundStackPreamble;
}

// If we save the return addr, we're not a leaf function.
// Doesn't matter if the function actually makes calls, but
// where to find the return addr.
// Should be "isReturnAddrSave", but that would recompiling
// _everything_.
// As above; looking for "mflr r0", which should be followed
// by a save.

bool InstrucIter::isReturnValueSave() {
  Address currentAddr = current;
  // We check the entire block. Don't know when it ends,
  // so go until we hit a jump.
  bool foundMFLR = false;
  bool foundR0Save = false;
  while (instPtr != NULL &&
	 !isAReturnInstruction() &&
	 !isACondBranchInstruction() &&
	 !isACallInstruction() &&
	 !isADynamicCallInstruction() &&
	 !isAJumpInstruction() &&
	 insn.valid()) {

    // We need to bit twiddle.
    if (insn.asInt() == MFLR0raw) {
      foundMFLR = true;
    }

    if ((DFORM_OP(insn) == STop) &&
	(DFORM_RT(insn) == 0) &&
	(DFORM_RA(insn) == 1)) {
      foundR0Save = true;
      break;
    }
    (*this)++;
  }
  setCurrentAddress(currentAddr);
  return (foundR0Save && foundMFLR);
}

bool InstrucIter::isFrameSetup()
{
  return false;
}

bool InstrucIter::isFramePush()
{
  return false;
}

bool InstrucIter::isALeaveInstruction()
{
  return false;
}

bool InstrucIter::isAnInterruptInstruction()
{
    // TODO: not implemented
    return false;
}

bool InstrucIter::isAnAbortInstruction()
{
  assert(instPtr);
  return !insn.valid();
}


bool InstrucIter::isAnAllocInstruction()
{
  return false;
}

bool InstrucIter::isDelaySlot()
{
  return false;
}

bool InstrucIter::isInterModuleCallSnippet(Address &info) {
  Address currentAddr = current;

  // Template for linkage functions:
  // l      r12,<offset>(r2) // address of call into R12
  // st     r2,20(r1)        // Store old TOC on the stack
  // l      r0,0(r12)        // Address of callee func
  // l      r2,4(r12)        // callee TOC
  // mtctr  0                // We keep the LR static, use the CTR
  // bctr                    // non-saving branch to CTR
    
  bool retval = true;

  for (unsigned i = 0; i < 6; i++) {
    parsing_printf("Checking for linkage at addr 0x%lx\n", current);
    if (!retval) break;

    if (!hasMore()) {
      parsing_printf("failed hasMore check %d: %p, %d, 0x%lx <= 0x%lx <= 0x%lx",
		     i, instPtr, range, base, current, base+range);
      retval = false;
      break;
    }
    instruction scratch = getInstruction();
    parsing_printf("Slot %d: raw bytes 0x%x\n",
		   i, scratch.asInt());
    switch(i) {
    case 0: // l r12, <offset> (r2)
      if ((DFORM_OP(scratch) != Lop) ||
	  (DFORM_RT(scratch) != 12) ||
	  (DFORM_RA(scratch) != 2))  {
	parsing_printf("Insn 0 not load\n");
	retval = false;
      }
      info = DFORM_SI(scratch);
      break;
    case 1: // st     r2,20(r1)
      break; // Don't check this one. 
    case 2: // l      r0,0(r12) 
      if ((DFORM_OP(scratch) != Lop) ||
	  (DFORM_RT(scratch) != 0) ||
	  ((DFORM_RA(scratch) != 1) && (DFORM_RA(scratch) != 12))) {
	parsing_printf("Insn 2 not load\n");
	retval = false;
      }
      break;
    case 3: // l      r2,4(r12)
      break;
    case 4: // mtctr  0  
      break; // Could check individually...
    case 5:// bctr 
      if ((XLFORM_OP(scratch) != BCLRop) ||
	  (XLFORM_XO(scratch) != BCCTRxop)) {
	parsing_printf("Insn 2 not BCTR\n");
	retval = false;
      }
      break;
    default:
      assert(0);
      break;
    }
    (*this)++;
  }
  setCurrentAddress(currentAddr);
  return retval;

}

void InstrucIter::getAllRegistersUsedAndDefined(std::set<Register> &used,
                                                std::set<Register> &defined) {
    // Register numbers are by the numbering in registerSpace::powerRegisters_t...

    if (isA_MX_Instruction()) {
        // Used and defined, by the comments.
        // TODO: MX register, not MQ... or is it MX?
        used.insert(registerSpace::mq);
        defined.insert(registerSpace::mq);
    }

    // The instrucIter calls tell us if the particular encoding in
    // an instruction is on or off, not which register is used...
    if (isA_RT_ReadInstruction()) {
        // Return of getRTValue is a register number from 0...
        Register reg = getRTValue() + registerSpace::r0;
        used.insert(reg);
    }
    if (isA_RA_ReadInstruction()) {
        used.insert(getRAValue() + registerSpace::r0);
    }
    if (isA_RB_ReadInstruction()) {
        used.insert(getRBValue() + registerSpace::r0);
    }
    if (isA_MRT_ReadInstruction()) {
        // Multiple store - mark all GPRs as used.
        // If we ever use more than r12, increase this.
        // Technically, there's an encoded lower bound... but over-approximate.
        for (unsigned i = registerSpace::r0; i <= registerSpace::r12; i++) {
            used.insert(i);
        }
    }


    if (isA_RT_WriteInstruction()) {
        // Return of getRTValue is a register number from 0...
        Register reg = getRTValue() + registerSpace::r0;
        defined.insert(reg);
    }
    if (isA_RA_WriteInstruction()) {
        defined.insert(getRAValue() + registerSpace::r0);
    }
    if (isA_MRT_WriteInstruction()) {
        // Multiple store - mark all GPRs as used.
        // If we ever use more than r12, increase this.
        // Technically, there's an encoded lower bound... but over-approximate.
        for (unsigned i = registerSpace::r0; i <= registerSpace::r12; i++) {
            used.insert(i);
        }
    }

    //////////////////////////////////
    // FPRs
    //////////////////////////////////

    if (isA_FRT_ReadInstruction()) {
        used.insert(getFRTValue() + registerSpace::fpr0);
    }
    if (isA_FRA_ReadInstruction()) {
        used.insert(getFRAValue() + registerSpace::fpr0);
    }
    if (isA_FRB_ReadInstruction()) {
        used.insert(getFRBValue() + registerSpace::fpr0);
    }
    if (isA_FRC_ReadInstruction()) {
        used.insert(getFRCValue() + registerSpace::fpr0);
    }

    if (isA_FRT_WriteInstruction()) {
        used.insert(getFRTValue() + registerSpace::fpr0);
    }
    if (isA_FRA_WriteInstruction()) {
        used.insert(getFRAValue() + registerSpace::fpr0);
    }

    // SPRs

    std::set<Register> sprsUsed;
    std::set<Register> sprsDefined;

    if (definesSPR(sprsDefined)) {
        std::set<Register> tmp; 
        std::set_union(defined.begin(),
                       defined.end(),
                       sprsDefined.begin(),
                       sprsDefined.end(),
                       insert_iterator<std::set<Register> >(tmp, tmp.begin()) );
        defined = tmp;
    }
    if (usesSPR(sprsUsed)) {
        std::set<Register> tmp;
        std::set_union(used.begin(),
                       used.end(),
                       sprsUsed.begin(),
                       sprsUsed.end(),
                       insert_iterator<std::set<Register> >(tmp, tmp.begin()));
        used = tmp;
    }
}

bool InstrucIter::isSyscall() {
    return (IFORM_OP(insn) == SVCop);
}

bool InstrucIter::definesSPR(std::set<Register> &sprs) {
    if ((XFXFORM_OP(insn) == MTSPRop) && (XFXFORM_XO(insn) == MTSPRxop)) {
        unsigned spr = XFORM_RA(insn) + (XFORM_RB(insn) << 5);
        switch (spr) {
        case SPR_XER:
            sprs.insert(registerSpace::xer);
            break;
        case SPR_LR:
            sprs.insert(registerSpace::lr);
            break;
        case SPR_CTR:
            sprs.insert(registerSpace::ctr);
            break;
        case SPR_MQ:
            sprs.insert(registerSpace::mq);
            break;
        default:
            break;
        }
        return true;
    }

    // Linking branches define the LR
    if (((XLFORM_OP(insn) == BCCTRop) && (XLFORM_XO(insn) == BCCTRxop) && (XLFORM_LK(insn) == 1)) ||
        ((XLFORM_OP(insn) == BCLRop) && (XLFORM_XO(insn) == BCLRop) && (XLFORM_LK(insn) == 1)) ||
        ((IFORM_OP(insn) == Bop) && (IFORM_LK(insn) == 1)) ||
        ((BFORM_OP(insn) == BCop) && (BFORM_LK(insn) == 1))) {
        sprs.insert(registerSpace::lr);
    }

    return false;
}


bool InstrucIter::usesSPR(std::set<Register> &sprs) {
    if ((XFXFORM_OP(insn) == MFSPRop) && (XFXFORM_XO(insn) == MFSPRxop)) {
        unsigned spr = XFORM_RA(insn) + (XFORM_RB(insn) << 5);
        switch (spr) {
        case SPR_XER:
            sprs.insert(registerSpace::xer);
            break;
        case SPR_LR:
            sprs.insert(registerSpace::lr);
            break;
        case SPR_CTR:
            sprs.insert(registerSpace::ctr);
            break;
        case SPR_MQ:
            sprs.insert(registerSpace::mq);
            break;
        default:
            break;
        }
        return true;
    }

    // Branches to SPRs
    if ((XLFORM_OP(insn) == BCCTRop) && (XLFORM_XO(insn) == BCCTRxop)) {
        sprs.insert(registerSpace::ctr);
        return true;
    }
    if ((XLFORM_OP(insn) == BCLRop) && (XLFORM_XO(insn) == BCLRxop)) {
        sprs.insert(registerSpace::lr);
        return true;
    }
    
    return false;
}

bool InstrucIter::isANopInstruction()
{
   return false;
}

bool InstrucIter::isTailCall(Function * /* f */)
{
    return false;
}

bool InstrucIter::isIndirectTailCall(Function * /* f */)
{
    return false;
}

bool InstrucIter::isRealCall(CodeObject * obj, 
    bool &validTarget,
    bool & /*simulateJump*/)
{
    Address callTarget;

    if(isADynamicCallInstruction())
        return true;

    callTarget = getBranchTargetAddress();
    validTarget = obj->isValidAddress(callTarget);
    return validTarget;
}
