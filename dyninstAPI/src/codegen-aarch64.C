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

#include "dyninstAPI/src/codegen.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/registerSpace.h"
#include "dyninstAPI/src/addressSpace.h"
#include "dyninstAPI/src/inst-aarch64.h"
#include "dyninstAPI/src/emit-aarch64.h"
#include "dyninstAPI/src/function.h"

#if defined(os_vxworks)
#include "common/src/wtxKludges.h"
#endif


// "Casting" methods. We use a "base + offset" model, but often need to
// turn that into "current instruction pointer".
codeBuf_t *insnCodeGen::insnPtr(codeGen &gen) {
    assert(0);
    return (instructUnion *)gen.cur_ptr();
}

#if 0
// Same as above, but increment offset to point at the next insn.
codeBuf_t *insnCodeGen::ptrAndInc(codeGen &gen) {
  // MAKE SURE THAT ret WILL STAY VALID!
  gen.realloc(gen.used() + sizeof(instruction));

  instructUnion *ret = insnPtr(gen);
  gen.moveIndex(instruction::size());
  return ret;
}
#endif

void insnCodeGen::generate(codeGen &gen, instruction&insn) {
    assert(0);
#if defined(endian_mismatch)
  // Writing an instruction.  Convert byte order if necessary.
  unsigned raw = swapBytesIfNeeded(insn.asInt());
#else
  unsigned raw = insn.asInt();
#endif

  gen.copy(&raw, sizeof(unsigned));
}

void insnCodeGen::generateIllegal(codeGen &gen) { // instP.h
    assert(0);
    instruction insn;
    generate(gen,insn);
}

void insnCodeGen::generateTrap(codeGen &gen) {
    assert(0);
    instruction insn(BREAK_POINT_INSN);
    generate(gen,insn);
}

void insnCodeGen::generateBranch(codeGen &gen, long disp, bool link)
{
    assert(0);
}

void insnCodeGen::generateBranch(codeGen &gen, Address from, Address to, bool link) {
    assert(0);

    long disp = (to - from);

    if (ABS(disp) > MAX_BRANCH) {
        return generateLongBranch(gen, from, to, link);
    }

    return generateBranch(gen, disp, link);

}

void insnCodeGen::generateCall(codeGen &gen, Address from, Address to) {
    assert(0);
    generateBranch(gen, from, to, true);
}

void insnCodeGen::generateInterFunctionBranch(codeGen &gen,
                                              Address from,
                                              Address to,
                                              bool link) {
    assert(0);
}

void insnCodeGen::generateLongBranch(codeGen &gen,
                                     Address from,
                                     Address to,
                                     bool isCall) {
    assert(0);
}

void insnCodeGen::generateBranchViaTrap(codeGen &gen, Address from, Address to, bool isCall) {
    assert(0);
}

void insnCodeGen::generateAddReg (codeGen & gen, int op, Register rt,
				   Register ra, Register rb)
{
    assert(0);
}

void insnCodeGen::generateLoadReg(codeGen &gen, Register rt,
                                  Register ra, Register rb)
{
    assert(0);
//#warning "This function is not implemented yet!"
}

void insnCodeGen::generateStoreReg(codeGen &gen, Register rt,
                                   Register ra, Register rb)
{
    assert(0);
//#warning "This function is not implemented yet!"
}

void insnCodeGen::generateLoadReg64(codeGen &gen, Register rt,
                                    Register ra, Register rb)
{
assert(0);
//#warning "This function is not implemented yet!"
}

void insnCodeGen::generateStoreReg64(codeGen &gen, Register rs,
                                     Register ra, Register rb)
{
assert(0);
//#warning "This function is not implemented yet!"
}

void insnCodeGen::generateImm(codeGen &gen, int op, Register rt, Register ra, int immd)
 {
assert(0);
  // something should be here to make sure immd is within bounds
  // bound check really depends on op since we have both signed and unsigned
  //   opcodes.
  // We basically check if the top bits are 0 (unsigned, or positive signed)
  // or 0xffff (negative signed)
  // This is because we don't enforce calling us with LOW(immd), and
  // signed ints come in with 0xffff set. C'est la vie.
  // TODO: This should be a check that the high 16 bits are equal to bit 15,
  // really.
//#warning "This function is not implemented yet!"
}

void insnCodeGen::generateMemAccess64(codeGen &gen, int op, int xop, Register r1, Register r2, int immd)
{
assert(0);
//#warning "This function is not implemented yet!"
}

// rlwinm ra,rs,n,0,31-n
void insnCodeGen::generateLShift(codeGen &gen, Register rs, int shift, Register ra)
{
assert(0);
//#warning "This function is not implemented yet!"
}

// rlwinm ra,rs,32-n,n,31
void insnCodeGen::generateRShift(codeGen &gen, Register rs, int shift, Register ra)
{
assert(0);
//#warning "This function is not implemented yet!"
}

// sld ra, rs, rb
void insnCodeGen::generateLShift64(codeGen &gen, Register rs, int shift, Register ra)
{
assert(0);
//#warning "This function is not implemented yet!"
}

// srd ra, rs, rb
void insnCodeGen::generateRShift64(codeGen &gen, Register rs, int shift, Register ra)
{
assert(0);
//not implemented
}

