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

#include "common/src/arch-aarch64.h"
#include "unaligned_memory_access.h"

namespace NS_aarch64 {

ATOMIC_t ATOMIC;
UNCOND_BR_t UNCOND_BR;
COND_BR_t COND_BR;

unsigned int swapBytesIfNeeded(unsigned int i)
{
	assert(0);
    return i;
}


// i = signed int value to be extended
// pos = the total length of signed value to be extended
int instruction::signExtend(unsigned int i, unsigned int pos)
{
    int ret;
    if (((i >> (--pos)) & 0x1) == 0x1) {
        ret = i |  (~0u << pos);
    } else {
        ret = i & ~(~0u << pos);
    }
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

unsigned instruction::getTargetReg() const {
    if( isBranchReg() ){
        // for this instruction, the reg contains the target address directly.
        return getBranchTargetReg();
    }
    return -1;
}

Dyninst::Address instruction::getTarget(Dyninst::Address addr) const {
    if (isUncondBranch() || isCondBranch()) {
        return getBranchOffset() + addr;
    }

    return 0;
}

// TODO: argument _needs_ to be an int, or ABS() doesn't work.
void instruction::setBranchOffset(Dyninst::Address /*newOffset*/) {
		assert(0);
}


bool instruction::isCall() const
{
		assert(0);
#define CALLmatch 0x48000001 /* bl */
    return false;
}

void instruction::setInstruction(codeBuf_t * /*ptr*/, Dyninst::Address) {
		assert(0);
}

void instruction::setInstruction(unsigned char *ptr, Dyninst::Address) {
    // We don't need the addr on this platform
    insn_ = Dyninst::read_memory_as<instructUnion>(ptr);
}

bool instruction::isBranchReg() const{
    return CHECK_INST(UNCOND_BR.REG );
}

bool instruction::isUncondBranch() const {
    if( CHECK_INST(UNCOND_BR.IMM ) == true
        || CHECK_INST(UNCOND_BR.REG ) == true
      )
        return true;

    return false;
}

bool instruction::isCondBranch() const {
    if( CHECK_INST(COND_BR.CB) == true ||
        CHECK_INST(COND_BR.BR) == true ||
        CHECK_INST(COND_BR.TB) == true )
        return true;

    return false;
}

unsigned instruction::jumpSize(Dyninst::Address /*from*/, Dyninst::Address /*to*/, unsigned /*addr_width*/) {
		assert(0);
        return -1;
}

// -1 is infinite, don't ya know.
unsigned instruction::jumpSize(Dyninst::Address /*disp*/, unsigned /*addr_width*/) {
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

bool instruction::getUsedRegs(std::vector<int> &) {
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

unsigned instruction::getBranchTargetReg() const{
    // keep sure this instruction is uncond b reg.
    assert( isUncondBranch() );
    unsigned regNum;
    if( CHECK_INST(UNCOND_BR.REG) ){
        // in this case, we should retrieve the offset from the reg
        // shift right 2 to overcome the <<2 for address values
        regNum = GET_OFFSET32(UNCOND_BR.REG)>>2;

        // be sure the reg num is in the range
        assert(regNum <= 30);

        return regNum;
    }
    return -1;
}

Dyninst::Address instruction::getBranchOffset() const {
    if (isUncondBranch()) {
        if( CHECK_INST(UNCOND_BR.IMM) ){
            return signExtend(GET_OFFSET32(UNCOND_BR.IMM), 26+2 );
        }
        if( CHECK_INST(UNCOND_BR.REG) ){
            // branch reg doesn't return offset.
            assert(0);
        }
    }
    else if (isCondBranch()) {
        if( CHECK_INST(COND_BR.CB) ){
            return signExtend(GET_OFFSET32(COND_BR.CB),19+2 );
        }
        if( CHECK_INST(COND_BR.TB) ){
            return signExtend(GET_OFFSET32(COND_BR.TB),14+2 );
        }
        if( CHECK_INST(COND_BR.BR) ){
            return signExtend(GET_OFFSET32(COND_BR.BR),19+2 );
        }
    }
    assert(0); //never goes here.
    return 0;

}

unsigned instruction::opcode() const {
	assert(0);
    return -1;
}

bool instruction::isAtomicLoad() const {
    if( CHECK_INST(ATOMIC.LD) == true)
        return true;
    return false;
}

bool instruction::isAtomicStore() const {
    if( CHECK_INST(ATOMIC.ST) == true)
        return true;
    return false;
}

} // namespace NS_aarch64
