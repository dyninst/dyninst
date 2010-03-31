/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

/*
 * inst-power.C - Identify instrumentation points for a RS6000/PowerPCs
 * $Id: arch-power.C,v 1.28 2008/06/19 19:53:06 legendre Exp $
 */

#include "common/h/Types.h"
#include "arch.h"
#include "util.h"
#include "debug.h"
#include "symtab.h"
#include "addressSpace.h"
#include "bitArray.h"
#include "registerSpace.h"
#include "instPoint.h"

#if defined(os_vxworks)
#include "vxworks.h"
#endif

unsigned int swapBytesIfNeeded(unsigned int i)
{
    static int one = 0x1;

    if ( *((unsigned char *)&one) )
        return instruction::swapBytes( *((instructUnion *)&i) ).raw;

    return i;
}

int instruction::signExtend(unsigned int i, unsigned int pos)
{
    int ret;
    if ((i >> (--pos)) & 0x1 == 0x1) {
        ret = i |  (~0 << pos);
    } else {
        ret = i & ~(~0 << pos);
    }

     return ret;
}

instructUnion &instruction::swapBytes(instructUnion &i)
{
    i.byte[0] = i.byte[0] ^ i.byte[3];
    i.byte[3] = i.byte[0] ^ i.byte[3];
    i.byte[0] = i.byte[0] ^ i.byte[3];

    i.byte[1] = i.byte[1] ^ i.byte[2];
    i.byte[2] = i.byte[1] ^ i.byte[2];
    i.byte[1] = i.byte[1] ^ i.byte[2];

    return i;
}

instruction *instruction::copy() const {
    return new instruction(*this);
}

void instruction::generateIllegal(codeGen &gen) { // instP.h
    instruction insn;
    insn.generate(gen);
}

void instruction::generateTrap(codeGen &gen) {
    instruction insn(BREAK_POINT_INSN);
    insn.generate(gen);
}

void instruction::generateBranch(codeGen &gen, long disp, bool link)
{
    if (ABS(disp) > MAX_BRANCH) {
	// Too far to branch, and no proc to register trap.
	fprintf(stderr, "ABS OFF: 0x%lx, MAX: 0x%lx\n",
           ABS(disp), (unsigned long) MAX_BRANCH);
	bperr( "Error: attempted a branch of 0x%lx\n", disp);
	logLine("a branch too far\n");
	showErrorCallback(52, "Internal error: branch too far");
	bperr( "Attempted to make a branch of offset 0x%lx\n", disp);
	assert(0);
    }

    instruction insn;
    IFORM_OP_SET(insn, Bop);
    IFORM_LI_SET(insn, disp >> 2);
    IFORM_AA_SET(insn, 0);
    if (link)
        IFORM_LK_SET(insn, 1);
    else
        IFORM_LK_SET(insn, 0);

    insn.generate(gen);
}

void instruction::generateBranch(codeGen &gen, Address from, Address to, bool link) {
    if (to < MAX_BRANCH) {
        // Generate an absolute branch
        instruction insn;
        IFORM_OP_SET(insn, Bop);
        IFORM_LI_SET(insn, to >> 2);
        IFORM_AA_SET(insn, 1);
        if (link) 
            IFORM_LK_SET(insn, 1);
        else
            IFORM_LK_SET(insn, 0);
        insn.generate(gen);
        return;
    }

    long disp = (to - from);

    if (ABS(disp) > MAX_BRANCH) {
        return generateLongBranch(gen, from, to, link);
    }

    return generateBranch(gen, disp, link);
   
}

void instruction::generateCall(codeGen &gen, Address from, Address to) {
    generateBranch(gen, from, to, true);
}

void instruction::generateInterFunctionBranch(codeGen &gen,
                                              Address from,
                                              Address to,
                                              bool link) {
    long disp = to - from;

    if (ABS(disp) <= MAX_BRANCH) {
        // We got lucky...
        return generateBranch(gen, from, to);
    }

    instruction::loadImmIntoReg(gen, 0, to);

    instruction insn;
    
    //mtspr:  mtctr scratchReg
    insn.clear();
    XFORM_OP_SET(insn, MTSPRop);
    XFORM_RT_SET(insn, 0);
    XFORM_RA_SET(insn, SPR_CTR & 0x1f);
    XFORM_RB_SET(insn, (SPR_CTR >> 5) & 0x1f);
    XFORM_XO_SET(insn, MTSPRxop);
    insn.generate(gen);

    // And branch to CTR
    instruction btctr(link ? BCTRLraw : BCTRraw);
    btctr.generate(gen);
}

