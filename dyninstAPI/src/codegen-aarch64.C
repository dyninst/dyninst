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
#if defined(endian_mismatch)
  // Writing an instruction.  Convert byte order if necessary.
  unsigned raw = swapBytesIfNeeded(insn.asInt());
#else
  unsigned raw = insn.asInt();
#endif

  gen.copy(&raw, sizeof(unsigned));
}

void insnCodeGen::generateIllegal(codeGen &gen) { // instP.h
    instruction insn;
    generate(gen,insn);
}

void insnCodeGen::generateTrap(codeGen &gen) {
    instruction insn(BREAK_POINT_INSN);
    generate(gen,insn);
}

void insnCodeGen::generateBranch(codeGen &gen, long disp, bool link) {
    if (ABS(disp) > MAX_BRANCH_OFFSET) {
        fprintf(stderr, "ABS OFF: 0x%lx, MAX: 0x%lx\n",
                ABS(disp), (unsigned long) MAX_BRANCH_OFFSET);
        bperr( "Error: attempted a branch of 0x%lx\n", disp);
        logLine("a branch too far\n");
        showErrorCallback(52, "Internal error: branch too far");
        bperr( "Attempted to make a branch of offset 0x%lx\n", disp);
        assert(0);
    }

    instruction insn;
    INSN_SET(insn, 26, 30, BOp);
    //Set the displacement immediate
    INSN_SET(insn, 0, 25, disp >> 2);

    //Bit 31 is set if it's a branch-and-link (essentially, a call), unset if it's just a branch
    if(link)
        INSN_SET(insn, 31, 31, 1);
    else
        INSN_SET(insn, 31, 31, 0);

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateBranch(codeGen &gen, Address from, Address to, bool link) {
    long disp = (to - from);

    if (ABS(disp) > MAX_BRANCH_OFFSET) {
        generateLongBranch(gen, from, to, link);
    }

    generateBranch(gen, disp, link);

}

void insnCodeGen::generateCall(codeGen &gen, Address from, Address to) {
    generateBranch(gen, from, to, true);
}

void insnCodeGen::generateLongBranch(codeGen &gen,
                                     Address from,
                                     Address to,
                                     bool isCall) {
    assert(0);
}

void insnCodeGen::generateBranchViaTrap(codeGen &gen, Address from, Address to, bool isCall) {
    long disp = to - from;
    if (ABS(disp) <= MAX_BRANCH_OFFSET) {
        // We shouldn't be here, since this is an internal-called-only func.
        generateBranch(gen, disp, isCall);
    }

    assert (!isCall); // Can't do this yet

    if (gen.addrSpace()) {
        // Too far to branch.  Use trap-based instrumentation.
        gen.addrSpace()->trapMapping.addTrapMapping(from, to, true);
        insnCodeGen::generateTrap(gen);
    } else {
        // Too far to branch and no proc to register trap.
        fprintf(stderr, "ABS OFF: 0x%lx, MAX: 0x%lx\n",
                ABS(disp), (unsigned long) MAX_BRANCH_OFFSET);
        bperr( "Error: attempted a branch of 0x%lx\n", disp);
        logLine("a branch too far\n");
        showErrorCallback(52, "Internal error: branch too far");
        bperr( "Attempted to make a branch of offset 0x%lx\n", disp);
        assert(0);
    }
}

void insnCodeGen::generateAddReg (codeGen & gen, int op, Register rt,
				   Register ra, Register rb)
{
    assert(0);
    instruction insn;
    insn.clear();

    insnCodeGen::generate(gen, insn);
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
void insnCodeGen::generateNOOP(codeGen &gen, unsigned size) {
    assert((size % instruction::size()) == 0);
    while (size) {
        instruction insn(NOOP);
        insnCodeGen::generate(gen, insn);
        size -= instruction::size();
    }
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
                           Address /*origAddr*/,
                           Address /*relocAddr*/,
                           patchTarget */*fallthroughOverride*/,
                           patchTarget */*targetOverride*/) {
  assert(0 && "Deprecated!");
  return false;
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
    long disp = target - gen.currAddr();
    if (ABS(disp) > MAX_BRANCH_OFFSET) {
        generateBranchViaTrap(gen, gen.currAddr(), target, INSN_GET_ISCALL(insn));
        return true;
    }

    generateBranch(gen,
                   gen.currAddr(),
                   target,
                   INSN_GET_ISCALL(insn));
    return true;
}

/* TODO and/or FIXME
 * The logic used by this function is common across architectures but is replicated in architecture-specific manner in all codegen-* files.
 * This means that the logic itself needs to be refactored into the (platform independent) codegen.C file. Appropriate architecture-specific,
 * bit-twiddling functions can then be defined if necessary in the codegen-* files and called as necessary by the common, refactored logic.
*/
bool insnCodeGen::modifyJcc(Address target,
			    NS_aarch64::instruction &insn,
			    codeGen &gen) {
    long disp = target - gen.currAddr();

    if(ABS(disp) > MAX_CBRANCH_OFFSET) {
        const unsigned char *origInsn = insn.ptr();
        Address origFrom = gen.currAddr();

        /*
         * A conditional branch of the form
         *    b.cond A
         * C: ...next insn...:
         *  gets converted to
         *    b.cond B
         *    b      C
         * B: b      A
         * C: ...next insn...
         */

        //Store start index of code buffer to later calculate how much the original instruction's will have moved
        codeBufIndex_t startIdx = gen.getIndex();

        /* Generate the --b.cond B-- instruction. Directly modifying the offset bits of the instruction passed since other bits are to remain the same anyway.
           B will be 4 bytes from the next instruction. */
        instruction newInsn(insn);
        INSN_SET(newInsn, 5, 23, 0x1);
        generate(gen, newInsn);

        /* Generate the --b C-- instruction. C will be 4 bytes from the next instruction, hence offset for this instruction is set to 1.
          (it will get multiplied by 4 by the CPU) */
        newInsn.clear();
        INSN_SET(newInsn, 0, 25, 0x1);
        INSN_SET(newInsn, 26, 31, 0x05);
        generate(gen, newInsn);

        /* Generate the final --b A-- instruction.
         * The 'from' address to be passed in to generateBranch is now several bytes (8 actually, but I'm not hardcoding this) ahead of the original 'from' address.
         * So adjust it accordingly.*/
        codeBufIndex_t curIdx = gen.getIndex();
        Address newFrom = origFrom + (unsigned)(curIdx - startIdx);
        insnCodeGen::generateBranch(gen, newFrom, target);
    } else {
        instruction condBranchInsn(insn);

        //Bit 4 for the conditional branch instruction is 0
        INSN_SET(condBranchInsn, 4, 4, 0);
        INSN_SET(condBranchInsn, 24, 31, BCondOp);
        //Set the displacement immediate
        INSN_SET(condBranchInsn, 5, 23, disp >> 2);

        generate(gen, condBranchInsn);
    }

    return true;
}

bool insnCodeGen::modifyCall(Address target,
                             NS_aarch64::instruction &insn,
                             codeGen &gen) {
    if (insn.isUncondBranch())
        return modifyJump(target, insn, gen);
    else
        return modifyJcc(target, insn, gen);
}

bool insnCodeGen::modifyData(Address target,
                             NS_aarch64::instruction &insn,
                             codeGen &gen) {
    //nothing to do for and not applicable to ARM
    return false;
}

