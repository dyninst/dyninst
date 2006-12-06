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

// $Id: image-power.C,v 1.13 2006/12/06 21:17:25 bernat Exp $

// Determine if the called function is a "library" function or a "user" function
// This cannot be done until all of the functions have been seen, verified, and
// classified
//

#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/Vector.h"
#include "image-func.h"
#include "instPoint.h"
#include "symtab.h"
#include "dyninstAPI/h/BPatch_Set.h"
#include "InstrucIter.h"
#include "debug.h"
#include "arch.h"

// Not used on power
bool image_func::archIsRealCall(InstrucIter & /* ah */,
                bool &/*validTarget*/,
                bool & /*simulateJump*/)
{   
    return true;
}

bool image_func::archCheckEntry(InstrucIter &ah, image_func * /* func */)
{   
    // XXX Cheating a little -- this has nothing to do with the
    // "check entry" as seen on x86 & sparc, but is just a convenient place
    // to put this code.

    parsing_printf("calling archCheckEntry for 0x%lx, function %s\n", *ah, symTabName().c_str());

    if (ah.isReturnValueSave())
        makesNoCalls_ = false;
    else
        makesNoCalls_ = true;

    // end cheating

    if (!ah.getInstruction().valid()) return false;

    // We see if we're a procedure linkage table; if so, we are _not_
    // a function (and return false)

    // We don't consider linkage snippets "functions". 
    Address dontcare1;
    if (ah.isInterModuleCallSnippet(dontcare1))
        return false;

    return true;
}

// Not used on power
bool image_func::archIsUnparseable()
{   
    return false;
}

// Not used on power
bool image_func::archAvoidParsing()
{   
    return false;
}

void image_func::archGetFuncEntryAddr(Address & /* funcEntryAddr */)
{   
    return;
}

// Not used on power
bool image_func::archNoRelocate()
{   
    return false;
}

// Not used on power
void image_func::archSetFrameSize(int /* fsize */)                  
{
    return;
}

// As Drew has noted, this really, really should not be an InstructIter
// operation. The extraneous arguments support architectures like x86,
// which (rightly) treat jump table processing as a control-sensitive
// data flow operation.
bool image_func::archGetMultipleJumpTargets(
                                BPatch_Set< Address >& targets,
                                image_basicBlock * /* currBlk */,
                                InstrucIter &ah,
                                pdvector< instruction >& /* allInstructions */)
{   
    return ah.getMultipleJumpTargets( targets );
}

// not implemented on power
bool image_func::archProcExceptionBlock(Address & /* catchStart */, Address /* a */)
{   
    // Agnostic about exception blocks; the policy of champions
    return false;
}

// not implemented on power
bool image_func::archIsATailCall(InstrucIter & /* ah */,
                                 pdvector< instruction >& /* allInstructions */)
{   
    return false;
}

// not implemented on power
bool image_func::archIsIndirectTailCall(InstrucIter & /* ah */)
{   
    return false;
}

bool image_func::archIsAbortOrInvalid(InstrucIter &ah)
{
    return ah.isAnAbortInstruction();
}

// not implemented on power
void image_func::archInstructionProc(InstrucIter & /* ah */)
{
    return;
}

/* This does a linear scan to find out which registers are used in the function,
   it then stores these registers so the scan only needs to be done once.
   It returns true or false based on whether the function is a leaf function,
   since if it is not the function could call out to another function that
   clobbers more registers so more analysis would be needed */
bool image_func::usedRegs()
{
  if (usedRegisters != NULL)
    return leafFunc; /* this gets the proper value AFTER usedRegisters is initialized */
  else
    {
      usedRegisters = new image_func_registers();
      //printf("In function %s\n", symTabName().c_str()); 
      InstrucIter ah(this);
      
      //while there are still instructions to check for in the
      //address space of the function      
      leafFunc = true;
      while (ah.hasMore()) 
	{
	  if (ah.isA_RT_WriteInstruction())
	    if (ah.getRTValue() >= 3 && ah.getRTValue() <= 12)
	      usedRegisters->generalPurposeRegisters.insert(ah.getRTValue());
	  if (ah.isA_RA_WriteInstruction())
	    if (ah.getRAValue() >= 3 && ah.getRAValue() <= 12)
	      usedRegisters->generalPurposeRegisters.insert(ah.getRAValue());
	  if (ah.isA_FRT_WriteInstruction())
	    if (ah.getRTValue() >= 0 && ah.getRTValue() <= 13)
	      usedRegisters->floatingPointRegisters.insert(ah.getRTValue());
	  if (ah.isA_FRA_WriteInstruction())
	    if (ah.getRAValue() >= 0 && ah.getRAValue() <= 13)
	      usedRegisters->floatingPointRegisters.insert(ah.getRAValue());
	  if (ah.isACallInstruction()){
	    leafFunc = false;
	    return false;
	  }
	  ah++;
	}
    }
  return true; 
}
    
