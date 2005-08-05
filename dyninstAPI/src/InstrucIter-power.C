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
  printf("OpCode num %d",(*i).xlform.op);
}

/* Returns the value of the RT Register */
unsigned InstrucIter::getRTValue(void)
{
  const instruction i = getInstruction();
  return (*i).xform.rt;  
}

/* Returns the value of the RA Register */
unsigned InstrucIter::getRAValue(void)
{
  const instruction i = getInstruction();
  return (*i).xform.ra;
}

unsigned InstrucIter::getRBValue(void)
{
  const instruction i = getInstruction();
  return (*i).xform.rb;
}

/* This function returns true if the instruction affects the RD Register */
bool InstrucIter::isA_RT_WriteInstruction()
{
  const instruction i = getInstruction();
  if (
      /* XO Form */
      ((*i).xoform.op == XO_EXTENDEDop
       && ( (*i).xoform.xo == CAXxop || (*i).xoform.xo == Axop 
	    || (*i).xoform.xo == AExop || (*i).xoform.xo == NABSxop 
	    || (*i).xoform.xo == ANDCxop || (*i).xoform.xo == DIVWxop
	    || (*i).xoform.xo == SUBFxop || (*i).xoform.xo == SFxop
	    || (*i).xoform.xo == SFExop || (*i).xoform.xo == SFMExop 
	    || (*i).xoform.xo == SFZExop || (*i).xoform.xo == MULHWxop 
	    || (*i).xoform.xo == DIVWUxop || (*i).xoform.xo == MULSxop 
	    || (*i).xoform.xo == NEGxop || (*i).xoform.xo == ABSxop 
	    || (*i).xoform.xo == DIVxop || (*i).xoform.xo == DIVSxop
	    || (*i).xoform.xo == DOZxop || (*i).xoform.xo == MULxop 
	    || (*i).xoform.xo == SFExop || (*i).xoform.xo == SFMExop 
	    || (*i).xoform.xo == SFZExop || (*i).xoform.xo == MULHWUxop )) ||
	
	/* D Form */
	((*i).dform.op == MULIop || (*i).dform.op == SFIop || (*i).dform.op == DOZIop   ) || 
	((*i).dform.op >= SIop && (*i).dform.op  <= CAUop) || 
	((*i).dform.op >= Lop && (*i).dform.op <= LBZUop) || 
	((*i).dform.op >= LHZop && (*i).dform.op <= LHAUop) || 
	
	/* X Form */
      ((*i).xform.op == X_EXTENDEDop 
       && ( (*i).xform.xo == LBZUXxop || (*i).xform.xo == LBZXxop || 
	    (*i).xform.xo == ANDxop || (*i).xform.xo == LHAUXxop  || 
	    (*i).xform.xo == LHAXxop || (*i).xform.xo == LHBRXxop ||
	    (*i).xform.xo == LHZUXxop  || (*i).xform.xo == LHZXxop || 
	    (*i).xform.xo == LWARXxop || (*i).xform.xo == LWBRXxop  || 
	    (*i).xform.xo == LUXxop || (*i).xform.xo == LXxop ||
	    (*i).xform.xo == MFCRxop  ||  
	    (*i).xform.xo == MFMSRxop || (*i).xform.xo == MFSPRxop  || 
	    (*i).xform.xo == CLCSxop  || (*i).xform.xo == MFSRxop || 
	    (*i).xform.xo == MFSRIxop ||
	    (*i).xform.xo == MFSRINxop  || (*i).xform.xo == RACxop)))
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
      ((*i).aform.op == A_FP_EXTENDEDop2 
       && ( (*i).aform.xo == FAxop || (*i).aform.xo == FDxop || 
	    (*i).aform.xo == FMxop || (*i).aform.xo == FMAxop  || 
	    (*i).aform.xo == FMSxop || (*i).aform.xo == FNMAxop ||
	    (*i).aform.xo == FNMSxop || (*i).aform.xo == FSQRTxop || 
	    (*i).aform.xo == FSxop )) ||  

      ((*i).aform.op == A_FP_EXTENDEDop1
       && ( (*i).aform.xo == FDIVSxop || (*i).aform.xo == FMADDSxop || 
	    (*i).aform.xo == FMULSxop || (*i).aform.xo == FMSUBSxop  || 
	    (*i).aform.xo == FNMSUBSxop || (*i).aform.xo == FNMADDSxop ||
	    (*i).aform.xo == FSUBSxop )) ||  
      /* X Form */
      ((*i).xform.op == X_FP_EXTENDEDop 
       && ( (*i).xform.xo == MFFSxop || (*i).xform.xo == FABSxop ||
	    (*i).xform.xo == FCIRZxop  || (*i).xform.xo == FMRxop || 
	    (*i).xform.xo == FNABSxop || (*i).xform.xo == FCIRxop ||
	    (*i).xform.xo == FNEGxop || (*i).xform.xo == FRSPxop )) ||

      ((*i).xform.op == X_EXTENDEDop
       && ( (*i).xform.xo == LFDUXxop  || (*i).xform.xo == LFDXxop || 
	    (*i).xform.xo == LFSUXxop || (*i).xform.xo == LFSXxop )))
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
      ((*i).dform.op >= ORILop && (*i).dform.op  <= ANDIUop) || 
      ((*i).dform.op >= STop && (*i).dform.op  <= STBUop) ||
      (*i).dform.op == STMop ||
      ((*i).mform.op >= RLIMIop && (*i).mform.op <= RLNMop) ||

      /*  XFX Form */
      ((*i).xfxform.op == X_EXTENDEDop && (*i).xfxform.xo == MTCRFxop ) ||
      
       /* X Form */
      ((*i).xfxform.op == X_EXTENDEDop
       && ( (*i).xform.xo == SLxop || (*i).xform.xo == CNTLZxop ||
	    (*i).xform.xo == ANDxop  || (*i).xform.xo == MASKGxop || 
	    (*i).xform.xo == ANDCxop || (*i).xform.xo == EXTSxop ||
	    (*i).xform.xo == NORxop  || (*i).xform.xo == STWCXxop ||
	    (*i).xform.xo == STXxop  || (*i).xform.xo == SLQxop || 
	    (*i).xform.xo == SLExop ||
	    (*i).xform.xo == STUXxop  || (*i).xform.xo == SLIQxop || 
	    (*i).xform.xo == STBXxop ||
	    (*i).xform.xo == SLLQxop  || (*i).xform.xo == SLEQxop || 
	    (*i).xform.xo == STBUXxop ||
	    (*i).xform.xo == SLLIQxop  || (*i).xform.xo == EQVxop || 
	    (*i).xform.xo == XORxop ||
	    (*i).xform.xo == ORCxop  || (*i).xform.xo == ORxop ||
	    (*i).xform.xo == MTSPRxop  || (*i).xform.xo == NANDxop || 
	    (*i).xform.xo == SRxop ||
	    (*i).xform.xo == RRIBxop  || (*i).xform.xo == MASKIRxop || 
	    (*i).xform.xo == STSXxop ||
	    (*i).xform.xo == STBRXxop  || (*i).xform.xo == SRQxop || 
	    (*i).xform.xo == SRExop ||
	    (*i).xform.xo == SRIQxop  || (*i).xform.xo == STSIxop || 
	    (*i).xform.xo == SRLQxop ||
	    (*i).xform.xo == SREQxop  || (*i).xform.xo == SRLIQxop ||
	    (*i).xform.xo == SRAxop ||
	    (*i).xform.xo == SRAIxop  || (*i).xform.xo == SRAQxop || 
	    (*i).xform.xo == SREAxop ||
	    (*i).xform.xo == EXTSxop || (*i).xform.xo == SRAIQxop || 
	    (*i).xform.xo == EXTSBxop )))
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
      ((*i).dform.op == STHop || (*i).dform.op == STHUop ) || 
      ((*i).dform.op >= STFSop && (*i).dform.op  <= STFDUop) ||
      /* X Form */
      ((*i).xform.op == X_EXTENDEDop 
       && ( (*i).xform.xo == STFSXxop || (*i).xform.xo == STFSUXxop  || 
	    (*i).xform.xo == STFDXxop || (*i).xform.xo == STFDUXxop ||  
	    (*i).xform.xo == STFIWXxop )))
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
      ((*i).xoform.op == XO_EXTENDEDop 
       && ( (*i).xoform.xo == SFxop || (*i).xoform.xo == Axop || (*i).xoform.xo == MULHWUxop ||
	    (*i).xoform.xo == SUBFxop  || (*i).xoform.xo == MULHWxop || (*i).xoform.xo == NEGxop ||
	    (*i).xoform.xo == MULxop || (*i).xoform.xo == SFExop || (*i).xoform.xo == AExop || 
	    (*i).xoform.xo == SFZExop || (*i).xoform.xo == AZExop || (*i).xoform.xo == SFMExop ||  
	    (*i).xoform.xo == AMExop || (*i).xoform.xo == DOZxop || (*i).xoform.xo == CAXxop ||
	    (*i).xoform.xo == DIVxop || (*i).xoform.xo == ABSxop || (*i).xoform.xo == DIVSxop ||
	    (*i).xoform.xo == DIVWUxop || (*i).xoform.xo == NABSxop || (*i).xoform.xo == DIVWxop)) ||
      /* D Form */
      ((*i).dform.op == TIop || (*i).dform.op == MULIop || 
       (*i).dform.op == SFIop || (*i).dform.op == DOZIop   ) || 
      ((*i).dform.op >= CMPLIop && (*i).dform.op  <= CAUop) ||
      ((*i).dform.op >= Lop && (*i).dform.op <= STFDUop) ||
      
      /* X Form */
      ((*i).xform.op == X_EXTENDEDop
       && ( (*i).xform.xo == CMPxop || (*i).xform.xo == Txop ||
	    (*i).xform.xo == LWARXxop  || (*i).xform.xo == LXxop || (*i).xform.xo == CMPLxop ||
	    (*i).xform.xo == DCBSxop  || (*i).xform.xo == LUXxop || (*i).xform.xo == DCBFxop ||
	    (*i).xform.xo == LBZXxop  || (*i).xform.xo == CLFxop || (*i).xform.xo == LBZUXxop ||
	    (*i).xform.xo == STWCXxop  || (*i).xform.xo == STXxop || (*i).xform.xo == STUXxop ||
	    (*i).xform.xo == STBXxop  || (*i).xform.xo == DCBTSTxop || (*i).xform.xo == STBUXxop ||
	    (*i).xform.xo == LSCBXxop  || (*i).xform.xo == DCBTxop || (*i).xform.xo == LHZXxop ||
	    (*i).xform.xo == TLBIxop  || (*i).xform.xo == DIVxop || (*i).xform.xo == LHAXxop ||
	    (*i).xform.xo == LHAUXxop  || (*i).xform.xo == STHXxop || (*i).xform.xo == STHUXxop ||
	    (*i).xform.xo == DCBIxop || (*i).xform.xo == CLIxop || (*i).xform.xo == CLCSxop ||
	    (*i).xform.xo == LSXxop  || (*i).xform.xo == LWBRXxop || (*i).xform.xo == LFSXxop ||
	    (*i).xform.xo == LFSUXxop  || (*i).xform.xo == LSIxop || (*i).xform.xo == LFDXxop ||
	    (*i).xform.xo == DCLSTxop  || (*i).xform.xo == LFDUXxop || (*i).xform.xo == STSXxop ||
	    (*i).xform.xo == STBRXxop  || (*i).xform.xo == STFSXxop || (*i).xform.xo == STFSUXxop ||
	    (*i).xform.xo == STSIxop  || (*i).xform.xo == STFDXxop || (*i).xform.xo == STFDUXxop ||
	    (*i).xform.xo == LHBRXxop  || (*i).xform.xo == RACxop ||
	    (*i).xform.xo == STHBRXxop || (*i).xform.xo == ICBIxop || 
	    (*i).xform.xo == STFIWXxop || (*i).xform.xo == DCLZxop )))
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
      ((*i).xoform.op == XO_EXTENDEDop
       && ( (*i).xoform.xo == SFxop || (*i).xoform.xo == Axop || (*i).xoform.xo == MULHWUxop ||
	    (*i).xoform.xo == SUBFxop  || (*i).xoform.xo == MULHWxop || 
	    (*i).xoform.xo == MULxop || (*i).xoform.xo == SFExop 
	    || (*i).xoform.xo == AExop || 
	    (*i).xoform.xo == DOZxop || (*i).xoform.xo == CAXxop ||
	    (*i).xoform.xo == DIVxop || (*i).xoform.xo == DIVSxop ||
	    (*i).xoform.xo == DIVWUxop || (*i).xoform.xo == DIVWxop)) ||
      /* M Form */
      ((*i).mform.op == RLMIop || (*i).mform.op == RLNMop) ||
     
      /* X Form */
      ((*i).xform.op == X_EXTENDEDop
       && ( (*i).xform.xo == CMPxop || (*i).xform.xo == Txop ||
	    (*i).xform.xo == LWARXxop  || (*i).xform.xo == LXxop || (*i).xform.xo == SLxop ||
	    (*i).xform.xo == ANDxop  || (*i).xform.xo == MASKGxop || (*i).xform.xo == CMPLxop ||
	    (*i).xform.xo == DCBSxop  || (*i).xform.xo == LUXxop || (*i).xform.xo == ANDCxop ||
	    (*i).xform.xo == DCBFxop  || (*i).xform.xo == LBZXxop || (*i).xform.xo == CLFxop ||
	    (*i).xform.xo == LBZUXxop  || (*i).xform.xo == NORxop || (*i).xform.xo == STWCXxop ||
	    (*i).xform.xo == STXxop  || (*i).xform.xo == SLQxop || (*i).xform.xo == SLExop ||
	    (*i).xform.xo == STUXxop  || (*i).xform.xo == STBXxop || (*i).xform.xo == SLLQxop ||
	    (*i).xform.xo == SLEQxop  || (*i).xform.xo == DCBTSTxop || (*i).xform.xo == STBUXxop ||
	    (*i).xform.xo == LSCBXxop  || (*i).xform.xo == DCBTxop || (*i).xform.xo == LHZXxop ||
	    (*i).xform.xo == EQVxop  || (*i).xform.xo == TLBIxop || (*i).xform.xo == XORxop ||
	    (*i).xform.xo == DIVxop  || (*i).xform.xo == LHAXxop || (*i).xform.xo == LHAUXxop ||
	    (*i).xform.xo == STHXxop  || (*i).xform.xo == ORCxop || (*i).xform.xo == STHUXxop ||
	    (*i).xform.xo == ORxop || (*i).xform.xo == DCBIxop || (*i).xform.xo == NANDxop ||
	    (*i).xform.xo == CLIxop  || (*i).xform.xo == LSXxop || (*i).xform.xo == LWBRXxop ||
	    (*i).xform.xo == LFSXxop  || (*i).xform.xo == SRxop || (*i).xform.xo == RRIBxop ||
	    (*i).xform.xo == MASKIRxop  || (*i).xform.xo == LFSUXxop || (*i).xform.xo == LFDXxop ||
	    (*i).xform.xo == DCLSTxop  || (*i).xform.xo == LFDUXxop || (*i).xform.xo == STSXxop ||
	    (*i).xform.xo == STBRXxop  || (*i).xform.xo == STFSXxop || (*i).xform.xo == SRQxop ||
	    (*i).xform.xo == SRExop  || (*i).xform.xo == SRIQxop || (*i).xform.xo == STFDXxop ||
	    (*i).xform.xo == SREQxop  || (*i).xform.xo == STFDUXxop || (*i).xform.xo == LHBRXxop ||
	    (*i).xform.xo == SRAxop || (*i).xform.xo == RACxop ||
	    (*i).xform.xo == STHBRXxop ||
	    (*i).xform.xo == SRAQxop  || (*i).xform.xo == SREAxop ||
	    (*i).xform.xo == ICBIxop || (*i).xform.xo == STFIWXxop ||
	    (*i).xform.xo == DCLZxop )))
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
      ((*i).aform.op == A_FP_EXTENDEDop2 
       && ( (*i).aform.xo == FDxop || (*i).aform.xo == FSxop || (*i).aform.xo == FAxop ||
	    (*i).aform.xo == FMxop  || (*i).aform.xo == FMSxop || (*i).aform.xo == FMAxop ||
	    (*i).aform.xo == FNMSxop || (*i).aform.xo == FNMAxop )) ||

      ((*i).aform.op == A_FP_EXTENDEDop1
       && ( (*i).aform.xo == FDIVSxop || (*i).aform.xo == FSUBSxop || 
	    (*i).aform.xo == FADDSxop || (*i).aform.xo == FMULSxop  || 
	    (*i).aform.xo == FMSUBSxop || (*i).aform.xo == FMADDSxop ||
	    (*i).aform.xo == FNMSUBSxop || (*i).aform.xo == FNMADDSxop )) ||  
      
	/* XL Form */
      ((*i).xlform.op == X_FP_EXTENDEDop && (*i).xlform.xo == FCMPUxop ) || 
	/* X Form */
      ((*i).xform.op == X_FP_EXTENDEDop && (*i).xform.xo == FCMPOxop ))

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
      ((*i).aform.op == A_FP_EXTENDEDop2 
       && (  (*i).aform.xo == FDxop || (*i).aform.xo == FSxop || 
	     (*i).aform.xo == FAxop ||
	     (*i).aform.xo == FSQRTxop ||
	     (*i).aform.xo == FMxop  || (*i).aform.xo == FMSxop || 
	     (*i).aform.xo == FMAxop ||
	     (*i).aform.xo == FNMSxop || (*i).aform.xo == FNMAxop )) ||

      ((*i).aform.op == A_FP_EXTENDEDop1
       && ( (*i).aform.xo == FDIVSxop || (*i).aform.xo == FSUBSxop || 
	    (*i).aform.xo == FADDSxop ||
	    (*i).aform.xo == FMULSxop  || (*i).aform.xo == FMSUBSxop || 
	    (*i).aform.xo == FMAxop ||
	    (*i).aform.xo == FNMSxop || (*i).aform.xo == FNMAxop )) ||  
      
	/* XL Form */
      ((*i).xlform.op == X_EXTENDEDop 
       && (*i).xlform.xo == FCMPUxop ) || 
     

	/* X Form */
      ((*i).xform.op == X_EXTENDEDop 
       && ( (*i).xform.xo == FRSPxop || (*i).xform.xo == FCIRxop || 
	    (*i).xform.xo == FCIRZxop || (*i).xform.xo == FCMPOxop || 
	    (*i).xform.xo == FNEGxop ||
	    (*i).xform.xo == FMRxop ||
	    (*i).xform.xo == FNABSxop || (*i).xform.xo == FABSxop )))
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
      ((*i).aform.op == A_FP_EXTENDEDop2
       && ( (*i).aform.xo == FAxop || (*i).aform.xo == FDxop || (*i).aform.xo == FMxop ||
	    (*i).aform.xo == FMAxop  || (*i).aform.xo == FMSxop || (*i).aform.xo == FNMAxop ||
	    (*i).aform.xo == FNMSxop || (*i).aform.xo == FSxop )) ||  
      
      ((*i).aform.op == A_FP_EXTENDEDop1
       && ( (*i).aform.xo == FDIVSxop || (*i).aform.xo == FMADDSxop ||
	    (*i).aform.xo == FMULSxop ||
	    (*i).aform.xo == FMSUBSxop  || (*i).aform.xo == FNMSUBSxop 
	    || (*i).aform.xo == FNMADDSxop ||
	    (*i).aform.xo == FSUBSxop )) ||  
      
	/* X Form */
      ((*i).xform.op == X_FP_EXTENDEDop 
       && ((*i).xform.xo == FCMPOxop || (*i).xform.xo == FCMPUxop )))

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
  //if ( (*i).xform.op == 31 )
  //  printf("la la %d \n", (*i).xform.xo);

  if (
      /* X Form */
      ( (*i).xform.op == X_EXTENDEDop
	&& ( (*i).xform.xo == NORxop ||  (*i).xform.xo == STBUXxop || (*i).xform.xo == ANDxop ||
	     (*i).xform.xo == ORxop  || (*i).xform.xo == ORCxop || (*i).xform.xo == SLxop ||
	     (*i).xform.xo == ANDCxop  || (*i).xform.xo == EQVxop ||
	     (*i).xform.xo == EXTSBxop  || (*i).xform.xo == EXTSxop  || (*i).xform.xo == LHZUXxop  ||
	     (*i).xform.xo == LUXxop   || (*i).xform.xo == LBZUXxop  || (*i).xform.xo == LHAUXxop ||
	     (*i).xform.xo == NANDxop   || (*i).xform.xo == STHUXxop   || (*i).xform.xo == STUXxop  ||
	     (*i).xform.xo == XORxop  || (*i).xform.xo == SRQxop   || (*i).xform.xo == SRxop   ||  
	     (*i).xform.xo == CLFxop   || (*i).xform.xo == CLIxop   || (*i).xform.xo == DCLSTxop  ||
	     (*i).xform.xo == MASKGxop   || (*i).xform.xo == MASKIRxop   || (*i).xform.xo == MTSRIxop  ||
	     (*i).xform.xo == RRIBxop   || (*i).xform.xo == SLxop   || (*i).xform.xo == SLExop  ||
	     (*i).xform.xo == SLEQxop   || (*i).xform.xo == SLIQxop   || (*i).xform.xo == SLLIQxop  ||
	     (*i).xform.xo == SLLQxop   || (*i).xform.xo == SLQxop   || (*i).xform.xo == SRAIQxop  ||
	     (*i).xform.xo == SRAQxop   || (*i).xform.xo == SRAxop   || (*i).xform.xo == SRAIxop  ||
	     (*i).xform.xo == SRExop   || (*i).xform.xo == SREAxop   || (*i).xform.xo == SREQxop  ||
	     (*i).xform.xo == LFDUXxop   || (*i).xform.xo == LFDXxop   ||
	     (*i).xform.xo == LFSUXxop   || (*i).xform.xo == LFSXxop  ||
	     (*i).xform.xo == SRIQxop   || (*i).xform.xo == SRLIQxop   || (*i).xform.xo == SRLQxop  ||
	     (*i).xform.xo == SRAxop  || (*i).xform.xo == SRAIxop || (*i).xform.xo == SRxop )) ||
      /* XO Form */
      ((*i).xoform.op == XO_EXTENDEDop 
       && ( (*i).xoform.xo == SFxop || (*i).xoform.xo == SFExop || (*i).xoform.xo == SFMExop ||
			      (*i).xoform.xo == SFZExop)) ||
      
      /* D Form */
      ((*i).dform.op == ANDILop || (*i).dform.op == ANDIUop || (*i).dform.op == LBZUop 
       || (*i).dform.op == LHAUop ||(*i).dform.op == LHZUop || (*i).dform.op == LUop || 
       (*i).dform.op == ORILop || (*i).dform.op == ORIUop || (*i).dform.op == STBUop || 
       (*i).dform.op == STHUop || (*i).dform.op == STUop || (*i).dform.op == XORILop ||
       (*i).dform.op == XORIUop || (*i).dform.op == SFIop || (*i).dform.op == SIop || 
       (*i).dform.op == AIDOTop) ||
      
      /* M Form */
      ((*i).mform.op == RLIMIop || (*i).mform.op == RLINMxop || (*i).mform.op == RLMIop 
       ||  (*i).mform.op == RLNMop))
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
  if(((*i).xlform.op == BCLRop) &&
     ((*i).xlform.xo == BCLRxop) && 
     ((*i).xlform.bt & 0x10) && ((*i).xlform.bt & 0x4))
      return true;
  return false;
}

