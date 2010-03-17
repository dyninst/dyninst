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
#include "slicing.h"
#include "AbslocInterface.h"

#include <deque>
#include <iostream>
#include <sstream>

using namespace Dyninst;
using namespace InstructionAPI;


bool IA_IAPI::isFrameSetupInsn(Instruction::Ptr i) const
{
    return false;
}

bool IA_IAPI::isNop() const
{
    return false;
}

bool IA_IAPI::isMovAPSTable(pdvector<std::pair< Address, EdgeTypeEnum > >& outEdges) const
{
    return false;
}

bool IA_IAPI::isTableInsn(Instruction::Ptr i) const
{
    return false;
}
        
std::map<Address, Instruction::Ptr>::const_iterator IA_IAPI::findTableInsn() const
{
    return allInsns.end();
}

struct isDone
{
    isDone(image_basicBlock* block) : m_block(block) {}
    bool operator() (Assignment::Ptr a) {
        return a->addr < m_block->firstInsnOffset() ||
                a->addr > m_block->lastInsnOffset();
    }
    image_basicBlock* m_block;
};        
bool widen(Assignment::Ptr) { return false; }
                
// This should only be called on a known indirect branch...
bool IA_IAPI::parseJumpTable(image_basicBlock* currBlk,
                             pdvector<std::pair< Address, EdgeTypeEnum> >& outEdges) const
{
    static const int jumpTableNumber = 0;
    jumpTableNumber++;
    using namespace std;
    AbsLoc thePC = AbsLoc::makePC(img->getArch());
    AssignmentConverter c(true);
    vector<Assignment::Ptr> assignments;
    c.convert(curInsn(), current, context, assignments);
    Assignment::Ptr sliceStart;
    for(vector<Assignment::Ptr>::iterator curAssign = assignments.begin();
        curAssign != assignments.end();
       ++curAssign)
    {
        cerr << "assignment in indirect branch at " << hex << current << ": " <<
                (*curAssign) << dec << endl;
        if((*curAssign)->out().contains(thePC))
        {
            sliceStart = *curAssign;
        }
    }
    if(sliceStart) {
        cerr << "found assignment to PC: " << sliceStart << endl;
    } else {
        cerr << "ERROR IN SEMANTICS: indirect branch didn't assign to PC" << endl;
        return false;
    }
    Slicer s(sliceStart, currBlk, context);
    GraphPtr jumpTableSlice = s.backwardslice(isDone(currBlk), &widen);
    stringstream filename << "jumpTableGraph" << jumpTableNumber;
    jumpTableSlice->printDOT(filename.str());
        
    parsing_printf("\tparseJumpTable returning FALSE (not implemented on POWER yet)\n");
    return false;
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


Address IA_IAPI::findThunkInBlock(image_basicBlock* curBlock, Address& thunkOffset) const
{
    return 0;
}


std::pair<Address, Address> IA_IAPI::findThunkAndOffset(image_basicBlock* start) const
{
    return std::make_pair(0, 0);
}

boost::tuple<Instruction::Ptr,
 Instruction::Ptr,
 bool> IA_IAPI::findMaxSwitchInsn(image_basicBlock *start) const
{
    return boost::make_tuple(Instruction::Ptr(), Instruction::Ptr(), false);
}
 
Address IA_IAPI::getTableAddress(Instruction::Ptr tableInsn, Address thunkOffset) const
{
    return 0;
}

bool IA_IAPI::fillTableEntries(Address thunkOffset,
                               Address tableBase,
                               unsigned tableSize,
                               unsigned tableStride,
                               pdvector<std::pair< Address, EdgeTypeEnum> >& outEdges) const
{
    return false;
}


bool IA_IAPI::computeTableBounds(Instruction::Ptr maxSwitchInsn,
                                 Instruction::Ptr branchInsn,
                                 Instruction::Ptr tableInsn,
                                 bool foundJCCAlongTaken,
                                 unsigned& tableSize,
                                 unsigned& tableStride) const
{
    return false;
}

bool IA_IAPI::isThunk() const {
    return false;
}

bool IA_IAPI::isTailCall(unsigned int) const
{
    return false;
}

bool IA_IAPI::checkEntry() const
{
    return true;
}

bool IA_IAPI::savesFP() const
{
    return false;
}

bool IA_IAPI::isStackFramePreamble(int& /*frameSize*/) const
{
    return false;
}

bool IA_IAPI::cleansStack() const
{
    return false;
}
