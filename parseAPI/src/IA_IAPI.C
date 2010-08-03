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

#include "dyntypes.h"
#include "IA_IAPI.h"

#include "Register.h"
#include "Dereference.h"
#include "Immediate.h"
#include "BinaryFunction.h"
#include "debug_parse.h"

#include <deque>
#include <map>

using namespace Dyninst;
using namespace InstructionAPI;
using namespace Dyninst::InsnAdapter;
using namespace Dyninst::ParseAPI;

std::map<Architecture, RegisterAST::Ptr> IA_IAPI::framePtr;
std::map<Architecture, RegisterAST::Ptr> IA_IAPI::stackPtr;
std::map<Architecture, RegisterAST::Ptr> IA_IAPI::thePC;

void IA_IAPI::initASTs()
{
    if(framePtr.empty())
    {
        framePtr[Arch_x86] = RegisterAST::Ptr(new RegisterAST(MachRegister::getFramePointer(Arch_x86)));
        framePtr[Arch_x86_64] = RegisterAST::Ptr(new RegisterAST(MachRegister::getFramePointer(Arch_x86_64)));
        framePtr[Arch_ppc32] = RegisterAST::Ptr(new RegisterAST(MachRegister::getFramePointer(Arch_ppc32)));
        framePtr[Arch_ppc64] = RegisterAST::Ptr(new RegisterAST(MachRegister::getFramePointer(Arch_ppc64)));
    }
    if(stackPtr.empty())
    {
        stackPtr[Arch_x86] = RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(Arch_x86)));
        stackPtr[Arch_x86_64] = RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(Arch_x86_64)));
        stackPtr[Arch_ppc32] = RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(Arch_ppc32)));
        stackPtr[Arch_ppc64] = RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(Arch_ppc64)));
    }
    if(thePC.empty())
    {
        thePC[Arch_x86] = RegisterAST::Ptr(new RegisterAST(MachRegister::getPC(Arch_x86)));
        thePC[Arch_x86_64] = RegisterAST::Ptr(new RegisterAST(MachRegister::getPC(Arch_x86_64)));
        thePC[Arch_ppc32] = RegisterAST::Ptr(new RegisterAST(MachRegister::getPC(Arch_ppc32)));
        thePC[Arch_ppc64] = RegisterAST::Ptr(new RegisterAST(MachRegister::getPC(Arch_ppc64)));
    }
}

IA_IAPI::IA_IAPI(InstructionDecoder dec_, 
        Address where_,
        CodeObject * o,
        CodeRegion * r,
        InstructionSource *isrc) :
    InstructionAdapter(where_, o, r, isrc), 
    dec(dec_),
    validCFT(false), 
    cachedCFT(0)
{
    hascftstatus.first = false;
    tailCall.first = false;
    boost::tuples::tie(curInsnIter, boost::tuples::ignore) = allInsns.insert(std::make_pair(current, dec.decode()));
    initASTs();
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
    boost::tuples::tie(curInsnIter, boost::tuples::ignore) = allInsns.insert(std::make_pair(current, dec.decode()));
    if(!curInsn())
    {
        parsing_printf("......WARNING: after advance at 0x%lx, curInsn() NULL\n", current);
    }
    validCFT = false;
    hascftstatus.first = false;
    tailCall.first = false;
}

void IA_IAPI::retreat()
{
    if(!curInsn()) {
        parsing_printf("..... WARNING: failed to retreat InstructionAdapter at 0x%lx, allInsns.size() = %d\n", current,
                       allInsns.size());
        return;
    }
    InstructionAdapter::retreat();
    std::map<Address,Instruction::Ptr>::iterator remove = curInsnIter;
    if(curInsnIter != allInsns.begin()) {
        --curInsnIter;
        allInsns.erase(remove);
        current = curInsnIter->first;
        if(curInsnIter != allInsns.begin()) {
            std::map<Address,Instruction::Ptr>::iterator pit = curInsnIter;
            --pit;
            previous = curInsnIter->first;
        } else {
            previous = -1;
        }
    } else {
        parsing_printf("..... WARNING: cowardly refusal to retreat past first instruction at 0x%lx\n", current);
    }

    /* blind duplication -- nate */
    validCFT = false;
    hascftstatus.first = false;
    tailCall.first = false;
} 
    
    

size_t IA_IAPI::getSize() const
{
    Instruction::Ptr ci = curInsn();
    assert(ci);
    return ci->size();
}

