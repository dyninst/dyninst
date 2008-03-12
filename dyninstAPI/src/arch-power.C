/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/*
 * inst-power.C - Identify instrumentation points for a RS6000/PowerPCs
 * $Id: arch-power.C,v 1.26 2008/03/12 20:09:00 legendre Exp $
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
		ABS(disp), MAX_BRANCH);
	bperr( "Error: attempted a branch of 0x%lx\n", disp);
	logLine("a branch too far\n");
	showErrorCallback(52, "Internal error: branch too far");
	bperr( "Attempted to make a branch of offset 0x%lx\n", disp);
	assert(0);
    }

    instruction insn;
    insn.insn_.iform.op = Bop;
    insn.insn_.iform.li = disp >> 2;
    insn.insn_.iform.aa = 0;
    if (link)
        insn.insn_.iform.lk = 1;
    else
        insn.insn_.iform.lk = 0;

    insn.generate(gen);
}

void instruction::generateBranch(codeGen &gen, Address from, Address to, bool link) {
    if (to < MAX_BRANCH) {
        // Generate an absolute branch
        instruction insn;
        insn.insn_.iform.op = Bop;
        insn.insn_.iform.li = to >> 2;
        insn.insn_.iform.aa = 1;
        if (link) 
            insn.insn_.iform.lk = 1;
        else
            insn.insn_.iform.lk = 0;
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
                                              Address to) {
    long disp = to - from;

    if (ABS(disp) <= MAX_BRANCH) {
        // We got lucky...
        return generateBranch(gen, from, to);
    }

    instruction::loadImmIntoReg(gen, 0, to);

    instruction insn;
    (*insn).raw = 0;                    //mtspr:  mtctr scratchReg
    (*insn).xform.op = MTSPRop;
    (*insn).xform.rt = 0;
    (*insn).xform.ra = SPR_CTR & 0x1f;
    (*insn).xform.rb = (SPR_CTR >> 5) & 0x1f;
    (*insn).xform.xo = MTSPRxop;
    insn.generate(gen);

    // And branch to CTR
    instruction btctr(BCTRraw);
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
    assert(gen.func());
    
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
    
    instructUnion moveToBr;
    moveToBr.raw = 0;
    moveToBr.xfxform.op = MTSPRop;
    moveToBr.xfxform.rt = scratch;
    if (branchRegister == registerSpace::lr) {
        moveToBr.xform.ra = SPR_LR & 0x1f;
        moveToBr.xform.rb = (SPR_LR >> 5) & 0x1f;
        // The two halves (top 5 bits/bottom 5 bits) are _reversed_ in this encoding. 
    }
    else {
        moveToBr.xform.ra = SPR_CTR & 0x1f;
        moveToBr.xform.rb = (SPR_CTR >> 5) & 0x1f;
    }
    moveToBr.xfxform.xo = MTSPRxop; // From assembly manual
    
    instruction m1(moveToBr.raw);
    m1.generate(gen);
    
    if (mustRestore) {
        if (gen.addrSpace()->getAddressWidth() == 4)
            instruction::generateImm(gen, Lop, 0, 1, 4*4);
        else /* gen.addrSpace()->getAddressWidth() == 8 */
            instruction::generateMemAccess64(gen, LDop, LDxop, 0, 1, 4*8);
    }
    
    // Aaaand now branch, linking if appropriate
    instructUnion branchToBr;
    branchToBr.raw = 0;
    branchToBr.xlform.op = BCLRop;
    branchToBr.xlform.bt = 0x14; // From architecture manual
    branchToBr.xlform.ba = 0; // Unused
    branchToBr.xlform.bb = 0; // Unused
    if (branchRegister == registerSpace::lr) {
        branchToBr.xlform.xo = BCLRxop;
    }
    else {
        branchToBr.xlform.xo = BCCTRxop;
    }
    if (isCall)
        branchToBr.xlform.lk = 1;
    else
        branchToBr.xlform.lk = 0;
    
    instruction m2(branchToBr.raw);
    m2.generate(gen);
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
                ABS(disp), MAX_BRANCH);
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
  
  (*insn).raw = 0;
  (*insn).dform.op = op;
  (*insn).dform.rt = rt;
  (*insn).dform.ra = ra;
  if (op==SIop) immd = -immd;
  (*insn).dform.d_or_si = immd;

  insn.generate(gen);
}