void instruction::generateLongBranch(codeGen &gen, 
                                     Address from, 
                                     Address to, 
                                     bool isCall) {
    // First, see if we can cheap out
    long disp = (to - from);
    if (ABS(disp) <= MAX_BRANCH) {
        return generateBranch(gen, disp, isCall);
    }

    // We can use a register branch via the LR or CTR, if either of them
    // is free.
    
    // Let's see if we can grab a free GPregister...
    instPoint *point = gen.point();
    if (!point) {
        return generateBranchViaTrap(gen, from, to, isCall);
    }

    assert(point);
    
    // Could see if the codeGen has it, but right now we have assert
    // code there and we don't want to hit that.
    registerSpace *rs = registerSpace::actualRegSpace(point,
                                                      callPreInsn);
    
    Register scratch = rs->getScratchRegister(gen, true);

    bool mustRestore = false;
    
    if (scratch == REG_NULL) { 
        // On AIX we use an open slot in the stack frame. 
        // On Linux we save under the stack and hope it doesn't
        // cause problems.
#if !defined(os_aix)
        return generateBranchViaTrap(gen, from, to, isCall);
#endif
        
        // 32-bit: we can use the word at +16 from the stack pointer
        if (gen.addrSpace()->getAddressWidth() == 4)
            instruction::generateImm(gen, STop, 0, 1, 4*4);
        else /* gen.addrSpace()->getAddressWidth() == 8 */
            instruction::generateMemAccess64(gen, STDop, STDxop, 0, 1, 4*8);

        mustRestore = true;
        scratch = 0;
    }
    
    // Load the destination into our scratch register
    instruction::loadImmIntoReg(gen, scratch, to);
    
    // Find out whether the LR or CTR is "dead"...
    bitArray liveRegs = point->liveRegisters(callPreInsn);
    unsigned branchRegister = 0;
    if (liveRegs[registerSpace::lr] == false) {
        branchRegister = registerSpace::lr;
    }
    else if (liveRegs[registerSpace::ctr] == false) {
        branchRegister = registerSpace::ctr;
    }

    if (!branchRegister) {
        return generateBranchViaTrap(gen, from, to, isCall); 
    }
    
    assert(branchRegister);
    
    instruction moveToBr;
    moveToBr.clear();
    XFXFORM_OP_SET(moveToBr, MTSPRop);
    XFXFORM_RT_SET(moveToBr, scratch);
    if (branchRegister == registerSpace::lr) {
        XFORM_RA_SET(moveToBr, SPR_LR & 0x1f);
        XFORM_RB_SET(moveToBr, (SPR_LR >> 5) & 0x1f);
        // The two halves (top 5 bits/bottom 5 bits) are _reversed_ in this encoding. 
    }
    else {
        XFORM_RA_SET(moveToBr, SPR_CTR & 0x1f);
        XFORM_RB_SET(moveToBr, (SPR_CTR >> 5) & 0x1f);
    }
    XFXFORM_XO_SET(moveToBr, MTSPRxop); // From assembly manual
    moveToBr.generate(gen);

    if (mustRestore) {
        if (gen.addrSpace()->getAddressWidth() == 4)
            instruction::generateImm(gen, Lop, 0, 1, 4*4);
        else /* gen.addrSpace()->getAddressWidth() == 8 */
            instruction::generateMemAccess64(gen, LDop, LDxop, 0, 1, 4*8);
    }
    
    // Aaaand now branch, linking if appropriate
    instruction branchToBr;
    branchToBr.clear();
    XLFORM_OP_SET(branchToBr, BCLRop);
    XLFORM_BT_SET(branchToBr, 0x14); // From architecture manual
    XLFORM_BA_SET(branchToBr, 0); // Unused
    XLFORM_BB_SET(branchToBr, 0); // Unused
    if (branchRegister == registerSpace::lr) {
        XLFORM_XO_SET(branchToBr, BCLRxop);
    }
    else {
        XLFORM_XO_SET(branchToBr, BCCTRxop);
    }
    XLFORM_LK_SET(branchToBr, (isCall ? 1 : 0));
    branchToBr.generate(gen);
}

