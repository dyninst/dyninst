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


// $Id: image-sparc.C,v 1.12 2006/12/06 21:17:26 bernat Exp $

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
#include "inst-sparc.h" // REG_? should be in arch-sparc, but isn't

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


static inline bool CallRestoreTC(instruction instr, instruction nexti) {
    return (instr.isCall() && nexti.isRestore());
}

/****************************************************************************/
/****************************************************************************/

static inline bool MovCallMovTC(instruction instr, instruction nexti) {
    return (instr.isCall() && nexti.isMovToO7());
}

/****************************************************************************/
/****************************************************************************/


/*
    Return bool value indicating whether instruction sequence
     found signals tail-call jmp; nop; sequence.  Note that this should 
     NOT include jmpl; nop;, ret; nop;, retl; nop;....

    Current heuristic to detect such sequences :
     look for jmp %reg, nop in function w/ no stack frame, if jmp, nop
     are last 2 instructions, return true (definate TC), at any other point,
     return false (not TC).  Otherwise, return false (no TC).
     w/ no stack frame....
    instr is instruction being examioned.
    nexti is instruction after
    addr is address of <instr>
    func is pointer to function class object describing function
     instructions come from....
 */
static inline bool JmpNopTC(instruction instr, instruction nexti,
			    Address addr, image_func *func) {

    if (!instr.isInsnType(JMPLmask, JMPLmatch)) {
        return 0;
    }
    
    assert((*instr).resti.op3 == 0x38);
    
    // only looking for jump instructions which don't overwrite a register
    //  with the PC which the jump comes from (g0 is hardwired to 0, so a write
    //  there has no effect?)....  
    //  instr should have gdb disass syntax : 
    //      jmp  %reg, 
    //  NOT jmpl %reg1, %reg2
    if ((*instr).resti.rd != REG_G(0)) {
        return 0;
    }

    // only looking for jump instructions in which the destination is
    //  NOT %i7 + 8/12/16 or %o7 + 8/12/16 (ret and retl synthetic 
    //  instructions, respectively)
    if ((*instr).resti.i == 1) {
        if ((*instr).resti.rs1 == REG_I(7) || (*instr).resti.rs1 == REG_O(7)) {
	    // NOTE : some return and retl instructions jump to {io}7 + 12,
	    //  or (io)7 + 16, not + 8, to have some extra space to store the size of a 
	    //  return structure....
            if ((*instr).resti.simm13 == 0x8 || (*instr).resti.simm13 == 12 ||
		    (*instr).resti.simm13 == 16) {
	        return 0;
	    }
        }
    }  

    // jmp, foloowed by NOP....
    if (!nexti.isNop()) {
        return 0;
    }

    // in function w/o stack frame....
    if (!func->hasNoStackFrame()) {
        return 0;
    }
    
    // if sequence is detected, but not at end of fn 
    //  (last 2 instructions....), return value indicating possible TC.
    //  This should (eventually) mark the fn as uninstrumenatble....
    if (addr != (func->getEndOffset() - 2*instruction::size())) {
        return 0;
    }
    
    return 1;
}
bool image_func::archIsRealCall(InstrucIter &ah, bool &validTarget,
                                bool &simulateJump)
{
    if(!ah.isADynamicCallInstruction())
    {
        Address callTarget = ah.getBranchTargetAddress();
        if (callTarget == 0) {
            // Call to self; skip
            return false;
        }

        if(!img()->isValidAddress(callTarget))
        {
            validTarget = false;
            return false;
        }

        // We have the annoying "call to return" combo like
        // on x86. Those aren't calls, and we handle them
        // in instrumentation

        // Grab the insn at target
        codeBuf_t *target = (codeBuf_t *)img()->getPtrToInstruction(callTarget);
        instruction callTargetInsn;
        callTargetInsn.setInstruction(target);

        if (((*callTargetInsn).raw & 0xfffff000) == 0x81c3e000)
        {
            parsing_printf("Skipping call to retl at 0x%x, func %s\n",
                            *ah, symTabName().c_str());
            return false;
        }

        // We also have "get-my-pc" combos, which we also handle in
        // instrumentation. These may be offset by 8 or 12 bytes from   
        // the call, in one of these forms:
        //
        //  call, nop, <target>
        // or
        //  call, nop, unimpl, <target>

        if(callTarget == *ah + 2*instruction::size() ||
           callTarget == *ah + 3*instruction::size())
        {
            parsing_printf("Skipping \"get-my-pc\" combo at 0x%x\n", *ah);
            // tell the parser to treat this call as an indirect jump
            simulateJump = true;
            return false;
        }
    }

    return true;
}                                
                                 
bool image_func::archCheckEntry( InstrucIter &ah, image_func * /* func */ )                                    
{                                                              
    return ah.getInstruction().valid();
}

bool image_func::archIsUnparseable()
{
    // And here we have hackage. Our jumptable code is b0rken, but I don't know
    // how to fix it. So we define anything that doesn't work as... argh.
    // Better: a size limit on a jump table.
    if (symTabName().c_str() == "__rtboot") {
        return true;
    }

    return false;
}

bool image_func::archAvoidParsing()
{
    return false;
}

void image_func::archGetFuncEntryAddr(Address & /* funcEntryAddr */)
{
    return;
}

bool image_func::archNoRelocate()
{
    return false;
}

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
                                image_basicBlock * /* currBlk */,
                                InstrucIter &ah,
                                pdvector< instruction >& /* allInstructions */)
{
    return ah.getMultipleJumpTargets( targets );
}


// XXX We are currently not doing anything special on SPARC with regards
// to tail calls. Function relocation currently does not do any unwinding
// of tail calls, so this is not terribly important. However, the whole
// notion of what a tail call is at the parsing level and where it should
// be handled with regards to instrumentation needs to be revisited.
//
// Currently, this function returns false regardless of whether the
// instruction matches our tail call heuristics.
bool image_func::archIsATailCall(InstrucIter &ah,
                                 pdvector< instruction >& /* allInstructions */)
{
    if( CallRestoreTC(ah.getInstruction(), ah.getNextInstruction()) ||
        JmpNopTC(ah.getInstruction(), ah.getNextInstruction(), *ah, this) ||            MovCallMovTC(ah.getInstruction(), ah.getNextInstruction())) 
    {
        parsing_printf("ERROR: tail call (?) not handled in func %s at 0x%x\n",
                       symTabName().c_str(), *ah);
        return false;
    }
    else
        return false;
}

// not implemented?
bool image_func::archIsIndirectTailCall(InstrucIter & /* ah */)
{
    return false;
}

bool image_func::archIsAbortOrInvalid(InstrucIter &ah)
{
    return ah.isAnAbortInstruction();
}

void image_func::archInstructionProc(InstrucIter &ah)
{
    // Check whether "07" is live, AKA we can't call safely.
    // Could we just always assume this?
    if (!o7_live) {
        InsnRegister rd, rs1, rs2;
        ah.getInstruction().get_register_operands(&rd, &rs1, &rs2);
        if (rs1.is_o7() || rs2.is_o7()) {
            o7_live = true;
            parsing_printf("Setting o7 to live at 0x%x, func %s\n",
                    *ah, symTabName().c_str());
        }
    }
}

bool image_func::archProcExceptionBlock(Address & /* catchStart */, 
                                        Address /* a */)
{
    return false;
}

