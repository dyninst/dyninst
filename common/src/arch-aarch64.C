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

#warning "This file is not implemented yet."

#include "common/src/Types.h"
#include "common/src/arch-aarch64.h"

#if defined(os_vxworks)
#include "common/src/wtxKludges.h"
#endif

using namespace NS_aarch64;

unsigned int NS_aarch64::swapBytesIfNeeded(unsigned int i)
{
		assert(0);
    return i;
}

int instruction::signExtend(unsigned int i, unsigned int pos)
{
		assert(0);
    int ret;
		return ret;
}

instructUnion &instruction::swapBytes(instructUnion &i)
{
		assert(0);
    return i;
}

instruction *instruction::copy() const {
		assert(0);
    return new instruction(*this);
}

Address instruction::getTarget(Address addr) const {
		assert(0);
#if defined(os_vxworks)
    Address ret;
    // FIXME requires vxworks in Dyninst
    if (relocationTarget(addr, &ret))
        return ret;
#endif

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
void instruction::setBranchOffset(Address newOffset) {
		assert(0);
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
		assert(0);
#define CALLmatch 0x48000001 /* bl */
    
    return(isInsnType(OPmask | AALKmask, CALLmatch));
}

void instruction::setInstruction(codeBuf_t *ptr, Address) {
		assert(0);
}

void instruction::setInstruction(unsigned char *ptr, Address) {
    // We don't need the addr on this platform
    instructUnion *insnPtr = (instructUnion *)ptr;
    insn_ = *insnPtr;
}

bool instruction::isUncondBranch() const {
		assert(0);
    return isInsnType(Bmask, Bmatch);
}

bool instruction::isCondBranch() const {
		assert(0);
    return isInsnType(Bmask, BCmatch);
}

unsigned instruction::jumpSize(Address from, Address to, unsigned addr_width) {
		assert(0);
    Address disp = ABS((long)(to - from));
    return jumpSize(disp, addr_width);
}

// -1 is infinite, don't ya know.
unsigned instruction::jumpSize(Address disp, unsigned addr_width) {
		assert(0);
   return instruction::size();
}

unsigned instruction::maxJumpSize(unsigned addr_width) {
		assert(0);
   // TODO: some way to do a full-range branch
   // For now, a BRL-jump'll do.
   // plus two - store r0 and restore afterwards
   if (addr_width == 4)
      return 30*instruction::size();
   else
      return 7*instruction::size();
}

unsigned instruction::maxInterFunctionJumpSize(unsigned addr_width) {
		assert(0);
   if (addr_width == 8)
      return 7*instruction::size();
   else
      return 4*instruction::size();
}

unsigned instruction::spaceToRelocate() const {
		assert(0);
    return instruction::size();
}

bool instruction::getUsedRegs(pdvector<int> &) {
		assert(0);
	return false;
}

// A thunk is a "get PC" operation. We consider
// an instruction to be a thunk if it fulfills the following
// requirements:
//  1) It is unconditional or a "branch always" conditional
//  2) It has an offset of 4
//  3) It saves the return address in the link register
bool instruction::isThunk() const {
	assert(0);
 	return true;
}

Address instruction::getBranchOffset() const {
		assert(0);
    if (isUncondBranch()) {
        return (IFORM_LI(*this) << 2);
    }
    else if (isCondBranch()) {
        return (BFORM_BD(*this) << 2);
    }
    return 0;

}

unsigned instruction::opcode() const {
	assert(0);
  return MDFORM_OP(*this);
}