void instruction::generateBranchViaTrap(codeGen &gen, Address from, Address to, bool isCall) {

    long disp = to - from;
    if (ABS(disp) <= MAX_BRANCH) {
        // We shouldn't be here, since this is an internal-called-only func.
        return generateBranch(gen, disp, isCall);
    }
    
    assert (isCall == false); // Can't do this yet
    
    if (gen.addrSpace()) {
        // Too far to branch.  Use trap-based instrumentation.
        gen.addrSpace()->trapMapping.addTrapMapping(from, to, true);
        instruction::generateTrap(gen);        
    } else {
        // Too far to branch and no proc to register trap.
        fprintf(stderr, "ABS OFF: 0x%lx, MAX: 0x%lx\n",
                ABS(disp), (unsigned long) MAX_BRANCH);
        bperr( "Error: attempted a branch of 0x%lx\n", disp);
        logLine("a branch too far\n");
        showErrorCallback(52, "Internal error: branch too far");
        bperr( "Attempted to make a branch of offset 0x%lx\n", disp);
        assert(0);
    }
}

void instruction::generateImm(codeGen &gen, int op, Register rt, Register ra, int immd)
 {
  // something should be here to make sure immd is within bounds
  // bound check really depends on op since we have both signed and unsigned
  //   opcodes.
  // We basically check if the top bits are 0 (unsigned, or positive signed)
  // or 0xffff (negative signed)
  // This is because we don't enforce calling us with LOW(immd), and
  // signed ints come in with 0xffff set. C'est la vie.
  // TODO: This should be a check that the high 16 bits are equal to bit 15,
  // really.
  assert (((immd & 0xffff0000) == (0xffff0000)) ||
          ((immd & 0xffff0000) == (0x00000000)));

  instruction insn;
  
  insn.clear();
  DFORM_OP_SET(insn, op);
  DFORM_RT_SET(insn, rt);
  DFORM_RA_SET(insn, ra);
  if (op==SIop) immd = -immd;
  DFORM_SI_SET(insn, immd);

  insn.generate(gen);
}

void instruction::generateMemAccess64(codeGen &gen, int op, int xop, Register r1, Register r2, int immd)
{
    assert(MIN_IMM16 <= immd && immd <= MAX_IMM16);
    assert((immd & 0x3) == 0);

    instruction insn;

    insn.clear();
    DSFORM_OP_SET(insn, op);
    DSFORM_RT_SET(insn, r1);
    DSFORM_RA_SET(insn, r2);
    DSFORM_DS_SET(insn, immd >> 2);
    DSFORM_XO_SET(insn, xop);

    insn.generate(gen);
}

// rlwinm ra,rs,n,0,31-n
void instruction::generateLShift(codeGen &gen, Register rs, int shift, Register ra)
{
    instruction insn;

    if (gen.addrSpace()->getAddressWidth() == 4) {
	assert(shift<32);
	insn.clear();
	MFORM_OP_SET(insn, RLINMxop);
	MFORM_RS_SET(insn, rs);
	MFORM_RA_SET(insn, ra);
	MFORM_SH_SET(insn, shift);
	MFORM_MB_SET(insn, 0);
	MFORM_ME_SET(insn, 31-shift);
	MFORM_RC_SET(insn, 0);
	insn.generate(gen);

    } else /* gen.addrSpace()->getAddressWidth() == 8 */ {
	instruction::generateLShift64(gen, rs, shift, ra);
    }
}

