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
#include "../../symEval/h/slicing.h"
#include "../../symEval/h/AbslocInterface.h"
#include "Graph.h"

#include <deque>
#include <iostream>
#include <sstream>
#include <functional>
#include <algorithm>

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

bool affectsKnownInput(Assignment::Ptr a) {
  Absloc toc(ppc32::r2);
  for(std::vector<AbsRegion>::iterator i = a->inputs().begin();
      i != a->inputs().end();
      ++i)
    {
      if(i->contains(toc)) return true;
    }
  if(a->out().contains(Absloc::Heap)) return true;
  return false;
}

struct isDone
{
  isDone(image_func* f, image_basicBlock* b) : func(f), block(b) {}
  bool operator()(Assignment::Ptr a)
  {
    return (a->addr() <= block->firstInsnOffset()) ||
      (a->addr() > block->lastInsnOffset());
  }
  image_func* func;
  image_basicBlock* block;
};

bool no(Assignment::Ptr) { return false; }

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


template <typename _output_iterator>
GraphPtr sliceOnPC(image_basicBlock* block,
	       image_func* const func,
	       const Instruction::Ptr& insn,
	       Address addr,
	       _output_iterator inputs_to_slice)
{
  using namespace std;
  Absloc thePC = Absloc::makePC(func->img()->getArch());
  AssignmentConverter c(true);
  vector<Assignment::Ptr> assignments;
  c.convert(insn, addr, func, assignments);
  Assignment::Ptr sliceStart;
  for(vector<Assignment::Ptr>::iterator curAssign = assignments.begin();
      curAssign != assignments.end();
      ++curAssign) {
    if((*curAssign)->out().contains(thePC)) {
      sliceStart = *curAssign;
    }
  }
  if(sliceStart) {
    cerr << "found assignment to PC: " << sliceStart << " ";
    for(std::vector<AbsRegion>::iterator curInput = sliceStart->inputs().begin();
	curInput != sliceStart->inputs().end();
	++curInput) {
      cerr << *curInput << " ";
    }
    cerr << endl;
  } else {
    cerr << "ERROR IN SEMANTICS: indirect branch didn't assign to PC" << endl;
    return GraphPtr();
  }
  cerr << "slicing within block " << hex << block->firstInsnOffset() << "..." <<
    block->lastInsnOffset() << endl;
  Slicer s(sliceStart, block, func);
  GraphPtr pcSlice = s.backwardSlice(&affectsKnownInput, isDone(func, block));
  NodeIterator entryBegin, entryEnd;
  pcSlice->entryNodes(entryBegin, entryEnd);
  while(entryBegin != entryEnd) {
    Assignment::Ptr currAssign = 
      dyn_detail::boost::dynamic_pointer_cast<AssignNode>(*entryBegin)->assign();
    if(currAssign) {
      for(std::vector<AbsRegion>::const_iterator curInput = currAssign->inputs().begin();
	  curInput != currAssign->inputs().end();
	  ++curInput) {
	*(inputs_to_slice++) = *curInput;
      }
    }
    entryBegin++;
  }
  return pcSlice;
}


// This should only be called on a known indirect branch...
bool IA_IAPI::parseJumpTable(image_basicBlock* currBlk,
                             pdvector<std::pair< Address, EdgeTypeEnum> >& outEdges) const
{
  // we only try to parse guarded jump tables; this means that we should reach this block via a conditional branch.
  pdvector<image_edge*> srcs;
  currBlk->getSources(srcs);
  if(srcs.size() != 1) {
    cerr << "multiple source blocks for block at " << hex << current << dec << ", skipping" << endl;
    return false;
  }
  static int jumpTableNumber = 0;
  jumpTableNumber++;
  image_basicBlock* predecessor = srcs[0]->getSource();
  cerr << "predecessor block to jump table candidate " << jumpTableNumber << ": " << hex 
       << predecessor->firstInsnOffset() << "..." 
       << predecessor->lastInsnOffset() << dec << endl;

  stringstream filename;
  filename << "jumpTableGraph" << jumpTableNumber;
  std::set<AbsRegion> inputs;
  GraphPtr jumpTableSlice = sliceOnPC(currBlk, context, curInsn(), current, inserter(inputs, inputs.begin()));
  jumpTableSlice->printDOT(filename.str());
  /*  std::map<Address, Instruction::Ptr>::const_iterator found = allInsns.find(predecessor->lastInsnOffset());
  
  if(found != allInsns.end()) {
    cerr << "predecessor block not parsed...WTF?" << endl;
    return false;
  }
  Instruction::Ptr predEnd = found->second;
  assert(predEnd);
  GraphPtr dominatorSlice = sliceOnPC(predecessor, context, predEnd, predecessor->lastInsnOffset(),
  inserter(inputs, inputs.begin()));*/
  fprintf(stderr, "Candidate jump table at 0x%lx, inputs are:\n", current);
  for(std::set<AbsRegion>::iterator curInput = inputs.begin();
      curInput != inputs.end();
      ++curInput) {
    cerr << "\t" << curInput->format() << endl;
  }
  parsing_printf("\tparseJumpTable returning FALSE (not implemented on POWER yet)\n");
  return false;
}


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
