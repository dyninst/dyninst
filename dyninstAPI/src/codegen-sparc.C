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

#include <assert.h>
#include <stdio.h>

#include "codegen.h"
#include "inst-sparc.h"

#include "parseAPI/src/InstrucIter.h"

// "Casting" methods. We use a "base + offset" model, but often need to 
// turn that into "current instruction pointer".
instructUnion *insnCodeGen::insnPtr(codeGen &gen) {
    return (instructUnion *)gen.cur_ptr();
}

// Same as above, but increment offset to point at the next insn.
instructUnion *insnCodeGen::ptrAndInc(codeGen &gen) {
    instructUnion *ret = insnPtr(gen);
    gen.moveIndex(instruction::size());
    return ret;
}

void insnCodeGen::generateTrap(codeGen &gen) {
    instruction insn(BREAK_POINT_INSN);
    insnCodeGen::generate(gen,insn);
}

void insnCodeGen::generateIllegal(codeGen &gen) {
    instruction insn;
    insnCodeGen::generate(gen,insn);
}

void insnCodeGen::generateNOOP(codeGen &gen,
                               unsigned size)
{
    assert((size % instruction::size()) == 0);
    instruction insn;

    (*insn).raw = 0;
    (*insn).branch.op = 0;
    (*insn).branch.op2 = NOOPop2;
    // logLine("nop\n");
    while (size > 0) {
        insnCodeGen::generate(gen,insn);
        size -= instruction::size();
    }
}

void insnCodeGen::generateTrapRegisterSpill(codeGen &gen) {
    instruction insn;
    (*insn).raw = SPILL_REGISTERS_INSN;
    insnCodeGen::generate(gen,insn);
}

void insnCodeGen::generateFlushw(codeGen &gen) {
    instruction insn;
    
    (*insn).raw = 0;
    (*insn).rest.op = RESTop;
    (*insn).rest.op3 = FLUSHWop3;
    (*insn).rest.i = 0;
    insnCodeGen::generate(gen,insn);
}

void insnCodeGen::generateBranch(codeGen &gen, int jump_off)
{
    instruction insn;
    
    if (instruction::offsetWithinRangeOfBranchInsn(jump_off)) {
        (*insn).raw = 0;
        (*insn).branch.op = 0;
        (*insn).branch.cond = BAcond;
        (*insn).branch.op2 = BICCop2;
        (*insn).branch.anneal = true;
        (*insn).branch.disp22 = jump_off >> 2;
        // logLine("ba,a %x\n", offset);
        insnCodeGen::generate(gen,insn);
    }
    else {
        insnCodeGen::generateImm(gen, SAVEop3, 14, -112, 14);
        jump_off -= instruction::size(); // To compensate for the saveOp
        // Copied from generateCall...
        (*insn).raw = 0;
        (*insn).call.op = 01;
        (*insn).call.disp30 = jump_off >> 2;
        insnCodeGen::generate(gen,insn);
        insnCodeGen::generateSimple(gen, RESTOREop3, 0, 0, 0);
    }
}

void insnCodeGen::generateBranch(codeGen &gen, Address from, Address to)
{
    int disp = to - from;
    generateBranch(gen, disp);
}

void insnCodeGen::generateCall(codeGen &gen, 
                               Address fromAddr, 
                               Address toAddr)
{
    instruction insn;
    int dist = toAddr - fromAddr;
    (*insn).call.op = 01;
    (*insn).call.disp30 = dist >> 2;
    insnCodeGen::generate(gen,insn);
}

void insnCodeGen::generateJmpl(codeGen &gen, int rs1, int jump_off, 
                                   int rd)
{
    instruction insn;

    (*insn).resti.op = 10;
    (*insn).resti.rd = rd;
    (*insn).resti.op3 = JMPLop3;
    (*insn).resti.rs1 = rs1;
    (*insn).resti.i = 1;
    assert(jump_off >= MIN_IMM13 && jump_off <= MAX_IMM13);
    (*insn).resti.simm13 = jump_off;
    insnCodeGen::generate(gen,insn);
}    