// rlwinm ra,rs,32-n,n,31
void instruction::generateRShift(codeGen &gen, Register rs, int shift, Register ra)
{
    instruction insn;

    if (gen.addrSpace()->getAddressWidth() == 4) {
	assert(shift<32);
	insn.clear();
	MFORM_OP_SET(insn, RLINMxop);
	MFORM_RS_SET(insn, rs);
	MFORM_RA_SET(insn, ra);
	MFORM_SH_SET(insn, 32-shift);
	MFORM_MB_SET(insn, shift);
	MFORM_ME_SET(insn, 31);
	MFORM_RC_SET(insn, 0);
	insn.generate(gen);

    } else /* gen.addrSpace()->getAddressWidth() == 8 */ {
	instruction::generateRShift64(gen, rs, shift, ra);
    }
}

// sld ra, rs, rb
void instruction::generateLShift64(codeGen &gen, Register rs, int shift, Register ra)
{
    instruction insn;

    assert(shift<64);
    insn.clear();
    MDFORM_OP_SET( insn, RLDop);
    MDFORM_RS_SET( insn, rs);
    MDFORM_RA_SET( insn, ra);
    MDFORM_SH_SET( insn, shift % 32);
    MDFORM_MB_SET( insn, (63-shift) % 32);
    MDFORM_MB2_SET(insn, (63-shift) / 32);
    MDFORM_XO_SET( insn, ICRxop);
    MDFORM_SH2_SET(insn, shift / 32);
    MDFORM_RC_SET( insn, 0);

    insn.generate(gen);
}

// srd ra, rs, rb
void instruction::generateRShift64(codeGen &gen, Register rs, int shift, Register ra)
{
    instruction insn;

    assert(shift<64);
    insn.clear();
    MDFORM_OP_SET( insn, RLDop);
    MDFORM_RS_SET( insn, rs);
    MDFORM_RA_SET( insn, ra);
    MDFORM_SH_SET( insn, (64 - shift) % 32);
    MDFORM_MB_SET( insn, shift % 32);
    MDFORM_MB2_SET(insn, shift / 32);
    MDFORM_XO_SET( insn, ICLxop);
    MDFORM_SH2_SET(insn, (64 - shift) / 32);
    MDFORM_RC_SET( insn, 0);

    insn.generate(gen);
}

//
// generate an instruction that does nothing and has to side affect except to
//   advance the program counter.
//
void instruction::generateNOOP(codeGen &gen, unsigned size)
{
    assert ((size % instruction::size()) == 0);
    while (size) {
        instruction insn(NOOPraw);
        insn.generate(gen);
        size -= instruction::size();
    }
}

void instruction::generateSimple(codeGen &gen, int op, 
                                 Register src1, Register src2, 
                                 Register dest)
{
  instruction insn;

  int xop=-1;
  insn.clear();
  XFORM_OP_SET(insn, op);
  XFORM_RT_SET(insn, src1);
  XFORM_RA_SET(insn, dest);
  XFORM_RB_SET(insn, src2);
  if (op==ANDop) {
      xop=ANDxop;
  } else if (op==ORop) {
      xop=ORxop;
  } else {
      // only AND and OR are currently designed to use genSimpleInsn
      assert(0);
  }
  XFORM_XO_SET(insn, xop);
  insn.generate(gen);
}

void instruction::generateRelOp(codeGen &gen, int cond, int mode, Register rs1,
                                Register rs2, Register rd)
{
    instruction insn;

    // cmp rs1, rs2
    insn.clear();
    XFORM_OP_SET(insn, CMPop);
    XFORM_RT_SET(insn, 0);    // really bf & l sub fields of rt we care about
    XFORM_RA_SET(insn, rs1);
    XFORM_RB_SET(insn, rs2);
    XFORM_XO_SET(insn, CMPxop);

    insn.generate(gen);

    // li rd, 1
    instruction::generateImm(gen, CALop, rd, 0, 1);

    // b??,a +2
    insn.clear();
    BFORM_OP_SET(insn, BCop);
    BFORM_BI_SET(insn, cond);
    BFORM_BO_SET(insn, mode);
    BFORM_BD_SET(insn, 2);		// + two instructions */
    BFORM_AA_SET(insn, 0);
    BFORM_LK_SET(insn, 0);
    insn.generate(gen);

    // clr rd
    instruction::generateImm(gen, CALop, rd, 0, 0);
}