/** is the instruction an indirect jump instruction 
  * @param i the instruction value 
  */
bool InstrucIter::isAIndirectJumpInstruction()
{
  const instruction i = getInstruction();
	if(((*i).xlform.op == BCLRop) && ((*i).xlform.xo == BCCTRxop) &&
	   !(*i).xlform.lk && ((*i).xlform.bt & 0x10) && ((*i).xlform.bt & 0x4))
		return true;
#if 0
        // Disabled; a branch to link register is a return until we can prove
        // otherwise, not a indirect branch.
	if(((*i).xlform.op == BCLRop) && ((*i).xlform.xo == BCLRxop) &&
	   ((*i).xlform.bt & 0x10) && ((*i).xlform.bt & 0x4)){
		if(!ah.hasPrev())
			return false;
                --ah;
		if(!ah.hasPrev())
			return false;
		instruction j = ah.getInstruction();
		if(((*j).xfxform.op == 31) && ((*j).xfxform.xo == 467) &&
		   ((*j).xfxform.spr == 0x100))
			return true;
		++ah;
                j = ah.getInstruction();
		if(((*j).xfxform.op == 31) && ((*j).xfxform.xo == 467) &&
		   ((*j).xfxform.spr == 0x100))
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
	if(((*i).bform.op == BCop) && !(*i).bform.lk &&
	   !(((*i).bform.bo & 0x10) && ((*i).bform.bo & 0x4)))
		return true;
	return false;
}
/** is the instruction an unconditional branch instruction 
  * @param i the instruction value 
  */
bool InstrucIter::isAJumpInstruction()
{
  const instruction i = getInstruction();
	if(((*i).iform.op == Bop) && !(*i).iform.lk)
		return true;
	if(((*i).bform.op == BCop) && !(*i).bform.lk &&
	   ((*i).bform.bo & 0x10) && ((*i).bform.bo & 0x4))
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

    return (i.isInsnType(OPmask | AALKmask, CALLmatch));
/*
  const instruction i = getInstruction();
  if((*i).iform.lk && 
	   (((*i).iform.op == Bop) || ((*i).bform.op == BCop) ||
       (((*i).xlform.op == BCLRop) && 
       (((*i).xlform.xo == 16) || ((*i).xlform.xo == 528))))){
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

  if ((*i).xlform.op == BCLRop && (*i).xlform.xo == BCLRxop)
    {
      if ((*i).xlform.lk)
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
  if ((*i).xlform.op == BCLRop && (*i).xlform.xo == BCCTRxop)
    {
      if ((*i).xlform.lk)
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

#define MK_LD1(bytes, i, imm, ra) (new BPatch_memoryAccess(&(*i).raw, instruction::size(), true, false, (bytes), (imm), (ra), -1))
#define MK_SD1(bytes, i, imm, ra) (new BPatch_memoryAccess(&(*i).raw, instruction::size(), false, true, (bytes), (imm), (ra), -1))

#define MK_LX1(bytes, i, ra, rb) (new BPatch_memoryAccess(&(*i).raw, instruction::size(), true, false, (bytes), 0, (ra), (rb)))
#define MK_SX1(bytes, i, ra, rb) (new BPatch_memoryAccess(&(*i).raw, instruction::size(), false, true, (bytes), 0, (ra), (rb)))

#define MK_LD(bytes, i) (MK_LD1((bytes), i, (*i).dform.d_or_si, (signed)(*i).dform.ra))
#define MK_SD(bytes, i) (MK_SD1((bytes), i, (*i).dform.d_or_si, (signed)(*i).dform.ra))

// VG(11/20/01): X-forms ignore ra if 0, but not rb...
#define MK_LX(bytes, i) (MK_LX1((bytes), i, ((*i).xform.ra ? (signed)(*i).xform.ra : -1), (*i).xform.rb))
#define MK_SX(bytes, i) (MK_SX1((bytes), i, ((*i).xform.ra ? (signed)(*i).xform.ra : -1), (*i).xform.rb))

#define MK_LDS(bytes, i) (MK_LD1((bytes), i, ((*i).dsform.d << 2), (signed)(*i).dsform.ra))
#define MK_SDS(bytes, i) (MK_SD1((bytes), i, ((*i).dsform.d << 2), (signed)(*i).dsform.ra))

#define MK_LI(bytes, i) (MK_LX1((bytes), i, (*i).xform.ra, -1))
#define MK_SI(bytes, i) (MK_SX1((bytes), i, (*i).xform.ra, -1))

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

  int op = (*i).dform.op;
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
	  return MK_LD((32 - (*i).dform.rt)*4, i);
	}
	else {
	  logIS_A("IS_A: stm");
	  return MK_SD((32 - (*i).dform.rt)*4, i);
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
      b = (*i).dsform.xo < 2 ? 8 : 4;
      assert((*i).dsform.xo < 3);
      return MK_LDS(b, i);
    }
    else if(op == STDop) {
      logIS_A("IS_A: std-stdu");
      cerr << "****" << (*i).dsform.xo << endl; 
      assert((*i).dsform.xo < 2);
      
      

      return MK_SDS(8, i);
    }
    else
      return BPatch_memoryAccess::none;
  }
  else if(op == LXop) { // X-forms
    unsigned int xop = (*i).xform.xo;
    //char buf[100];

    //snprintf(buf, 100, "XOP:: %d\n", xop);
    //logIS_A(buf);

    opCodeInfo *oci = xopCodes[xop];

    if(oci->bytes > 0)
      return oci->direc ? MK_SX(oci->bytes, i) : MK_LX(oci->bytes, i);
    else if(xop == LSIxop || xop == STSIxop) {
      b = (*i).xform.rb == 0 ? 32 : (*i).xform.rb; 
      return oci->direc ? MK_SI(b, i) : MK_LI(b, i);
    }
    else if(xop == LSXxop || xop == STSXxop) {
      return new BPatch_memoryAccess(&(*i).raw, instruction::size(), 
				     oci->direc == 0, oci->direc == 1,
                                     0, (*i).xform.ra, (*i).xform.rb,
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
  in = new BPatch_instruction(&(*i).raw, instruction::size());

  return in;
}

/** function which returns the offset of control transfer instructions
  * @param i the instruction value 
  */
Address InstrucIter::getBranchTargetAddress()
{
    const instruction i = getInstruction();
    Address ret = 0;
    if(((*i).iform.op == Bop) || ((*i).bform.op == BCop)){
        int disp = 0;
        if((*i).iform.op == Bop)
            disp = (*i).iform.li;
        else if((*i).bform.op == BCop)
            disp = (*i).bform.bd;
        disp <<= 2;
        if((*i).iform.aa)
            ret = (Address)disp;
        else
            ret = (Address)(current+disp);
    }
    return (Address)ret;
}


void InstrucIter::getMultipleJumpTargets(BPatch_Set<Address>& result)
{
    Address initialAddress = current;
    Address TOC_address = 0;
    if (img_) { 
        TOC_address = img_->getObject().getTOCoffset();
    }
    else {
        pdvector<mapped_object *> m_objs = proc_->mappedObjects();
        for (unsigned i = 0; i < m_objs.size(); i++) {
            void *ptr = m_objs[i]->getPtrToOrigInstruction(current);
            if (ptr) {
                TOC_address = m_objs[i]->parse_img()->getObject().getTOCoffset();
                TOC_address += m_objs[i]->dataBase();
                break;
            }
        }
    }

    assert(TOC_address);
    
    instruction check;
    
    // If there are no prior instructions then we can't be looking at a
    // jump through a jump table.
    if( !hasPrev() ) {
        result += (initialAddress + instruction::size());
        setCurrentAddress(initialAddress);
        return;
    }
    
    // Check if the previous instruction is a move to CTR or LR;
    // if it is, then this is the pattern we're familiar with.  The
    // register being moved into CTR or LR has the address to jump to.
    (*this)--;
    int jumpAddrReg;
    check = getInstruction();
    if ((*check).xfxform.op == STXop && (*check).xfxform.xo == MTSPRxop &&
        ((*check).xfxform.spr == 0x100 || (*check).xfxform.spr == 0x120)) {
        jumpAddrReg = (*check).xfxform.rt;
    } else {
        result += (initialAddress + instruction::size());
        setCurrentAddress(initialAddress);
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
        if ((*check).xoform.op == CAXop && (*check).xoform.xo == CAXxop &&
            (*check).xoform.rt == (unsigned)jumpAddrReg) {
            tableIsRelative = true; 
            //fprintf(stderr, "table is relative...\n");
        }
        else
            (*this)++;
    }
    
    Address jumpStartAddress = 0;
    Address adjustEntry = 0;
    Address tableStartAddress = 0;
    
    if (tableIsRelative) {
        while( hasPrev() ){
            check = getInstruction();
            if(((*check).dform.op == Lop) && ((*check).dform.ra == 2)){
                //fprintf(stderr, "Jump offset from TOC: %d\n", (*check).dform.d_or_si);
                jumpStartAddress = 
                    (Address)(TOC_address + (*check).dform.d_or_si);
                break;
            }
            (*this)--;
        }
        // Anyone know what this does?
        (*this)--;
        check = getInstruction();
        if(((*check).dform.op == Lop)) {
            adjustEntry = (*check).dform.d_or_si;
            //fprintf(stderr, "adjustEntry is 0x%x (%d)\n",
            //adjustEntry, adjustEntry);
        }
       
        while(hasPrev()){
            instruction check = getInstruction();
            if(((*check).dform.op == Lop) && ((*check).dform.ra == 2)){
                //fprintf(stderr, "Table offset from TOC: %d\n", (*check).dform.d_or_si);
                tableStartAddress = 
                    (Address)(TOC_address + (*check).dform.d_or_si);
                //fprintf(stderr, "tableStartAddr is 0x%x\n", tableStartAddress);
                break;
            }
            (*this)--;
        }
    } else {
        bool foundAdjustEntry = false;
        while( hasPrev() ){
            check = getInstruction();
            if((*check).dform.op == CALop &&
               (*check).dform.rt == (unsigned)jumpAddrReg &&
               !foundAdjustEntry){
                foundAdjustEntry = true;
                adjustEntry = (*check).dform.d_or_si;
                jumpAddrReg = (*check).dform.ra;
            } else if((*check).dform.op == Lop &&
                      (*check).dform.ra == 2 &&
                      (*check).dform.rt == (unsigned)jumpAddrReg){
                tableStartAddress = 
                    (Address)(TOC_address + (*check).dform.d_or_si);
                break;
            }
            (*this)--;
        }
    }

    // We could also set this = jumpStartAddress...
    if (tableStartAddress == 0)  {
        //fprintf(stderr, "No table start addr, returning\n"); 
        setCurrentAddress(initialAddress);
        return;
    }

    setCurrentAddress(initialAddress);
    int maxSwitch = 0;
    
    while( hasPrev() ){
        instruction check = getInstruction();
        if(((*check).bform.op == BCop) && 
           !(*check).bform.aa && !(*check).bform.lk){
            (*this)--;
            check = getInstruction();
            if(10 != (*check).dform.op)
                break;
            maxSwitch = (*check).dform.d_or_si + 1;
            break;
        }
        (*this)--;
    }
    //fprintf(stderr, "After checking: max switch %d\n", maxSwitch);
    if(!maxSwitch){
        result += (initialAddress + instruction::size());
        //fprintf(stderr, "No maximum, returning\n");
        setCurrentAddress(initialAddress);
        return;
    }

    Address jumpStart = 0;
    Address tableStart = 0;

    if (img_) {
        void *jumpStartPtr = img_->getPtrToData(jumpStartAddress);
        //fprintf(stderr, "jumpStartPtr (0x%x) = %p\n", jumpStartAddress, jumpStartPtr);
        if (jumpStartPtr)
            jumpStart = *((Address *)jumpStartPtr);
        //fprintf(stderr, "jumpStart 0x%x, initialAddr 0x%x\n",
        //jumpStart, initialAddress);
        if (jumpStartPtr == NULL ||
            (jumpStart != (initialAddress+instruction::size()))) {
            setCurrentAddress(initialAddress);
            return;
        }
        void *tableStartPtr = img_->getPtrToData(tableStartAddress);
        //fprintf(stderr, "tableStartPtr (0x%x) = %p\n", tableStartAddress, tableStartPtr);
        if (tableStartPtr)
            tableStart = *((Address *)tableStartPtr);
        else {
            setCurrentAddress(initialAddress);                    
            return;
        }
        //fprintf(stderr, "... tableStart 0x%x\n", tableStart);

        // We're getting an absolute out of the TOC. Figure out
        // whether we're in code or data.
        const fileDescriptor &desc = img_->desc();
        Address textStart = desc.code();
        Address dataStart = desc.data();

        assert(jumpStart < dataStart);

        bool tableData = false;

        if (proc_) {
            if (tableStart > dataStart) {
                tableData = true;
                tableStart -= dataStart;
                //fprintf(stderr, "Table in data, offset 0x%x\n", tableStart);
            }
            else {
                tableData = false;
                tableStart -= textStart;
                //fprintf(stderr, "Table in text, offset 0x%x\n", tableStart);
            }
        }
        else {
            //fprintf(stderr, "tableStart 0x%x, codeOff 0x%x, codeLen 0x%x, dataOff 0x%x, dataLen 0x%x\n",
            //tableStart, img_->codeOffset(), img_->codeLength(),
            //img_->dataOffset(), img_->dataLength());

            // Not sure what to do, really. We don't know where it is, and I'm
            // not sure how to check.
        }

        for(int i=0;i<maxSwitch;i++){                    
            Address tableEntry = adjustEntry + tableStart + (i * instruction::size());
            //fprintf(stderr, "Table entry at 0x%x\n", tableEntry);
            if (img_->isValidAddress(tableEntry)) {
                int jumpOffset;
                if (tableData) {
                    jumpOffset = *((int *)img_->getPtrToData(tableEntry));
                }
                else
                    jumpOffset = *((int *)img_->getPtrToOrigInstruction(tableEntry));
                
                //fprintf(stderr, "jumpOffset 0x%x\n", jumpOffset);
                Address res = (Address)(jumpStart + jumpOffset);
                if (img_->isCode(res))
                    result += (Address)(jumpStart+jumpOffset);
                //fprintf(stderr, "Entry of 0x%x\n", (Address)(jumpStart + jumpOffset));
            }
            else {
                //fprintf(stderr, "Address not valid!\n");
            }
        }
    }
    else {
        void *ptr = proc_->getPtrToOrigInstruction(jumpStartAddress);
        assert(ptr);
        jumpStart = *((Address *)ptr);
        ptr = NULL;
        ptr = proc_->getPtrToOrigInstruction(tableStartAddress);
        assert(ptr);
        tableStart = *((Address *)ptr);

        for(int i=0;i<maxSwitch;i++){
            Address tableEntry = adjustEntry + tableStart + (i * instruction::size());
            ptr = proc_->getPtrToOrigInstruction(tableEntry);
            assert(ptr);
            int jumpOffset = *((int *)ptr);
            result += (Address)(jumpStart+jumpOffset);
        }
    }        
    setCurrentAddress(initialAddress);

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

instruction InstrucIter::getNextInstruction()
{
    instruction ret;
    if (img_)
        (*ret) = *((instructUnion *)img_->getPtrToOrigInstruction(current + instruction::size()));
    else {
        (*ret) = *((instructUnion *)proc_->getPtrToOrigInstruction(current + instruction::size()));
    }
    return ret;
}

instruction InstrucIter::getPrevInstruction()
{
    instruction ret;
    if (img_)
        (*ret) = *((instructUnion *)img_->getPtrToOrigInstruction(current - instruction::size()));
    else {
        (*ret) = *((instructUnion *)proc_->getPtrToOrigInstruction(current - instruction::size()));
    }
    return ret;
}

Address InstrucIter::operator++()
{
    current += instruction::size();
    initializeInsn();
    return current;
}

Address InstrucIter::operator--()
{
	current -= instruction::size();
    initializeInsn();
	return current;
}

Address InstrucIter::operator++(int)
{
    
	Address ret = current;
	current += instruction::size();
    initializeInsn();
    return ret;
}

Address InstrucIter::operator--(int)
{
	Address ret = current;
	current -= instruction::size();
        initializeInsn();
	return ret;
}

Address InstrucIter::operator*(){
    return current;
}

// Stack frame creation on Power is easy:
// "stu r1, -120(r1)"
// That being, "store the current value of r1 at *r1 and subtract 120 from r1"
bool InstrucIter::isStackFramePreamble(int &/*unset*/) {
    // We check the entire block. Don't know when it ends,
    // so go until we hit a jump.
    bool foundStackPreamble = false;
    while (!isAReturnInstruction() &&
           !isACondBranchInstruction() &&
           !isACallInstruction() &&
           !isADynamicCallInstruction() &&
           !isAJumpInstruction() &&
           insn.valid()) {
        // We need to bit twiddle.
        instructUnion iu = (*insn);
        if ((iu.dform.op == STUop) &&
            (iu.dform.rt == 1) &&
            (iu.dform.ra == 1)) {
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
    // We check the entire block. Don't know when it ends,
    // so go until we hit a jump.
    bool foundMFLR = false;
    bool foundR0Save = false;
    while (!isAReturnInstruction() &&
           !isACondBranchInstruction() &&
           !isACallInstruction() &&
           !isADynamicCallInstruction() &&
           !isAJumpInstruction() &&
           insn.valid()) {
        // We need to bit twiddle.
        instructUnion iu = (*insn);

        if (iu.raw == MFLR0raw) {
            foundMFLR = true;
        }

        if ((iu.dform.op == STop) &&
            (iu.dform.rt == 0) &&
            (iu.dform.ra == 1)) {
            foundR0Save = true;
            break;
        }
        (*this)++;
    }
    return (foundR0Save && foundMFLR);
}
