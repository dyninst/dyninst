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

#include "common/src/arch-power.h"
#include "unaligned_memory_access.h"
#include <cassert>
using namespace NS_power;

unsigned int NS_power::swapBytesIfNeeded(unsigned int i)
{
    static int one = 0x1;

    if ( *((unsigned char *)&one) )
        return instruction::swapBytes( *((instructUnion *)&i) ).raw;

    return i;
}

int instruction::signExtend(unsigned int i, unsigned int pos)
{
    int ret;
    if (((i >> (--pos)) & 0x1) == 0x1) {
        ret = i |  (~0U << pos);
    } else {
        ret = i & ~(~0U << pos);
    }

     return ret;
}

instructUnion &instruction::swapBytes(instructUnion &i)
{
    unsigned char tmp = i.byte[0];
    i.byte[0] = i.byte[3];
    i.byte[3] = tmp;

    tmp = i.byte[1];
    i.byte[1] = i.byte[2];
    i.byte[2] = tmp;

    return i;
}

instruction *instruction::copy() const {
    return new instruction(*this);
}

Dyninst::Address instruction::getTarget(Dyninst::Address addr) const {
    if (isUncondBranch() || isCondBranch()) {
        return getBranchOffset() + addr;
    }
    else if (isInsnType(Bmask, BAAmatch)) // Absolute
        return (IFORM_LI(*this) << 2);
    else if (isInsnType(Bmask, BCAAmatch)) // Absolute
        return (BFORM_BD(*this) << 2);

    return 0;
}

// TODO: argument _needs_ to be an int, or ABS() doesn't work.
void instruction::setBranchOffset(Dyninst::Address newOffset) {
    if (isUncondBranch()) {
        assert(ABS((int) newOffset) < MAX_BRANCH);
        IFORM_LI_SET(*this, newOffset >> 2);
    }
    else if (isCondBranch()) {
        assert(ABS(newOffset) < MAX_CBRANCH);
        BFORM_BD_SET(*this, newOffset >> 2);
    }
    else {
        assert(0);
    }
}


bool instruction::isCall() const
{
#define CALLmatch 0x48000001 /* bl */
    
    // Only look for 'bl' instructions for now, although a branch
    // could be a call function, and it doesn't need to set the link
    // register if it is the last function call
    return(isInsnType(OPmask | AALKmask, CALLmatch));
}

void instruction::setInstruction(codeBuf_t *ptr, Dyninst::Address) {
    // We don't need the addr on this platform

    instructUnion *insnPtr = (instructUnion *)ptr;
    insn_.raw = (*insnPtr).raw;
}
void instruction::setInstruction(unsigned char *ptr, Dyninst::Address) {
    // We don't need the addr on this platform
    insn_ = Dyninst::read_memory_as<instructUnion>(ptr);
}

bool instruction::isUncondBranch() const {
    return isInsnType(Bmask, Bmatch);
}

bool instruction::isCondBranch() const {
    return isInsnType(Bmask, BCmatch);
}

unsigned instruction::jumpSize(Dyninst::Address from, Dyninst::Address to, unsigned addr_width) {
    Dyninst::Address disp = ABS((long)(to - from));
    return jumpSize(disp, addr_width);
}

// -1 is infinite, don't ya know.
unsigned instruction::jumpSize(Dyninst::Address disp, unsigned addr_width) {
   if (ABS(disp) >= MAX_BRANCH) {
      return maxInterFunctionJumpSize(addr_width);
   }
   return instruction::size();
}

unsigned instruction::maxJumpSize(unsigned addr_width) {
   // TODO: some way to do a full-range branch
   // For now, a BRL-jump'll do.
   // plus two - store r0 and restore afterwards
   if (addr_width == 4)
      return 30*instruction::size();
   else
      return 7*instruction::size();
}

unsigned instruction::maxInterFunctionJumpSize(unsigned addr_width) {
   // 4 for 32-bit...
   // move <high>, r0
   // move <low>, r0
   // move r0 -> ctr
   // branch to ctr
   
   // 7 for 64-bit...
   // move <top-high>, r0
   // move <top-low>, r0
   // lshift r0, 32
   // move <bot-high>, r0
   // move <bot-low>, r0
   // move r0 -> ctr
   // branch to ctr
   if (addr_width == 8)
      return 7*instruction::size();
   else
      return 4*instruction::size();
}

unsigned instruction::spaceToRelocate() const {

    // We currently assert instead of fixing out-of-range
    // branches. In the spirit of "one thing at a time",
    // we'll handle that _later_.

    // Actually, since conditional branches have such an abysmally
    // short range, we _do_ handle moving them through a complicated
    // "jump past an unconditional branch" combo.
    
  if (isThunk()) {
    // Load high; load low; move to LR
    return 30*instruction::size();
  }
  else if (isCondBranch()) {
        // Maybe... so worst-case
        if ((BFORM_BO(*this) & BALWAYSmask) != BALWAYScond) {
            return 3*instruction::size();
        }
    }
    if (isUncondBranch()) {
        // Worst case... branch to LR
        // and save/restore r0
	//
	// Upgraded from 6 to 9 for 64-bit
        return 9*instruction::size();
    }
    return instruction::size();
}

bool instruction::getUsedRegs(std::vector<int> &) {
	return false;
}

// A thunk is a "get PC" operation. We consider
// an instruction to be a thunk if it fulfills the following
// requirements:
//  1) It is unconditional or a "branch always" conditional
//  2) It has an offset of 4
//  3) It saves the return address in the link register
bool instruction::isThunk() const {
  switch (BFORM_OP(*this)) {
  case Bop:
    // Unconditional branch, do nothing
    break;
  case BCop:
    // Must be an "always" condition
    if (!(BFORM_BO(*this) & 0x14))
      return false;
    break;
  default:
    return false;
    break;
  }

  // 2
  // The displacement is always right shifted 2 (because you can't
  // jump to an unaligned address) so we can check if the displacement
  // encoded is 1...
  if (BFORM_BD(*this) != 1) return false;

  // 3
  if (!BFORM_LK(*this)) return false;

  // Oh, and it better not be an absolute...
  if (BFORM_AA(*this)) return false;

  return true;
}

Dyninst::Address instruction::getBranchOffset() const {
    if (isUncondBranch()) {
        return (IFORM_LI(*this) << 2);
    }
    else if (isCondBranch()) {
        return (BFORM_BD(*this) << 2);
    }
    return 0;

}

unsigned instruction::opcode() const {
  return MDFORM_OP(*this);
}
