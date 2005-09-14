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

// $Id: arch-alpha.C,v 1.5 2005/09/14 21:21:31 bernat Exp $

#include "common/h/headers.h"
#include "alpha.h"
#include "arch.h"
#include "showerror.h"
#include "util.h"

// BIS (or)  R31, R31, R31

void instruction::generateNOOP(codeGen &gen,
                               unsigned size) { 
  assert((size % instruction::size()) == 0);

    instruction insn(0);
    
    (*insn).raw = 0;
    // insn.oper.zero = 0; insn.oper.sbz = 0;
    (*insn).oper.opcode = OP_BIS;
    (*insn).oper.function = FC_BIS;
    (*insn).oper.ra = 31;
    (*insn).oper.rb = 31;
    (*insn).oper.rc = 31;
    while (size > 0) {
        insn.generate(gen);
        size -= instruction::size();
    }
}


void instruction::generateIllegal(codeGen &gen) {
    instruction insn(0);
    insn.generate(gen);
}

void instruction::generateTrap(codeGen &gen) {
    instruction insn(BREAK_POINT_INSN);
    insn.generate(gen);
}

// 
void instruction::generateOperate(codeGen &gen, Register reg_a, 
				  Register reg_b, 
				  Register reg_c, 
				  unsigned int opcode, 
				  unsigned int func_code) {
  instruction insn(0);
  
  assert(reg_a < Num_Registers);
  assert(reg_b < Num_Registers);
  assert(reg_c < Num_Registers);
  (*insn).raw = 0;
  // (*insn).oper.zero = 0; (*insn).oper.sbz = 0;
  (*insn).oper.ra = reg_a; (*insn).oper.rb = reg_b; (*insn).oper.rc = reg_c;
  (*insn).oper.opcode = opcode;
  (*insn).oper.function = func_code;
  insn.generate(gen);
}

// 
//
// unsigned
// generate_lit_operate(instruction *insn, unsigned reg_a, int literal,
//		     unsigned reg_c, unsigned opcode, unsigned func_code) {
void instruction::generateLitOperate(codeGen &gen, Register reg_a, int literal,
				     Register reg_c, unsigned int opcode, 
				     unsigned int func_code) {
  instruction insn(0);

  assert(reg_a < Num_Registers);
  // literal is 0 to 255
  assert((literal >= 0) && (literal < MAX_IMM));
  assert(reg_c < Num_Registers);
  (*insn).raw = 0;
  (*insn).oper_lit.one = 1;
  (*insn).oper_lit.ra = reg_a;
  (*insn).oper_lit.lit = literal;
  (*insn).oper_lit.rc = reg_c;
  (*insn).oper_lit.opcode = opcode;
  (*insn).oper_lit.function = func_code;

  insn.generate(gen);
}

// j?? ra, (rb), hint
// jump to rb, ra gets return address, displacement field provides 
// instruction fetch hint
// TODO -- provide a decent hint
// unsigned
// generate_jump(instruction *insn, unsigned dest_reg, unsigned ext_code,
//	      unsigned ret_reg, int hint) {
void instruction::generateJump(codeGen &gen,
			       Register dest_reg, unsigned long ext_code,
			       Register ret_reg, int hint) {
  instruction insn(0);

  assert(dest_reg < Num_Registers); 
  assert(ret_reg < Num_Registers); 
  assert((ext_code == MD_JSR) || (ext_code == MD_JMP));

  // If you want to use this for other ext_code values, then modify the branch
  // prediction

  (*insn).raw = 0;
  (*insn).mem_jmp.opcode = OP_MEM_BRANCH;
  (*insn).mem_jmp.ext = ext_code;
  (*insn).mem_jmp.ra = ret_reg;
  (*insn).mem_jmp.rb = dest_reg;
  (*insn).mem_jmp.disp = hint;
  insn.generate(gen);
}


