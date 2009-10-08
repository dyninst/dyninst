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
#if defined(cap_instruction_api)
#include "instructionAPI/h/Instruction.h"
#include "instructionAPI/h/InstructionDecoder.h"
#endif
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

#if 0
bool image_func::archIsIPRelativeBranch(InstrucIter& ah)
{
	// These don't exist on IA32...
#if !defined(arch_x86_64)
  return false;
#endif
  instruction branchCandidate = ah.getInstruction();
  // Branch candidate should have jump opcode, mod r/m, displacement...
  if(branchCandidate.isJumpIndir())
  {
    ia32_locations locs;
    ia32_memacc mac[3];
    ia32_condition c;
    ia32_instruction ll_insn(mac, &c, &locs);
    ia32_decode(IA32_FULL_DECODER,
		static_cast<const unsigned char*>(img()->getPtrToInstruction(*ah)),
		ll_insn);
    if(locs.modrm_mod == 0x0 && locs.modrm_rm == 0x05)
    {
      parsing_printf("%s: Found RIP-offset indirect branch at 0x%lx\n", FILE__,
		     *ah);
      return true;
    }
  }
  return false;
}

bool image_func::archIsRealCall(InstrucIter &ah, bool &validTarget,
                                bool & /* simulateJump */)
{
    instruction insn = ah.getInstruction();
    Address adr = *ah;

   // Initialize return value
   validTarget = true;
   //parsing_printf("*** Analyzing call, offset 0x%x target 0x%x\n",
   //adr, insn.getTarget(adr));
   // calls to adr+5 are not really calls, they are used in 
   // dynamically linked libraries to get the address of the code.
   if (ah.isADynamicCallInstruction()) {
     parsing_printf("... Call 0x%lx is indirect\n", adr);
     validTarget = false;
     return true;
   }
   if (insn.getTarget(adr) == adr + 5) {
       parsing_printf("... getting PC\n");
       // XXX we can do this like on sparc, but we don't need to: we do it
       // on sparc because the intervening instructions don't get executed;
       // on x86 for this heuristic there *are* no intervening instructions.
       //simulateJump = true;
       return false;
   }

   // Calls to a mov instruction followed by a ret instruction, where the 
   // source of the mov is the %esp register, are not real calls.
   // These sequences are used to set the destination register of the mov 
   // with the pc of the instruction instruction that follows the call.

   // This sequence accomplishes this because the call instruction has the 
   // side effect of placing the value of the %eip on the stack and setting the
   // %esp register to point to that location on the stack. (The %eip register
   // maintains the address of the next instruction to be executed).
   // Thus, when the value at the location pointed to by the %esp register 
   // is moved, the destination of the mov is set with the pc of the next
   // instruction after the call.   

   //    Here is an example of this sequence:
   //
   //       mov    %esp, %ebx
   //       ret    
   //
   //    These two instructions are specified by the bytes 0xc3241c8b
   //

   Address targetOffset = insn.getTarget(adr);
 
   if ( !img()->isValidAddress(targetOffset) ) {
       parsing_printf("... Call to 0x%lx is invalid (outside code or data)\n",
       targetOffset);
       validTarget = false;
       return false;
   }    

   // Get a pointer to the call target
   const unsigned char *target =
      (const unsigned char *)img()->getPtrToInstruction(targetOffset);

   // The target instruction is a  mov
   if (*(target) == 0x8b) {
      // The source register of the mov is specified by a SIB byte 
      if (*(target + 1) == 0x1c || *(target + 1) == 0x0c) {

         // The source register of the mov is the %esp register (0x24) and 
         // the instruction after the mov is a ret instruction (0xc3)
         if ( (*(target + 2) == 0x24) && (*(target + 3) == 0xc3)) {
            return false;
         }
      }
   }

   return true;
}

// Determine if the called function is a "library" function or a "user"
// function This cannot be done until all of the functions have been seen,
// verified, and classified
// 
// DELAYED UNTIL PROCESS SPECIALIZATION
//






/********************************************************/
/* Architecture dependent parsing support methods       */
/********************************************************/

bool image_func::archCheckEntry( InstrucIter &ah, image_func * /*func*/ )
{
    instruction insn = ah.getInstruction();

  // check if the entry point contains another point
  if (insn.isJumpDir()) 
    {
      
      return false;
    } 
  else if (insn.isReturn()) 
    {
      // this is an empty function
      return false;
    } 
  else if (insn.isCall()) 
    {
        // So?
        return true;
    }
  return true;
}
#endif

// Architecture-specific a-priori determination that we can't
// parse this function.
bool image_func::archIsUnparseable()
{
    if( !isInstrumentableByFunctionName() )
    {   
        if (!isInstrumentableByFunctionName())
            parsing_printf("... uninstrumentable by func name\n");

        //endOffset_ = startOffset_;
        endOffset_ = getOffset();
        instLevel_ = UNINSTRUMENTABLE; 
        return true;
    }           
    else
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
        const unsigned char* buf = (const unsigned char*)(img()->getPtrToInstruction(getOffset()));
        InstructionDecoder d(buf,
                             endOffset_ - getOffset());
        d.setMode(img()->getAddressWidth() == 8);
        Instruction::Ptr i;
        static RegisterAST::Ptr st0(new RegisterAST(r_ST0));
        static RegisterAST::Ptr st1(new RegisterAST(r_ST1));
        static RegisterAST::Ptr st2(new RegisterAST(r_ST2));
        static RegisterAST::Ptr st3(new RegisterAST(r_ST3));
        static RegisterAST::Ptr st4(new RegisterAST(r_ST4));
        static RegisterAST::Ptr st5(new RegisterAST(r_ST5));
        static RegisterAST::Ptr st6(new RegisterAST(r_ST6));
        static RegisterAST::Ptr st7(new RegisterAST(r_ST7));
        while (i = d.decode()) {
            if(i->isWritten(st0) ||
               i->isWritten(st1) ||
               i->isWritten(st2) ||
               i->isWritten(st3) ||
               i->isWritten(st4) ||
               i->isWritten(st5) ||
               i->isWritten(st6) ||
               i->isWritten(st7)
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