bool IA_IAPI::hasCFT() const
{
  parsing_cerr << "hasCFT called" << endl;
  if(hascftstatus.first) {
    parsing_cerr << "\t Returning cached entry: " << hascftstatus.second << endl;
    return hascftstatus.second;
  }
  InsnCategory c = curInsn()->getCategory();
  hascftstatus.second = false;
  if(c == c_BranchInsn ||
     c == c_ReturnInsn) {
     if ( ! _obj->defensiveMode() ||
         ! isNopJump() ) {
        parsing_cerr << "\t branch or return, ret true" << endl;
        hascftstatus.second = true;
     }
  }
  if(c == c_CallInsn) {
    if(isRealCall()) {
      parsing_cerr << "\t is real call, ret true" << endl;
      hascftstatus.second = true;
    }
    if(isDynamicCall()) {
      parsing_cerr << "\t is dynamic call, ret true" << endl;
      hascftstatus.second = true;
    }
    if(simulateJump()) {
      parsing_cerr << "\t is a simulated jump, ret true" << endl;
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

bool IA_IAPI::isFrameSetupInsn() const
{
    return isFrameSetupInsn(curInsn());
}

bool IA_IAPI::isDynamicCall() const
{
    Instruction::Ptr ci = curInsn();
    if(ci && (ci->getCategory() == c_CallInsn))
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
    Instruction::Ptr ci = curInsn();
    if(ci->getCategory() == c_CallInsn)
    {
        Expression::Ptr cft = ci->getControlFlowTarget();
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
    
    Instruction::Ptr ci = curInsn();

    return (((ci->getOperation().getID() == e_call) &&
            /*(curInsn()->getOperation().isRead(gs))) ||*/
            (ci->getOperand(0).format() == "16")) ||
            (ci->getOperation().getID() == e_syscall) || 
            (ci->getOperation().getID() == e_int) || 
            (ci->getOperation().getID() == power_op_sc));
}


bool IA_IAPI::isInterrupt() const
{
    Instruction::Ptr ci = curInsn();
    return ((ci->getOperation().getID() == e_int) ||
            (ci->getOperation().getID() == e_int3));
}

void IA_IAPI::getNewEdges(std::vector<std::pair< Address, EdgeTypeEnum> >& outEdges,
			  Function* context,
			  Block* currBlk,
			  unsigned int num_insns,
			  dyn_hash_map<Address, std::string> *plt_entries) const
{
    Instruction::Ptr ci = curInsn();

    // Only call this on control flow instructions!
    if(ci->getCategory() == c_CallInsn)
    {
        Address target = getCFT();
        if(isRealCall() || isDynamicCall())
        {
            outEdges.push_back(std::make_pair(target, NOEDGE));
        }
        else
        {
            if(_isrc->isValidAddress(target))
            {
                if(simulateJump())
                {
                    parsing_printf("[%s:%u] call at 0x%lx simulated as "
                            "jump to 0x%lx\n",
                    FILE__,__LINE__,getAddr(),getCFT());
                    outEdges.push_back(std::make_pair(target, DIRECT));
                    return;
                }
            }
        }
        if ( ! _obj->defensiveMode()  // add fallthrough edge unless we're in
             || ( ( ! isDynamicCall() // defensive mode and this is call with
#if defined (os_windows)              // a bad call target or an indirect call 
                    || isIATcall()    // that doesn't pass through the PE's
#endif                                // Import Address Table (i.e., the IAT)
                   )                  // otherwise, the call is unresolved.
                 && _isrc->isValidAddress(target) ) ) 
        {
            outEdges.push_back(std::make_pair(getAddr() + getSize(),CALL_FT));
        }
        return;
      }
    else if(ci->getCategory() == c_BranchInsn)
    {
        Address target;
        if(ci->allowsFallThrough())
        {
            outEdges.push_back(std::make_pair(getCFT(),
                               COND_TAKEN));
            outEdges.push_back(std::make_pair(getNextAddr(), COND_NOT_TAKEN));
            return;
        }
        // Direct jump
        else if((target = getCFT()) != 0)
        {
            Address catchStart;
            if(_cr->findCatchBlock(getNextAddr(),catchStart))
            {
                outEdges.push_back(std::make_pair(catchStart, CATCH));
            }

            if(!isTailCall(context,num_insns))
            {
                if(plt_entries->find(target) == plt_entries->end())
                {
                    outEdges.push_back(std::make_pair(target,DIRECT));
                }
                else
                {
                    parsing_printf("%s[%d]: PLT tail call to %x (%s)\n", 
                        FILE__, __LINE__, target,
                        (*plt_entries)[target].c_str());
                    outEdges.push_back(std::make_pair(target, NOEDGE));
                    tailCall.second = true;
                }
            }
            else
            {
                parsing_printf("%s[%d]: tail call to %x\n", 
                    FILE__, __LINE__, target);
                outEdges.push_back(std::make_pair(target, NOEDGE));
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
            if(isTailCall(context,num_insns)) {
                parsing_printf("%s[%d]: indirect tail call %s at 0x%lx\n", FILE__, __LINE__,
                               ci->format().c_str(), current);
                return;
            }
            parsing_printf("%s[%d]: jump table candidate %s at 0x%lx\n", FILE__, __LINE__,
                           ci->format().c_str(), current);
            parsedJumpTable = true;
            successfullyParsedJumpTable = parseJumpTable(currBlk, outEdges);

            if(!successfullyParsedJumpTable || outEdges.empty()) {
                outEdges.push_back(std::make_pair((Address)-1,INDIRECT));
            }
            return;
        }
    }
    else if(ci->getCategory() == c_ReturnInsn)
    {
        if(ci->allowsFallThrough())
        {
            outEdges.push_back(std::make_pair(getNextAddr(), FALLTHROUGH));
            return;
        }
        return;
    }
    fprintf(stderr, "Unhandled instruction %s\n", ci->format().c_str());
    assert(0);
}

bool IA_IAPI::isIPRelativeBranch() const
{
            // These don't exist on IA32...
#if !defined(arch_x86_64)
    return false;
#endif
    Instruction::Ptr ci = curInsn();

    if(ci->getCategory() == c_BranchInsn &&
        !getCFT())
{
    Expression::Ptr cft = ci->getControlFlowTarget();
    if(cft->isUsed(thePC[_isrc->getArch()]))
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
    Instruction::Ptr ci = curInsn();
    return ci && (ci->getOperation().getID() == e_leave);
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
#if 0
  // Obviated by simulateJump
    if(getCFT() == getNextAddr())
    {
        parsing_printf("... getting PC\n");
        return false;
    }
#endif
    if(!_isrc->isValidAddress(getCFT()))
    {
        parsing_printf(" isREalCall whacked by _isrc->isVAlidAddress(%lx)\n",
            getCFT());
        return false;
    }
    return true;
}

std::map<Address, bool> IA_IAPI::thunkAtTarget;


bool IA_IAPI::isConditional() const
{
    return curInsn()->allowsFallThrough();
}

bool IA_IAPI::simulateJump() const
{
    // obfuscated programs simulate jumps by calling into a block that 
    // discards the return address from the stack, we check for these
    // fake calls in malware mode
    if (_obj->defensiveMode()) {
        return isFakeCall();
    }
    // TODO: we don't simulate jumps on x86 architectures; add logic as we need it.                
    return false;
}

Address IA_IAPI::getCFT() const
{
    if(validCFT) return cachedCFT;
    Expression::Ptr callTarget = curInsn()->getControlFlowTarget();
        // FIXME: templated bind(),dammit!
    callTarget->bind(thePC[_isrc->getArch()].get(), Result(s64, current));
    parsing_printf("%s[%d]: binding PC %s in %s to 0x%x...", FILE__, __LINE__,
                   thePC[_isrc->getArch()]->format().c_str(), curInsn()->format().c_str(), current);
    Result actualTarget = callTarget->eval();
    if(actualTarget.defined)
    {
        cachedCFT = actualTarget.convert<Address>();
        parsing_printf("SUCCESS (CFT=0x%x)\n", cachedCFT);
    }
    else
    {
        cachedCFT = 0;
        parsing_printf("FAIL (CFT=0x%x), callTarget exp: %s\n", cachedCFT,callTarget->format().c_str());
    }
    validCFT = true;
    return cachedCFT;
}

bool IA_IAPI::isRelocatable(InstrumentableLevel lvl) const
{
    Instruction::Ptr ci = curInsn();
    if(ci && (ci->getCategory() == c_CallInsn))
    {
        if(!isDynamicCall())
        {
            if(!_isrc->isValidAddress(getCFT()))
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

InstrumentableLevel IA_IAPI::getInstLevel(Function * context, unsigned int num_insns) const
{
    InstrumentableLevel ret = InstructionAdapter::getInstLevel(context, num_insns);
/*    if(ret == HAS_BR_INDIR && isIPRelativeBranch())
    {
        return NORMAL;
}*/
    return ret;
}