// src_reg --> for unconditional branch, gets return address
// src_reg --> for conditional branch, is register to test
// b??  r31, offset   <-- return address written to r31, which is ignored
//
// unsigned
// generate_branch(instruction *insn, unsigned src_reg, int offset, unsigned opcode) {
void instruction::generateBranch(codeGen &gen, Register src_reg, int disp,
				 unsigned int opcode) {

  instruction insn(0);

  if (ABS(disp >> 2) > MAX_BRANCH) 
        { logLine("a branch too far\n"); assert(0); }
  assert(src_reg < Num_Registers);
  (*insn).raw = 0;
  (*insn).branch.opcode = opcode;
  (*insn).branch.ra = src_reg;
  (*insn).branch.disp = disp >> 2;

  insn.generate(gen);
}

void instruction::generateBSR(codeGen &gen,int disp) {
  generateBranch(gen,REG_RA,disp,OP_BSR);
}

void instruction::generateBranch(codeGen &gen, int disp) {
    generateBranch(gen, REG_ZERO, disp, OP_BR);
}

void instruction::generateBranch(codeGen &gen, Address from, Address to) {
    int displacement = to - (from + instruction::size());
    generateBranch(gen, displacement);
}

unsigned instruction::jumpSize(Address from, Address to) {
    int disp = (to - from);
    return jumpSize(disp);
}

unsigned instruction::jumpSize(int disp) {
    if (ABS(disp >> 2) > MAX_BRANCH) {
        fprintf(stderr, "Warning: Alpha doesn't handle multi-word jumps!\n");
    }
    return instruction::size();
}

unsigned instruction::maxJumpSize() {
    // TODO: full-range branch
    return instruction::size();
}

// 
// Set condition flag (a register)
// unsigned
// genRelOp(instruction *insn, unsigned opcode, unsigned fcode,
//	 unsigned src1, unsigned src2, unsigned dest, bool do_not=false) {
void instruction::generateRel(codeGen &gen,
			      unsigned opcode, unsigned fcode, Register src1,
			      Register src2, Register dest, 
			      bool Imm, bool do_not) {
  // cmp?? src1, src2, t8      : leave result in t8
  //  unsigned words = generate_operate(insn, src1, src2, dest, opcode, fcode);
  if (Imm)
    generateLitOperate(gen, src1, src2, dest, opcode, fcode);
  else
    generateOperate(gen, src1, src2, dest, opcode, fcode);

  if (do_not) {
    // since there is no cmpne -- cmpeq is used and the result must be inverted
    // 0 -->  1,  1 ---> 0
    // negate t8, and add 1  (1 --> -1 + 1 = 0,  0 --> -0 +1 = 1 )
    generateOperate(gen, REG_ZERO, dest, dest, OP_SUBL, FC_SUBL);
    generateLitOperate(gen, dest, 1, dest, OP_ADDL, FC_ADDL);
    // dest now contains a 0 or a 1 
  }
}
// lda(h) rdest, literal:16(rstart)
// put a literal in a register
// offset is the value before the shift
// unsigned 
// generate_lda(instruction *insn, unsigned rdest, unsigned rstart,
//	     int offset, bool do_low) {
void instruction::generateLDA(codeGen &gen, Register rdest, Register rstart,
			      long disp, bool do_low) {


  instruction insn(0);

  // this is not wuite the correct check, but some calls supply the actual
  //   signed displacement and others supply the raw 16 bits (i.e. not sign
  //   extended.  - jkh
  assert(ABS(disp) < (int) shifted_16);
  // assert(disp >= (- (long) 0xffffffff));
  assert(rdest < Num_Registers);
  assert(rstart < Num_Registers);

  (*insn).raw = 0;
  (*insn).mem.opcode = do_low ? OP_LDA : OP_LDAH;
  (*insn).mem.ra = rdest;
  (*insn).mem.rb = rstart;
  (*insn).mem.disp = disp;
  insn.generate(gen);
}

// left == true, shift left
// left == false, shift right
// unsigned
// amount is a literal
// generate_sxl(instruction *insn, unsigned rdest, unsigned amount,
//	     bool left, unsigned rsrc) {
void instruction::generateSXL(codeGen &gen, Register rdest, unsigned long amount,
			      bool left, Register rsrc) {

  instruction insn(0);


  assert(rdest < Num_Registers);
  assert(amount < Num_Registers);
  assert(rsrc < Num_Registers);
  (*insn).raw = 0;
  // (*insn).oper.sbz = 0;
  // (*insn).oper.zero = 0;

  (*insn).oper_lit.opcode = left ? OP_SLL : OP_SRL;
  (*insn).oper_lit.function = left ? FC_SLL : FC_SRL;
  (*insn).oper_lit.one = 1;
  (*insn).oper_lit.ra = rsrc;
  (*insn).oper_lit.lit = amount;
  (*insn).oper_lit.rc = rdest;
  insn.generate(gen);
}