void instruction::generateMemAccess64(codeGen &gen, int op, int xop, Register r1, Register r2, int immd)
{
    assert(MIN_IMM16 <= immd && immd <= MAX_IMM16);
    assert((immd & 0x3) == 0);

    instruction insn;

    (*insn).raw = 0;
    (*insn).dsform.op = op;
    (*insn).dsform.rt = r1;
    (*insn).dsform.ra = r2;
    (*insn).dsform.ds = immd >> 2;
    (*insn).dsform.xo = xop;

    insn.generate(gen);
}

// rlwinm ra,rs,n,0,31-n
void instruction::generateLShift(codeGen &gen, Register rs, int shift, Register ra)
{
    instruction insn;

    if (gen.addrSpace()->getAddressWidth() == 4) {
	assert(shift<32);
	(*insn).raw = 0;
	(*insn).mform.op = RLINMxop;
	(*insn).mform.rs = rs;
	(*insn).mform.ra = ra;
	(*insn).mform.sh = shift;
	(*insn).mform.mb = 0;
	(*insn).mform.me = 31-shift;
	(*insn).mform.rc = 0;
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
	(*insn).raw = 0;
	(*insn).mform.op = RLINMxop;
	(*insn).mform.rs = rs;
	(*insn).mform.ra = ra;
	(*insn).mform.sh = 32-shift;
	(*insn).mform.mb = shift;
	(*insn).mform.me = 31;
	(*insn).mform.rc = 0;
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
    (*insn).raw = 0;
    (*insn).mdform.op  = RLDop;
    (*insn).mdform.rs  = rs;
    (*insn).mdform.ra  = ra;
    (*insn).mdform.sh  = shift % 32;
    (*insn).mdform.mb  = (63-shift) % 32;
    (*insn).mdform.mb2 = (63-shift) / 32;
    (*insn).mdform.xo  = ICRxop;
    (*insn).mdform.sh2 = shift / 32;
    (*insn).mdform.rc  = 0;

    insn.generate(gen);
}

// srd ra, rs, rb
void instruction::generateRShift64(codeGen &gen, Register rs, int shift, Register ra)
{
    instruction insn;

    assert(shift<64);
    (*insn).raw = 0;
    (*insn).mdform.op  = RLDop;
    (*insn).mdform.rs  = rs;
    (*insn).mdform.ra  = ra;
    (*insn).mdform.sh  = (64 - shift) % 32;
    (*insn).mdform.mb  = shift % 32;
    (*insn).mdform.mb2 = shift / 32;
    (*insn).mdform.xo  = ICLxop;
    (*insn).mdform.sh2 = (64 - shift) / 32;
    (*insn).mdform.rc  = 0;

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
  (*insn).raw = 0;
  (*insn).xform.op = op;
  (*insn).xform.rt = src1;
  (*insn).xform.ra = dest;
  (*insn).xform.rb = src2;
  if (op==ANDop) {
      xop=ANDxop;
  } else if (op==ORop) {
      xop=ORxop;
  } else {
      // only AND and OR are currently designed to use genSimpleInsn
      assert(0);
  }
  (*insn).xform.xo = xop;
  insn.generate(gen);
}

void instruction::generateRelOp(codeGen &gen, int cond, int mode, Register rs1,
                                Register rs2, Register rd)
{
    instruction insn;

    // cmp rs1, rs2
    (*insn).raw = 0;
    (*insn).xform.op = CMPop;
    (*insn).xform.rt = 0;    // really bf & l sub fields of rt we care about
    (*insn).xform.ra = rs1;
    (*insn).xform.rb = rs2;
    (*insn).xform.xo = CMPxop;

    insn.generate(gen);

    // li rd, 1
    instruction::generateImm(gen, CALop, rd, 0, 1);

    // b??,a +2
    (*insn).raw = 0;
    (*insn).bform.op = BCop;
    (*insn).bform.bi = cond;
    (*insn).bform.bo = mode;
    (*insn).bform.bd = 2;		// + two instructions */
    (*insn).bform.aa = 0;
    (*insn).bform.lk = 0;
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
        return (insn_.iform.li << 2);
    }
    else if (isCondBranch()) {
        return (insn_.bform.bd << 2);
    }
    return 0;

}

