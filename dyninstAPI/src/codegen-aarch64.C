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

#include <stdlib.h>
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
    if (abs(disp) > MAX_BRANCH_OFFSET) {
        fprintf(stderr, "ABS OFF: 0x%lx, MAX: 0x%lx\n",
                abs(disp), (unsigned long) MAX_BRANCH_OFFSET);
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

    if (abs(disp) > MAX_BRANCH_OFFSET) {
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
    instPoint *point = gen.point();
    if(!point)
        generateBranchViaTrap(gen, from, to, isCall);
    assert(point);

    registerSpace *rs = registerSpace::actualRegSpace(point);
    gen.setRegisterSpace(rs);

    Register scratch = rs->getScratchRegister(gen, true);
    if (scratch == REG_NULL) {
        fprintf(stderr, " %s[%d] No registers. Calling generateBranchViaTrap...\n", FILE__, __LINE__);
        generateBranchViaTrap(gen, from, to, isCall);
    }

    insnCodeGen::loadImmIntoReg<Address>(gen, scratch, to);

    instruction branchInsn;
    branchInsn.clear();

    //Set bits which are 0 for both BR and BLR
    INSN_SET(branchInsn, 0, 4, 0);
    INSN_SET(branchInsn, 10, 15, 0);

    //Set register
    INSN_SET(branchInsn, 5, 9, scratch);

    //Set other bits . Basically, these are the opcode bits. The only difference between BR and BLR is that bit 21 is 1 for BLR.
    INSN_SET(branchInsn, 16, 31, BRegOp);
    if(isCall)
        INSN_SET(branchInsn, 21, 21, 1);

    insnCodeGen::generate(gen, branchInsn);
}

void insnCodeGen::generateBranchViaTrap(codeGen &gen, Address from, Address to, bool isCall) {
    long disp = to - from;
    if (abs(disp) <= MAX_BRANCH_OFFSET) {
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
                abs(disp), (unsigned long) MAX_BRANCH_OFFSET);
        bperr( "Error: attempted a branch of 0x%lx\n", disp);
        logLine("a branch too far\n");
        showErrorCallback(52, "Internal error: branch too far");
        bperr( "Attempted to make a branch of offset 0x%lx\n", disp);
        assert(0);
    }
}