// remainder returns 15 bit offset to be included in load or store
// returns number of instruction words
// unsigned generate_address(instruction *insn, 
//			 unsigned rdest,
//			 const Address &addr,
//			 int& remainder) {
void instruction::generateAddress(codeGen &gen,
				  Register rdest,
				  const Address &addr,
				  int& remainder) {
  Offset tempAddr = addr;
  Offset low = tempAddr & 0xffff;
  tempAddr -= SEXT_16(low);
  Offset high = (tempAddr >> 16) & 0xffff;
  tempAddr -= (SEXT_16(high) << 16);
  Offset third = (tempAddr >> 32) & 0xffff;
  
  assert((Address)SEXT_16(low) +
	 ((Address)SEXT_16(high)<<16) +
	 ((Address)SEXT_16(third)<<32) == addr);
  
  bool primed = false;
  if (third) {
    primed = true;
    assert(third < shifted_16);
    generateLDA(gen, rdest, REG_ZERO, third, false);
    generateSXL(gen, rdest, 16, true, rdest);  // move (31:15) to (47:32)
  }
  
  if (high) {
    if (primed)
      generateLDA(gen, rdest, rdest, high, false);
    else
      generateLDA(gen, rdest, REG_ZERO, high, false);
    primed = true;
  }
  // a better solution is to use register zero as the base
  // clear the base register
  if (!primed) 
    generateOperate(gen, REG_ZERO, REG_ZERO, rdest, 
		    OP_BIS, FC_BIS);

  remainder = low;
}

// stx rsrc, [rbase + sign_extend(offset:16)]
// unsigned
// generate_store(instruction* insn, unsigned rsrc, unsigned rbase, int disp,
//	       data_width width) {
void instruction::generateStore(codeGen &gen, Register rsrc, Register rbase, 
				int disp, data_width width) {

  instruction insn(0);

  assert(ABS(disp) < (int) shifted_16); 
  assert(rsrc < Num_Registers); 
  assert(rbase < Num_Registers);
  (*insn).raw = 0;
  (*insn).mem.disp = FIRST_16(disp);
  (*insn).mem.ra = rsrc;
  (*insn).mem.rb = rbase;
  switch (width) {
  case dw_long: (*insn).mem.opcode = OP_STL; break;
  case dw_quad: (*insn).mem.opcode = OP_STQ; break;
  default: assert(0);
  }

  insn.generate(gen);
}

// ldx rdest, [rbase + sign_extend(offset:16)]
// unsigned
// generate_load(instruction *insn, unsigned rdest, unsigned rbase, int disp,
//	      data_width width) {
void instruction::generateLoad(codeGen &gen, Register rdest, Register rbase, 
			       int disp, data_width width, bool aligned) {

  instruction insn(0);


  assert(ABS(disp) < (int) shifted_16); 
  assert(rdest < Num_Registers);
  assert(rbase < Num_Registers);
  (*insn).raw = 0;
  (*insn).mem.disp = disp;
  (*insn).mem.ra = rdest;
  (*insn).mem.rb = rbase;
  switch (width) {
  case dw_long: (*insn).mem.opcode = OP_LDL; break;
  case dw_quad: (*insn).mem.opcode = OP_LDQ; break;
  case dw_byte: (*insn).mem.opcode = OP_LDBU; break;
  case dw_word: (*insn).mem.opcode = OP_LDWU; break;
  default: assert(0);
  }
  if (aligned == false)
      (*insn).mem.opcode = OP_LDQ_U;

  insn.generate(gen);

  if (width == dw_long || width == dw_quad)
      return;

  // Add in the sign extension code
  (*insn).raw = 0;
  (*insn).oper.ra = 31;
  (*insn).oper.rb = rdest;
  (*insn).oper.rc = rdest;
  (*insn).oper.opcode = OP_SEXTX;
  if (width == dw_byte)
     (*insn).oper.function = FC_SEXTB;
  else
     (*insn).oper.function = FC_SEXTW;
  insn.generate(gen);
}


