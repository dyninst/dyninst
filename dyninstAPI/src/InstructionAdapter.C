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

#include "common/h/Types.h"
#include "arch.h"
#include "InstructionAdapter.h"
#include "Register.h"
#include "debug.h"

using namespace Dyninst;
using namespace InstructionAPI;

InstructionAdapter::InstructionAdapter(Address start)
    : current(start), previous(-1), parsedJumpTable(false), successfullyParsedJumpTable(false),
    isDynamicCall_(false), checkedDynamicCall_(false),
    isInvalidCallTarget_(false), checkedInvalidCallTarget_(false)
{
}

InstructionAdapter::~InstructionAdapter()
{
}
IA_InstrucIter::IA_InstrucIter(InstrucIter from)
    : InstructionAdapter(*from), ii(from)
{
}

IA_IAPI::IA_IAPI(InstructionDecoder dec_, Address where_)
    : InstructionAdapter(where_), dec(dec_)
{
    curInsn = dec.decode();
}

void IA_InstrucIter::advance()
{
    previous = current;
    ++ii;
    current = *ii;
    parsedJumpTable = false;
    successfullyParsedJumpTable = false;
    checkedDynamicCall_ = false;
    checkedInvalidCallTarget_ = false;
}

void IA_IAPI::advance()
{
    previous = current;
    current += curInsn->size();
    curInsn = dec.decode();
    parsedJumpTable = false;
    successfullyParsedJumpTable = false;
    checkedDynamicCall_ = false;
    checkedInvalidCallTarget_ = false;
}

size_t IA_IAPI::getSize() const
{
    return curInsn->size();
}
size_t IA_InstrucIter::getSize() const
{
    return ii.peekNext() - *ii;
}

bool IA_IAPI::hasCFT() const
{
    if(curInsn->getCategory() == c_NoCategory)
    {
        return false;
    }
    return true;
}

bool IA_InstrucIter::hasCFT() const
{
    return ii.isACallInstruction()
        || ii.isAJumpInstruction()
        || ii.isAReturnInstruction()
            || ii.isACondBranchInstruction()
            || ii.isACondReturnInstruction()
            || ii.isADynamicCallInstruction()
            || ii.isAIndirectJumpInstruction();
}

bool IA_IAPI::isAbortOrInvalidInsn() const
{
    return curInsn->getOperation().getID() == e_No_Entry ||
            curInsn->getOperation().getID() == e_int ||
            curInsn->getOperation().getID() == e_int1 ||
            curInsn->getOperation().getID() == e_int3;
}

bool IA_InstrucIter::isAbortOrInvalidInsn() const
{
    return ii.isAnAbortInstruction();
}
bool IA_IAPI::isAllocInsn() const
{
    // IA64 only
    return false;

}
bool IA_InstrucIter::isAllocInsn() const
{
    return ii.isAnAllocInstruction();
}

bool IA_IAPI::isFrameSetupInsn() const
{
    if(curInsn->getOperation().getID() == e_mov)
    {
        RegisterAST::Ptr ebp(new RegisterAST(r_EBP));
        RegisterAST::Ptr esp(new RegisterAST(r_ESP));
        RegisterAST::Ptr rbp(new RegisterAST(r_RBP));
        RegisterAST::Ptr rsp(new RegisterAST(r_RSP));
        if((curInsn->isRead(ebp) ||
            curInsn->isRead(esp)) &&
            (curInsn->isWritten(rbp) ||
            curInsn->isWritten(rsp)))
        {
            return true;
        }
    }
    return false;

}
bool IA_InstrucIter::isFrameSetupInsn() const
{
    return ii.isFrameSetup();
}

bool IA_IAPI::isNop() const
{
    return curInsn && curInsn->getOperation().getID() == e_nop;
}
bool IA_InstrucIter::isNop() const
{
    return ii.isANopInstruction();
}

Address InstructionAdapter::getAddr() const
{
    return current;
}

Address InstructionAdapter::getPrevAddr() const
{
    return previous;
}

Address InstructionAdapter::getNextAddr() const
{
    return current + getSize();
}

bool IA_IAPI::isDynamicCall(image_func* context) const
{
    assert(0);
    return false;
}

bool IA_InstrucIter::isDynamicCall(image_func* context) const
{
    if(ii.isACallInstruction() &&
        ii.isADynamicCallInstruction())
    {
        return true;
    }
    return false;
}

bool IA_IAPI::isAbsoluteCall() const
{
    assert(0);
    return false;
}

bool IA_InstrucIter::isAbsoluteCall() const
{
    bool isAbsolute = false;
    if(ii.isACallInstruction())
    {
        ii.getBranchTargetAddress(&isAbsolute);
    }
    return isAbsolute;
}

InstrumentableLevel IA_IAPI::getInstLevel(image_func* context,
                           image_basicBlock* currBlk,
                           std::vector<instruction>& all_insns) const
{
    assert(0);
    return NORMAL;
}