//
// generate an instruction that does nothing and has to side affect except to
//   advance the program counter.
//
void insnCodeGen::generateNOOP(codeGen &gen, unsigned size)
{
assert(0);
//#warning "This function is not implemented yet!"
}

void insnCodeGen::generateSimple(codeGen &gen, int op,
                                 Register src1, Register src2,
                                 Register dest)
{
assert(0);
//#warning "This function is not implemented yet!"
}

void insnCodeGen::generateRelOp(codeGen &gen, int cond, int mode, Register rs1,
                                Register rs2, Register rd)
{
assert(0);
//#warning "This function is not implemented yet!"
}

// Given a value, load it into a register.
void insnCodeGen::loadImmIntoReg(codeGen &gen, Register rt, long value)
{
assert(0);
//#warning "This function is not implemented yet!"
}

// Helper method.  Fills register with partial value to be completed
// by an operation with a 16-bit signed immediate.  Such as loads and
// stores.
void insnCodeGen::loadPartialImmIntoReg(codeGen &gen, Register rt, long value)
{
assert(0);
//#warning "This function is not implemented yet!"
}

int insnCodeGen::createStackFrame(codeGen &gen, int numRegs, pdvector<Register>& freeReg, pdvector<Register>& excludeReg){
assert(0);
//#warning "This function is not implemented yet!"
		return freeReg.size();
}

void insnCodeGen::removeStackFrame(codeGen &gen) {
assert(0);
//#warning "This function is not implemented yet!"
}

bool insnCodeGen::generate(codeGen &gen,
                           instruction &insn,
                           AddressSpace * /*proc*/,
                           Address origAddr,
                           Address relocAddr,
                           patchTarget *fallthroughOverride,
                           patchTarget *targetOverride) {
assert(0);
//#warning "This function is not implemented yet!"
  assert(0 && "Deprecated!");
  return false;
#if 0
    assert(fallthroughOverride == NULL);

    Address targetAddr = targetOverride ? targetOverride->get_address() : 0;
    long newOffset = 0;
    Address to;

    if (insn.isThunk()) {
    }
    else if (insn.isUncondBranch()) {
        // unconditional pc relative branch.

#if defined(os_vxworks)
        if (!targetOverride) relocationTarget(origAddr, &targetAddr);
#endif

        // This was a check in old code. Assert it isn't the case,
        // since this is a _conditional_ branch...
        assert(insn.isInsnType(Bmask, BCAAmatch) == false);

        // We may need an instPoint for liveness calculations

        instPoint *point = gen.func()->findInstPByAddr(origAddr);
        if (!point)
            point = instPoint::createArbitraryInstPoint(origAddr,
                                                        gen.addrSpace(),
                                                        gen.func());
        gen.setPoint(point);


        if (targetAddr) {
            generateBranch(gen,
                           relocAddr,
                           targetAddr,
                           IFORM_LK(insn));
        }
        else {
            generateBranch(gen,
                           relocAddr,
                           insn.getTarget(origAddr),
                           IFORM_LK(insn));
        }
    }
    else if (insn.isCondBranch()) {
        // conditional pc relative branch.
#if defined(os_vxworks)
        if (!targetOverride) relocationTarget(origAddr, &targetAddr);
#endif

        if (!targetAddr) {
          newOffset = origAddr - relocAddr + insn.getBranchOffset();
          to = origAddr + insn.getBranchOffset();
        } else {
	  newOffset = targetAddr - relocAddr;
          to = targetAddr;
        }
    }
    else {
#if defined(os_vxworks)
        if (relocationTarget(origAddr + 2, &targetAddr)) DFORM_SI_SET(insn, targetAddr);
#endif
        generate(gen,insn);
    }
    return true;
#endif
}

bool insnCodeGen::generateMem(codeGen &,
                              instruction&,
                              Address,
                              Address,
                              Register,
                  Register) {
assert(0);
//#warning "This function is not implemented yet!"
return false; }

void insnCodeGen::generateMoveFromLR(codeGen &gen, Register rt) {
assert(0);
//#warning "This function is not implemented yet!"
}

void insnCodeGen::generateMoveToLR(codeGen &gen, Register rs) {
assert(0);
//#warning "This function is not implemented yet!"
}
void insnCodeGen::generateMoveToCR(codeGen &gen, Register rs) {
assert(0);
//#warning "This function is not implemented yet!"
}

bool insnCodeGen::modifyJump(Address target,
			     NS_aarch64::instruction &insn,
			     codeGen &gen) {
assert(0);
//#warning "This function is not implemented yet!"
  return true;
}

bool insnCodeGen::modifyJcc(Address target,
			    NS_aarch64::instruction &insn,
			    codeGen &gen) {
assert(0);
//#warning "This function is not implemented yet!"
  return false;
}

bool insnCodeGen::modifyCall(Address target,
			     NS_aarch64::instruction &insn,
			     codeGen &gen) {
assert(0);
//#warning "This function is not implemented yet!"
    return false;
}

bool insnCodeGen::modifyData(Address target,
			     NS_aarch64::instruction &insn,
			     codeGen &gen) {
assert(0);
//#warning "This function is not implemented yet!"
  return false;
}