void insnCodeGen::generateCondBranch(codeGen &gen, int jump_off, 
                                     unsigned condition, bool annul) 
{
    instruction insn;
    
    if (!instruction::offsetWithinRangeOfBranchInsn(jump_off)) {
        char buffer[80];
	sprintf(buffer, "a branch too far, jump_off=%d\n", jump_off);
	logLine(buffer);
	showErrorCallback(52, buffer);
	abort();
    }

    (*insn).raw = 0;
    (*insn).branch.op = 0;
    (*insn).branch.cond = condition;
    (*insn).branch.op2 = BICCop2;
    (*insn).branch.anneal = annul;
    (*insn).branch.disp22 = jump_off >> 2;
    insnCodeGen::generate(gen,insn);
}

void insnCodeGen::generateAnnulledBranch(codeGen &gen, int 
                                         jump_off)
{
    generateCondBranch(gen, jump_off, BAcond, true);
}


void insnCodeGen::generateSimple(codeGen &gen, int op,
        Register rs1, Register rs2, Register rd)
{
    instruction insn;

    (*insn).raw = 0;
    (*insn).rest.op = RESTop;
    (*insn).rest.rd = rd;
    (*insn).rest.op3 = op;
    (*insn).rest.rs1 = rs1;
    (*insn).rest.rs2 = rs2;
    insnCodeGen::generate(gen,insn);
}

void insnCodeGen::generateImm(codeGen &gen, int op,
        Register rs1, int immd, Register rd)
{
    instruction insn;

    (*insn).raw = 0;
    (*insn).resti.op = RESTop;
    (*insn).resti.rd = rd;
    (*insn).resti.op3 = op;
    (*insn).resti.rs1 = rs1;
    (*insn).resti.i = 1;
    assert(immd >= MIN_IMM13 && immd <= MAX_IMM13);
    (*insn).resti.simm13 = immd;
    insnCodeGen::generate(gen,insn);
}

void insnCodeGen::generateImmRelOp(codeGen &gen, int cond, Register rs1,
		        int immd, Register rd)
{
    // cmp rs1, rs2
    generateImm(gen, SUBop3cc, rs1, immd, 0);
    // mov 1, rd
    generateImm(gen, ORop3, 0, 1, rd);

    // b??,a +2

    generateCondBranch(gen, 2*instruction::size(), cond, true);

    // clr rd
    generateImm(gen, ORop3, 0, 0, rd);
}

void insnCodeGen::generateRelOp(codeGen &gen, int cond, Register rs1,
                                Register rs2, Register rd)
{
    // cmp rs1, rs2
    generateSimple(gen, SUBop3cc, rs1, rs2, 0);
    // mov 1, rd
    generateImm(gen, ORop3, 0, 1, rd);

    // b??,a +2
    generateCondBranch(gen, 2*instruction::size(), cond, true);

    // clr rd
    generateImm(gen, ORop3, 0, 0, rd);
}

void insnCodeGen::generateSetHi(codeGen &gen, int src1, int dest)
{
    instruction insn;

    (*insn).raw = 0;
    (*insn).sethi.op = FMT2op;
    (*insn).sethi.rd = dest;
    (*insn).sethi.op2 = SETHIop2;
    (*insn).sethi.imm22 = HIGH22(src1);
    insnCodeGen::generate(gen,insn);
}

// st rd, [rs1 + jump_off]
void insnCodeGen::generateStore(codeGen &gen, int rd, int rs1, 
                                int store_off)
{
    instruction insn;

    (*insn).resti.op = STop;
    (*insn).resti.rd = rd;
    (*insn).resti.op3 = STop3;
    (*insn).resti.rs1 = rs1;
    (*insn).resti.i = 1;
    assert(store_off >= MIN_IMM13 && store_off <= MAX_IMM13);
    (*insn).resti.simm13 = store_off;
    insnCodeGen::generate(gen,insn);
}

// sll rs1,rs2,rd
void insnCodeGen::generateLShift(codeGen &gen, int rs1, int shift, 
                                 int rd)
{
    instruction insn;

    (*insn).restix.op = SLLop;
    (*insn).restix.op3 = SLLop3;
    (*insn).restix.rd = rd;
    (*insn).restix.rs1 = rs1;
    (*insn).restix.i = 1;
    (*insn).restix.x = 0;
    (*insn).restix.rs2 = shift;
    insnCodeGen::generate(gen,insn);
}