// Given a value, load it into a register.
void instruction::loadImmIntoReg(codeGen &gen, Register rt, long value)
{
   // Writing a full 64 bits takes 5 instructions in the worst case.
   // Let's see if we use sign-extention to cheat.
   if (MIN_IMM16 <= value && value <= MAX_IMM16) {
      instruction::generateImm(gen, CALop,  rt, 0,  BOT_LO(value));
      
   } else if (MIN_IMM32 <= value && value <= MAX_IMM32) {
      instruction::generateImm(gen, CAUop,  rt, 0,  BOT_HI(value));
      instruction::generateImm(gen, ORILop, rt, rt, BOT_LO(value));
   } 
#if defined(arch_64bit)
   else if (MIN_IMM48 <= value && value <= MAX_IMM48) {
      instruction::generateImm(gen, CALop,  rt, 0,  TOP_LO(value));
      instruction::generateLShift64(gen, rt, 32, rt);
      if (BOT_HI(value))
         instruction::generateImm(gen, ORIUop, rt, rt, BOT_HI(value));
      if (BOT_LO(value))
         instruction::generateImm(gen, ORILop, rt, rt, BOT_LO(value));
      
   } else {
      instruction::generateImm(gen, CAUop,  rt,  0, TOP_HI(value));
      if (TOP_LO(value))
         instruction::generateImm(gen, ORILop, rt, rt, TOP_LO(value));
      instruction::generateLShift64(gen, rt, 32, rt);
      if (BOT_HI(value))
         instruction::generateImm(gen, ORIUop, rt, rt, BOT_HI(value));
      if (BOT_LO(value))
         instruction::generateImm(gen, ORILop, rt, rt, BOT_LO(value));
   }
#endif
}

// Helper method.  Fills register with partial value to be completed
// by an operation with a 16-bit signed immediate.  Such as loads and
// stores.
void instruction::loadPartialImmIntoReg(codeGen &gen, Register rt, long value)
{
   if (MIN_IMM16 <= value && value <= MAX_IMM16) return;
   
   if (BOT_LO(value) & 0x8000) {
      // high bit of lowest half-word is set, so the sign extension of
      // the next op will cause the wrong effective addr to be computed.
      // so we subtract the sign ext value from the other half-words.
      // sounds odd, but works and saves an instruction - jkh 5/25/95
      
      // Modified to be 64-bit compatible.  Use (-1 >> 16) instead of
      // 0xFFFF constant.
      value = ((value >> 16) - (-1 >> 16)) << 16;
   }
   
   if (MIN_IMM32 <= value && value <= MAX_IMM32) {
      instruction::generateImm(gen, CAUop,  rt, 0,  BOT_HI(value));       
   } 
#if defined(arch_64bit)
   else if (MIN_IMM48 <= value && value <= MAX_IMM48) {
      instruction::generateImm(gen, CALop,  rt, 0,  TOP_LO(value));
      instruction::generateLShift64(gen, rt, 32, rt);
      if (BOT_HI(value))
         instruction::generateImm(gen, ORIUop, rt, rt, BOT_HI(value));
      
   } else {
      instruction::generateImm(gen, CAUop,  rt,  0, TOP_HI(value));
      if (TOP_LO(value))
         instruction::generateImm(gen, ORILop, rt, rt, TOP_LO(value));
      instruction::generateLShift64(gen, rt, 32, rt);
      if (BOT_HI(value))
         instruction::generateImm(gen, ORIUop, rt, rt, BOT_HI(value));
   }
#endif
}

Address instruction::getBranchOffset() const {
    if (isUncondBranch()) {
        return (IFORM_LI(*this) << 2);
    }
    else if (isCondBranch()) {
        return (BFORM_BD(*this) << 2);
    }
    return 0;

}

