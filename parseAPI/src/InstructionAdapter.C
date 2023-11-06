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

//#include "arch.h"
#include "InstructionAdapter.h"
#include "debug_parse.h"

#include "parseAPI/h/CodeObject.h"
#include "boost/tuple/tuple.hpp"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::InsnAdapter;
using namespace Dyninst::ParseAPI;

InstructionAdapter::InstructionAdapter(Address start, CodeObject *o, CodeRegion * r, InstructionSource * isrc, Block * curBlk)
    : current(start), previous((Address)-1), parsedJumpTable(false), successfullyParsedJumpTable(false),
    isDynamicCall_(false), checkedDynamicCall_(false),
    isInvalidCallTarget_(false), checkedInvalidCallTarget_(false),
    //context(NULL), 
    _obj(o), _cr(r), _isrc(isrc), _curBlk(curBlk)
{
}

void
InstructionAdapter::reset(
    Address start,
    CodeObject *o,
    CodeRegion *r,
    InstructionSource *isrc,
    Block *curBlk)
{
    current = start;
    previous = (Address)-1;    
    parsedJumpTable = false;
    successfullyParsedJumpTable = false;
    isDynamicCall_ = false;
    checkedDynamicCall_ = false;
    isInvalidCallTarget_ = false;
    checkedInvalidCallTarget_ = false;
    _obj = o;
    _cr = r;
    _isrc = isrc;
    _curBlk = curBlk;
}


void InstructionAdapter::advance()
{
    previous = current;
    parsedJumpTable = false;
    successfullyParsedJumpTable = false;
    checkedDynamicCall_ = false;
    checkedInvalidCallTarget_ = false;

}

bool InstructionAdapter::retreat()
{
    // anything? FIXME
    return false;
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

FuncReturnStatus InstructionAdapter::getReturnStatus(Function * context ,
        unsigned int num_insns) const
{
    // Branch that's not resolvable by binding IP,
    // therefore indirect...
   bool valid; Address addr;
   boost::tie(valid, addr) = getCFT();
   if(isBranch() && !valid)
    {
        if(num_insns == 2)
        {
            return UNKNOWN;
        }
        if(isTailCall(context, INDIRECT, num_insns, set<Address>()))
        {
            return UNKNOWN;
        }
        if(!parsedJumpTable)
        {
            return UNSET;
        }
        else if(!successfullyParsedJumpTable)
        {
            return UNKNOWN;
        }
            
    }
    if(isReturn(context, _curBlk))
    {
        return RETURN;
    }
    return UNSET;
}

/* Returns true for indirect calls and unresolved indirect branches 
 */
bool InstructionAdapter::hasUnresolvedControlFlow(Function* context, unsigned int num_insns) const
{
    if(isDynamicCall())
    {
        return true;
    }
    if(getReturnStatus(context, num_insns) == UNKNOWN)
    { // true for indirect branches
        return true;
    }
    return false;
}

InstrumentableLevel InstructionAdapter::getInstLevel(Function * context, unsigned int num_insns) const
{
   bool valid; Address target;
   boost::tie(valid, target) = getCFT();
   if(isBranch() && !valid)
    {
        if(num_insns == 2)
        {
            return UNINSTRUMENTABLE;
        }
        else if(isTailCall(context, INDIRECT, num_insns, set<Address>()))
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
