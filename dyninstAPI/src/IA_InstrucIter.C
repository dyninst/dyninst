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

#include "IA_InstrucIter.h"
#include "debug.h"

IA_InstrucIter::IA_InstrucIter(InstrucIter from, image_func* f)
    : InstructionAdapter(*from, f), ii(from)
{
}

IA_InstrucIter::IA_InstrucIter(InstrucIter from, image * im)
    : InstructionAdapter(*from, im), ii(from)
{
}

void IA_InstrucIter::advance()
{
    InstructionAdapter::advance();
    ++ii;
    current = *ii;
}

size_t IA_InstrucIter::getSize() const
{
    return ii.peekNext() - *ii;
}

bool IA_InstrucIter::hasCFT() const
{
    if(ii.isACallInstruction()
            || ii.isAJumpInstruction()
            || ii.isAReturnInstruction()
            || ii.isACondBranchInstruction()
            || ii.isACondReturnInstruction()
            || ii.isADynamicCallInstruction()
       || ii.isAIndirectJumpInstruction())
    {
        if(!ii.isACallInstruction())
        {
            return true;
        }
        if(isRealCall())
        {
            return true;
        }
        if(simulateJump())
        {
            return true;
        }
    }
    return false;
}

bool IA_InstrucIter::isAbortOrInvalidInsn() const
{
    return ii.isAnAbortInstruction();
}

bool IA_InstrucIter::isAllocInsn() const
{
    return ii.isAnAllocInstruction();
}

bool IA_InstrucIter::isFrameSetupInsn() const
{
    return ii.isFrameSetup();
}

bool IA_InstrucIter::isNop() const
{
    return ii.isANopInstruction();
}

bool IA_InstrucIter::isDynamicCall() const
{
    if(ii.isACondBranchInstruction() || ii.isAIndirectJumpInstruction() ||
      ii.isAJumpInstruction() || ii.isACondReturnInstruction())
        return false;
    if(ii.isADynamicCallInstruction())
    {
            return true;
    }
    return false;
}

bool IA_InstrucIter::isAbsoluteCall() const
{
    if(ii.isACondBranchInstruction() || ii.isAIndirectJumpInstruction() ||
       ii.isAJumpInstruction() || ii.isACondReturnInstruction() ||
       ii.isAReturnInstruction())
        return false;
    bool isAbsolute = false;
    if(ii.isACallInstruction())
    {
        ii.getBranchTargetAddress(&isAbsolute);
    }
    return isAbsolute;
}
       
bool IA_InstrucIter::isReturn() const
{
    if(ii.isACondBranchInstruction() || ii.isAIndirectJumpInstruction() ||
       ii.isAJumpInstruction())
        return false;
    return ii.isACondReturnInstruction() || ii.isAReturnInstruction();
}

bool IA_InstrucIter::isBranch() const
{
    return (ii.isAJumpInstruction() || ii.isACondBranchInstruction() ||
            ii.isAIndirectJumpInstruction());
}

bool IA_InstrucIter::isInterruptOrSyscall() const
{
    return (ii.isAnInterruptInstruction() || ii.isSyscall());
}

bool IA_InstrucIter::isConditional() const
{
    return ii.isACondBranchInstruction() ||
            ii.isACondReturnInstruction();
}
        
Address IA_InstrucIter::getCFT() const
{
    if(ii.isAIndirectJumpInstruction() ||
       ii.isADynamicCallInstruction())
    {
        return 0;
    }
    return ii.getBranchTargetAddress();
}

bool IA_InstrucIter::isCall() const
{
    return ii.isACallInstruction();
}

