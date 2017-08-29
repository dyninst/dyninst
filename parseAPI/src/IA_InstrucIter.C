/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

#include "parseAPI/h/CodeSource.h"

#include "IA_InstrucIter.h"
#include "debug_parse.h"

using namespace Dyninst;
using namespace Dyninst::InsnAdapter;
using namespace Dyninst::ParseAPI;

IA_InstrucIter::IA_InstrucIter(InstrucIter from, 
    CodeObject * o,
    CodeRegion * r,
    InstructionSource * s)
    : InstructionAdapter(*from, o,r,s), ii(from)
{
}

void IA_InstrucIter::advance()
{
    InstructionAdapter::advance();
    ++ii;
    current = *ii;
}

bool IA_InstrucIter::retreat()
{
    bool ret = ii.hasPrev();
    InstructionAdapter::retreat();
    --ii;
    if(ii.hasPrev())
        previous = ii.peekPrev();
    else
        previous = (Address)-1;
    current = *ii;
    return ret;
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

bool IA_InstrucIter::isAbort() const
{
    return ii.isAnAbortInstruction();
}

bool IA_InstrucIter::isInvalidInsn() const
{
   return false;
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
        std::vector<std::pair< Address, EdgeTypeEnum> >& outEdges,
        Function* context,
        Block* currBlk,
        unsigned int num_insns,
        dyn_hash_map<Address, std::string> *plt_entries) const
{
    if(!hasCFT()) return;
    else if(ii.isACondBranchInstruction())
    {
        outEdges.push_back(std::make_pair(ii.getBranchTargetAddress(NULL),
                           COND_TAKEN));
        if(ii.isDelaySlot()) {
	  outEdges.push_back(std::make_pair(current + 2 * getSize(), COND_NOT_TAKEN));
        } else {
	  outEdges.push_back(std::make_pair(current + getSize(), COND_NOT_TAKEN));
	}
        return;
    }
    else if(ii.isAIndirectJumpInstruction())
    {
        parsing_printf("... indirect jump at 0x%x\n", current);
        std::set<Address> targets;
        if( num_insns == 2 ) {
            parsing_printf("... uninstrumentable due to 0 size\n");
            return;
        }
        if(!ii.isIndirectTailCall(context))
        {
            pdvector<instruction> dummy;
            successfullyParsedJumpTable = ii.getMultipleJumpTargets(targets);
            parsedJumpTable = true;
            if(successfullyParsedJumpTable)
            {
                for(std::set<Address>::iterator it = targets.begin();
                    it != targets.end();
                    it++)
                {
                    outEdges.push_back(std::make_pair(*it, INDIRECT));
                }
            }
        }
        return;
    }
    else if(ii.isAJumpInstruction())
    {
        Address target = ii.getBranchTargetAddress(NULL);
        
        if(!ii.isTailCall(context))
        {
            if(plt_entries->find(target) == plt_entries->end())
            {
                outEdges.push_back(std::make_pair(target,
                                   DIRECT));
            }
            else
            {
                parsing_printf("%s[519]: PLT tail call to %x\n", FILE__, target);
                outEdges.push_back(std::make_pair(target, NOEDGE));
            }
        }
        else
        {
            parsing_printf("%s[%d]: tail call to %x\n", FILE__, __LINE__, target);
            outEdges.push_back(std::make_pair(target, NOEDGE));
        }
        return;
    }
    else if(ii.isACondReturnInstruction())
    {
        outEdges.push_back(std::make_pair(current + getSize(), FALLTHROUGH));
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
        if(ii.isRealCall(_isrc,validTarget,simulateJump))
        {
            outEdges.push_back(std::make_pair(target, NOEDGE));
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
                    outEdges.push_back(std::make_pair(target, DIRECT));
                    return;
                }
            }
        }
        if(ii.isDelaySlot()) {
            outEdges.push_back(std::make_pair(getAddr() + 2 * getSize(),
                               CALL_FT));
          
        } else {
            outEdges.push_back(std::make_pair(getAddr() + getSize(),
                               CALL_FT));
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
    return ii.isRealCall(_isrc,ignored,ignored);
}

bool IA_InstrucIter::simulateJump() const
{
    bool simulateJump = false;
    bool ignored;
    (void) ii.isRealCall(_isrc,ignored,simulateJump);
    return simulateJump;
}

bool IA_InstrucIter::isRelocatable(InstrumentableLevel lvl) const
{
    if(lvl == HAS_BR_INDIR)
    {
        return false;
    }
    if(!ii.isACallInstruction())
    {
        return true;
    }
    bool valid = false, simjump = false;
    if(!ii.isRealCall(_isrc,valid,simjump))
    {
        if(!valid)
        {
            return false;
        }
    }
    return true;
}

bool IA_InstrucIter::isTailCall(Function * context, EdgeTypeEnum /*ignored*/, unsigned int/* num_insns */) const
{
    if(ii.isACondBranchInstruction()) return false;
    if(ii.isAIndirectJumpInstruction() && ii.isIndirectTailCall(context))
    {
        return true;
    }
    pdvector<instruction> dummy;
    if(ii.isAJumpInstruction() && ii.isTailCall(context))
    {
        return true;
    }
    return false;
}

bool IA_InstrucIter::isStackFramePreamble() const
{
    InstrucIter tmp(ii);
    bool ret = ii.isStackFramePreamble();
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

bool IA_InstrucIter::isReturnAddrSave() const
{
    return ii.isReturnValueSave();
}

bool IA_InstrucIter::isNopJump() const
{
    return false;
}
