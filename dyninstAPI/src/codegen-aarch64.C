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

void insnCodeGen::generate(codeGen &gen, instruction &insn) {
#if defined(endian_mismatch)
  // Writing an instruction.  Convert byte order if necessary.
  unsigned raw = swapBytesIfNeeded(insn.asInt());
#else
  unsigned raw = insn.asInt();
#endif

  gen.copy(&raw, sizeof(unsigned));
}

void insnCodeGen::generate(codeGen &gen, instruction &insn, unsigned position) {
#if defined(endian_mismatch)
    // Writing an instruction.  Convert byte order if necessary.
    unsigned raw = swapBytesIfNeeded(insn.asInt());
#else
    unsigned raw = insn.asInt();
#endif

    gen.insert(&raw, sizeof(unsigned), position);
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
    if (labs(disp) > MAX_BRANCH_OFFSET) {
        fprintf(stderr, "ABS OFF: 0x%lx, MAX: 0x%lx\n",
                (unsigned long)labs(disp), (unsigned long) MAX_BRANCH_OFFSET);
        bperr( "Error: attempted a branch of 0x%lx\n", (unsigned long)disp);
        logLine("a branch too far\n");
        showErrorCallback(52, "Internal error: branch too far");
        bperr( "Attempted to make a branch of offset 0x%lx\n", (unsigned long)disp);
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

void insnCodeGen::generateBranch(codeGen &gen, Dyninst::Address from, Dyninst::Address to, bool link) {
    long disp = (to - from);

    if (labs(disp) > MAX_BRANCH_OFFSET) {
        generateLongBranch(gen, from, to, link);
    }else
        generateBranch(gen, disp, link);
}

void insnCodeGen::generateCall(codeGen &gen, Dyninst::Address from, Dyninst::Address to) {
    generateBranch(gen, from, to, true);
}

void insnCodeGen::generateLongBranch(codeGen &gen,
                                     Dyninst::Address from,
                                     Dyninst::Address to,
                                     bool isCall) 
{
    auto generateBReg = [&isCall, &gen](Dyninst::Register s) -> void
    {
        instruction branchInsn;
        branchInsn.clear();

        //Set bits which are 0 for both BR and BLR
        INSN_SET(branchInsn, 0, 4, 0);
        INSN_SET(branchInsn, 10, 15, 0);

        //Set register
        INSN_SET(branchInsn, 5, 9, s);

        // Set other bits . Basically, these are the opcode bits.
        // The only difference between BR and BLR is that bit 21 is 1 for BLR.
        INSN_SET(branchInsn, 16, 31, BRegOp);
        if(isCall)
            INSN_SET(branchInsn, 21, 21, 1);

        insnCodeGen::generate(gen, branchInsn);
    };

    Dyninst::Register scratch = Null_Register;

    if(isCall)
    {
        // use Link Dyninst::Register as scratch since it will be overwritten at return
        scratch = 30;
        //load disp to r30
        loadImmIntoReg(gen, scratch, to);
        //generate call
        generateBReg(scratch);
        return;
    }

    instPoint *point = gen.point();
    if(point)
    {
        registerSpace *rs = registerSpace::actualRegSpace(point);
        gen.setRegisterSpace(rs);

        scratch = rs->getScratchRegister(gen, true);
    }

    if (scratch == Null_Register)
    {
        //fprintf(stderr, " %s[%d] No registers. Calling generateBranchViaTrap...\n", FILE__, __LINE__);
        generateBranchViaTrap(gen, from, to, isCall);
        return;
    }

    loadImmIntoReg(gen, scratch, to);
    generateBReg(scratch);
}

void insnCodeGen::generateBranchViaTrap(codeGen &gen, Dyninst::Address from, Dyninst::Address to, bool isCall) {
    long disp = to - from;
    if (labs(disp) <= MAX_BRANCH_OFFSET) {
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
                (unsigned long)labs(disp), (unsigned long) MAX_BRANCH_OFFSET);
        bperr( "Error: attempted a branch of 0x%lx\n", (unsigned long)disp);
        logLine("a branch too far\n");
        showErrorCallback(52, "Internal error: branch too far");
        bperr( "Attempted to make a branch of offset 0x%lx\n", (unsigned long)disp);
        assert(0);
    }
}

void insnCodeGen::generateConditionalBranch(codeGen& gen, Dyninst::Address to, unsigned opcode, bool s)
{
    instruction insn;
    insn.clear();

    //Set opcode
    INSN_SET(insn, 25, 31, BCondOp);

    //Set imm19 field
    INSN_SET(insn, 5, 23, to >> 2);

    auto getConditionCode = [&opcode, &s]() -> unsigned
    {
        switch(opcode){
            case lessOp:
	      if (s) return 0xB; else return 0x3;
            case leOp:      
	      if (s) return 0xD; else return 0x9;
            case greaterOp: 
	      if (s) return 0xC; else return 0x8;
            case geOp:      
	      if (s) return 0xA; else return 0x2;
            case eqOp:      return 0x0;
            case neOp:      return 0x1;
            default:
                assert(0); // wrong condition passed
                break;
        }
    };

    //Set condition 
    INSN_SET(insn, 0, 3, getConditionCode());

    insnCodeGen::generate(gen, insn);
}


void insnCodeGen::generateAddSubShifted(
        codeGen &gen, insnCodeGen::ArithOp op, int shift, int imm6, Dyninst::Register rm,
        Dyninst::Register rn, Dyninst::Register rd, bool is64bit)
{
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

void insnCodeGen::generateAddSubImmediate(
        codeGen &gen, insnCodeGen::ArithOp op, int shift, int imm12, Dyninst::Register rn, Dyninst::Register rd, bool is64bit)
{
    instruction insn;
    insn.clear();

    //Set bit 31 to 1 if using 64-bit registers
    if(is64bit)
        INSN_SET(insn, 31, 31, 1);
    //Set opcode
    INSN_SET(insn, 24, 30, op == Add ? ADDImmOp : SUBImmOp);

    //Set shift field
    assert(shift >= 0 && shift <= 3);
    INSN_SET(insn, 22, 23, (shift & 0x3));

    //Set imm12 field
    INSN_SET(insn, 10, 21, imm12);

    //Set registers
    INSN_SET(insn, 5, 9, rn);
    INSN_SET(insn, 0, 4, rd);

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateMul(codeGen &gen, Dyninst::Register rm, Dyninst::Register rn, Dyninst::Register rd, bool is64bit) {
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

//#sasha is rm or rn the denominator?
void insnCodeGen::generateDiv(
        codeGen &gen, Dyninst::Register rm, Dyninst::Register rn, Dyninst::Register rd, bool is64bit, bool s)
{
    instruction insn;
    insn.clear();

    // Set bit 31 to 1 if using 64-bit registers
    if(is64bit)
        INSN_SET(insn, 31, 31, 1);

    // Set opcode
    INSN_SET(insn, 21, 30, SDIVOp);

    INSN_SET(insn, 11, 15, 0x1);
    if (s) {
        INSN_SET(insn, 10, 10, 0x1); // signed: SDIV
    } else {
        INSN_SET(insn, 10, 10, 0x0); // unsigned: UDIV
    }

    //Set registers
    INSN_SET(insn, 16, 20, rm);
    INSN_SET(insn, 5, 9, rn);
    INSN_SET(insn, 0, 4, rd);

    insnCodeGen::generate(gen, insn);

}

void insnCodeGen::generateBitwiseOpShifted(
        codeGen &gen, insnCodeGen::BitwiseOp op, int shift, Dyninst::Register rm, int imm6,
        Dyninst::Register rn, Dyninst::Register rd, bool is64bit)
{
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
        default:
            assert(!"insnCodeGen::generateBitwiseOpShifted op is not And, Or or Eor");
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

void insnCodeGen::generateLoadReg(codeGen &, Dyninst::Register,
                                  Dyninst::Register, Dyninst::Register)
{
    assert(0);
//#warning "This function is not implemented yet!"
}

void insnCodeGen::generateStoreReg(codeGen &, Dyninst::Register,
                                   Dyninst::Register, Dyninst::Register)
{
    assert(0);
//#warning "This function is not implemented yet!"
}

void insnCodeGen::generateLoadReg64(codeGen &, Dyninst::Register,
                                    Dyninst::Register, Dyninst::Register)
{
assert(0);
//#warning "This function is not implemented yet!"
}

void insnCodeGen::generateStoreReg64(codeGen &, Dyninst::Register,
                                     Dyninst::Register, Dyninst::Register)
{
assert(0);
//#warning "This function is not implemented yet!"
}

void insnCodeGen::generateMove(codeGen &gen, int imm16, int shift, Dyninst::Register rd, MoveOp movOp)
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

void insnCodeGen::generateMove(
        codeGen &gen, Dyninst::Register rd, Dyninst::Register rm, bool is64bit)
{
    insnCodeGen::generateBitwiseOpShifted(gen, insnCodeGen::Or, 0, rm, 0, 0x1f, rd, is64bit);  
}

void insnCodeGen::generateMoveSP(codeGen &gen, Dyninst::Register rn, Dyninst::Register rd, bool is64bit) {
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


Dyninst::Register insnCodeGen::moveValueToReg(codeGen &gen, long int val, std::vector<Dyninst::Register> *exclude) {
    Dyninst::Register scratchReg;
    if(exclude)
	    scratchReg = gen.rs()->getScratchRegister(gen, *exclude, true);
    else
	    scratchReg = gen.rs()->getScratchRegister(gen, true);

    if (scratchReg == Null_Register) {
        fprintf(stderr, " %s[%d] No scratch register available to generate add instruction!", FILE__, __LINE__);
        assert(0);
    }

    loadImmIntoReg(gen, scratchReg, static_cast<Dyninst::Address>(val));

    return scratchReg;
}

// Generate memory access through Load or Store
// Instructions generated:
//     LDR/STR (immediate) for 32-bit or 64-bit
//     LDRB/STRB (immediate) for 8-bit
//     LDRH/STRH  (immediate) for 16-bit
//
// Encoding classes allowed: Post-index, Pre-index and Unsigned Offset
void insnCodeGen::generateMemAccess(codeGen &gen, LoadStore accType,
        Dyninst::Register r1, Dyninst::Register r2, int immd, unsigned size, IndexMode im)
{
    instruction insn;
    insn.clear();

    assert( size==1 || size==2 || size==4 || size==8 );

    static unsigned short map_size[9] = {0,0,1,0,2,0,0,0,3}; // map `size` to 00,01,10,11
    INSN_SET(insn, 30, 31, map_size[size]);

    switch(im){
        case Post:
        case Pre:
            assert(immd >= -256 && immd <= 255);
            INSN_SET(insn, 21, 29, (accType == Load) ? LDRImmOp : STRImmOp);
            INSN_SET(insn, 10, 11, im==Post?0x1:0x3);
            INSN_SET(insn, 12, 20, immd); // can be negative so no enconding
            break;
        case Offset:
            assert(immd>=0); // this offset is supposed to be unsigned, i.e. positive
            INSN_SET(insn, 22, 29, (accType == Load) ? LDRImmUIOp : STRImmUIOp);
            INSN_SET(insn, 10, 21, immd/size); // always positive so encode
            break;
    }

    //Set memory access register and register for address calculation.
    INSN_SET(insn, 0, 4, r1 & 0x1F);
    INSN_SET(insn, 5, 9, r2 & 0x1F);

    insnCodeGen::generate(gen, insn);
}

// This is for generating STR/LDR (SIMD&FP) (immediate) for indexing modes of Post, Pre and Offset
void insnCodeGen::generateMemAccessFP(codeGen &gen, LoadStore accType,
        Dyninst::Register rt, Dyninst::Register rn, int immd, int size, bool is128bit, IndexMode im)
{
    instruction insn;
    insn.clear();

    switch(im){
        case Post:
        case Pre:
            //Set opcode, index and offset bits
            if(immd >= -256 && immd <= 255) {
                INSN_SET(insn, 21, 29, (accType == Load) ? LDRFPImmOp : STRFPImmOp);
                INSN_SET(insn, 10, 11, im==Post?0x1:0x3);
                INSN_SET(insn, 12, 20, immd); // can be negative so no enconding
            } else {
                assert(!"Cannot perform a post/pre-indexed memory access for offsets not in range [-256, 255]!");
            }
            break;
        case Offset:
            INSN_SET(insn, 22, 29, (accType == Load) ? LDRFPImmUOp : STRFPImmUOp);
            assert(immd>=0); // this offset is supposed to be unsigned, i.e. positive
            INSN_SET(insn, 10, 21, immd>>4); //#sasha change encoding to appropriate
            break;
    }

    // STR/LDR can be 8, 16, 32, 64, and 128, this might need to change
    // it's more complicated than just the bit 23. 31 and 30 also decides
    if(is128bit)
        INSN_SET(insn, 23, 23, 1);

    if(size < 0 || size > 3)
        assert(!"Size field for STR (immediate, SIMD&FP) variant has to be in the range [0-3]!");
    INSN_SET(insn, 30, 31, size & 0x3);

    //Set memory access register and register for address calculation.
    INSN_SET(insn, 0, 4, rt);
    INSN_SET(insn, 5, 9, rn);

    insnCodeGen::generate(gen, insn);
}

// rlwinm ra,rs,n,0,31-n
void insnCodeGen::generateLShift(codeGen &, Dyninst::Register, int, Dyninst::Register)
{
assert(0);
//#warning "This function is not implemented yet!"
}

// rlwinm ra,rs,32-n,n,31
void insnCodeGen::generateRShift(codeGen &, Dyninst::Register, int, Dyninst::Register)
{
assert(0);
//#warning "This function is not implemented yet!"
}

// sld ra, rs, rb
void insnCodeGen::generateLShift64(codeGen &, Dyninst::Register, int, Dyninst::Register)
{
assert(0);
//#warning "This function is not implemented yet!"
}

// srd ra, rs, rb
void insnCodeGen::generateRShift64(codeGen &, Dyninst::Register, int, Dyninst::Register)
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

void insnCodeGen::generateRelOp(codeGen &, int, int, Dyninst::Register,
                                Dyninst::Register, Dyninst::Register)
{
assert(0);
//#warning "This function is not implemented yet!"
}


void insnCodeGen::saveRegister(codeGen &gen, Dyninst::Register r, int sp_offset, IndexMode im)
{
    generateMemAccess(gen, Store, r, REG_SP, sp_offset, 8, im);
}


void insnCodeGen::restoreRegister(codeGen &gen, Dyninst::Register r, int sp_offset, IndexMode im)
{
    generateMemAccess(gen, Load, r, REG_SP, sp_offset, 8, im);
}


// Helper method.  Fills register with partial value to be completed
// by an operation with a 16-bit signed immediate.  Such as loads and
// stores.
void insnCodeGen::loadPartialImmIntoReg(codeGen &, Dyninst::Register, long)
{
assert(0);
//#warning "This function is not implemented yet!"
}

int insnCodeGen::createStackFrame(codeGen &, int, std::vector<Dyninst::Register>& freeReg, std::vector<Dyninst::Register>&){
assert(0);
//#warning "This function is not implemented yet!"
		return freeReg.size();
}

void insnCodeGen::removeStackFrame(codeGen &) {
assert(0);
//#warning "This function is not implemented yet!"
}

bool insnCodeGen::generateMem(codeGen &,
                              instruction&,
                              Dyninst::Address,
                              Dyninst::Address,
                              Dyninst::Register,
                  Dyninst::Register) {
assert(0);
//#warning "This function is not implemented yet!"
return false; }

void insnCodeGen::generateMoveFromLR(codeGen &, Dyninst::Register) {
assert(0);
//#warning "This function is not implemented yet!"
}

void insnCodeGen::generateMoveToLR(codeGen &, Dyninst::Register) {
assert(0);
//#warning "This function is not implemented yet!"
}
void insnCodeGen::generateMoveToCR(codeGen &, Dyninst::Register) {
assert(0);
//#warning "This function is not implemented yet!"
}

bool insnCodeGen::modifyJump(Dyninst::Address target,
                             NS_aarch64::instruction &insn,
                             codeGen &gen) {
    long disp = target - gen.currAddr();

    if(INSN_GET_ISCALL(insn))
    {
        generateBranch(gen, gen.currAddr(), target, INSN_GET_ISCALL(insn));
        return true;
    }

    if (labs(disp) > MAX_BRANCH_OFFSET) {
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
 * The logic used by this function is common across architectures but is replicated 
 * in architecture-specific manner in all codegen-* files.
 * This means that the logic itself needs to be refactored into the (platform 
 * independent) codegen.C file. Appropriate architecture-specific,
 * bit-twiddling functions can then be defined if necessary in the codegen-* files 
 * and called as necessary by the common, refactored logic.
*/
bool insnCodeGen::modifyJcc(Dyninst::Address target,
			    NS_aarch64::instruction &insn,
			    codeGen &gen) {
    long disp = target - gen.currAddr();
    auto isTB = insn.isInsnType(COND_BR_t::TB_MASK, COND_BR_t::TB);
    
    if(labs(disp) > MAX_CBRANCH_OFFSET ||
            (isTB && labs(disp) > MAX_TBRANCH_OFFSET))
    {
        Dyninst::Address origFrom = gen.currAddr();

        /*
         * A conditional branch of the form:
         *    b.cond A
         * C: ...next insn...:
         * [Note that b.cond could also be cbz, cbnz, tbz or tbnz -- all valid conditional branch instructions]
         *
         * Gets converted to:
         *    b.cond B
         *    b      C
         * B: b      A
         * C: ...next insn...
         */

        // Store start index of code buffer to later calculate how much the original instruction's will have moved
        codeBufIndex_t startIdx = gen.getIndex();

        /* Generate the --b.cond B-- instruction. Directly modifying the offset 
         * bits of the instruction passed since other bits are to remain the same anyway.
           B will be 4 bytes from the next instruction. (it will get multiplied by 4 by the CPU) */
        instruction newInsn(insn);
        if(insn.isInsnType(COND_BR_t::TB_MASK, COND_BR_t::TB))
            INSN_SET(newInsn, 5, 18, 0x1);
        else
            INSN_SET(newInsn, 5, 23, 0x1);
        generate(gen, newInsn);

        /* Generate the --b C-- instruction. C will be 4 bytes from the next 
         * instruction, hence offset for this instruction is set to 1.
          (it will get multiplied by 4 by the CPU) */
        newInsn.clear();
        INSN_SET(newInsn, 0, 25, 0x1);
        INSN_SET(newInsn, 26, 31, 0x05);
        generate(gen, newInsn);

        /* Generate the final --b A-- instruction.
         * The 'from' address to be passed in to generateBranch is now several
         * bytes (8 actually, but I'm not hardcoding this) ahead of the original 'from' address.
         * So adjust it accordingly.*/
        codeBufIndex_t curIdx = gen.getIndex();
        Dyninst::Address newFrom = origFrom + (unsigned)(curIdx - startIdx);
        insnCodeGen::generateBranch(gen, newFrom, target);
    } 
    else
    {
        instruction condBranchInsn(insn);

        // Set the displacement immediate
        if(isTB)
            INSN_SET(condBranchInsn, 5, 18, disp >> 2);
        else
            INSN_SET(condBranchInsn, 5, 23, disp >> 2);

        generate(gen, condBranchInsn);
    }

    return true;
}

bool insnCodeGen::modifyCall(Dyninst::Address target,
                             NS_aarch64::instruction &insn,
                             codeGen &gen) {
    if (insn.isUncondBranch())
        return modifyJump(target, insn, gen);
    else
        return modifyJcc(target, insn, gen);
}

bool insnCodeGen::modifyData(Dyninst::Address target,
        NS_aarch64::instruction &insn,
        codeGen &gen) 
{
    int raw = insn.asInt();
    bool isneg = false;

    if (target < gen.currAddr())
        isneg = true;

    if (((raw >> 24) & 0x1F) == 0x10) {
        Dyninst::Address offset;
        if((static_cast<uint32_t>(raw) >> 31) & 0x1) {
            target &= 0xFFFFF000;
            Dyninst::Address cur = gen.currAddr() & 0xFFFFF000;
            offset = isneg ? (cur - target) : (target - cur);
            offset >>= 12;
        } else {
            Dyninst::Address cur = gen.currAddr();
            offset = isneg ? (cur - target) : (target - cur);
        }
        signed long imm = isneg ? -((signed long)offset) : offset;

        //If offset is within +/- 1 MB, modify the instruction (ADR/ADRP) with the new offset
        if (offset <= (1 << 20)) {
            instruction newInsn(insn);
            INSN_SET(newInsn, 5, 23, ((imm >> 2) & 0x7FFFF));
            INSN_SET(newInsn, 29, 30, (imm & 0x3));
            generate(gen, newInsn);
        }
        //Else, generate move instructions to move the value to the same register
        else {
            //Dyninst::Register rd = raw & 0x1F;
            //loadImmIntoReg(gen, rd, target);
            instruction newInsn;
            instruction newInsn2;
            newInsn.clear();
            signed long page_rel = ((long)(target >> 12)) - ((long)(gen.currAddr() >> 12));
            signed long off = target & 0xFFFF;
            INSN_SET(newInsn, 0, 4, raw & 0x1F);
            INSN_SET(newInsn, 5, 23, ((page_rel >> 2) & 0x7FFFF));
            INSN_SET(newInsn, 24, 28, 0x10);
            INSN_SET(newInsn, 29, 30, (page_rel & 0x3));
            INSN_SET(newInsn, 31, 31, 1);
            generate(gen, newInsn);
            newInsn2.clear();
            INSN_SET(newInsn2, 0, 4, raw & 0x1F);
            INSN_SET(newInsn2, 5, 9, raw & 0x1F);
            INSN_SET(newInsn2, 10, 21, off);
            INSN_SET(newInsn2, 22, 31, 0x244);
            generate(gen , newInsn2);

        }
    } else if (((raw >> 24) & 0x3F) == 0x18 || ((raw >> 24) & 0x3F) == 0x1C) {
        Dyninst::Address offset = !isneg ? (target - gen.currAddr()) : (gen.currAddr() - target);
        //If offset is within +/- 1 MB, modify the instruction (LDR/LDRSW) with the new offset
        if (offset <= (1 << 20)) {
            instruction newInsn(insn);

            isneg ? (offset += 4) : (offset -= 4);
            signed long imm = isneg ? -(offset >> 2) : (offset >> 2);
            INSN_SET(newInsn, 5, 23, (imm & 0x7FFFF));

            generate(gen, newInsn);
        }
        //If it's larger than |1MB|, move target to register and generate LDR
        else {
            // Get scratch register
            Dyninst::Register scratch = gen.rs()->getScratchRegister(gen, true);
            if(scratch == Null_Register)
                assert(!"No scratch register available to load the target \
                        address into for a PC-relative data access using LDR/LDRSW!");

            // Load the target address into scratch register
            loadImmIntoReg(gen, scratch, target);

            // Generate LDR(immediate) to load into r the the content of [scratch]
            Dyninst::Register r = raw & 0x1F;
            generateMemAccess(gen, Load, r, scratch, 0, 8, Offset);
        }
    } else {
        assert(!"Got an instruction other than ADR/ADRP/LDR(literal)/LDRSW(literal) in PC-relative data access!");
    }

    return true;
}