Address instruction::getTarget(Address addr) const {
    if (isUncondBranch() || isCondBranch()) {
        return getBranchOffset() + addr;
    }
    else if (isInsnType(Bmask, BAAmatch)) // Absolute
        return (insn_.iform.li << 2);
    else if (isInsnType(Bmask, BCAAmatch)) // Absolute
        return (insn_.bform.bd << 2);

    return 0;
}

// TODO: argument _needs_ to be an int, or ABS() doesn't work.
void instruction::setBranchOffset(Address newOffset) {
    if (isUncondBranch()) {
        assert(ABS((int) newOffset) < MAX_BRANCH);
        insn_.iform.li = (newOffset >> 2);
    }
    else if (isCondBranch()) {
        assert(ABS(newOffset) < MAX_CBRANCH);
        insn_.bform.bd = (newOffset >> 2);
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
    insn_ = *insnPtr;
}

void instruction::generate(codeGen &gen) {
    instructUnion *ptr = ptrAndInc(gen);
    *ptr = insn_;
}

void instruction::write(codeGen &gen) {
    instructUnion *ptr = insnPtr(gen);
    *ptr = insn_;
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
    
    if (isCondBranch()) {
        // Maybe... so worst-case
        if ((insn_.bform.bo & BALWAYSmask) != BALWAYScond) {
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
                           AddressSpace *proc,
                           Address origAddr,
                           Address relocAddr,
                           patchTarget */* fallthroughOverride */,
                           patchTarget *targetOverride) {
    Address targetAddr = targetOverride ? targetOverride->get_address() : 0;
    long newOffset = 0;
    Address to;

    if (isUncondBranch()) {
        // unconditional pc relative branch.

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
                           (insn_.iform.lk));
        }
        else {
            generateBranch(gen,
                           relocAddr,
                           getTarget(origAddr),
                           (insn_.iform.lk));
        }
    } 
    else if (isCondBranch()) {
        // conditional pc relative branch.
        if (!targetAddr) {
          newOffset = origAddr - relocAddr + getBranchOffset();
          to = origAddr + getBranchOffset();
        } else {
	  newOffset = targetAddr - relocAddr;
          to = targetAddr;
        }

        if (ABS(newOffset) >= MAX_CBRANCH) {
            if ((insn_.bform.bo & BALWAYSmask) == BALWAYScond) {
                assert(insn_.bform.bo == BALWAYScond);

                bool link = (insn_.bform.lk == 1);
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
		int flags = insn_.bform.bo;

		if (insn_.bform.bd < 0) {
		    // Flip the bit.
		    // xor operator
		    flags ^= BPREDICTbit;
		}
              
		instruction newCondBranch(insn_);
		(*newCondBranch).bform.lk = 0; // This one is non-linking for sure

		// Set up the flags
		(*newCondBranch).bform.bo = flags;

		// Change the branch to move one instruction ahead
		(*newCondBranch).bform.bd = 2;

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

              bool link = (insn_.bform.lk == 1);
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
          (*newInsn).bform.bd = (newOffset >> 2);
          newInsn.generate(gen);
      }
#if defined(os_aix)
	// I don't understand why this is here, so we'll allow relocation
	// for Linux since it's a new port.
    } else if (insn_.iform.op == SVCop) {
        logLine("attempt to relocate a system call\n");
        assert(0);
#endif
    } 
    else {
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

bool image::isAligned(const Address where) const {
   return (!(where & 0x3));
}