InstrumentableLevel IA_InstrucIter::getInstLevel(image_func* context,
        image_basicBlock* currBlk,
        std::vector<instruction>& all_insns) const
{
    if(ii.isAIndirectJumpInstruction())
    {
        BPatch_Set<Address> targets;
        if(all_insns.size() == 2)
        {
            return UNINSTRUMENTABLE;
        }
        else if(context->archIsIndirectTailCall(ii))
        {
            return NORMAL;
        }
        else if(!parsedJumpTable)
        {
            if(!context->archGetMultipleJumpTargets(targets, currBlk, ii,
                all_insns))
            {
                return HAS_BR_INDIR;
            }
        }
        else if(!successfullyParsedJumpTable)
        {
            return HAS_BR_INDIR;
        }
    }
    return NORMAL;
}
        
FuncReturnStatus IA_IAPI::getReturnStatus(image_func* context,
        image_basicBlock* currBlk,
        std::vector<instruction>& all_insns) const
{
    assert(0);
    return RS_UNSET;
}
        
FuncReturnStatus IA_InstrucIter::getReturnStatus(image_func* context,
        image_basicBlock* currBlk,
        std::vector<instruction>& all_insns) const
{
    if(ii.isACondReturnInstruction() || ii.isAReturnInstruction())
    {
        return RS_RETURN;
    }
    // FIXME
    BPatch_Set<Address> targets;
    if(ii.isAIndirectJumpInstruction())
    {
        if(all_insns.size() == 2)
        {
            return RS_UNKNOWN;
        }
        if(context->archIsIndirectTailCall(ii))
        {
            return RS_UNKNOWN;
        }
        else if(!parsedJumpTable)
        {
            if(!context->archGetMultipleJumpTargets(targets, currBlk, ii,
                all_insns))
            {
                return RS_UNKNOWN;
            }
        }
        else if(!successfullyParsedJumpTable)
        {
            return RS_UNKNOWN;
        }
    }
    return RS_UNSET;
}

bool IA_IAPI::hasUnresolvedControlFlow(image_func* context,
                                              image_basicBlock* currBlk, std::vector<instruction>& all_insns) const
{
    assert(0);
    return false;
}

bool IA_InstrucIter::hasUnresolvedControlFlow(image_func* context,
    image_basicBlock* currBlk, std::vector<instruction>& all_insns) const
{
    // Invalid addresses will get handled when we check each edge
    if(ii.isADynamicCallInstruction())
    {
        return true;
    }
    if(getReturnStatus(context, currBlk, all_insns) == RS_UNKNOWN)
    {
        return true;
    }
    return false;       
}
        
instPointType_t IA_IAPI::getPointType(image_func* context,
                                             std::vector<instruction>& all_insns,
                                             dictionary_hash<Address, std::string> *pltFuncs) const
{
    assert(0);
    return noneType;
}
instPointType_t IA_InstrucIter::getPointType(image_func* context,
         std::vector<instruction>& all_insns,
         dictionary_hash<Address, std::string> *pltFuncs) const
{
    if(ii.isAReturnInstruction() || ii.isACondReturnInstruction())
    {
#ifdef VERBOSE_POINT_PARSING        
        parsing_printf("%s[%d]: RETURN at %x, creating exit point\n", FILE__, __LINE__,
                       current);
#endif //VERBOSE_POINT_PARSING
        return functionExit;
    }
    if (ii.isAIndirectJumpInstruction() && context->archIsIndirectTailCall(ii))
    {
#ifdef VERBOSE_POINT_PARSING
        parsing_printf("%s[%d]: INDIRECT TAIL CALL at %x, creating exit point\n", FILE__, __LINE__,
                       current);
#endif //VERBOSE_POINT_PARSING
        return functionExit;
    }
    if (ii.isAIndirectJumpInstruction() && all_insns.size() == 2)
    {
#ifdef VERBOSE_POINT_PARSING
        parsing_printf("%s[%d]: UNPARSABLE INDIRECT JUMP at %x, creating exit point\n", FILE__, __LINE__,
                       current);
#endif //VERBOSE_POINT_PARSING
        return functionExit;
    }
    if(ii.isAJumpInstruction() && context->archIsATailCall(ii, all_insns))
    {
#ifdef VERBOSE_POINT_PARSING
        parsing_printf("%s[%d]: TAIL CALL at %x, creating exit point\n", FILE__, __LINE__,
                       current);
#endif //VERBOSE_POINT_PARSING
        return functionExit;
    }
    if(ii.isAJumpInstruction() && (*pltFuncs).defines(ii.getBranchTargetAddress()))
    {
#ifdef VERBOSE_POINT_PARSING
        parsing_printf("%s[%d]: JUMP TO PLT at %x, creating exit point\n", FILE__, __LINE__,
                       current);
#endif //VERBOSE_POINT_PARSING
        return functionExit;
    }
    if(ii.isACallInstruction())
    {
        bool validTarget = true;
        bool simulateJump = false;
        if(!context->archIsRealCall(ii, validTarget, simulateJump))
        {
            if(simulateJump)
            {
#ifdef VERBOSE_POINT_PARSING
                parsing_printf("%s[%d]: simulating jump at %x, no instpoint\n", FILE__, __LINE__,
                               current);
#endif //VERBOSE_POINT_PARSING
                return noneType;
            }
#ifdef VERBOSE_POINT_PARSING
            parsing_printf("%s[%d]: archIsRealCall FALSE at %x\n", FILE__, __LINE__,
                           current);
#endif //VERBOSE_POINT_PARSING
            if(validTarget) {
#ifdef VERBOSE_POINT_PARSING
                parsing_printf("%s[%d]: target valid, thunk, no instpoint\n", FILE__, __LINE__);
#endif //VERBOSE_POINT_PARSING
                return noneType;
            }
                
        }
#ifdef VERBOSE_POINT_PARSING
        parsing_printf("%s[%d]: call at %x, creating callSite\n", FILE__, __LINE__,
                       current);
#endif //VERBOSE_POINT_PARSING
        
        return callSite;
    }
    return noneType;
}

