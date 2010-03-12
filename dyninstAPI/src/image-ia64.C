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

// $Id: image-ia64.C,v 1.14 2008/04/07 22:32:42 giri Exp $

#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/Vector.h"
#include "image-func.h"
#include "instPoint.h"
#include "symtab.h"
#include "InstrucIter.h"
#include "debug.h"
#include "arch.h"

bool image_func::archIsRealCall(InstrucIter & ah , bool &validTarget,
                                bool & /* simulateJump */)
{
    Address callTarget;

    if(ah.isADynamicCallInstruction())
        return true;

    callTarget = ah.getBranchTargetAddress();
    validTarget = img()->isValidAddress( callTarget );
    return validTarget;
}

bool image_func::archCheckEntry(InstrucIter &ah, image_func * /*func*/)
{
    // x86 checks for functions in the PLT here and so should we.
    // Actually, this should probably be checked when binding a
    // stub function object to a call site during parsing. XXX
    Region *sec;
    if(!image_->getObject()->findRegion(sec, ".plt"))
    	return true;
    if(!sec->isOffsetInRegion(*ah))
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
                                std::set< Address >& targets,
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


// Not used on IA64
void image_func::archInstructionProc(InstructionAdapter &/*ah*/)
{
    return;
}
    

bool image_func::archIsIPRelativeBranch(InstrucIter& ah)
{
  return false;
}
