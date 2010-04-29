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

#include "IA_IAPI.h"

#include "Register.h"
#include "Dereference.h"
#include "Immediate.h"
#include "BinaryFunction.h"
#include "debug.h"
#include "symtab.h"


#include <deque>

using namespace Dyninst;
using namespace InstructionAPI;


IA_IAPI::IA_IAPI(dyn_detail::boost::shared_ptr<Dyninst::InstructionAPI::InstructionDecoder> dec_, Address where_,
                image_func* f)
    : InstructionAdapter(where_, f), dec(dec_),
    validCFT(false), cachedCFT(0)
{
    hascftstatus.first = false;
    tailCall.first = false;
    boost::tuples::tie(curInsnIter, boost::tuples::ignore) = allInsns.insert(std::make_pair(current, dec->decode()));
    stackPtr.reset(new RegisterAST(MachRegister::getStackPointer(img->getArch())));
    framePtr.reset(new RegisterAST(MachRegister::getFramePointer(img->getArch())));
    thePC.reset(new RegisterAST(MachRegister::getPC(img->getArch())));
}

IA_IAPI::IA_IAPI(dyn_detail::boost::shared_ptr<Dyninst::InstructionAPI::InstructionDecoder> dec_, Address where_,
                image * im)
    : InstructionAdapter(where_, im), dec(dec_),
    validCFT(false), cachedCFT(0)
{
    hascftstatus.first = false;
    tailCall.first = false;
    boost::tuples::tie(curInsnIter, boost::tuples::ignore) = allInsns.insert(std::make_pair(current, dec->decode()));
    stackPtr.reset(new RegisterAST(MachRegister::getStackPointer(img->getArch())));
    framePtr.reset(new RegisterAST(MachRegister::getFramePointer(img->getArch())));
    thePC.reset(new RegisterAST(MachRegister::getPC(img->getArch())));
}

void IA_IAPI::advance()
{
    if(!curInsn()) {
        parsing_printf("..... WARNING: failed to advance InstructionAdapter at 0x%lx, allInsns.size() = %d\n", current,
                       allInsns.size());
        return;
    }
    InstructionAdapter::advance();
    current += curInsn()->size();
    boost::tuples::tie(curInsnIter, boost::tuples::ignore) = allInsns.insert(std::make_pair(current, dec->decode()));
    if(!curInsn())
    {
        parsing_printf("......WARNING: after advance at 0x%lx, curInsn() NULL\n", current);
    }
    validCFT = false;
    hascftstatus.first = false;
    tailCall.first = false;
}

size_t IA_IAPI::getSize() const
{
    assert(curInsn());
    return curInsn()->size();
}

bool IA_IAPI::hasCFT() const
{
    if(hascftstatus.first) return hascftstatus.second;
    InsnCategory c = curInsn()->getCategory();
    hascftstatus.second = false;
    if(c == c_BranchInsn ||
       c == c_ReturnInsn)
    {
        hascftstatus.second = true;
    }
    if(c == c_CallInsn)
    {
        if(isRealCall()) {
            hascftstatus.second = true;
        }
        if(isDynamicCall()) {
            hascftstatus.second = true;
        }
        if(simulateJump()) {
            hascftstatus.second = true;
        }
    }
    hascftstatus.first = true;
    return hascftstatus.second;
}

bool IA_IAPI::isAbortOrInvalidInsn() const
{
    entryID e = curInsn()->getOperation().getID();
    if(e == e_No_Entry)
    {
        parsing_printf("...WARNING: un-decoded instruction at 0x%x\n", current);
    }
    return e == e_No_Entry ||
            e == e_int3 ||
            e == e_hlt;
}

bool IA_IAPI::isAllocInsn() const
{
#if !defined(arch_ia64)
    // IA64 only
    return false;
#else
    assert(!"Implement for IA64\n");
    return false;
#endif
}


bool IA_IAPI::isFrameSetupInsn() const
{
    return isFrameSetupInsn(curInsn());
}

