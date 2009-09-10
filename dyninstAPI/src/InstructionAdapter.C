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
#include "Dereference.h"
#include "Immediate.h"
#include "BinaryFunction.h"
#include "debug.h"
#include "symtab.h"

#include <deque>
#if defined(arch_x86) || defined(arch_x86_64)
#include "RegisterIDs-x86.h"
#endif

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
    : InstructionAdapter(where_), dec(dec_),
    validCFT(false), cachedCFT(0)
{
    allInsns.insert(std::make_pair(current, dec.decode()));
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
    if(!curInsn()) return;
    previous = current;
    current += curInsn()->size();
    allInsns.insert(std::make_pair(current, dec.decode()));
    parsedJumpTable = false;
    successfullyParsedJumpTable = false;
    checkedDynamicCall_ = false;
    checkedInvalidCallTarget_ = false;
    validCFT = false;
}

size_t IA_IAPI::getSize() const
{
    assert(curInsn());
    return curInsn()->size();
}
size_t IA_InstrucIter::getSize() const
{
    return ii.peekNext() - *ii;
}

bool IA_IAPI::hasCFT() const
{
    if(curInsn()->getCategory() == c_NoCategory ||
       curInsn()->getCategory() == c_CompareInsn)
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
    if(curInsn()->getOperation().getID() == e_No_Entry)
    {
        parsing_printf("...WARNING: un-decoded instruction at 0x%x\n", current);
    }
    return curInsn()->getOperation().getID() == e_No_Entry ||
            curInsn()->getOperation().getID() == e_int ||
            curInsn()->getOperation().getID() == e_int1 ||
            curInsn()->getOperation().getID() == e_int3 ||
            curInsn()->getOperation().getID() == e_hlt;
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
    if(curInsn()->getOperation().getID() == e_mov)
    {
        RegisterAST::Ptr ebp(new RegisterAST(r_EBP));
        RegisterAST::Ptr esp(new RegisterAST(r_ESP));
        RegisterAST::Ptr rbp(new RegisterAST(r_RBP));
        RegisterAST::Ptr rsp(new RegisterAST(r_RSP));
        if((curInsn()->isRead(ebp) ||
            curInsn()->isRead(esp)) &&
            (curInsn()->isWritten(rbp) ||
            curInsn()->isWritten(rsp)))
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
    // TODO: add LEA no-ops
    assert(curInsn());
    if(curInsn()->getOperation().getID() == e_nop)
        return true;
    if(curInsn()->getOperation().getID() == e_lea)
    {
        std::set<Expression::Ptr> memReadAddr;
        curInsn()->getMemoryReadOperands(memReadAddr);
        std::set<RegisterAST::Ptr> writtenRegs;
        curInsn()->getWriteSet(writtenRegs);
        
        if(memReadAddr.size() == 1 && writtenRegs.size() == 1)
        {
            if(**(memReadAddr.begin()) == **(writtenRegs.begin()))
            {
                return true;
            }
            // TODO: check for zero displacement--do we want to bind here?
        }
    }
    return false;
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

bool IA_IAPI::isDynamicCall() const
{
    if(curInsn() && curInsn()->getCategory() == c_CallInsn)
    {
        if(getCFT() == 0)
        {
            parsing_printf("... Call 0x%lx is indirect\n", current);
            return true;
        }
    }
    return false;
}

bool IA_InstrucIter::isDynamicCall() const
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
    if(curInsn()->getCategory() == c_CallInsn)
    {
        Expression::Ptr cft = curInsn()->getControlFlowTarget();
        if(cft && dyn_detail::boost::dynamic_pointer_cast<Immediate>(cft))
        {
            return true;
        }
    }
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
                           image_basicBlock* ,
                           std::vector<instruction>& all_insns) const
{
    if(curInsn()->getCategory() == c_BranchInsn &&
       getCFT() == 0)
    {
        if(all_insns.size() == 2)
        {
            return UNINSTRUMENTABLE;
        }
        else if(isTailCall(context, all_insns))
        {
            return NORMAL;
        }
        else if(!parsedJumpTable)
        {
            // check for unparseable    
            return HAS_BR_INDIR;
        }
        else if(!successfullyParsedJumpTable)
        {
            return HAS_BR_INDIR;
        }
    }
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
        image_basicBlock* ,
        std::vector<instruction>& all_insns) const
{
    assert(curInsn());
    if(curInsn()->getCategory() == c_ReturnInsn)
    {
        return RS_RETURN;
    }
    // Branch that's not resolvable by binding IP,
    // therefore indirect...
    if(curInsn()->getCategory() == c_BranchInsn &&
       getCFT() == 0)
    {
        if(all_insns.size() == 2)
        {
            return RS_UNKNOWN;
        }
        if(isTailCall(context, all_insns))
        {
            return RS_UNKNOWN;
        }
        if(!parsedJumpTable)
        {
            // TODO: check for unparseable jump tables
            if(true)
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
        
FuncReturnStatus IA_InstrucIter::getReturnStatus(image_func* context,
        image_basicBlock* currBlk,
        std::vector<instruction>& all_insns) const
{
    if(ii.isACondReturnInstruction() || ii.isAReturnInstruction())
    {
        return RS_RETURN;
    }
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
    if(isDynamicCall())
    {
        return true;
    }
    if(getReturnStatus(context, currBlk, all_insns) == RS_UNKNOWN)
    {
        return true;
    }
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
    if(curInsn()->getCategory() == c_ReturnInsn)
    {
#ifdef VERBOSE_POINT_PARSING
        parsing_printf("%s[%d]: RETURN at %x, creating exit point\n", FILE__, __LINE__,
                       current);
#endif //VERBOSE_POINT_PARSING
        return functionExit;
    }
    if(isTailCall(context, all_insns))
    {
#ifdef VERBOSE_POINT_PARSING
        parsing_printf("%s[%d]: TAIL CALL (possibly indirect) at %x, creating exit point\n", FILE__, __LINE__,
                       current);
#endif //VERBOSE_POINT_PARSING
        return functionExit;
    }
    if((*pltFuncs).defines(getCFT()))
    {
#ifdef VERBOSE_POINT_PARSING
        parsing_printf("%s[%d]: JUMP TO PLT at %x, creating exit point\n", FILE__, __LINE__,
                       current);
#endif //VERBOSE_POINT_PARSING
        return functionExit;
    }
    if((curInsn()->getCategory() == c_BranchInsn) &&
        !getCFT() &&
        all_insns.size() == 2)
    {
#ifdef VERBOSE_POINT_PARSING
        parsing_printf("%s[%d]: UNPARSABLE INDIRECT JUMP at %x, creating exit point\n", FILE__, __LINE__,
                       current);
#endif //VERBOSE_POINT_PARSING
        return functionExit;
    }
    if(curInsn()->getCategory() == c_CallInsn)
    {
        if(!isRealCall(context->img()))
        {
            if(simulateJump(context))
            {
#ifdef VERBOSE_POINT_PARSING
                parsing_printf("%s[%d]: simulating jump at %x, no instpoint\n", FILE__, __LINE__,
                               current);
#endif //VERBOSE_POINT_PARSING
                return noneType;
            }
            if(context->img()->isValidAddress(getCFT())) {
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
    if(!hasCFT()) return;
    if(curInsn()->getCategory() == c_CallInsn)
    {
        Address target = getCFT();
        if(isRealCall(context->img()))
        {
            outEdges.push_back(std::make_pair(target, ET_NOEDGE));
        }
        else
        {
            if(context->img()->isValidAddress(getCFT()))
            {
                if(simulateJump(context))
                {
                    parsing_printf("[%s:%u] call at 0x%lx simulated as "
                            "jump to 0x%lx\n",
                    FILE__,__LINE__,getAddr(),getCFT());
                    outEdges.push_back(std::make_pair(target, ET_DIRECT));
                    return;
                }
            }
        }
        outEdges.push_back(std::make_pair(getAddr() + getSize(),
                           ET_FUNLINK));
        return;
    }
    else if(curInsn()->getCategory() == c_BranchInsn) 
    {
        if(curInsn()->allowsFallThrough())
        {
            outEdges.push_back(std::make_pair(getCFT(),
                               ET_COND_TAKEN));
            outEdges.push_back(std::make_pair(getNextAddr(), ET_COND_NOT_TAKEN));
            return;
        }
        // Direct jump
        else if(getCFT() != 0)
        {
            Address catchStart;
            if(context->archProcExceptionBlock(catchStart, getNextAddr()))
            {
                outEdges.push_back(std::make_pair(catchStart, ET_CATCH));
            }
        

            if(!isTailCall(context, all_insns))
            {
                if(!(*pltFuncs).defines(getCFT()))
                {
                    outEdges.push_back(std::make_pair(getCFT(),
                                       ET_DIRECT));
                }
                else
                {
                    parsing_printf("%s[%d]: PLT tail call to %x\n", FILE__, __LINE__, getCFT());
                    outEdges.push_back(std::make_pair(getCFT(), ET_NOEDGE));
                }
            }
            else
            {
                parsing_printf("%s[%d]: tail call to %x\n", FILE__, __LINE__, getCFT());
                outEdges.push_back(std::make_pair(getCFT(), ET_NOEDGE));
            }
            return;
        }
        else
        {
            parsing_printf("... indirect jump at 0x%x\n", current);
            if( all_insns.size() == 2 ) {
                parsing_printf("... uninstrumentable due to 0 size\n");
                return;
            }
            if(isTailCall(context, all_insns))
                return;
            parsedJumpTable = true;
            successfullyParsedJumpTable = parseJumpTable(context, currBlk, outEdges);
            return;
        }
    }
    else if(curInsn()->getCategory() == c_ReturnInsn)
    {
        if(curInsn()->allowsFallThrough())
        {
            outEdges.push_back(std::make_pair(getNextAddr(), ET_FALLTHROUGH));
            return;
        }
        return;
    }
    fprintf(stderr, "Unhandled instruction %s\n", curInsn()->format().c_str());
    assert(0);
}

bool IA_IAPI::isMovAPSTable(image* img, std::vector<std::pair< Address, EdgeTypeEnum > >& outEdges) const
{
    /**
    * AMD-64 gcc emits a re-accuring idiom of:
    *         jmpq   *%r8
    *         movaps %xmm7,-0xf(%rax)
    *         movaps %xmm6,-0x1f(%rax)
    *         movaps %xmm5,-0x2f(%rax)
    *         movaps %xmm4,-0x3f(%rax)
    *         movaps %xmm3,-0x4f(%rax)
    *         movaps %xmm2,-0x5f(%rax)
    *         movaps %xmm1,-0x6f(%rax)
    *         movaps %xmm0,-0x7f(%rax)
    *         <other>
    *
    * The jump register is calculated in such a way that it'll be difficult
    * for our analysis to figure it out.  Instead we'll recognize the pattern
    * of the 'movaps'.  Note that the following instruction is also a valid jump
    * target
    **/
    std::set<Address> found;
    InstructionDecoder d((const unsigned char*)(img->getPtrToInstruction(current)),
                        (img->imageOffset() + img->imageLength()) - current);
    d.setMode(img->getAddressWidth() == 8);
    Address cur = current;
    unsigned last_insn_size = 0;
    InstructionAPI::Instruction::Ptr i = d.decode();
    cur += i->size();
    for (;;) {
        InstructionAPI::Instruction::Ptr insn = d.decode();
        //All insns in sequence are movaps
        if (insn->getOperation().getID() != e_movapd &&
            insn->getOperation().getID() != e_movaps)
        {
            found.insert(cur);
            break;
        }
        //All insns are same size
        if (last_insn_size == 0)
            last_insn_size = insn->size();
        else if (last_insn_size != insn->size())
            break;

        found.insert(cur);

        cur += insn->size();
    }
    if (found.size() == 8) {
        //It's a match
        for (std::set<Address>::iterator i=found.begin(); i != found.end(); i++) {
            outEdges.push_back(std::make_pair(*i, ET_INDIR));
        }
        return true;
    }
    return false;
}
        
bool IA_IAPI::isIPRelativeBranch() const
{
            // These don't exist on IA32...
    #if !defined(arch_x86_64)
    return false;
    #endif
    if(curInsn()->getCategory() == c_BranchInsn &&
        !getCFT())
    {
        static RegisterAST::Ptr thePC(new RegisterAST(RegisterAST::makePC()));
        static RegisterAST::Ptr rip(new RegisterAST(r_RIP));
        if(curInsn()->getControlFlowTarget()->isUsed(thePC) ||
           curInsn()->getControlFlowTarget()->isUsed(rip))
        {
            return true;
        }
    }
    return false;
    
}        

bool IA_IAPI::isTableInsn(Instruction::Ptr i) const
{
    if(i->readsMemory() && !i->writesMemory())
    {
        return true;
    }
    if(i->getOperation().getID() == e_lea)
    {
        return true;
    }
    return false;
}
        
std::pair<Address, Instruction::Ptr> IA_IAPI::findTableInsn() const
{
    std::map<Address, Instruction::Ptr>::const_iterator c =
            allInsns.find(current);
    while(!isTableInsn(c->second) && c != allInsns.begin())
    {
        --c;
    }
    if(isTableInsn(c->second))
    {
        return *c;
    }
    return std::make_pair(0, Instruction::Ptr());
}
        
// This should only be called on a known indirect branch...
bool IA_IAPI::parseJumpTable(image_func* context,
                            image_basicBlock* currBlk,
                            std::vector<std::pair< Address, EdgeTypeEnum> >& outEdges) const
{
    if(isIPRelativeBranch())
    {
        return false;
    }
    std::vector<image_edge*> sourceEdges;
    currBlk->getSources(sourceEdges);
    if(sourceEdges.empty())
    {
        return false;
    }
    if(isMovAPSTable(context->img(), outEdges))
    {
        return true;
    }
    bool foundJCCAlongTaken = false;
    Instruction::Ptr tableInsn;
    Address tableInsnAddr;
    boost::tie(tableInsnAddr, tableInsn) = findTableInsn();
    if(!tableInsn)
    {
        parsing_printf("\tunable to find table insn\n");
        return false;
    }
    Instruction::Ptr maxSwitchInsn, branchInsn;
    boost::tie(maxSwitchInsn, branchInsn, foundJCCAlongTaken) = findMaxSwitchInsn(context->img(), currBlk);
    if(!maxSwitchInsn)
    {
        parsing_printf("\tunable to fix max switch size\n");
        return false;
    }
    Address thunkOffset = findThunkAndOffset(context->img(), currBlk);
    parsing_printf("\ttableInsn at 0x%lx\n",tableInsnAddr);
    if(thunkOffset) {
        parsing_printf("\tThunk-calculated table base address: 0x%lx\n",
                       thunkOffset);
    }
    unsigned tableSize = 0, tableStride = 0;
    bool ok = computeTableBounds(context->img(), maxSwitchInsn, branchInsn, tableInsn, foundJCCAlongTaken,
                                 tableSize, tableStride);
    if(!ok)
    {
        return false;
    }
    Address tableBase = getTableAddress(context->img(), tableInsn, thunkOffset);
    return fillTableEntries(context->img(), thunkOffset, tableBase,
                            tableSize, tableStride, outEdges);
}
namespace detail
{
bool isNonCallEdge(image_edge* e)
{
    return e->getType() != ET_CALL;
}
bool leadsToVisitedBlock(image_edge* e, const std::set<image_basicBlock*>& visited)
{
    image_basicBlock* src = e->getSource();
    return visited.find(src) != visited.end();
}
};


bool IA_IAPI::findThunkInBlock(image* img, image_basicBlock* curBlock, Address& thunkOffset) const
{
    const unsigned char* buf =
            (const unsigned char*)(img->getPtrToInstruction(curBlock->firstInsnOffset()));
    InstructionDecoder dec(buf, curBlock->getSize());
    dec.setMode(img->getAddressWidth() == 8);
    IA_IAPI block(dec, curBlock->firstInsnOffset());
    
    while(block.getAddr() < curBlock->endOffset())
    {
        if(block.getInstruction()->getCategory() == c_CallInsn && !block.isRealCall(img))
        {
            block.advance();
            thunkOffset = block.getAddr();
            Instruction::Ptr addInsn = block.getInstruction();
            if(addInsn && addInsn->getOperation().getID() == e_add)
            {
                Result imm = addInsn->getOperand(1).getValue()->eval();
                if(imm.defined)
                {
                    Address thunkDiff = imm.convert<Address>();
                    parsing_printf("\tsetting thunkOffset to 0x%lx (0x%lx + 0x%lx)\n",
                                   thunkOffset+thunkDiff, thunkOffset, thunkDiff);
                    return true;
                }
                else
                {
                    thunkOffset = 0;
                }
            }
        }
        else if(block.getInstruction()->getOperation().getID() == e_lea)
            // Look for an AMD64 IP-relative LEA.  If we find one, it should point to the start of a
        {    // relative jump table.
            Expression::Ptr IPRelAddr = block.getInstruction()->getOperand(1).getValue();
            static RegisterAST* eip(new RegisterAST(r_EIP));
            static RegisterAST* rip(new RegisterAST(r_RIP));
            IPRelAddr->bind(eip, Result(s64, block.getAddr()));
            IPRelAddr->bind(rip, Result(s64, block.getAddr()));
            Result iprel = IPRelAddr->eval();
            if(iprel.defined)
            {
                thunkOffset = iprel.convert<Address>();
                parsing_printf("\tsetting thunkOffset to 0x%lx at 0x%lx (3)\n",thunkOffset, block.getAddr());
                return true;
            }
        }
        block.advance();
    }
    return false;
}


Address IA_IAPI::findThunkAndOffset(image* img, image_basicBlock* start) const
{
    std::set<image_basicBlock*> visited;
    std::deque<image_basicBlock*> worklist;
    pdvector<image_edge*> sources;
    image_basicBlock* curBlock;
    worklist.insert(worklist.begin(), start);
    visited.insert(start);
    Address thunkOffset = 0;
    while(!worklist.empty())
    {
        curBlock = worklist.front();
        worklist.pop_front();
        if(findThunkInBlock(img, curBlock, thunkOffset))
        {
            return thunkOffset;
        }

        sources.clear();
        curBlock->getSources(sources);
        for (unsigned i=0; i<sources.size(); i++) {
            image_edge *e = sources[i];
            if(detail::isNonCallEdge(e) && !detail::leadsToVisitedBlock(e, visited))
            {
                worklist.push_back(e->getSource());
                visited.insert(e->getSource());
            }
        }
    }
    return thunkOffset;

}

boost::tuple<Instruction::Ptr,
 Instruction::Ptr,
 bool> IA_IAPI::findMaxSwitchInsn(image* img, image_basicBlock *start) const
{
    std::set<image_basicBlock *> visited;
    std::vector<image_basicBlock *> WL;
    std::vector<image_edge *> sources;
    std::vector<image_edge *> targets;
    image_basicBlock *curBlk;
    int depth = 0;

    bool foundMaxSwitch = false;
    bool foundCondBranch = false;
    Address maxSwitchAddr = 0;

    WL.push_back(start);
    Instruction::Ptr compareInsn, condBranchInsn;
    bool compareOnTakenBranch = false;
    for(unsigned j=0;j < WL.size(); j++)
    {
        curBlk = WL[j];
        visited.insert(curBlk);

        foundMaxSwitch = false;
        foundCondBranch = false;
        const unsigned char* buf =
                (const unsigned char*)(img->getPtrToInstruction(curBlk->firstInsnOffset()));
        InstructionDecoder dec(buf, curBlk->getSize());
        dec.setMode(img->getAddressWidth() == 8);
        Instruction::Ptr i;
        Address curAdr = curBlk->firstInsnOffset();
        while(i = dec.decode())
        {
            if(i->getCategory() == c_CompareInsn)
            // check for cmp
            {
                parsing_printf("\tFound jmp table cmp instruction at 0x%lx\n",
                            curAdr);
                compareInsn = i;
                maxSwitchAddr = curAdr;
                foundMaxSwitch = true;
            }
            if(i->getCategory() == c_BranchInsn &&
               i->allowsFallThrough())
            {
                parsing_printf("\tFound jmp table cond br instruction at 0x%lx\n",
                               curAdr);
                condBranchInsn = i;
                foundCondBranch = true;

                targets.clear();
                curBlk->getTargets(targets);
                bool taken_hit = false;
                bool fallthrough_hit = false;
                for (unsigned i=0; i<targets.size(); i++) {
                    if (targets[i]->getType() == ET_COND_TAKEN &&
                        (visited.find(targets[i]->getTarget()) != visited.end()))
                    {
                        taken_hit = true;
                    }
                    if ((targets[i]->getType() == ET_COND_NOT_TAKEN ||
                         targets[i]->getType() == ET_FALLTHROUGH) &&
                         (visited.find(targets[i]->getTarget()) != visited.end()))
                    {
                        fallthrough_hit = true;
                    }
                }
                parsing_printf("\tfindMaxSwitchInsn: taken_hit: %d, fallthrough_hit: %d\n", taken_hit, fallthrough_hit);
                compareOnTakenBranch = taken_hit && !fallthrough_hit;
                break;
            }
            curAdr += i->size();
        }

        if(foundMaxSwitch && foundCondBranch)
            break; // done

            // look further back
        sources.clear();
        curBlk->getSources( sources );
        depth++;
            // We've seen depth 2 in libc et al
        if(depth > 2) return boost::make_tuple(Instruction::Ptr(), Instruction::Ptr(), false);
            
        for(unsigned i=0;i<sources.size();i++)
        {
            if(sources[i]->getType() == ET_CALL)
                return boost::make_tuple(Instruction::Ptr(), Instruction::Ptr(), false);

            image_basicBlock * src = sources[i]->getSource();
            if( (visited.find( src ) == visited.end())) {
                WL.push_back(src);
            }
        }
    }
    WL.clear();
    parsing_printf("\tfindMaxSwitchInsn: table on taken branch: %d, returning: %d\n", compareOnTakenBranch, foundMaxSwitch &&
foundCondBranch);
    
    return boost::make_tuple(compareInsn, condBranchInsn, compareOnTakenBranch);
}

Address IA_IAPI::getTableAddress(image* img, Instruction::Ptr tableInsn, Address thunkOffset) const
{
    // Extract displacement from table insn
    Expression::Ptr displacementSrc;
    if(tableInsn->getCategory() == c_BranchInsn)
    {
        displacementSrc = tableInsn->getOperand(0).getValue();
    }
    else
    {
        parsing_printf("\tcracking table instruction %s\n", tableInsn->format().c_str());
        std::vector<InstructionAST::Ptr> tmp;
        Expression::Ptr op = tableInsn->getOperand(1).getValue();
        if(!op)
        {
            parsing_printf("\ttable insn BAD! (no second operand)\n");
            return 0;
        }
        op->getChildren(tmp);
        if(tmp.empty())
        {
            parsing_printf("\ttable insn BAD! (second operand not a deref)\n");
            return 0;
        }
        displacementSrc = dyn_detail::boost::dynamic_pointer_cast<Expression>(tmp[0]);
    }
    static RegisterAST* allGPRs = new RegisterAST(r_ALLGPRS);
    displacementSrc->bind(allGPRs, Result(u32, 0));
    Result disp = displacementSrc->eval();
    if(!disp.defined)
    {
        parsing_printf("\ttable insn: %s, displacement %s, bind of all GPRs FAILED\n",
                       tableInsn->format().c_str(), displacementSrc->format().c_str());
        return 0;
    }
    Address jumpTable = disp.convert<Address>();

    parsing_printf("\tjumpTable set to 0x%lx\n",jumpTable);

    if(!jumpTable && !thunkOffset)
    {
        return 0;
    }
    jumpTable += thunkOffset;
    parsing_printf("\tjumpTable revised to 0x%lx\n",jumpTable);
    // On Windows, all of our other addresses have had the base address
    // of their module subtracted off before we ever use them.  We need to fix this up
    // to conform to that standard so that Symtab actually believes that there's code
    // at the table's destination.
    #if defined(os_windows)
    jumpTable -= img->getObject()->getLoadOffset();
    #endif
    if( !img->isValidAddress(jumpTable) )
    {
        // If the "jump table" has a start address that is outside
        // of the valid range of the binary, we can say with high
        // probability that we have misinterpreted some other
        // construct (such as a function pointer comparison & tail
        // call, for example) as a jump table. Give up now.
        return 0;
    }
    return jumpTable;
}

bool IA_IAPI::fillTableEntries(image* img, Address thunkOffset,
                              Address tableBase,
                              unsigned tableSize,
                              unsigned tableStride,
                              std::vector<std::pair< Address, EdgeTypeEnum> >& outEdges) const
{
    #if defined(os_windows)
    tableBase -= img->getObject()->getLoadOffset();
    #endif
    if( !img->isValidAddress(tableBase) )
    {
        return false;
    }
    for(unsigned int i=0; i < tableSize; i++)
    {
        Address tableEntry = tableBase + (i * tableStride);
        Address jumpAddress = 0;
        
        if(img->isValidAddress(tableEntry))
        {
            if(tableStride == sizeof(Address)) {
                // Unparseable jump table
                jumpAddress = *(const Address *)img->getPtrToInstruction(tableEntry);
            }
            else {
                assert(img->getPtrToInstruction(tableEntry));
                jumpAddress = *(const int *)img->getPtrToInstruction(tableEntry);
            }
        }
    #if defined(os_windows)
    if(img)
    {
        jumpAddress -= img->getObject()->getLoadOffset();
    }
    #endif
        if (!img->isExecutableAddress(jumpAddress)) {
        parsing_printf("\tentry %d [0x%lx] -> 0x%x, invalid, skipping\n",
                    i, tableEntry, jumpAddress);
        continue;
        }
        // FIXME: this really should go before the isExecutableAddress check.
        // Replicating previous buggy behavior to get block numbers to match...
        if(thunkOffset)
        {
            jumpAddress += thunkOffset;
        }
    
        parsing_printf("\tentry %d [0x%lx] -> 0x%x\n",i,tableEntry,jumpAddress);
    
        outEdges.push_back(std::make_pair(jumpAddress, ET_INDIR));
        
    }
    return true;
    
}


bool IA_IAPI::computeTableBounds(image* img, Instruction::Ptr maxSwitchInsn,
                                 Instruction::Ptr branchInsn,
                                 Instruction::Ptr tableInsn,
                                bool foundJCCAlongTaken,
                                unsigned& tableSize,
                                unsigned& tableStride) const
{
    assert(maxSwitchInsn && branchInsn);
    Result compareBound = maxSwitchInsn->getOperand(1).getValue()->eval();
    if(!compareBound.defined) return false;
    tableSize = compareBound.convert<unsigned>();
    if(foundJCCAlongTaken)
    {
        if(branchInsn->getOperation().getID() == e_jbe ||
           branchInsn->getOperation().getID() == e_jle)
        {
            tableSize++;
        }
    }
    else
    {
        if(branchInsn->getOperation().getID() == e_jnbe ||
           branchInsn->getOperation().getID() == e_jnle)
        {
            tableSize++;
        }
    }
    parsing_printf("\tmaxSwitch set to %d\n", tableSize);
    tableStride = img->getAddressWidth();
    std::set<Expression::Ptr> tableInsnReadAddr;
    tableInsn->getMemoryReadOperands(tableInsnReadAddr);
    static Immediate::Ptr four(new Immediate(Result(u8, 4)));
    static BinaryFunction::funcT::Ptr multiplier(new BinaryFunction::multResult());
    static Expression::Ptr dummy(new DummyExpr());
    static BinaryFunction::Ptr scaleCheck(new BinaryFunction(four, dummy, (tableStride == 8) ? u64: u32, multiplier));
    for(std::set<Expression::Ptr>::const_iterator curExpr = tableInsnReadAddr.begin();
        curExpr != tableInsnReadAddr.end();
        ++curExpr)
    {
        if((*curExpr)->isUsed(scaleCheck))
        {
            tableSize = tableSize >> 1;
            parsing_printf("\tmaxSwitch revised to %d\n",tableSize);
        }
    }
    return true;
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
    return;
}


bool IA_InstrucIter::isLeave() const
{
    return ii.isALeaveInstruction();
}

Instruction::Ptr IA_IAPI::curInsn() const
{
    std::map<Address, Instruction::Ptr>::const_iterator curInsnIter =
            allInsns.find(current);
    return curInsnIter->second;
}

bool IA_IAPI::isLeave() const
{
    return curInsn() && 
            (curInsn()->getOperation().getID() == e_leave);
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
    return curInsn();
}

bool IA_IAPI::isRealCall(image* img) const
{
    if(getCFT() == getNextAddr())
    {
        parsing_printf("... getting PC\n");
        return false;
    }
    if(!img->isValidAddress(getCFT()))
    {
        parsing_printf("... Call to 0x%lx is invalid (outside code or data)\n",
                       getCFT());
        return false;
    }
    const unsigned char *target =
            (const unsigned char *)img->getPtrToInstruction(getCFT());

    // We're decoding two instructions: possible move and possible return.
    // Check for move from the stack pointer followed by a return.
    InstructionDecoder targetChecker(target, 32);
    Instruction::Ptr thunkFirst = targetChecker.decode();
    Instruction::Ptr thunkSecond = targetChecker.decode();
    if(thunkFirst && thunkFirst->getOperation().getID() == e_mov)
    {
        RegisterAST::Ptr esp(new RegisterAST(r_ESP));
        RegisterAST::Ptr rsp(new RegisterAST(r_RSP));
        if(thunkFirst->isRead(esp) || thunkFirst->isRead(rsp))
        {
            return (thunkSecond && thunkSecond->getCategory() == c_ReturnInsn);
        } 
    }
    return true;            
}

bool IA_InstrucIter::isRealCall(image_func* context) const
{
    bool ignored;
    return context->archIsRealCall(ii, ignored, ignored);
}

bool IA_IAPI::simulateJump(image_func*) const
{
    // TODO: we don't simulate jumps on x86 architectures; add logic as we need it.                
    return false;
}
bool IA_InstrucIter::simulateJump(image_func* context) const
{
    bool simulateJump = false;
    bool ignored;
    (void) context->archIsRealCall(ii, ignored, simulateJump);
    return simulateJump;
}



Address IA_IAPI::getCFT() const
{
    static RegisterAST thePC = RegisterAST::makePC();
    if(!hasCFT())
    {
        return 0;
    }
    if(validCFT) return cachedCFT;
    Expression::Ptr callTarget = curInsn()->getControlFlowTarget();
        // FIXME: templated bind(),dammit!
    callTarget->bind(&thePC, Result(s64, current));
//    parsing_printf("%s[%d]: binding PC in %s to 0x%x...", FILE__, __LINE__,
//                   curInsn()->format().c_str(), current);
    Result actualTarget = callTarget->eval();
    if(actualTarget.defined)
    {
       cachedCFT = actualTarget.convert<Address>();
//       parsing_printf("SUCCESS (CFT=0x%x)\n", cachedCFT);
    }
    else
    {
        cachedCFT = 0;
//        parsing_printf("FAIL (CFT=0x%x)\n", cachedCFT);
    }
    validCFT = true;
    return cachedCFT;
}

bool IA_IAPI::isRelocatable(image_func* context, InstrumentableLevel lvl) const
{
    if(curInsn() && (curInsn()->getCategory() == c_CallInsn))
    {
        if(!isDynamicCall())
        {
            if(!context->img()->isValidAddress(getCFT()))
            {
                parsing_printf("... Call to 0x%lx is invalid (outside code or data)\n",
                               getCFT());
                return false;
            }
        }
    }
    if(lvl == HAS_BR_INDIR)
    {
        return false;
    }
    return true;
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

bool IA_IAPI::isTailCall(image_func* context, std::vector<instruction>& ) const
{
    if(allInsns.size() < 2) return false;
    if(curInsn()->getCategory() == c_BranchInsn ||
       curInsn()->getCategory() == c_CallInsn)
    {
        std::map<Address, Instruction::Ptr>::const_iterator prevIter =
                allInsns.find(current);
        --prevIter;
        Instruction::Ptr prevInsn = prevIter->second;
        if(prevInsn->getOperation().getID() == e_leave)
        {
            return true;
        }
        if(prevInsn->getOperation().getID() == e_pop)
        {
            static RegisterAST::Ptr ebp(new RegisterAST(r_EBP));
            static RegisterAST::Ptr rbp(new RegisterAST(r_RBP));
            if(prevInsn->isWritten(ebp) || prevInsn->isWritten(rbp))
            {
                return true;
            }
        }
        if(curInsn()->getCategory() == c_BranchInsn)
        {
            if(context->img()->findFuncByEntry(getCFT()))
            {
                return true;
            }
        }
        
    }
                  
    return false;
}