void IA_IAPI::getNewEdges(
        std::vector<std::pair< Address, EdgeTypeEnum> >& outEdges,
        image_func* context, image_basicBlock* currBlk,
        std::vector<instruction>& all_insns,
        dictionary_hash<Address,
        std::string> *pltFuncs) const
{
    assert(0);
}
void IA_InstrucIter::getNewEdges(
        std::vector<std::pair< Address, EdgeTypeEnum> >& outEdges,
        image_func* context, image_basicBlock* currBlk,
        std::vector<instruction>& all_insns,
        dictionary_hash<Address,
std::string> *pltFuncs) const
{
    if(!hasCFT()) return;
    if(ii.isACallInstruction())
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
        outEdges.push_back(std::make_pair(getAddr() + getSize(),
                            ET_FUNLINK));
        return;
    }
    else if(ii.isACondBranchInstruction())
    {
        outEdges.push_back(std::make_pair(ii.getBranchTargetAddress(NULL),
                        ET_COND_TAKEN));
        outEdges.push_back(std::make_pair(current + getSize(), ET_COND_NOT_TAKEN));
        return;
    }
    else if(ii.isAIndirectJumpInstruction())
    {
        parsing_printf("... indirect jump at 0x%x\n", current);
        BPatch_Set<Address> targets;
        if( all_insns.size() == 2 ) {
            parsing_printf("... uninstrumentable due to 0 size\n");
            return;
        }
        if(!(context->archIsIndirectTailCall(ii)))
        {
            successfullyParsedJumpTable = context->archGetMultipleJumpTargets(targets, currBlk, ii,
                    all_insns);
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
        

        if(!(context->archIsATailCall( ii, all_insns )))
        {
            if(!(*pltFuncs).defines(target))
            {
                outEdges.push_back(std::make_pair(target,
                                   ET_DIRECT));
            }
            else
            {
                parsing_printf("%s[%d]: PLT tail call to %x\n", FILE__, __LINE__, target);
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
    return;
}


bool IA_InstrucIter::isLeave() const
{
    return ii.isALeaveInstruction();
}

bool IA_IAPI::isLeave() const
{
    return curInsn && curInsn->getOperation().getID() == e_leave;
}

bool IA_InstrucIter::isDelaySlot() const
{
    return ii.isDelaySlot();
}

bool IA_IAPI::isDelaySlot() const
{
#if defined(arch_sparc)
    assert(!"Implement delay slots on SPARC!");
#endif
    return false;
}
instruction IA_InstrucIter::getInstruction()
{
    return ii.getInstruction();
}

Instruction::Ptr IA_IAPI::getInstruction()
{
    return curInsn;
}

bool IA_IAPI::isRealCall(image_func* context) const
{
    assert(0);
    return false;
}

bool IA_InstrucIter::isRealCall(image_func* context) const
{
    bool ignored;
    return context->archIsRealCall(ii, ignored, ignored);
}

bool IA_IAPI::simulateJump(image_func* context) const
{
    assert(0);
    return false;
}
bool IA_InstrucIter::simulateJump(image_func* context) const
{
    bool simulateJump = false;
    bool ignored;
    (void) context->archIsRealCall(ii, ignored, simulateJump);
    return simulateJump;
}


bool IA_IAPI::isRelocatable(image_func* context, InstrumentableLevel lvl) const
{
    assert(0);
    return lvl != HAS_BR_INDIR;
}

bool IA_InstrucIter::isRelocatable(image_func* context, InstrumentableLevel lvl) const
{
    bool valid, simjump;
    if(!context->archIsRealCall(ii, valid, simjump))
    {
        if(!valid)
        {
            return false;
        }
    }
    if(lvl == HAS_BR_INDIR)
    {
        return false;
    }
    return true;
}

bool IA_InstrucIter::isTailCall(image_func* context, std::vector<instruction>& all_insns) const
{
    if(ii.isAJumpInstruction() && context->archIsATailCall(ii, all_insns))
    {
        return true;
    }
    if(ii.isACallInstruction() && context->archIsIndirectTailCall(ii))
    {
        return true;
    }
    return false;
}

bool IA_IAPI::isTailCall(image_func* context, std::vector<instruction>& all_insns) const
{
    assert(0);
    return false;
}