void insnCodeGen::generateAddSubShifted(codeGen &gen, insnCodeGen::AddSubOp op, int shift, int imm6, Register rm, Register rn, Register rd, bool is64bit) {
    instruction insn;
    insn.clear();

    //Set bit 31 to 1 if using 64-bit registers
    if(is64bit)
        INSN_SET(insn, 31, 31, 1);
    //Set opcode
    INSN_SET(insn, 24, 30, op == Add ? ADDShiftOp : SUBShiftOp);

    //Set shift field
    assert(shift >= 0 && shift <= 3);
    INSN_SET(insn, 22, 23, (shift & 0x3));

    //Set imm6 field
    assert(imm6 >= 0 && imm6 < (is64bit ? 64 : 32));
    INSN_SET(insn, 10, 15, imm6);

    //Set registers
    INSN_SET(insn, 0, 4, rd);
    INSN_SET(insn, 5, 9, rn);
    INSN_SET(insn, 16, 20, rm);

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateAddSubImmediate(codeGen &gen, insnCodeGen::AddSubOp op, int shift, int imm12, Register rn, Register rd, bool is64bit) {
    instruction insn;
    insn.clear();

    //Set bit 31 to 1 if using 64-bit registers
    if(is64bit)
        INSN_SET(insn, 31, 31, 1);
    //Set opcode
    INSN_SET(insn, 24, 30, op == Add ? ADDImmOp : SUBShiftOp);

    //Set shift field
    assert(shift >= 0 && shift <= 3);
    INSN_SET(insn, 22, 23, (shift & 0x3));

    //Set imm12 field
    INSN_SET(insn, 10, 21, imm12);

    //Set registers
    INSN_SET(insn, 5, 9, rn);
    INSN_SET(insn, 5, 9, rd);

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateMul(codeGen &gen, Register rm, Register rn, Register rd, bool is64bit) {
    instruction insn;
    insn.clear();

    //Set bit 31 to 1 if using 64-bit registers
    if(is64bit)
        INSN_SET(insn, 31, 31, 1);
    //Set opcode
    INSN_SET(insn, 21, 28, MULOp);

    //Bits 10 to 14 are 1 for MUL
    INSN_SET(insn, 10, 14, 0x1F);

    //Set registers
    INSN_SET(insn, 16, 20, rm);
    INSN_SET(insn, 5, 9, rn);
    INSN_SET(insn, 0, 4, rd);

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateBitwiseOpShifted(codeGen &gen, insnCodeGen::BitwiseOp op, int shift, Register rm, int imm6, Register rn, Register rd, bool is64bit) {
    instruction insn;
    insn.clear();

    //Set bit 31 to 1 if using 64-bit registers
    if(is64bit)
        INSN_SET(insn, 31, 31, 1);

    //Set opcode
    int opcode;
    switch(op) {
        case insnCodeGen::And: opcode = ANDShiftOp;
            break;
        case insnCodeGen::Or: opcode = ORRShiftOp;
            break;
        case insnCodeGen::Eor: opcode = EORShiftOp;
            break;
    }
    INSN_SET(insn, 24, 30, opcode);

    //Set shift field
    assert(shift >= 0 && shift <= 3);
    INSN_SET(insn, 22, 23, (shift & 0x3));

    //Set imm6 field
    assert(imm6 >= 0 && imm6 < (is64bit ? 64 : 32));
    INSN_SET(insn, 10, 15, imm6);

    //Set registers
    INSN_SET(insn, 16, 20, rm);
    INSN_SET(insn, 5, 9, rn);
    INSN_SET(insn, 0, 4, rd);

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

void insnCodeGen::generateMove(codeGen &gen, int imm16, int shift, Register rd, MoveOp movOp)
{
    instruction insn;
    insn.clear();

    //Set the sf bit to 1 since we always want to use 64-bit registers
    INSN_SET(insn, 31, 31, 1);

    //Set opcode
    INSN_SET(insn, 23, 30, movOp);

    //Set immediate
    INSN_SET(insn, 5, 20, imm16);

    //Set register
    INSN_SET(insn, 0, 4, rd);

    //Set shift amount for immediate
    INSN_SET(insn, 21, 22, (shift & 0x3));

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateMoveSP(codeGen &gen, Register rn, Register rd, bool is64bit) {
    instruction insn;
    insn.clear();

    //Set source and destination registers
    INSN_SET(insn, 0, 4, rd & 0x1f);
    INSN_SET(insn, 5, 9, rn & 0x1f);

    //Set opcode
    INSN_SET(insn, 10, 30, MOVSPOp);

    //Set if using 64-bit registers
    INSN_SET(insn, 31, 31, is64bit);

    insnCodeGen::generate(gen, insn);
}

Register insnCodeGen::moveValueToReg(codeGen &gen, long int val, pdvector<Register> *exclude) {
    Register scratchReg;
    if(exclude)
	    scratchReg = gen.rs()->getScratchRegister(gen, *exclude, true);
    else
	    scratchReg = gen.rs()->getScratchRegister(gen, true);

    if (scratchReg == REG_NULL) {
        fprintf(stderr, " %s[%d] No scratch register available to generate add instruction!", FILE__, __LINE__);
        assert(0);
    }

    insnCodeGen::generateMove(gen, (val & 0xFFFF), 0, scratchReg, insnCodeGen::MovOp_MOVZ);
    if (val >= MIN_IMM32 && val < MAX_IMM32)
        insnCodeGen::generateMove(gen, ((val >> 16) & 0xFFFF), 0x1, scratchReg, insnCodeGen::MovOp_MOVK);
    if (val < MIN_IMM32 || val > MAX_IMM32) {
        insnCodeGen::generateMove(gen, ((val >> 32) & 0xFFFF), 0x2, scratchReg, insnCodeGen::MovOp_MOVK);
        insnCodeGen::generateMove(gen, ((val >> 48) & 0xFFFF), 0x3, scratchReg, insnCodeGen::MovOp_MOVK);
    }

    return scratchReg;
}

/* Currently, I'm only considering generation of only STR/LDR and their register/immediate variants.*/
void insnCodeGen::generateMemAccess32or64(codeGen &gen, LoadStore accType, Register r1, Register r2, int immd, bool is64bit)
{
    instruction insn;
    insn.clear();

    //Bit 31 is always 1. Bit 30 is 1 if we're using the 64-bit variant.
    INSN_SET(insn, 31, 31, 1);
    if(is64bit)
        INSN_SET(insn, 30, 30, 1);

    //Set opcode, index and offset bits
    if(immd >= -256 && immd <= 255) {
        INSN_SET(insn, 21, 29, (accType == Load) ? LDRImmOp : STRImmOp);
        INSN_SET(insn, 10, 11, 0x1);
        INSN_SET(insn, 12, 20, immd);
    } else {
        assert(!"Cannot perform a post-indexed memory access for offsets not in range [-256, 255]!");
    }

    //Set memory access register and register for address calculation.
    INSN_SET(insn, 0, 4, r1 & 0x1F);
    INSN_SET(insn, 5, 9, r2 & 0x1F);

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateMemAccessFP(codeGen &gen, LoadStore accType, Register rt, Register rn, int immd, int size, bool is128bit) {
    instruction insn;
    insn.clear();

    if(size < 0 || size > 3)
        assert(!"Size field for STR (immediate, SIMD&FP) variant has to be in the range [0-3]!");

    //Set size, opcode, index and offset bits
    INSN_SET(insn, 30, 31, size & 0x3);
    INSN_SET(insn, 21, 29, (accType == Load) ? LDRFPImmOp : STRFPImmOp);
    if(is128bit)
        INSN_SET(insn, 23, 23, 1);
    INSN_SET(insn, 10, 11, 0x1);
    if(immd >= -256 && immd <= 255) {
        INSN_SET(insn, 12, 20, immd);
    } else {
        assert(!"Cannot perform a post-indexed memory access for offsets not in range [-256, 255]!");
    }

    //Set memory access register and register for address calculation.
    INSN_SET(insn, 0, 4, rt);
    INSN_SET(insn, 5, 9, rn);

    insnCodeGen::generate(gen, insn);
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

template <typename T>
void insnCodeGen::loadImmIntoReg(codeGen &gen, Register rt, T value)
{
    assert(value >= 0);

    insnCodeGen::generateMove(gen, (value & 0xFFFF), 0, rt, MovOp_MOVZ);
    if(value > MAX_IMM16)
        insnCodeGen::generateMove(gen, ((value >> 16) & 0xFFFF), 0x1, rt, MovOp_MOVK);
    if(value > MAX_IMM32) {
        insnCodeGen::generateMove(gen, ((value >> 32) & 0xFFFF), 0x2, rt, MovOp_MOVK);
        insnCodeGen::generateMove(gen, ((value >> 48) & 0xFFFF), 0x3, rt, MovOp_MOVK);
    }
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
    if (abs(disp) > MAX_BRANCH_OFFSET) {
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

    if(abs(disp) > MAX_CBRANCH_OFFSET) {
        const unsigned char *origInsn = insn.ptr();
        Address origFrom = gen.currAddr();

        /*
         * A conditional branch of the form
         *    b.cond A
	 * [Note that b.cond could also be cbz, cbnz, tbz or tbnz -- all valid conditional branch instructions]
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
    int raw = insn.asInt();
    bool isneg = false;

    if (target < gen.currAddr())
        isneg = true;

    if (((raw >> 24) & 0x1F) == 0x10) {
	Address offset;
	if((raw >> 31) & 0x1) {
	    target &= 0xFFFFF000;
	    Address cur = gen.currAddr() & 0xFFFFF000;
	    offset = isneg ? (cur - target) : (target - cur);
	    offset >>= 12;
	}
        signed long imm = isneg ? -offset : offset;

        //If offset is within +/- 1 MB, modify the instruction (ADR/ADRP) with the new offset
        if (offset <= (1 << 20)) {
            instruction newInsn(insn);

            INSN_SET(newInsn, 5, 23, ((imm >> 2) & 0x7FFFF));
            INSN_SET(newInsn, 29, 30, (imm & 0x3));

            generate(gen, newInsn);
        }
            //Else, generate move instructions to move the value to the same register
        else {
            Register rd = raw & 0x1F;
            loadImmIntoReg<Address>(gen, rd, target);
        }
    } else if (((raw >> 24) & 0x3F) == 0x18 || ((raw >> 24) & 0x3F) == 0x1C) {
    	Address offset = !isneg ? (target - gen.currAddr()) : (gen.currAddr() - target);
        //If offset is within +/- 1 MB, modify the instruction (LDR/LDRSW) with the new offset
        if (offset <= (1 << 20)) {
            instruction newInsn(insn);

            isneg ? (offset += 4) : (offset -= 4);
	    signed long imm = isneg ? -(offset >> 2) : (offset >> 2);
            INSN_SET(newInsn, 5, 23, (imm & 0x7FFFF));

            generate(gen, newInsn);
        }
            //Else, generate move instructions to move the value to the same register
        else {
            //Get scratch register for loading the target address in
            Register immReg = gen.rs()->getScratchRegister(gen, true);
            if(immReg == REG_NULL)
                assert(!"No scratch register available to load the target address into for a PC-relative data access using LDR/LDRSW!");
            //Generate sequence of instructions for loading the target address in scratch register
            loadImmIntoReg<Address>(gen, immReg, target);

            Register rt = raw & 0x1F;

            //Generate instruction for reading value at target address using unsigned-offset variant of the immediate variant of LDR/LDRSW
            instruction newInsn;
            newInsn.clear();

            if(((raw >> 31) & 0x1) == 0) {
                INSN_SET(newInsn, 30, 30, ((raw >> 30) & 0x1));
                INSN_SET(newInsn, 22, 29, LDRImmUIOp);
            } else {
                INSN_SET(newInsn, 22, 29, LDRSWImmUIOp);
            }

            INSN_SET(newInsn, 31, 31, 0x1);
            INSN_SET(newInsn, 10, 21, 0);
            INSN_SET(newInsn, 5, 9, immReg);
            INSN_SET(newInsn, 0, 4, rt);

            generate(gen, newInsn);
        }
    } else {
        assert(!"Got an instruction other than ADR/ADRP/LDR(literal)/LDRSW(literal) in PC-relative data access!");
    }

    return true;
}