// sll rs1,rs2,rd
void insnCodeGen::generateRShift(codeGen &gen, int rs1, int shift, 
                                 int rd)
{
    instruction insn;

    (*insn).restix.op = SRLop;
    (*insn).restix.op3 = SRLop3;
    (*insn).restix.rd = rd;
    (*insn).restix.rs1 = rs1;
    (*insn).restix.i = 1;
    (*insn).restix.x = 0;
    (*insn).restix.rs2 = shift;
    insnCodeGen::generate(gen,insn);
}

// load [rs1 + jump_off], rd
void insnCodeGen::generateLoad(codeGen &gen, int rs1, int load_off, 
                               int rd)
{
    instruction insn;

    (*insn).resti.op = LOADop;
    (*insn).resti.op3 = LDop3;
    (*insn).resti.rd = rd;
    (*insn).resti.rs1 = rs1;
    (*insn).resti.i = 1;
    assert(load_off >= MIN_IMM13 && load_off <= MAX_IMM13);
    (*insn).resti.simm13 = load_off;
    insnCodeGen::generate(gen,insn);
}

#if 0
// Unused
// swap [rs1 + jump_off], rd
void insnCodeGen::generateSwap(codeGen &gen, int rs1, int jump_off, 
                               int rd)
{
    instruction insn;

    (*insn).resti.op = SWAPop;
    (*insn).resti.rd = rd;
    (*insn).resti.op3 = SWAPop3;
    (*insn).resti.rs1 = rs1;
    (*insn).resti.i = 0;
    assert(jump_off >= MIN_IMM13 && jump_off <= MAX_IMM13);
    (*insn).resti.simm13 = jump_off;
}    
#endif

// std rd, [rs1 + jump_off]
void insnCodeGen::generateStoreD(codeGen &gen, int rd, int rs1, 
                            int store_off)
{
    instruction insn;

    (*insn).resti.op = STop;
    (*insn).resti.rd = rd;
    (*insn).resti.op3 = STDop3;
    (*insn).resti.rs1 = rs1;
    (*insn).resti.i = 1;
    assert(store_off >= MIN_IMM13 && store_off <= MAX_IMM13);
    (*insn).resti.simm13 = store_off;
    insnCodeGen::generate(gen,insn);
}

// ldd [rs1 + jump_off], rd
void insnCodeGen::generateLoadD(codeGen &gen, int rs1, int load_off, 
                           int rd)
{
    instruction insn;

    (*insn).resti.op = LOADop;
    (*insn).resti.op3 = LDDop3;
    (*insn).resti.rd = rd;
    (*insn).resti.rs1 = rs1;
    (*insn).resti.i = 1;
    assert(load_off >= MIN_IMM13 && load_off <= MAX_IMM13);
    (*insn).resti.simm13 = load_off;
    insnCodeGen::generate(gen,insn);
}

// ldub [rs1 + jump_off], rd
void insnCodeGen::generateLoadB(codeGen &gen, int rs1, int load_off, 
                           int rd)
{
    instruction insn;

    (*insn).resti.op = LOADop;
    (*insn).resti.op3 = LDSBop3;
    (*insn).resti.rd = rd;
    (*insn).resti.rs1 = rs1;
    (*insn).resti.i = 1;
    assert(load_off >= MIN_IMM13 && load_off <= MAX_IMM13);
    (*insn).resti.simm13 = load_off;
    insnCodeGen::generate(gen,insn);
}

// lduh [rs1 + jump_off], rd
void insnCodeGen::generateLoadH(codeGen &gen, int rs1, int load_off, 
                           int rd)
{
    instruction insn;

    (*insn).resti.op = LOADop;
    (*insn).resti.op3 = LDSHop3;
    (*insn).resti.rd = rd;
    (*insn).resti.rs1 = rs1;
    (*insn).resti.i = 1;
    assert(load_off >= MIN_IMM13 && load_off <= MAX_IMM13);
    (*insn).resti.simm13 = load_off;
    insnCodeGen::generate(gen,insn);
}