void instruction::generateIntegerOp(codeGen &gen, Register src1, Register src2, 
				    Register dest, opCode op, bool Imm) {

  // integer ops
//  unsigned op_code=0, func_code=0, words = 0;
  unsigned op_code=0, func_code=0;

  switch (op) {

  case plusOp:        op_code = OP_ADDQ; func_code = FC_ADDQ; break;
  case minusOp:       op_code = OP_SUBQ; func_code = FC_SUBQ; break;
  case timesOp:       op_code = OP_MULQ; func_code = FC_MULQ; break;
  case divOp:         assert(0); // shouldn't get here. Call software_divide from the ast
  case orOp:          op_code = OP_BIS;  func_code = FC_BIS; break;
  case andOp:         op_code = OP_AND;  func_code = FC_AND; break;

  // The compare instruction leave a one in dest if the compare is true
  // beq branch when dest is 0 --> and the compare is false

  case eqOp:
    generateRel(gen, OP_CMPEQ, FC_CMPEQ, 
		src1, src2, dest,
		Imm, false);
    return;

  case neOp:
    // last arg == true --> inverts value of comparison		      
    generateRel(gen, OP_CMPEQ, FC_CMPEQ, 
		src1, src2, dest,
		Imm,true);
    return;

  case lessOp:
    generateRel(gen, OP_CMPLT, FC_CMPLT, 
		src1, src2, dest,
		Imm, false);
    return;
		      
  case greaterOp:                          
    generateRel(gen, OP_CMPLE, FC_CMPLE, 
		src1, src2, dest,
		Imm,true);
    return;

  case leOp:
    generateRel(gen, OP_CMPLE, FC_CMPLE, 
		src1, src2, dest,
		Imm, false);
    return;

  case geOp:                               
    generateRel(gen, OP_CMPLE, FC_CMPLT, 
		src1, src2, dest,
		Imm,true);
    return;

  default:
    assert(0);
    break;
  }

  if (Imm) {
    int imm = (int) src2;
    if ((imm >= 0) && (imm < MAX_IMM)) {
      generateLitOperate(gen, src1, imm, dest, op_code, 
		  func_code);
    } else {
      generateOperate(gen, src1, imm, dest, op_code, func_code);
    }
  } else {
    generateOperate(gen, src1, src2, dest, op_code, func_code);
  }
}

bool instruction::isReturn() const {
    return ((insn_.mem_jmp.opcode == OP_MEM_BRANCH) &&
	    (insn_.mem_jmp.ext == MD_RET));
}

bool instruction::isCall() const {
  return (isBsr() || isJsr());
}

// "Casting" methods. We use a "base + offset" model, but often need to 
// turn that into "current instruction pointer".
codeBuf_t *instruction::insnPtr(codeGen &gen) {
    return (instructUnion *)gen.cur_ptr();
}

// Same as above, but increment offset to point at the next insn.
instructUnion *instruction::ptrAndInc(codeGen &gen) {
   instructUnion *ret = insnPtr(gen);
    gen.moveIndex(instruction::size());
    return ret;
}