void IA_InstrucIter::getNewEdges(
        pdvector<std::pair< Address, EdgeTypeEnum> >& outEdges,
        image_basicBlock* currBlk,
        unsigned int num_insns,
        dictionary_hash<Address,
        std::string> *pltFuncs) const
{
    if(!context) {
        fprintf(stderr, "[%s] getNewEdges not supported in non-image_func"
                        "context\n", FILE__);
        return;
    }

    if(!hasCFT()) return;
    else if(ii.isACondBranchInstruction())
    {
        outEdges.push_back(std::make_pair(ii.getBranchTargetAddress(NULL),
                           ET_COND_TAKEN));
        if(ii.isDelaySlot()) {
	  outEdges.push_back(std::make_pair(current + 2 * getSize(), ET_COND_NOT_TAKEN));
        } else {
	  outEdges.push_back(std::make_pair(current + getSize(), ET_COND_NOT_TAKEN));
	}
        return;
    }
    else if(ii.isAIndirectJumpInstruction())
    {
        parsing_printf("... indirect jump at 0x%x\n", current);
        BPatch_Set<Address> targets;
        if( num_insns == 2 ) {
            parsing_printf("... uninstrumentable due to 0 size\n");
            return;
        }
        if(!(context->archIsIndirectTailCall(ii)))
        {
            pdvector<instruction> dummy;
            successfullyParsedJumpTable = context->archGetMultipleJumpTargets(targets, currBlk, ii,
                    dummy);
            parsedJumpTable = true;
            if(successfullyParsedJumpTable)
            {
                for(BPatch_Set<Address>::iterator it = targets.begin();
                    it != targets.end();
                    it++)
                {
                    outEdges.push_back(std::make_pair(*it, ET_INDIR));
                }
            }
        }
        return;
    }
    else if(ii.isAJumpInstruction())
    {
        Address target = ii.getBranchTargetAddress(NULL);
        Address catchStart;
        if(context->archProcExceptionBlock(catchStart, current + getSize()))
        {
            outEdges.push_back(std::make_pair(catchStart, ET_CATCH));
        }
        
        pdvector<instruction> dummy;
        if(!(context->archIsATailCall( ii, dummy )))
        {
            if(!(*pltFuncs).defines(target))
            {
                outEdges.push_back(std::make_pair(target,
                                   ET_DIRECT));
            }
            else
            {
                parsing_printf("%s[519]: PLT tail call to %x\n", FILE__, target);
                outEdges.push_back(std::make_pair(target, ET_NOEDGE));
            }
        }
        else
        {
            parsing_printf("%s[%d]: tail call to %x\n", FILE__, __LINE__, target);
            outEdges.push_back(std::make_pair(target, ET_NOEDGE));
        }
        return;
    }
    else if(ii.isACondReturnInstruction())
    {
        outEdges.push_back(std::make_pair(current + getSize(), ET_FALLTHROUGH));
        return;
    }
    else if(ii.isAReturnInstruction())
    {
        return;
    }
    else if(ii.isACallInstruction() || ii.isADynamicCallInstruction())
    {
        //validTarget is set to false if the call target is not a
        //valid address in the applications process space
        bool validTarget = true;
        //simulateJump is set to true if the call should be treated
        //as an unconditional branch for the purposes of parsing
        //(a special case)
        bool simulateJump = false;

        Address target = ii.getBranchTargetAddress();
        if(context->archIsRealCall(ii, validTarget, simulateJump))
        {
            outEdges.push_back(std::make_pair(target, ET_NOEDGE));
        }
        else
        {
            if(validTarget)
            {
                if(simulateJump)
                {
                    parsing_printf("[%s:%u] call at 0x%lx simulated as "
                            "jump to 0x%lx\n",
                    FILE__,__LINE__,getAddr(),target);
                    outEdges.push_back(std::make_pair(target, ET_DIRECT));
                    return;
                }
            }
        }
        if(ii.isDelaySlot()) {
            outEdges.push_back(std::make_pair(getAddr() + 2 * getSize(),
                               ET_FUNLINK));
          
        } else {
            outEdges.push_back(std::make_pair(getAddr() + getSize(),
                               ET_FUNLINK));
        }
        return;
    }
    return;
}

bool IA_InstrucIter::isLeave() const
{
    return ii.isALeaveInstruction();
}

bool IA_InstrucIter::isDelaySlot() const
{
    return ii.isDelaySlot();
}

instruction IA_InstrucIter::getInstruction()
{
    return ii.getInstruction();
}

bool IA_InstrucIter::isRealCall() const
{
    bool ignored;

    if(!context) {
        fprintf(stderr, "[%s] isRealCall not supported in non-image_func"
                        "context\n", FILE__);
        return false;
    }

    return context->archIsRealCall(ii, ignored, ignored);
}

bool IA_InstrucIter::simulateJump() const
{
    bool simulateJump = false;
    bool ignored;
    (void) context->archIsRealCall(ii, ignored, simulateJump);
    return simulateJump;
}

bool IA_InstrucIter::isRelocatable(InstrumentableLevel lvl) const
{
    if(!context) {
        fprintf(stderr, "[%s] isRelocatable not supported in non-image_func"
                        "context\n", FILE__);
        return false;
    }

    if(lvl == HAS_BR_INDIR)
    {
        return false;
    }
    if(!ii.isACallInstruction())
    {
        return true;
    }
    bool valid, simjump;
    if(!context->archIsRealCall(ii, valid, simjump))
    {
        if(!valid)
        {
            return false;
        }
    }
    return true;
}

bool IA_InstrucIter::isTailCall(unsigned int num_insns) const
{
    if(!context) {
        fprintf(stderr, "[%s] isTailCall not supported in non-image_func"
                        "context\n", FILE__);
        return false;
    }

    if(ii.isACondBranchInstruction()) return false;
    if(ii.isAIndirectJumpInstruction() && context->archIsIndirectTailCall(ii))
    {
        return true;
    }
    pdvector<instruction> dummy;
    if(ii.isAJumpInstruction() && context->archIsATailCall(ii, dummy))
    {
        return true;
    }
    return false;
}

bool IA_InstrucIter::checkEntry() const
{
    if(!context) {
        fprintf(stderr, "[%s] checkEntry not supported in non-image_func"
                        "context\n", FILE__);
        return false;
    }

    return context->archCheckEntry(ii, context);
}

bool IA_InstrucIter::isStackFramePreamble(int& frameSize) const
{
    InstrucIter tmp(ii);
    bool ret = ii.isStackFramePreamble(frameSize);
    return ret;
            
}

bool IA_InstrucIter::savesFP() const
{
    return ii.isFramePush();
}


bool IA_InstrucIter::cleansStack() const
{
    return ii.getInstruction().isCleaningRet();
}

