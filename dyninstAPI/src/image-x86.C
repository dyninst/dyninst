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

/*
 * inst-x86.C - x86 dependent functions and code generator
 */

#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/Vector.h"
#include "image-func.h"
#include "instPoint.h"
#include "symtab.h"
#include "dyninstAPI/h/BPatch_Set.h"
#include "debug.h"
#include <deque>
#include <set>
#include <algorithm>
#include "arch.h"

#include "InstructionAdapter.h"
#include "instructionAPI/h/Instruction.h"
#include "instructionAPI/h/InstructionDecoder.h"
/**************************************************************
 *
 *  machine dependent methods of pdFunction
 *
 **************************************************************/

void checkIfRelocatable(instruction insn, bool &canBeRelocated) {
  const unsigned char *instr = insn.ptr();

  // Check if REG bits of ModR/M byte are 100 or 101 (Possible jump 
  // to jump table).
  if (instr[0] == 0xFF && 
     ( ((instr[1] & 0x38)>>3) == 4 || ((instr[1] & 0x38)>>3) == 5 )) {

    // function should not be relocated
    canBeRelocated = false;
  }
}


// Architecture-specific a-priori determination that we can't
// parse this function.
bool image_func::archIsUnparseable()
{
    /* TODO
     * For the time being, mark any functions in relocatable files as 
     * uninstrumentable. It should be possible to instrument functions
     * in relocatable files, but it isn't supported at this time.
     */
    if( !isInstrumentableByFunctionName() || img()->isRelocatableObj() )
    {   
        if (!isInstrumentableByFunctionName())
            parsing_printf("... uninstrumentable by func name\n");

        //endOffset_ = startOffset_;
        endOffset_ = getOffset();
        instLevel_ = UNINSTRUMENTABLE; 
        return true;

    }else
        return false;
}

// Architecture-specific hack to give up happily on parsing a
// function.
bool image_func::archAvoidParsing()
{
    //temporary convenience hack.. we don't want to parse the PLT as a function
    //but we need pltMain to show up as a function
    //so we set size to zero and make sure it has no instPoints.    
    if( prettyName() == "DYNINST_pltMain" )//|| 
        //prettyName() == "winStart" ||
        //prettyName() == "winFini" )
    {   
        //endOffset_ = startOffset_;
        endOffset_ = getOffset();
        return true;
    }
    else
        return false;
}

// Architecture-specific hack to prevent relocation of certain functions.
bool image_func::archNoRelocate()
{   
    return prettyName() == "__libc_start_main";
}

// Nop on x86
void image_func::archSetFrameSize(int /* frameSize */)
{
    return;
}

void image_func::archInstructionProc(InstructionAdapter & /* ah */)
{
    return;
}


bool image_func::archProcExceptionBlock(Address &catchStart, Address a)
{
   ExceptionBlock b;
  //    Symtab obj = img()->getObject();
    if (img()->getObject()->findCatchBlock(b,a)) {
        catchStart = b.catchStart();
        return true;
    } else {
        return false;
    }
}

bool image_func::writesFPRs(unsigned level) {
    using namespace Dyninst::InstructionAPI;
    // Oh, we should be parsed by now...
    if (!parsed_) image_->analyzeIfNeeded();

    if (containsFPRWrites_ == unknown) {
        // Iterate down and find out...
        // We know if we have callees because we can
        // check the instPoints; no reason to iterate over.
        // We also cache callee values here for speed.

        if (level >= 3) {
            return true; // Arbitrarily decided level 3 iteration.
        }        
        for (unsigned i = 0; i < calls.size(); i++) {
            if (calls[i]->getCallee() && calls[i]->getCallee() != this) {
                if (calls[i]->getCallee()->writesFPRs(level+1)) {
                    // One of our kids does... if we're top-level, cache it; in 
                    // any case, return
                    if (level == 0)
                        containsFPRWrites_ = used;
                    return true;
                }
            }
            else if(!calls[i]->getCallee()){
                // Indirect call... oh, yeah. 
                if (level == 0)
                    containsFPRWrites_ = used;
                return true;
            }
        }

        // No kids contain writes. See if our code does.
        const unsigned char* buf = (const unsigned char*)(img()->getPtrToInstruction(getOffset(), this));
        if( buf == NULL ) {
            parsing_printf("%s[%d]: failed to get pointer to instruction by offset\n",
                    FILE__, __LINE__);
            // if the function cannot be parsed, it is only safe to assume that the FPRs are written
            return true; 
        }
	InstructionDecoder d (buf, endOffset_ - getOffset(), img()->getArch());
        Instruction::Ptr insn;
        static RegisterAST::Ptr st0(new RegisterAST(x86::st0));
        static RegisterAST::Ptr st1(new RegisterAST(x86::st1));
        static RegisterAST::Ptr st2(new RegisterAST(x86::st2));
        static RegisterAST::Ptr st3(new RegisterAST(x86::st3));
        static RegisterAST::Ptr st4(new RegisterAST(x86::st4));
        static RegisterAST::Ptr st5(new RegisterAST(x86::st5));
        static RegisterAST::Ptr st6(new RegisterAST(x86::st6));
        static RegisterAST::Ptr st7(new RegisterAST(x86::st7));
        while (insn = d.decode()) {
            if(insn->isWritten(st0) ||
               insn->isWritten(st1) ||
               insn->isWritten(st2) ||
               insn->isWritten(st3) ||
               insn->isWritten(st4) ||
               insn->isWritten(st5) ||
               insn->isWritten(st6) ||
               insn->isWritten(st7)
              )
            {
                containsFPRWrites_ = used;
                return true;
            }
        }

        // No kids do, and we don't. Impressive.
        containsFPRWrites_ = unused;
        return false;
    }
    else if (containsFPRWrites_ == used) {
        return true;
    }
    else if (containsFPRWrites_ == unused) {
        return false;
    }

    fprintf(stderr, "ERROR: function %s, containsFPRWrites_ is %d (illegal value!)\n", 
	    symTabName().c_str(), containsFPRWrites_);
    
    assert(0);
    return false;
}