void instruction::setInstruction(codeBuf_t *ptr, Address) {
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

Address instruction::getTarget(Address origAddr) const {
  return origAddr + getBranchOffset();
}

Address instruction::getBranchOffset() const {
  return insn_.branch.disp << 2;
}

instruction *instruction::copy() const {
    return new instruction(*this);
}

bool instruction::valid() const {

#if 0
  unsigned opcode = insn_.mem.opcode;
  unsigned function = insn_.oper.function;
  unsigned ext = insn_.mem_jmp.ext;
#endif
  // Could do a big tree... but not worth it.
  return (insn_.raw != 0);
#if 0


  return (opcode == 0x00 ||
	  opcode == 0x01 ||
	  opcode == 0x02 ||
	  opcode == 0x03 ||
	  opcode == 0x04 ||
	  opcode == 0x05 ||
	  opcode == 0x06 ||
	  opcode == 0x07 ||
	  opcode == 0x08 ||
	  opcode == 0x09 ||
	  opcode == 0x0a ||
	  opcode == 0x0b ||
	  opcode == 0x0c ||
	  opcode == 0x0d ||
	  opcode == 0x0e ||
	  opcode == 0x0f ||
	  (opcode == 0x10 &&
	   (function == 0x00 ||
	    function == 0x02 ||
	    function == 0x09 ||
	    function == 0x00 ||
	    function == 0x00 ||
	    function == 0x00 ||
	    function == 0x00 ||
	    function == 0x00 ||
	    function == 0x00 ||
	    function == 0x00 ||
	    function == 0x00 ||
	    function == 0x00 ||
	    function == 0x00 ||
	    function == 0x00 ||
	    function == 0x00 ||
	    function == 0x00 ||
	    function == 0x00 ||
	    function == 0x00 ||
	    
	    
  return (((insn_.mem.opcode >= 0x20) &&
	   (insn_.mem.opcode <= 0x2f)) ||
	  (insn_.mem.opcode == 0x0b) ||
	  (insn_.mem.opcode == 0x0d) || // stw
	  (insn_.mem.opcode == 0x0e) || // stb
	  (insn_.mem.opcode == 0x0f) ||
	  (insn_.mem.opcode == 0x08) ||
	  (insn_.mem.opcode == 0x09) ||
	  ((insn_.mem.opcode == 0x18) && 
	   (insn_.mem.disp == 0x0000) ||
	   (insn_.mem.disp == 0x4000)) ||
	  // these are are larger than the field - jkh 12/1/98
	  // (insn_.mem.disp == 0x8000) ||
	  // (insn_.mem.disp == 0xe000) ||
	  // (insn_.mem.disp == 0xf000) ||
	  (insn_.mem.opcode == 0x1a) ||
	  ((insn_.branch.opcode >= 0x30) &&
	   (insn_.branch.opcode <= 0x3f)) ||
	  (insn_.oper.opcode == 0x10) ||        // integer operations
	  (insn_.oper.opcode == 0x11) ||
	  (insn_.oper.opcode == 0x12) ||
	  (insn_.oper.opcode == 0x14) ||
	  (insn_.oper.opcode == 0x13) ||
	  (insn_.oper.opcode == 0x15) ||	       // floating point
	  (insn_.oper.opcode == 0x16) ||	       // floating point
	  (insn_.oper.opcode == 0x17) ||        // floating point
	  (insn_.oper.opcode == 0x00));         // PAL code
#endif
}


//
// Given and instruction, relocate it to a new address, patching up
// any relative addressing that is present.
//

bool instruction::generate(codeGen &gen,
			   process *,
			   Address origAddr,
			   Address relocAddr,
			   Address fallthroughOverride,
			   Address targetOverride) {
    int newOffset = 0;
    
    if (isBranch()) {
        if (!targetOverride) {
            newOffset = getTarget(origAddr) - relocAddr;
        }
        else {
            newOffset = targetOverride - relocAddr;
        }
        if (ABS(newOffset >> 2) > MAX_BRANCH) {
            fprintf(stderr, "newOffset 0x%llx, origAddr 0x%llx, relocAddr 0x%llx, target 0x%llx, override 0x%llx\n",
                    newOffset, origAddr, relocAddr, getTarget(origAddr), targetOverride);
            
            
            
            logLine("A branch too far\n");
            assert(0);
        }
        instruction newBranch(insn_);
        (*newBranch).branch.disp = newOffset >> 2;
        newBranch.generate(gen);
    }
    else 
        generate(gen);
    
    // Calls are ANNOYING. I've seen behavior where RA (the return addr)
    // is later used in a memory calculation... so after all is said and done,
    // set RA to what it would have been.
    // Note: we do this after the original relocation because JSRs don't
    // trigger "isBranch"
    if (isCall()) {
        Address origReturn = origAddr + instruction::size();
        int remainder = 0;
        instruction::generateAddress(gen, REG_RA, origReturn, remainder);
        if (remainder)
            instruction::generateLDA(gen, REG_RA, REG_RA, remainder, true);
    }
    return true;
}

