
#include "common/h/Types.h"
#include "arch.h"
#include "InstrucIter-Function.h"
#include "function.h"
#include "image-func.h"
#include <sstream>
#include <ostream>
#include <string>
#include <algorithm>


InstrucIter makeIter(int_basicBlock* fromThis)
{
  return InstrucIter(fromThis);
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