bool IA_IAPI::isDynamicCall() const
{
    if(curInsn() && (curInsn()->getCategory() == c_CallInsn))
    {
        if(getCFT() == 0)
        {
            parsing_printf("... Call 0x%lx is indirect\n", current);
            return true;
        }
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


bool IA_IAPI::isReturn() const
{
    return curInsn()->getCategory() == c_ReturnInsn;
}
bool IA_IAPI::isBranch() const
{
    return curInsn()->getCategory() == c_BranchInsn;
}
bool IA_IAPI::isCall() const
{
    return curInsn()->getCategory() == c_CallInsn;
}

bool IA_IAPI::isInterruptOrSyscall() const
{
    return (isInterrupt() && isSyscall());
}

bool IA_IAPI::isSyscall() const
{
    static RegisterAST::Ptr gs(new RegisterAST(x86::gs));

    return (((curInsn()->getOperation().getID() == e_call) &&
            /*(curInsn()->getOperation().isRead(gs))) ||*/
            (curInsn()->getOperand(0).format() == "16")) ||
            (curInsn()->getOperation().getID() == e_syscall) || 
            (curInsn()->getOperation().getID() == e_int) || 
            (curInsn()->getOperation().getID() == power_op_sc));
}


bool IA_IAPI::isInterrupt() const
{
    return ((curInsn()->getOperation().getID() == e_int) ||
            (curInsn()->getOperation().getID() == e_int3));
}

void IA_IAPI::getNewEdges(
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

    // Only call this on control flow instructions!
    if(curInsn()->getCategory() == c_CallInsn)
    {
        Address target = getCFT();
        if(isRealCall())
        {
            outEdges.push_back(std::make_pair(target, ET_NOEDGE));
        }
        else
        {
            if(img->isValidAddress(getCFT()))
            {
                if(simulateJump())
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
        

            if(!isTailCall(num_insns))
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
                    tailCall.second = true;
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
            if( num_insns == 2 ) {
                parsing_printf("... uninstrumentable due to 0 size\n");
                return;
            }
            if(isTailCall(num_insns)) {
                parsing_printf("%s[%d]: indirect tail call %s at 0x%lx\n", FILE__, __LINE__,
                               curInsn()->format().c_str(), current);
                return;
            }
            parsing_printf("%s[%d]: jump table candidate %s at 0x%lx\n", FILE__, __LINE__,
                           curInsn()->format().c_str(), current);
            parsedJumpTable = true;
            successfullyParsedJumpTable = parseJumpTable(currBlk, outEdges);
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


bool IA_IAPI::isIPRelativeBranch() const
{
            // These don't exist on IA32...
#if !defined(arch_x86_64)
    return false;
#endif
    if(curInsn()->getCategory() == c_BranchInsn &&
        !getCFT())
{
    Expression::Ptr cft = curInsn()->getControlFlowTarget();
    if(cft->isUsed(thePC))
    {
        parsing_printf("\tIP-relative indirect jump to %s at 0x%lx\n",
                       cft->format().c_str(), current);
        return true;
    }
}
    return false;
    
}


Instruction::Ptr IA_IAPI::curInsn() const
{
    return curInsnIter->second;
}

bool IA_IAPI::isLeave() const
{
    return curInsn() &&
            (curInsn()->getOperation().getID() == e_leave);
}

bool IA_IAPI::isDelaySlot() const
{
#if defined(arch_sparc)
    assert(!"Implement delay slots on SPARC!");
#endif
    return false;
}

Instruction::Ptr IA_IAPI::getInstruction()
{
    return curInsn();
}

bool IA_IAPI::isRealCall() const
{
    if(getCFT() == getNextAddr())
    {
        parsing_printf("... getting PC\n");
        return false;
    }
    if(!img->isValidAddress(getCFT()))
    {
        return false;
    }

    return (!isThunk());
}


std::map<Address, bool> IA_IAPI::thunkAtTarget;

bool IA_IAPI::isConditional() const
{
    return curInsn()->allowsFallThrough();
}

bool IA_IAPI::simulateJump() const
{
    // TODO: we don't simulate jumps on x86 architectures; add logic as we need it.                
    return false;
}

Address IA_IAPI::getCFT() const
{
    if(validCFT) return cachedCFT;
    Expression::Ptr callTarget = curInsn()->getControlFlowTarget();
        // FIXME: templated bind(),dammit!
    callTarget->bind(thePC.get(), Result(s64, current));
    parsing_printf("%s[%d]: binding PC in %s to 0x%x...", FILE__, __LINE__,
                   curInsn()->format().c_str(), current);
    Result actualTarget = callTarget->eval();
    if(actualTarget.defined)
    {
        cachedCFT = actualTarget.convert<Address>();
        parsing_printf("SUCCESS (CFT=0x%x)\n", cachedCFT);
    }
    else
    {
        cachedCFT = 0;
        parsing_printf("FAIL (CFT=0x%x)\n", cachedCFT);
    }
    validCFT = true;
    return cachedCFT;
}

bool IA_IAPI::isRelocatable(InstrumentableLevel lvl) const
{
    if(curInsn() && (curInsn()->getCategory() == c_CallInsn))
    {
        if(!isDynamicCall())
        {
            if(!img->isValidAddress(getCFT()))
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


InstrumentableLevel IA_IAPI::getInstLevel(unsigned int num_insns) const
{
    InstrumentableLevel ret = InstructionAdapter::getInstLevel( num_insns);
/*    if(ret == HAS_BR_INDIR && isIPRelativeBranch())
    {
        return NORMAL;
}*/
    return ret;
}

        

