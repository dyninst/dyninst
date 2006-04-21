/* Copyright (c) 1996-2004 Barton P. Miller
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

// $Id: image-alpha.C,v 1.7 2006/04/21 18:56:56 nater Exp $

#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/Vector.h"
#include "image-func.h"
#include "instPoint.h"
#include "symtab.h"
#include "dyninstAPI/h/BPatch_Set.h"
#include "InstrucIter.h"
#include "showerror.h"
#include "arch.h"
#include "alpha.h"

// Not used on alpha
bool image_func::archIsRealCall(InstrucIter &ah, bool &/* validTarget */,
                                bool & /* simulateJump */)
{
    return true;
}

bool image_func::archCheckEntry(InstrucIter &ah, image_func *func)
{
    return ah.getInstruction().valid();
}

// Not used on alpha
bool image_func::archIsUnparseable()
{
    return false;
}

// Not used on alpha
bool image_func::archAvoidParsing()
{
    return false;
}

void image_func::archGetFuncEntryAddr(Address &funcEntryAddr)
{   
    bool gpKnown = false;
    long gpValue = 0;

    // normal linkage on alphas is that the first two instructions load gp.
    //   In this case, many functions jump past these two instructions, so
    //   we put the inst point at the third instruction.
    //   However, system call libs don't always follow this rule, so we
    //   look for a load of gp in the first two instructions.

    // If we don't do this, entry instrumentation gets skipz0red.

    // We have to manually create a basic block to represent these guys; the
    // "common" entry point is added as an in-edge, splitting the block.

    InstrucIter ah (funcEntryAddr, this);

    instruction firstInsn = ah.getInstruction();
    instruction secondInsn = ah.getNextInstruction();

    if (((*firstInsn).mem.opcode == OP_LDAH) && 
        ((*firstInsn).mem.ra == REG_GP) &&
        ((*secondInsn).mem.opcode == OP_LDA) && 
        ((*secondInsn).mem.ra == REG_GP))
    {
        // compute the value of the gp
        gpKnown = true;
        gpValue = ((long) funcEntryAddr)
                  + (SEXT_16((*firstInsn).mem.disp)<<16)
                  + (*secondInsn).mem.disp;

        funcEntryAddr += 2*instruction::size();
        
        parsing_printf("Func %s, found entry GP pair, entry at 0x%llx\n",
                        symTabName().c_str(),
                        funcEntryAddr);
    }
}

// Not used on alpha
bool image_func::archNoRelocate()
{   
    return false;
}

void image_func::archSetFrameSize(int fsize)
{
    frame_size = fsize;
}


// As Drew has noted, this really, really should not be an InstructIter
// operation. The extraneous arguments support architectures like x86,
// which (rightly) treat jump table processing as a control-sensitive
// data flow operation.
bool image_func::archGetMultipleJumpTargets(
                                BPatch_Set< Address >& targets,
                                image_basicBlock * currBlk,
                                InstrucIter &ah,
                                pdvector< instruction >& allInstructions)
{
    return ah.getMultipleJumpTargets( targets );
}

// not implemented on alpha
bool image_func::archProcExceptionBlock(Address &catchStart, Address a)
{
    // Agnostic about exception blocks; the policy of champions
    return false;
}

// not implemented on alpha
bool image_func::archIsATailCall(InstrucIter &ah,
                                 pdvector< instruction >& allInstructions)
{
    // Seems like doing it like x86 would be a good idea. FIXME
    return false;
}

// not implemented on alpha
bool image_func::archIsIndirectTailCall(InstrucIter &ah)
{
    return false;
}

// not implemented on alpha
bool image_func::archIsAbortOrInvalid(InstrucIter &ah)
{
    return false;
}

void image_func::archInstructionProc(InstrucIter & ah)
{
    return;
}