// std rd, [rs1 + jump_off]
void insnCodeGen::generateStoreFD(codeGen &gen, int rd, int rs1, 
                             int store_off)
{
    instruction insn;

    (*insn).resti.op = STop;
    (*insn).resti.rd = rd;
    (*insn).resti.op3 = STDFop3;
    (*insn).resti.rs1 = rs1;
    (*insn).resti.i = 1;
    assert(store_off >= MIN_IMM13 && store_off <= MAX_IMM13);
    (*insn).resti.simm13 = store_off;
    insnCodeGen::generate(gen,insn);
}

// ldd [rs1 + jump_off], rd
void insnCodeGen::generateLoadFD(codeGen &gen, int rs1, int load_off, 
                                 int rd)
{
    instruction insn;
    
    (*insn).resti.op = LOADop;
    (*insn).resti.op3 = LDDFop3;
    (*insn).resti.rd = rd;
    (*insn).resti.rs1 = rs1;
    (*insn).resti.i = 1;
    assert(load_off >= MIN_IMM13 && load_off <= MAX_IMM13);
    (*insn).resti.simm13 = load_off;
    insnCodeGen::generate(gen,insn);
}

bool insnCodeGen::generate(codeGen &gen,
                           instruction &insn,
                           AddressSpace *proc,
                           Address origAddr,
                           Address relocAddr,
                           patchTarget *fallthroughOverride,
                           patchTarget *targetOverride) {
    assert(fallthroughOverride == NULL);

   Address targetAddr = targetOverride ? targetOverride->get_address() : 0;
   long newLongOffset = 0;
   
   instruction newInsn(insn);
   
   // TODO: check the function relocation for any PC-calculation tricks.
   
   // If the instruction is a CALL instruction, calculate the new
   // offset
   if (insn.isInsnType(CALLmask, CALLmatch)) {
      // Check to see if we're a "get-my-pc" combo.
      // Two forms exist: call +8|+12, and call to a retl/nop 
      // pair. This is very similar to x86, amusingly enough.
      // If this is the case, we replace with an immediate load of 
      // the PC.
      
      // FIXME The target offset heuristic is rough may not capture all
      // cases. We should do a better job of detecting these false calls
      // during parsing and use that information (instead of inspection of
      // the individual call instruction) to determine whether this
      // is a real call or a "get-my-pc" combo.
      Address target = insn.getTarget(origAddr);
      if (target == origAddr + (2*instruction::size()))
      {
         inst_printf("Relocating get PC combo\n");
         insnCodeGen::generateSetHi(gen, origAddr, REG_O(7));
         insnCodeGen::generateImm(gen, ORop3, REG_O(7),
                                  LOW10(origAddr), REG_O(7));
      }
      else if(target == origAddr + (3*instruction::size())) {
         inst_printf("Relocating get PC combo (long form)\n");
         // for this case, we want to make sure the intervening
         // instructions are skipped, thereby preserving the
         // semantics of the original binary
         
         // load the PC into o7
         insnCodeGen::generateSetHi(gen, origAddr, REG_O(7));
         insnCodeGen::generateImm(gen, ORop3, REG_O(7),
                                  LOW10(origAddr), REG_O(7));
         
         // We've generated two instructions; update the offset accordingly
         newLongOffset = (long)target - (relocAddr + 2*instruction::size());
         //inst_printf("storing PC (0x%lx) into o7, adding branch to target 0x%lx (offset 0x%lx)\n",origAddr,target,newLongOffset);
         insnCodeGen::generateBranch(gen,newLongOffset);
      }
      else if (proc->isValidAddress(target)) {
         // Need to check the destination. Grab it with an InstrucIter
         InstrucIter callTarget(target, proc);
         instruction callInsn = callTarget.getInstruction();
         
         if (callInsn.isInsnType(RETLmask, RETLmatch)) {
            inst_printf("%s[%d]: Call to immediate return\n", FILE__, __LINE__);
            // The old version (in LocalAlteration-Sparc.C)
            // is _incredibly_ confusing. This should work, 
            // assuming an iterator-unwound delay slot.
            // We had a call to a retl/delay pair. 
            // Build O7, then copy over the delay slot.
            insnCodeGen::generateSetHi(gen, origAddr, REG_O(7));
            insnCodeGen::generateImm(gen, ORop3, REG_O(7),
                                     LOW10(origAddr), REG_O(7));
	    callTarget++;
	    instruction nextInsn = callTarget.getInstruction();
            if (nextInsn.valid()) {
               insnCodeGen::generate(gen,nextInsn);
            }
         }
         else {
            if (!targetAddr) {
               newLongOffset = origAddr;
               newLongOffset -= relocAddr;
               newLongOffset += ((*insn).call.disp30 << 2);
            }
            else {
               newLongOffset = targetAddr - relocAddr;
            }
            
            (*newInsn).call.disp30 = (int)(newLongOffset >> 2);
            
            insnCodeGen::generate(gen,newInsn);
            inst_printf("%s[%d]: Relocating call, displacement 0x%x, "
                        "relocAddr = 0x%x\n", FILE__, __LINE__, 
                        newLongOffset, relocAddr);
         }
      }
   } else if (insn.isInsnType(BRNCHmask, BRNCHmatch)||
              insn.isInsnType(FBRNCHmask, FBRNCHmatch)) {
      
      // If the instruction is a Branch instruction, calculate the 
      // new offset. If the new offset is out of reach after the 
      // instruction is moved to the base Trampoline, we would do
      // the following:
      //    b  address  ......    address: save
      //                                   call new_offset             
      //                                   restore 
      if (!targetAddr)
         newLongOffset = (long long)insn.getTarget(origAddr) - relocAddr;
      else
         newLongOffset = targetAddr - relocAddr;
      inst_printf("Orig 0x%x, target 0x%x (offset 0x%x), relocated 0x%x, "
                  "new dist 0x%lx\n", origAddr, insn.getTarget(origAddr), 
                  insn.getOffset(), relocAddr, newLongOffset);
      // if the branch is too far, then allocate more space in inferior
      // heap for a call instruction to branch target.  The base tramp 
      // will branch to this new inferior heap code, which will call the
      // target of the branch
      
      // XXX doesn't insnCodeGen::generateBranch take care of this for you?
      
      if (!instruction::offsetWithinRangeOfBranchInsn((int)newLongOffset)){
         inst_printf("Relocating branch (orig 0x%lx to 0x%lx, now 0x%lx to "
                     "0x%lx); new offset 0x%lx farther than branch range; " 
                     "replacing with call\n", origAddr, insn.getTarget(origAddr), 
                     relocAddr, targetAddr ? targetAddr : insn.getTarget(origAddr), 
                     newLongOffset);
         // Replace with a multi-branch series
         insnCodeGen::generateImm(gen,
                                  SAVEop3,
                                  REG_SPTR,
                                  -112,
                                  REG_SPTR);
         // Don't use relocAddr here, since we've moved the IP since then.
         insnCodeGen::generateCall(gen,
                                   relocAddr + instruction::size(),
                                   targetAddr ? targetAddr : insn.getTarget(origAddr));
         insnCodeGen::generateSimple(gen,
                                     RESTOREop3,
                                     0, 0, 0);
      } else {
         (*newInsn).branch.disp22 = (int)(newLongOffset >> 2);
         insnCodeGen::generate(gen,newInsn);
      }
   } else if (insn.isInsnType(TRAPmask, TRAPmatch)) {
      // There should be no probelm for moving trap instruction
      // logLine("attempt to relocate trap\n");
      insnCodeGen::generate(gen,insn);
   } 
   else {
      /* The rest of the instructions should be fine as is */
      insnCodeGen::generate(gen,insn);
   }
   return true;
}

void insnCodeGen::generate(codeGen &gen, instruction &insn) {
    instructUnion *ptr = ptrAndInc(gen);
    *ptr = *insn;
}

void insnCodeGen::write(codeGen &gen, instruction &insn) {
    instructUnion *ptr = insnPtr(gen);
    *ptr = *insn;
}

bool insnCodeGen::generateMem(codeGen &,
                              instruction &,
                              Address, 
                              Address,
                              Register,
                  Register) {return false; }

