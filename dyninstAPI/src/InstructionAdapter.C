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
#include "debug.h"
#include "symtab.h"


InstructionAdapter::InstructionAdapter(Address start, image_func* f)
    : current(start), previous(-1), parsedJumpTable(false), successfullyParsedJumpTable(false),
    isDynamicCall_(false), checkedDynamicCall_(false),
    isInvalidCallTarget_(false), checkedInvalidCallTarget_(false),
    context(f), img(f->img())
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
                                          std::vector<instruction>& all_insns) const
{
    if(isReturn())
    {
        return RS_RETURN;
    }
    // Branch that's not resolvable by binding IP,
    // therefore indirect...
    if(isBranch() &&
       getCFT() == 0)
    {
        if(all_insns.size() == 2)
        {
            return RS_UNKNOWN;
        }
        if(isTailCall(all_insns))
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
    return RS_UNSET;
}

bool InstructionAdapter::hasUnresolvedControlFlow(image_basicBlock* currBlk, std::vector<instruction>& all_insns) const
{
    if(isDynamicCall())
    {
        return true;
    }
    if(getReturnStatus(currBlk, all_insns) == RS_UNKNOWN)
    {
        return true;
    }
    return false;
}

instPointType_t InstructionAdapter::getPointType(std::vector<instruction>& all_insns,
                                      dictionary_hash<Address, std::string> *pltFuncs) const
{
    if(isReturn())
    {
        return functionExit;
    }
    if(isTailCall(all_insns))
    {
        return functionExit;
    }
    if((*pltFuncs).defines(getCFT()))
    {
        return functionExit;
    }
    if(isBranch() &&
        !getCFT() &&
        all_insns.size() == 2)
    {
        return functionExit;
    }
    if(isCall())
    {
        if(!isRealCall())
        {
            if(simulateJump())
            {
                return noneType;
            }
            if(context->img()->isValidAddress(getCFT())) {
                return noneType;
            }
        }       
        return callSite;
    }
    return noneType;
}

InstrumentableLevel InstructionAdapter::getInstLevel(std::vector<instruction>& all_insns) const
{
    if(isBranch() &&
       getCFT() == 0)
    {
        if(all_insns.size() == 2)
        {
            return UNINSTRUMENTABLE;
        }
        else if(isTailCall(all_insns))
        {
            return NORMAL;
        }
        else if(!parsedJumpTable)
        {
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
