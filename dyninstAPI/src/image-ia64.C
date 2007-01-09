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

// $Id: image-ia64.C,v 1.12 2007/01/09 02:01:56 giri Exp $

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

// Not used on IA64
bool image_func::archIsRealCall(InstrucIter &/* ah */, bool &/*validTarget*/,
                                bool & /* simulateJump */)
{
    return true;
}

bool image_func::archCheckEntry(InstrucIter &ah, image_func * /*func*/)
{
    // x86 checks for functions in the PLT here and so should we.
    // Actually, this should probably be checked when binding a
    // stub function object to a call site during parsing. XXX
    Dyn_Section *sec;
    if(!image_->getObject()->findSection(".plt",sec))
    	return true;
    if(!sec->isOffsetInSection(*ah))
    	return true;
//    if(image_->getObject()->is_offset_in_plt(*ah))
    return false;

//    return true;
}

// Not used on IA64
bool image_func::archIsUnparseable()
{
    return false;
}

// Not used on IA64? FIXME -- probably *should* be just like x86. 
bool image_func::archAvoidParsing()
{
    return false;
}

// Not used on IA64
void image_func::archGetFuncEntryAddr(Address &/* funcEntryAddr */)
{
    return;
}

// Not used on IA64? FIXME -- see above
bool image_func::archNoRelocate()
{
    return false;
}

// Not used on IA64
void image_func::archSetFrameSize(int /* frameSize */)
{
    return;
}

// As Drew has noted, this really, really should not be an InstructIter
// operation. The extraneous arguments support architectures like x86,
// which (rightly) treat jump table processing as a control-sensitive
// data flow operation.
bool image_func::archGetMultipleJumpTargets(
                                BPatch_Set< Address >& targets,
                                image_basicBlock * /*currBlk*/,
                                InstrucIter &ah,
                                pdvector< instruction >& /*allInstructions*/)
{
    return ah.getMultipleJumpTargets( targets );                 
}

bool image_func::archProcExceptionBlock(Address &/*catchStart*/, Address /*a*/)
{
    // Agnostic about exceptions: the policy of champions!
    return false;
}

// Not used on IA64
bool image_func::archIsATailCall(InstrucIter & /* ah */,
                                 pdvector< instruction >& /*allInstructions*/)
{
    // Seems like doing it like x86 would be a good idea. FIXME
    return false;
}

// Not used on IA64 FIXME YET!
bool image_func::archIsIndirectTailCall(InstrucIter & /*ah*/)
{
    return false;
}

bool image_func::archIsAbortOrInvalid(InstrucIter &ah)
{
    return ah.isAnAbortInstruction();
}

// Not used on IA64
void image_func::archInstructionProc(InstrucIter &/*ah*/)
{
    return;
}
    

