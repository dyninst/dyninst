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

#include "common/h/Types.h"
#include "arch.h"
#include "InstructionAdapter.h"
#include "debug.h"
#include "symtab.h"


InstructionAdapter::InstructionAdapter(Address start, image_func* f)
    : current(start), previous(-1), parsedJumpTable(false), successfullyParsedJumpTable(false),
    isDynamicCall_(false), checkedDynamicCall_(false),
    isInvalidCallTarget_(false), checkedInvalidCallTarget_(false),
    context(f), img(f->img())
{
}

InstructionAdapter::InstructionAdapter(Address start, image * im)
    : current(start), previous(-1), parsedJumpTable(false), successfullyParsedJumpTable(false),
    isDynamicCall_(false), checkedDynamicCall_(false),
    isInvalidCallTarget_(false), checkedInvalidCallTarget_(false),
    context(NULL), img(im)
{
}

InstructionAdapter::~InstructionAdapter()
{
}


void InstructionAdapter::advance()
{
    previous = current;
    parsedJumpTable = false;
    successfullyParsedJumpTable = false;
    checkedDynamicCall_ = false;
    checkedInvalidCallTarget_ = false;

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

FuncReturnStatus InstructionAdapter::getReturnStatus(image_basicBlock* ,
        unsigned int num_insns) const
{
    // Branch that's not resolvable by binding IP,
    // therefore indirect...
    if(isBranch() &&
       getCFT() == 0)
    {
        if(num_insns == 2)
        {
            return RS_UNKNOWN;
        }
        if(isTailCall(num_insns))
        {
            return RS_UNKNOWN;
        }
        if(!parsedJumpTable)
        {
            assert(0);
            return RS_UNSET;
        }
        else if(!successfullyParsedJumpTable)
        {
            return RS_UNKNOWN;
        }
            
    }
    if(isReturn())
    {
        return RS_RETURN;
    }
    return RS_UNSET;
}

bool InstructionAdapter::hasUnresolvedControlFlow(image_basicBlock* currBlk, unsigned int num_insns) const
{
    if(isDynamicCall())
    {
        return true;
    }
    if(getReturnStatus(currBlk, num_insns) == RS_UNKNOWN)
    {
        return true;
    }
    return false;
}

instPointType_t InstructionAdapter::getPointType(unsigned int num_insns,
                                      dictionary_hash<Address, std::string> *pltFuncs) const
{
    if(isBranch() && isTailCall(num_insns))
    {
        return functionExit;
    }
    if(isReturn())
    {
        return functionExit;
    }
    if(isBranch() && (*pltFuncs).defines(getCFT()))
    {
        return functionExit;
    }
    if(isBranch() &&
        !getCFT() &&
        num_insns == 2)
    {
        return functionExit;
    }
    if(isBranch())
    {
        return noneType;
    }
    if(isCall() || isDynamicCall())
    {
        if(!isRealCall())
        {
            if(simulateJump())
            {
                return noneType;
            }
            if(img->isValidAddress(getCFT())) {
                return noneType;
            }
        }       
        return callSite;
    }
    return noneType;
}

InstrumentableLevel InstructionAdapter::getInstLevel(unsigned int num_insns) const
{
    if(isBranch() &&
       getCFT() == 0)
    {
        if(num_insns == 2)
        {
            return UNINSTRUMENTABLE;
        }
        else if(isTailCall(num_insns))
        {
            return NORMAL;
        }
        else if(!parsedJumpTable)
        {
            fprintf(stderr, "expected jump table parsing attempt for insn at 0x%lx\n", current);
            assert(0);
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
