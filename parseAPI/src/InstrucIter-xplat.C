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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "common/h/Types.h"
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/arch.h"

#include "parseAPI/h/CodeObject.h"
#include "parseAPI/h/CodeSource.h"
#include "parseAPI/h/CFG.h"

#include "InstrucIter.h"

// FIXME InstrucIter-mandated polution
#include "dyninstAPI/src/legacy-instruction.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

void InstrucIter::initializeInsn() {
  if (!instructions_ || !instructions_->isValidAddress(current))
  {
    instPtr = NULL;
    return;
  }

  instPtr = instructions_->getPtrToInstruction(current);
  if (!instPtr)
     return;

  // ARGH 64-bit/32-bit...
#if defined(arch_x86_64)
  ia32_set_mode_64(instructions_->getAddressWidth() == 8);
#endif

  if (instPtr) 
    insn.setInstruction((codeBuf_t *)instPtr, current);

  // If there's no pointer we have an invalid instrucIter... however,
  // this can happen if you're using logic like "while hasMore()... iter++"
  // so we allow it. If the value gets used then choke.
}

/** copy constructor
 * @param ii InstrucIter to copy
 */
InstrucIter::InstrucIter(const InstrucIter& ii) :
  instructions_(ii.instructions_),
  base(ii.base),
  range(ii.range),
  current(ii.current)
{
#if defined(arch_x86) || defined(arch_x86_64) // arch_has_variable_length_insns...
  std::copy(ii.prevInsns.begin(), ii.prevInsns.end(), std::back_inserter(prevInsns));
#endif
  initializeInsn();
}

InstrucIter::InstrucIter(Block *b) :
  base(b->start()),
  range(b->size()),
  current(base)
{
  assert(current >= base);
  // The range might be 0.
  if (range) {
    if (current >= (base+range)) {
      fprintf(stderr, "Error: current 0x%p >= 0x%p (0x%p + 0x%x)\n",
	      (void *)current, (void*)(base+range), (void *)base, range);
      assert(current < base+range);
    }
  }

  instructions_ = b->region();
  initializeInsn();
}

InstrucIter::InstrucIter( Address addr, InstructionSource *source) :
  instructions_(source),
  current(addr)
{
  // There's all sorts of reasons to iterate over the middle of nowhere;
  // we may be grabbing the PLT or new code. On the other hand, nobody knows
  // what the range is.
  // Did I mention this is dangerous?
  base = addr;
  range = 0;

  initializeInsn();
}

// And truly generic
InstrucIter::InstrucIter( Address addr, Address size, InstructionSource *source) :
  instructions_(source),
  base(addr),
  range(size),
  current(addr)
{
  assert(current >= base);
  assert(current < base+range);
  initializeInsn();
}


bool InstrucIter::hasMore()
{
  if (instPtr == NULL) return false;
  
  if (range == 0)
      return true; // Unsafe iteration, but there is more
  
  if((current < (base + range )) &&
     (current >= base))
    return true;
  return false;
}

bool InstrucIter::hasPrev()
{
  if (instPtr == NULL) return false;
#if defined(arch_x86) || defined(arch_x86_64) // arch_has_variable_length_insns...
  if (prevInsns.size() == 0) 
    // There is more, but we can't access...
    return false;
#endif
    
  if( current > base && instructions_->isValidAddress(peekPrev()))
    //if((current < (baseAddress + range )) &&
    // (current > baseAddress))
    return true;

  return false;
}

Address InstrucIter::operator*() const
{
  return current;
}

// Postfix...
Address InstrucIter::operator++(int)
{
  // Removed to be consistent with our prefix guard clauses (or lack thereof)
  //assert(instPtr);
  Address retVal = **this;
  ++(*this);
  return retVal;
}

// Postfix...
Address InstrucIter::operator--(int)
{
  Address retVal = **this;
  --(*this);
  return retVal;
}

Address InstrucIter::operator++()
{
#if defined(arch_x86) || defined(arch_x86_64) // arch_has_variable_length_insns...
  prevInsns.push_back(std::make_pair(current, instPtr));
#endif
  //  assert(instructions_ && instructions_->isValidAddress(peekNext()));  
  current = peekNext();
  initializeInsn();
  return **this;
}

Address InstrucIter::operator--()
{
#if defined(arch_x86) || defined(arch_x86_64)
  if(hasPrev())
  {
    //assert(instructions_ && instructions_->isValidAddress(peekPrev()));
      current = peekPrev();
    instPtr = prevInsns.back().second;
    prevInsns.pop_back();
  }
#else
  current = peekPrev();
#endif
  initializeInsn();
  return **this;
}

BPatch_instruction *InstrucIter::getBPInstruction() {

  BPatch_memoryAccess *ma = isLoadOrStore();

  if (ma != BPatch_memoryAccess::none)
    return ma;

  instruction *i = getInsnPtr();
  return new BPatch_instruction(new internal_instruction(i), current);
}

bool InstrucIter::containsAddress(Address addr) { 
  if (range == 0)
     return true;

   return ((addr >= base) && 
           (addr < (base + range))); 
}
