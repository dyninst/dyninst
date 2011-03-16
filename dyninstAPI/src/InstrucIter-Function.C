/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
#include "common/h/arch.h"
#include "InstrucIter-Function.h"
#include "function.h"
#include "image-func.h"
#include <sstream>
#include <ostream>
#include <string>
#include <algorithm>

#include "process.h"


InstrucIter makeIter(int_basicBlock* fromThis)
{
  return InstrucIter(
        fromThis->origInstance()->firstInsnAddr(),
        fromThis->origInstance()->getSize(),
        fromThis->proc());
}

std::string dumpSubIter(const InstrucIter& dumpIt)
{
  std::stringstream retVal;
  retVal << "     - " << dumpIt.begin() << ":" << dumpIt.end();
  return retVal.str();
}


void InstrucIterFunction::debugPrint() const
{
  cerr << " *** II_Function contains " << endl;
  std::transform(subIters.begin(), subIters.end(), 
		 std::ostream_iterator<std::string>(cerr, "\n"),
		 dumpSubIter);
  cerr << " *** Current address is " << **(this) << endl;
}


InstrucIterFunction::InstrucIterFunction(int_function* func) : InstrucIter()
{
  assert(func);
  std::transform(func->blocks().begin(), func->blocks().end(), std::back_inserter(subIters), makeIter);
  currentBlock = subIters.begin();
}

InstrucIterFunction::InstrucIterFunction(Address start, int_function* func) : InstrucIter()
{
  assert(func);
  std::transform(func->blocks().begin(), func->blocks().end(), std::back_inserter(subIters), makeIter);
  setCurrentAddress(start);
}

InstrucIterFunction::InstrucIterFunction(const InstrucIterFunction& ii) : InstrucIter(ii)
{
  subIters.clear();
  std::copy(ii.subIters.begin(), ii.subIters.end(), std::back_inserter(subIters));
  // We can't just copy the iterator because it's over a different range
  // so we'll find where we should be by address
  setCurrentAddress(*ii);
}

Address InstrucIterFunction::operator++()
{
  if(currentBlock != subIters.end())
  {
    // Increment if not at end of block already
    if(**currentBlock != currentBlock->end())
    {
      ++(*currentBlock);
    }
    // If at end of block (either because we were set there, or because we just
    // incremented, then go ahead to the beginning of the next block
    if(**currentBlock == currentBlock->end())
    {
      ++currentBlock;
      if(currentBlock != subIters.end())
      {
	currentBlock->setCurrentAddress(currentBlock->begin());
      }
    }
  }
  return **this;
}

Address InstrucIterFunction::operator++(int)
{
  Address ret = **this;
  ++(*this);
  return ret;
}

Address InstrucIterFunction::operator--()
{
  if(currentBlock->hasPrev())
  {
    --(*currentBlock);
  }
  else if(currentBlock != subIters.begin())
  {
    --currentBlock;
    currentBlock->setCurrentAddress(currentBlock->end());
  }
  return **this;
}

Address InstrucIterFunction::operator--(int)
{
  Address ret = **this;
  --(*this);
  return ret;
}


Address InstrucIterFunction::operator*() const
{
  if(currentBlock != subIters.end())
  {
    return **currentBlock;
  }
  else
  {
    return 0;
  }
}

instruction InstrucIterFunction::getInstruction()
{
  assert(currentBlock != subIters.end());
  return currentBlock->getInstruction();
}

instruction* InstrucIterFunction::getInsnPtr()
{
  if(currentBlock != subIters.end())
  {
    return currentBlock->getInsnPtr();
  }
  return NULL;
}

bool InstrucIterFunction::hasMore()
{
  return currentBlock != subIters.end();
}

bool InstrucIterFunction::hasPrev()
{
  return currentBlock != subIters.begin() || currentBlock->hasPrev();
}

Address InstrucIterFunction::peekNext()
{
  assert(hasMore());
  if(currentBlock->hasMore())
  {
    return currentBlock->peekNext();
  }
  else
  {
    subIterContT::iterator nextBlock = currentBlock;
    ++nextBlock;
    if(nextBlock != subIters.end())
    {
      return nextBlock->begin();
    }
  }
  return 0;
}

Address InstrucIterFunction::peekPrev()
{
  assert(hasPrev());
  if(currentBlock->hasPrev())
  {
    return currentBlock->peekPrev();
  }
  else
  {
    subIterContT::iterator prevBlock = currentBlock;
    --prevBlock;
    return prevBlock->end();
  }
}

void InstrucIterFunction::setCurrentAddress(Address a)
{
  subIterContT::iterator tmpBlock = subIters.begin();
  while(tmpBlock != subIters.end())
  {
    if(tmpBlock->containsAddress(a))
    {
      currentBlock = tmpBlock;
      currentBlock->setCurrentAddress(a);
      break;
    }
  }
  debugPrint();
}

Address InstrucIterFunction::getCurrentAddress()
{
  assert(currentBlock != subIters.end());
  return currentBlock->getCurrentAddress();
}