Address instruction::getTarget(Address addr) const {
#if defined(os_vxworks)
    Address ret;
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

// "Casting" methods. We use a "base + offset" model, but often need to 
// turn that into "current instruction pointer".
codeBuf_t *instruction::insnPtr(codeGen &gen) {
    return (instructUnion *)gen.cur_ptr();
}

// Same as above, but increment offset to point at the next insn.
codeBuf_t *instruction::ptrAndInc(codeGen &gen) {
    instructUnion *ret = insnPtr(gen);
    gen.moveIndex(instruction::size());
    return ret;
}

void instruction::setInstruction(codeBuf_t *ptr, Address) {
    // We don't need the addr on this platform

    instructUnion *insnPtr = (instructUnion *)ptr;

#if defined(endian_mismatch)
    // Read an instruction from source.  Convert byte order if necessary.
    insn_.raw = swapBytesIfNeeded((*insnPtr).raw);
#else
    insn_.raw = (*insnPtr).raw;
#endif
}
void instruction::setInstruction(unsigned char *ptr, Address) {
    // We don't need the addr on this platform
    instructUnion *insnPtr = (instructUnion *)ptr;
    insn_ = *insnPtr;
}

void instruction::generate(codeGen &gen) {
    instructUnion *ptr = ptrAndInc(gen);

#if defined(endian_mismatch)
    // Writing an instruction.  Convert byte order if necessary.
    (*ptr).raw = swapBytesIfNeeded(insn_.raw);
#else
    (*ptr).raw = insn_.raw;
#endif
}

bool instruction::isUncondBranch() const {
    return isInsnType(Bmask, Bmatch);
}

bool instruction::isCondBranch() const {
    return isInsnType(Bmask, BCmatch);
}

unsigned instruction::jumpSize(Address from, Address to, unsigned addr_width) {
    Address disp = (to - from);
    return jumpSize(disp, addr_width);
}

// -1 is infinite, don't ya know.
unsigned instruction::jumpSize(Address disp, unsigned addr_width) {
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
      return 4*instruction::size();
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
    return 3*instruction::size();
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

bool instruction::generate(codeGen &gen,
                           AddressSpace * /*proc*/,
                           Address origAddr,
                           Address relocAddr,
                           patchTarget *fallthroughOverride,
                           patchTarget *targetOverride) {
    assert(fallthroughOverride == NULL);

    Address targetAddr = targetOverride ? targetOverride->get_address() : 0;
    long newOffset = 0;
    Address to;

    if (isThunk()) {
      // This is actually a "get PC" operation, and we want
      // to handle it as such. 
     
      // 1: get the original return address (value stored in LR)
      // This is origAddr + 4; optionally, check its displacement...
      Address origRet = origAddr + 4;
      
      // 2: find a scratch register and load this value into it
      instPoint *point = gen.point();
      // If we do not have a point then we have to invent one
      if (!point || (point->addr() != origAddr))
	point = instPoint::createArbitraryInstPoint(origAddr,
						    gen.addrSpace(),
						    gen.func());
      assert(point);
      
      // Could see if the codeGen has it, but right now we have assert
      // code there and we don't want to hit that.
      registerSpace *rs = registerSpace::actualRegSpace(point,
							callPreInsn);
      
      Register scratch = rs->getScratchRegister(gen, true);
      assert(scratch != REG_NULL);

      instruction::loadImmIntoReg(gen, scratch, origRet);

      // 3: push this value into the LR
      instruction::generateMoveToLR(gen, scratch);
    }
    else if (isUncondBranch()) {
        // unconditional pc relative branch.

#if defined(os_vxworks)
        if (!targetOverride) relocationTarget(origAddr, &targetAddr);
#endif

        // This was a check in old code. Assert it isn't the case,
        // since this is a _conditional_ branch...
        assert(isInsnType(Bmask, BCAAmatch) == false); 
        
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
                           IFORM_LK(*this));
        }
        else {
            generateBranch(gen,
                           relocAddr,
                           getTarget(origAddr),
                           IFORM_LK(*this));
        }
    } 
    else if (isCondBranch()) {
        // conditional pc relative branch.
#if defined(os_vxworks)
        if (!targetOverride) relocationTarget(origAddr, &targetAddr);
#endif

        if (!targetAddr) {
          newOffset = origAddr - relocAddr + getBranchOffset();
          to = origAddr + getBranchOffset();
        } else {
	  newOffset = targetAddr - relocAddr;
          to = targetAddr;
        }

        if (ABS(newOffset) >= MAX_CBRANCH) {
            if ((BFORM_BO(*this) & BALWAYSmask) == BALWAYScond) {
                assert(BFORM_BO(*this) == BALWAYScond);

                bool link = (BFORM_LK(*this) == 1);
                // Make sure to use the (to, from) version of generateBranch()
                // in case the branch is too far, and trap-based instrumentation
                // is needed.
                instruction::generateBranch(gen, relocAddr, to, link);

            } else {
                // Figure out if the original branch was predicted as taken or not
                // taken.  We'll set up our new branch to be predicted the same way
                // the old one was.
                
		// This makes my brain melt... here's what I think is happening. 
		// We have two sources of information, the bd (destination) 
		// and the predict bit. 
		// The processor predicts the jump as taken if the offset
		// is negative, and not taken if the offset is positive. 
		// The predict bit says "invert whatever you decided".
		// Since we're forcing the offset to positive, we need to
		// invert the bit if the offset was negative, and leave it
		// alone if positive.
              
		// Get the old flags (includes the predict bit)
		int flags = BFORM_BO(*this);

		if (BFORM_BD(*this) < 0) {
		    // Flip the bit.
		    // xor operator
		    flags ^= BPREDICTbit;
		}
              
		instruction newCondBranch(insn_);
		BFORM_LK_SET(newCondBranch, 0); // This one is non-linking for sure

		// Set up the flags
		BFORM_BO_SET(newCondBranch, flags);

		// Change the branch to move one instruction ahead
		BFORM_BD_SET(newCondBranch, 2);

		newCondBranch.generate(gen);

              // We don't "relocate" the fallthrough target of a conditional
              // branch; instead relying on a third party to make sure
              // we go back to where we want to. So in this case we 
              // generate a "dink" branch to skip past the next instruction.
              // We could also just invert the condition on the first branch;
              // but I don't have the POWER manual with me.
              // -- bernat, 15JUN05

              instruction::generateBranch(gen,
                                          2*instruction::size());

              bool link = (BFORM_LK(*this) == 1);
              // Make sure to use the (to, from) version of generateBranch()
              // in case the branch is too far, and trap-based instrumentation
              // is needed.
              instruction::generateBranch(gen,
                                          relocAddr + (2 * instruction::size()),
                                          to,
                                          link);
          }
      } else {
          instruction newInsn(insn_);
          BFORM_BD_SET(newInsn, newOffset >> 2);
          newInsn.generate(gen);
      }
#if defined(os_aix)
	// I don't understand why this is here, so we'll allow relocation
	// for Linux since it's a new port.
    } else if (IFORM_OP(*this) == SVCop) {
        logLine("attempt to relocate a system call\n");
        assert(0);
#endif
    }
    else {
#if defined(os_vxworks)
        if (relocationTarget(origAddr + 2, &targetAddr)) DFORM_SI_SET(*this, targetAddr);
#endif
        generate(gen);
    }
    return true;
}
                           
bool instruction::generateMem(codeGen &,
                              Address, 
                              Address,
                              Register,
                  Register) {return false; }

bool instruction::getUsedRegs(pdvector<int> &) {
	return false;
}

void instruction::generateMoveFromLR(codeGen &gen, Register rt) {
    instruction insn;
    insn.clear();
    XFORM_OP_SET(insn, MFSPRop);
    XFORM_RT_SET(insn, rt);
    XFORM_RA_SET(insn, SPR_LR & 0x1f);
    XFORM_RB_SET(insn, (SPR_LR >> 5) & 0x1f);
    XFORM_XO_SET(insn, MFSPRxop);
    insn.generate(gen);
}

void instruction::generateMoveToLR(codeGen &gen, Register rs) {
    instruction insn;
    insn.clear();
    XFORM_OP_SET(insn, MTSPRop);
    XFORM_RT_SET(insn, rs);
    XFORM_RA_SET(insn, SPR_LR & 0x1f);
    XFORM_RB_SET(insn, (SPR_LR >> 5) & 0x1f);
    XFORM_XO_SET(insn, MTSPRxop);
    insn.generate(gen);
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
