/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: inst-alpha.C,v 1.62 2003/04/17 20:55:53 jaw Exp $

#include "common/h/headers.h"

#ifdef BPATCH_LIBRARY
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#else
#include "rtinst/h/rtinst.h"
#endif
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/arch-alpha.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/instPoint.h"
#include <sys/procfs.h>
#include "dyninstAPI/src/stats.h"
#include "dyninstAPI/src/os.h"

/*
   This has not been fully tested.
   The Alpha likes to use jsrs much more than the sparc. 
   Paradyn assumes jsrs are jumps to library functions -- this is incorrect.
   It causes timers to be left on for many functions.  These timers are not 
   supposed to overlap.

   gcc compiles code in an interesting manner for the Alpha.  It creates an 
   alternate entry point for most functions.  If gcc -S is run, the alternate
   entry point has ..NG appended to the function name.  The alternate entry 
   point does not show up in the symbol table.  I don't know the rules for 
   choosing which entry point to use.  The alternate entry point is usually
   two instructions after the advertised entry point.  This code will not work
   with gcc compiled applications.  The runtime must also not be compiled with
   gcc.

   I have seen generated code that uses a register other than 'ra' for the return
   address (a temporary register had been used).  I find this disturbing and played
   it mostly safe when it came to saving registers.  At the base tramp, all registers
   that a callee can clobber are saved.  This is done before and after the relocated
   instruction.  One simple optimization is to avoid doing the save and restore
   unless a slot is used on that side of the relocated instruction.

   I have had to split the DYNINSTdata field into DYNINSTcode and DYNINSTdata.
   The branch displacement is not large enough to reach DYNINSTdata from text space.
   Programs compiled using cc should specify '-taso -non_shared -N'

   The 'pv' register must contain the address of the callee when the callee is called.
   I have not seen any documentation on this.  The API does not mention this.

   'TODO' is used to mark loose ends -- this file does not have too many.
   
   Optimizing branch behavior (not implemented)
   * branch to quadword or octaword aligned addresses
   * forward conditional branches are predicted not taken, take advantage of this

   Optimizing branch behavior (implemented)
   * use least significant 16 bits of JSR for branch prediction

 */


// HACK: This allows the bootstrap code for loading the dyninst library to
//    skip calling save/restore register code.
bool skipSaveCalls = false;

// The API for Alphas does not appear to be followed in all cases
// rather than spend much time and effort disassembling and cursing
// registers will be saved and restored in the base trampoline
// A save/restore pair will be executed before and after the relocated
// instruction.  An obvious optimization is to not do the save/restore
// when no code is inserted before or after the relocated instruction.
// 

// static inline Address generate_integer_op(instruction *insn, Address, Address,
// 					  Address, unsigned&, opCode);
// static inline Address generate_call_code(instruction*, Address, Address,
// 					 Address, unsigned&, process *proc);
// static inline Address generate_tramp_preamble(instruction*, Address,
// 					      Address, unsigned&);
static inline void generate_integer_op(instruction *insn, Address, Address,
					  Address, unsigned long&, opCode,bool Imm = false);
static inline Address generate_call_code(instruction*, Address, Address,
					 Address, unsigned long&, process *proc);
static inline void generate_tramp_preamble(instruction*, Address,
					      Address, unsigned long&);

typedef enum { dw_long, dw_quad } data_width;

#define MASK_4(x)     (x & 0xffffffff)
#define MASK_2(x)     (x & 0xffff)

const Address shifted_16 = (Address(1)) << 16;
const Address shifted_32 = (Address(1)) << 32;
const Address shifted_48 = (Address(1)) << 48;
const Address bit_15 =     (Address(1)) << 15;
const Address bit_31 =     (Address(1)) << 31;
const Address bit_47 =     (Address(1)) << 47;

const char *registerNames[] = { "ren", "stimpy" };
registerSpace *regSpace;
Register regList[] = {1, 2, 3, 4, 5, 6, 7, 8};

trampTemplate baseTemplate;

string getStrOp(int /* op */) {
  assert(0);
}

// BIS (or)  R31, R31, R31
// unsigned generate_nop(instruction* insn) {
unsigned long generate_nop(instruction* insn) {
  insn->raw = 0;
  // insn.oper.zero = 0; insn.oper.sbz = 0;
  insn->oper.opcode = OP_BIS;
  insn->oper.function = FC_BIS;
  insn->oper.ra = 31;
  insn->oper.rb = 31;
  insn->oper.rc = 31;
  return 1;
}

inline void generateNOOP(instruction *insn)
{
  insn->raw = 0;
  // insn.oper.zero = 0; insn.oper.sbz = 0;
  insn->oper.opcode = OP_BIS;
  insn->oper.function = FC_BIS;
  insn->oper.ra = 31;
  insn->oper.rb = 31;
  insn->oper.rc = 31;
}

#define FIRST_16(x)       ((x) & 0xffff)
#define SECOND_16(x)      ((x) >> 16)
#define THIRD_16(x)       ((x) >> 32)

#define ABS(x)		((x) > 0 ? (x) : -(x))

// TODO -- the max branch is (0x1 << 22)
// if we limited branches to this distance, dyn. inst. would not work
// on a 64 bit architecture
#define MAX_BRANCH	(0x1<<22)

#define MAX_IMM	0x1<<8  // 8 bits

Address getMaxBranch() { 
  return ((Address)(1 << 22));
}

bool doNotOverflow(int value)
{
  // we are assuming that we have 8 bits to store the immediate operand.
  if ( (value >= 0) && (value <= 255) ) return(true);
  else return(false);
}

void instWaitingList::cleanUp(process * , Address ) {
    P_abort();
}

// 
//
// unsigned
// generate_operate(instruction *insn, unsigned reg_a, unsigned reg_b, 
//		 unsigned reg_c, unsigned opcode, unsigned func_code) {
unsigned long
generate_operate(instruction *insn, Register reg_a, Register reg_b, 
	 Register reg_c, unsigned int opcode, unsigned int func_code) {
  assert(reg_a < Num_Registers);
  assert(reg_b < Num_Registers);
  assert(reg_c < Num_Registers);
  insn->raw = 0;
  // insn->oper.zero = 0; insn->oper.sbz = 0;
  insn->oper.ra = reg_a; insn->oper.rb = reg_b; insn->oper.rc = reg_c;
  insn->oper.opcode = opcode;
  insn->oper.function = func_code;
  return 1;
}

// 
//
// unsigned
// generate_lit_operate(instruction *insn, unsigned reg_a, int literal,
//		     unsigned reg_c, unsigned opcode, unsigned func_code) {
unsigned long
generate_lit_operate(instruction *insn, Register reg_a, int literal,
	     Register reg_c, unsigned int opcode, unsigned int func_code) {
  assert(reg_a < Num_Registers);
  // literal is 0 to 255
  assert((literal >= 0) && (literal < MAX_IMM));
  assert(reg_c < Num_Registers);
  insn->raw = 0;
  insn->oper_lit.one = 1;
  insn->oper_lit.ra = reg_a;
  insn->oper_lit.lit = literal;
  insn->oper_lit.rc = reg_c;
  insn->oper_lit.opcode = opcode;
  insn->oper_lit.function = func_code;
  return 1;
}

// j?? ra, (rb), hint
// jump to rb, ra gets return address, displacement field provides 
// instruction fetch hint
// TODO -- provide a decent hint
// unsigned
// generate_jump(instruction *insn, unsigned dest_reg, unsigned ext_code,
//	      unsigned ret_reg, int hint) {
unsigned long
generate_jump(instruction *insn, Register dest_reg, unsigned long ext_code,
	      Register ret_reg, int hint) {
  assert(dest_reg < Num_Registers); 
  assert(ret_reg < Num_Registers); 
  assert((ext_code == MD_JSR) || (ext_code == MD_JMP));

  // If you want to use this for other ext_code values, then modify the branch
  // prediction

  insn->raw = 0;
  insn->mem_jmp.opcode = OP_MEM_BRANCH;
  insn->mem_jmp.ext = ext_code;
  insn->mem_jmp.ra = ret_reg;
  insn->mem_jmp.rb = dest_reg;
  insn->mem_jmp.disp = hint;
  return 1;
}


// src_reg --> for unconditional branch, gets return address
// src_reg --> for conditional branch, is register to test
// b??  r31, offset   <-- return address written to r31, which is ignored
//
// unsigned
// generate_branch(instruction *insn, unsigned src_reg, int offset, unsigned opcode) {
unsigned long
generate_branch(instruction *insn, Register src_reg, int offset,
                unsigned int opcode) {
  if (ABS(offset >> 2) > MAX_BRANCH) 
        { logLine("a branch too far\n"); assert(0); }
  assert(src_reg < Num_Registers);
  insn->raw = 0;
  insn->branch.opcode = opcode;
  insn->branch.ra = src_reg;
  insn->branch.disp = offset >> 2;
  return 1;
}

void generateBSR(instruction *insn,int offset)
{
  generate_branch(insn,REG_RA,offset,OP_BSR);

}
void generateBranchInsn(instruction *insn, int offset) {
  generate_branch(insn, REG_ZERO, offset, OP_BR);
}

// 
// Set condition flag (a register)
// unsigned
// genRelOp(instruction *insn, unsigned opcode, unsigned fcode,
//	 unsigned src1, unsigned src2, unsigned dest, bool do_not=false) {
unsigned long
genRelOp(instruction *insn, unsigned opcode, unsigned fcode, Register src1,
	 Register src2, Register dest, bool Imm=false, bool do_not=false) {
  // cmp?? src1, src2, t8      : leave result in t8
//  unsigned words = generate_operate(insn, src1, src2, dest, opcode, fcode);
  unsigned long words;
  if (Imm)
    words = generate_lit_operate(insn, src1, src2, dest, opcode, fcode);
  else
    words = generate_operate(insn, src1, src2, dest, opcode, fcode);

  if (do_not) {
    // since there is no cmpne -- cmpeq is used and the result must be inverted
    // 0 -->  1,  1 ---> 0
    // negate t8, and add 1  (1 --> -1 + 1 = 0,  0 --> -0 +1 = 1 )
    words += generate_operate(insn+words, REG_ZERO, dest, dest, OP_SUBL, FC_SUBL);
    words += generate_lit_operate(insn+words, dest, 1, dest, OP_ADDL, FC_ADDL);
    // dest now contains a 0 or a 1 
  }
  return words;
}

instPoint::instPoint(pd_Function *f, const instruction &instr, 
		     const image *img, Address &adr,
		     const bool,instPointType pointType)
: addr(adr), originalInstruction(instr), inDelaySlot(false), isDelayed(false),
  callIndirect(false), callAggregate(false), callee(NULL), func(f),
  ipType(pointType), image_ptr(img)
{
}



// lda(h) rdest, literal:16(rstart)
// put a literal in a register
// offset is the value before the shift
// unsigned 
// generate_lda(instruction *insn, unsigned rdest, unsigned rstart,
//	     int offset, bool do_low) {
unsigned long
generate_lda(instruction *insn, Register rdest, Register rstart,
	     long offset, bool do_low) {

  // this is not wuite the correct check, but some calls supply the actual
  //   signed displacement and others supply the raw 16 bits (i.e. not sign
  //   extended.  - jkh
  assert(ABS(offset) < (int) shifted_16);
  // assert(offset >= (- (long) 0xffffffff));
  assert(rdest < Num_Registers);
  assert(rstart < Num_Registers);

  insn->raw = 0;
  insn->mem.opcode = do_low ? OP_LDA : OP_LDAH;
  insn->mem.ra = rdest;
  insn->mem.rb = rstart;
  insn->mem.disp = offset;
  return 1;
}

// left == true, shift left
// left == false, shift right
// unsigned
// amount is a literal
// generate_sxl(instruction *insn, unsigned rdest, unsigned amount,
//	     bool left, unsigned rsrc) {
unsigned long
generate_sxl(instruction *insn, Register rdest, unsigned long amount,
	     bool left, Register rsrc) {
  assert(rdest < Num_Registers);
  assert(amount < Num_Registers);
  assert(rsrc < Num_Registers);
  insn->raw = 0;
  // insn->oper.sbz = 0;
  // insn->oper.zero = 0;

  insn->oper_lit.opcode = left ? OP_SLL : OP_SRL;
  insn->oper_lit.function = left ? FC_SLL : FC_SRL;
  insn->oper_lit.one = 1;
  insn->oper_lit.ra = rsrc;
  insn->oper_lit.lit = amount;
  insn->oper_lit.rc = rdest;
  return 1;
}

#define SEXT_16(x) (((x) & bit_15) ? ((x) | 0xffffffffffff0000) : (x))

typedef unsigned long int Offset;

// remainder returns 15 bit offset to be included in load or store
// returns number of instruction words
// unsigned generate_address(instruction *insn, 
//			 unsigned rdest,
//			 const Address &addr,
//			 int& remainder) {
unsigned long generate_address(instruction *insn, 
			 unsigned long rdest,
			 const Address &addr,
			 int& remainder) {
//  unsigned words = 0;
  unsigned long words = 0;

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
    generate_lda(insn+words, rdest, REG_ZERO, third, false);
    words++;
    generate_sxl(insn+words, rdest, 16, true, rdest);  // move (31:15) to (47:32)
    words++;
  }

  if (high) {
    if (primed)
      generate_lda(insn+words, rdest, rdest, high, false);
    else
      generate_lda(insn+words, rdest, REG_ZERO, high, false);
    words++; 
    primed = true;
  }
  // a better solution is to use register zero as the base
  // clear the base register
  if (!primed) 
    words += generate_operate(insn+words, REG_ZERO, REG_ZERO, rdest, 
			      OP_BIS, FC_BIS);

  remainder = low;
  return words;
}

// stx rsrc, [rbase + sign_extend(offset:16)]
// unsigned
// generate_store(instruction* insn, unsigned rsrc, unsigned rbase, int disp,
//	       data_width width) {
unsigned long
generate_store(instruction* insn, Register rsrc, Register rbase, 
	int disp, data_width width) {
  assert(ABS(disp) < (int) shifted_16); 
  assert(rsrc < Num_Registers); 
  assert(rbase < Num_Registers);
  insn->raw = 0;
  insn->mem.disp = FIRST_16(disp);
  insn->mem.ra = rsrc;
  insn->mem.rb = rbase;
  switch (width) {
  case dw_long: insn->mem.opcode = OP_STL; break;
  case dw_quad: insn->mem.opcode = OP_STQ; break;
  default: assert(0);
  }
  return 1;
}

// ldx rdest, [rbase + sign_extend(offset:16)]
// unsigned
// generate_load(instruction *insn, unsigned rdest, unsigned rbase, int disp,
//	      data_width width) {
unsigned long
generate_load(instruction *insn, Register rdest, Register rbase, 
	int disp, data_width width,bool aligned = true) {
  assert(ABS(disp) < (int) shifted_16); 
  assert(rdest < Num_Registers);
  assert(rbase < Num_Registers);
  insn->raw = 0;
  insn->mem.disp = disp;
  insn->mem.ra = rdest;
  insn->mem.rb = rbase;
  switch (width) {
  case dw_long: insn->mem.opcode = OP_LDL; break;
  case dw_quad: insn->mem.opcode = OP_LDQ; break;
  default: assert(0);
  }
  if (aligned == false)
    insn->mem.opcode = OP_LDQ_U;
  return 1;
}


// Restore argument registers (a0-a4) or v0
void restoreRegister(instruction *&insn, Address &base, int reg, int dest)
{
  assert(reg >= 0);
  generate_load(insn,dest,REG_SP,(reg<<3),dw_quad);
  insn++;
  base += sizeof(instruction);
}

// Determine if the called function is a "library" function or a "user" function
// This cannot be done until all of the functions have been seen, verified, and
// classified
//
void pd_Function::checkCallPoints() {
//  unsigned i;
  unsigned long i;
  instPoint *p;
  Address loc_addr;

  pdvector<instPoint*> non_lib;

  for (i=0; i<calls.size(); ++i) {
    /* check to see where we are calling */
    p = calls[i];
    assert(p);

    if (isJsr(p->originalInstruction)) {
      // assume this is a library function
      // since it is a jump through a register
      // TODO -- should this be deleted here ?
      p->callIndirect = true;
      non_lib.push_back(p);
      //      delete p;
    } else if (isBsr(p->originalInstruction)) {
      loc_addr = p->addr + (p->originalInstruction.branch.disp << 2)+4;
      non_lib.push_back(p);
      pd_Function *pdf = (file_->exec())->findFuncByAddr(loc_addr);

      if (pdf == NULL)
	{
	  /* Try alternate entry point in the symbol table */
	  loc_addr = loc_addr - 8;
	  pdf = (file_->exec())->findFuncByAddr(loc_addr);
	}

      if (pdf && 1 /*!pdf->isLibTag()*/)
	  p->callee = pdf;
    } else {
      assert(0);
    }
  }
  calls = non_lib;
}

// TODO we cannot find the called function by address at this point in time
// because the called function may not have been seen.
//
Address pd_Function::newCallPoint(Address, const instruction,
				  const image *, bool &) {
    abort();
    // This is not used on Alpha
}

//
// Given and instruction, relocate it to a new address, patching up
// any relative addressing that is present.
//
// Note - currently - only 
//
void relocateInstruction(instruction* insn, Address origAddr,
			 Address targetAddr) {
  Address newOffset;

  if (isBranchType(*insn)) {
    newOffset = origAddr - targetAddr + (Address) (insn->branch.disp << 2);
    insn->branch.disp = newOffset >> 2;

    if (ABS(insn->branch.disp) > MAX_BRANCH) {
      logLine("a branch too far\n");
      assert(0);
    }
  }
  // The rest of the instructions should be fine as is 
}

registerSpace *createRegisterSpace()
{
  return new registerSpace(sizeof(regList)/sizeof(Register), regList,
                                0, NULL);
}

//
// We now generate tramps on demand, so all we do is init the reg space.
//
void initTramps() {
  static bool init_done=false;

  if (init_done) return;
  init_done = true;

  regSpace = new registerSpace(sizeof(regList)/sizeof(Register), regList,
                                0, NULL);
}

// Emit a func 64bit address for the call
inline void callAbsolute(instruction *code, 
			 unsigned long &words, 
			 Address funcAddr)
{
  int remainder;
  words += generate_address(code+(words), REG_GP, funcAddr, remainder);
  if (remainder)
    words += generate_lda(code+(words), REG_GP, REG_GP, remainder, true);
  words += generate_jump(code+(words), REG_GP, MD_JSR, REG_RA, remainder);
}

/*
 * Install a base tramp -- fill calls with nop's for now.
 * An obvious optimization is to turn off the save-restore calls when they 
 * are not needed.
 */
void installBaseTramp(instPoint *location,
		      process *proc,
		      trampTemplate &tramp
		      ) {
//  unsigned words = 0;
  unsigned long words = 0;
  const int MAX_BASE_TRAMP = 128;

  // XXX - for now assume base tramp is less than 1K
  instruction *code = new instruction[MAX_BASE_TRAMP]; assert(code);

  function_base *fun_save;
  function_base *fun_restore;

  Symbol info;
  Address baseAddr;

  if (location->ipType == otherPoint) {
      fun_save = proc->findOnlyOneFunction("DYNINSTsave_conservative");
      fun_restore = proc->findOnlyOneFunction("DYNINSTrestore_conservative");
      proc->getSymbolInfo("DYNINSTsave_conservative", info, baseAddr);
  } else {
      fun_save = proc->findOnlyOneFunction("DYNINSTsave_temp");
      fun_restore = proc->findOnlyOneFunction("DYNINSTrestore_temp");
      proc->getSymbolInfo("DYNINSTsave_temp", info, baseAddr);
  }

  assert(fun_save && fun_restore);
  Address dyn_save = fun_save->addr() + baseAddr;
  Address dyn_restore = fun_restore->addr() + baseAddr;

  // Pre branch
  instruction *skipPreBranch = &code[words];
  tramp.skipPreInsOffset = words*4;
  words += generate_nop(code+words);

  // decrement stack by 16
  tramp.savePreInsOffset = words*4;
  words += generate_lda(code+words, REG_SP, REG_SP, -16, true);

  // push ra onto the stack
  words += generate_store(code+words, REG_RA, REG_SP, 0, dw_quad);

  // push GP onto the stack
  words += generate_store(code+words, REG_GP, REG_SP, 8, dw_quad);

  // Call to DYNINSTsave_temp
  callAbsolute(code, words, dyn_save);

  // global_pre
  tramp.globalPreOffset = words * 4;
  words += generate_nop(code+words);

  // local_pre
  tramp.localPreOffset = words * 4;
  words += generate_nop(code+words);

  words += generate_nop(code+words);

  // **** When you change these code also look at emitFuncJump ****
  // Call to DYNINSTrestore_temp
  tramp.localPreReturnOffset = words * 4;
  callAbsolute(code, words, dyn_restore);

  // load ra from the stack
  words += generate_load(code+words, REG_RA, REG_SP, 0, dw_quad);

  // load GP from the stack
  words += generate_load(code+words, REG_GP, REG_SP, 8, dw_quad);

  // increment stack by 16
  tramp.restorePreInsOffset = words*4;
  words += generate_lda(code+words, REG_SP, REG_SP, 16, true);

  // *** end code cloned in emitFuncJump ****

  // slot for emulate instruction
  tramp.emulateInsOffset = words*4;
  instruction *reloc = &code[words]; 
  *reloc = location->originalInstruction;
  words += 1;
  // Do actual relocation once we know the base address of the tramp

  // compute effective address of this location
  Address pointAddr = location->addr;
  Address baseAddress;
  if(proc->getBaseAddress(location->image_ptr, baseAddress)){
      pointAddr += baseAddress;
  }

  // Post branch
  instruction *skipPostBranch = &code[words];
  tramp.skipPostInsOffset = words*4;
  words += generate_nop(code+words);

  // decrement stack by 16
  tramp.savePostInsOffset = words*4;
  words += generate_lda(code+words, REG_SP, REG_SP, -16, true);

  // push ra onto the stack
  words += generate_store(code+words, REG_RA, REG_SP, 0, dw_quad);

  // push GP onto the stack
  words += generate_store(code+words, REG_GP, REG_SP, 8, dw_quad);

  // Call to DYNINSTsave_temp
  callAbsolute(code, words, dyn_save);

  // global_post
  tramp.globalPostOffset = words * 4;
  words += generate_nop(code+words);

  // local_post
  tramp.localPostOffset = words * 4;
  words += generate_nop(code+words);

  // Call to DYNINSTrestore_temp
  tramp.localPostReturnOffset = words * 4;
  callAbsolute(code, words, dyn_restore);

  // load ra from the stack
  words += generate_load(code+words, REG_RA, REG_SP, 0, dw_quad);

  // load GP from the stack
  words += generate_load(code+words, REG_GP, REG_SP, 8, dw_quad);

  // increment stack by 16
  tramp.restorePostInsOffset = words*4;
  words += generate_lda(code+words, REG_SP, REG_SP, 16, true);

  // If the relocated insn is a Jsr or Bsr then 
  // appropriately set Register Ra
  if (isCallInsn(location->originalInstruction)) {
      int remainder;
      words += generate_address(code+words, REG_RA, pointAddr+4,remainder);
      if (remainder)
	words += generate_lda(code+words, REG_RA, REG_RA, remainder, true);
  }

  // slot for return (branch) instruction
  // actual code after we know its locations
  // branchFromAddr offset from base until we know base of tramp 
  tramp.returnInsOffset = (words * 4);		
  instruction *branchBack = &code[words];
  words += 1;

  words += generate_nop(code+words);

  assert(words < static_cast<const unsigned>(MAX_BASE_TRAMP));

  tramp.size = words * sizeof(instruction);
  tramp.baseAddr = proc->inferiorMalloc(tramp.size, textHeap, pointAddr);
  assert(tramp.baseAddr);

  // pointAddr + 4 is address of instruction after relocated instr
  // branchFromAddr = address of the branch insn that returns to user code
  // branchFromAddr + 4 is updated pc when branch instruction executes
  // we assumed this one was instruction long before

  // update now that we know base
  Address branchFromAddr = tramp.returnInsOffset + tramp.baseAddr;

  int count = generate_branch(branchBack, REG_ZERO,
			   (pointAddr+4) - (branchFromAddr+4), OP_BR);
  assert(count == 1);

  // Do actual relocation once we know the base address of the tramp
  relocateInstruction(reloc, pointAddr, 
       tramp.baseAddr + tramp.emulateInsOffset);

  // Generate skip pre and post instruction branches
  generateBranchInsn(skipPreBranch,
		     tramp.emulateInsOffset - (tramp.skipPreInsOffset+4));
  generateBranchInsn(skipPostBranch,
		     tramp.returnInsOffset - (tramp.skipPostInsOffset+4));

  tramp.prevInstru = false;
  tramp.postInstru = false;

  proc->writeDataSpace((caddr_t)tramp.baseAddr, tramp.size, (caddr_t) code);
  delete (code);
}

/*
 * emitSaveConservative - generate code to save all registers
 *      used as part of inferrior RPC
 *      We don't know where this will be located, so generate absolute addr
 *          for the function call.
 *
 */
void emitSaveConservative(process *proc, char *code, Address &offset)
{
  unsigned count = 0;
  function_base *fun_save;
  instruction *insn = (instruction *) ((void*)&code[offset]);

  Symbol info;
  Address baseAddr;
  fun_save = proc->findOnlyOneFunction("DYNINSTsave_conservative");
  proc->getSymbolInfo("DYNINSTsave_temp", info, baseAddr);
  assert(fun_save);

  Address dyn_save = fun_save->addr() + baseAddr;

  // decrement stack by 16
  count += generate_lda(&insn[count], REG_SP, REG_SP, -16, true);

  // push T10 onto the stack
  count += generate_store(&insn[count], REG_T10, REG_SP, 0, dw_quad);

  // push ra onto the stack
  count += generate_store(&insn[count], REG_RA, REG_SP, 8, dw_quad);

  // Call to DYNINSTsave_conservative
  int remainder;
  count += generate_address(&insn[count], REG_T10, dyn_save, remainder);
  if (remainder)
    count += generate_lda(&insn[count], REG_T10, REG_T10, remainder, true);
  count += generate_jump(&insn[count], REG_T10, MD_JSR, REG_RA, remainder);

  offset += count * sizeof(instruction);
}

/*
 * emitSaveConservative - generate code to restore all registers
 *      used as part of inferrior RPC
 *      We don't know where this will be located, so generate absolute addr
 *          for the function call.
 *
 */
void emitRestoreConservative(process *proc, char *code, Address &offset)
{
  unsigned count = 0;
  function_base *fun_restore;
  instruction *insn = (instruction *) ((void*)&code[offset]);

  Symbol info;
  Address baseAddr;
  fun_restore = proc->findOnlyOneFunction("DYNINSTrestore_conservative");
  proc->getSymbolInfo("DYNINSTsave_temp", info, baseAddr);
  assert(fun_restore);

  Address dyn_restore = fun_restore->addr() + baseAddr;

  // Call to DYNINSTrestore_temp
  int remainder;
  count += generate_address(&insn[count], REG_T10, dyn_restore, remainder);
  if (remainder)
    count += generate_lda(&insn[count], REG_T10, REG_T10, remainder, true);
  count += generate_jump(&insn[count], REG_T10, MD_JSR, REG_RA, remainder);

  // load t10 from the stack
  count += generate_load(&insn[count], REG_T10, REG_SP, 0, dw_quad);

  // load ra from the stack
  count += generate_load(&insn[count], REG_RA, REG_SP, 8, dw_quad);

  // increment stack by 16
  count += generate_lda(&insn[count], REG_SP, REG_SP, 16, true);

  offset += count * sizeof(instruction);
}

//
// move the passed parameter into the passed register, or if it is already
//    in a register return the register number.
//
Register getParameter(Register, int param) {
  if (param <= 5) {
    return(16+param);
  }
  assert(0); 
  return(Null_Register);
}

int getInsnCost(opCode op) {

  if (op == loadConstOp) {
    return(1);
  } else if (op ==  loadOp) {
    // assume two cycles to generate address, one to issue load
    return(1+2);
  } else if (op ==  storeOp) {
    // assume two cycles to generate address, one to issue store
    return(1+2);
  } else if (op ==  ifOp) {
    // beq
    return(1);
  } else if (op ==  callOp) {
    //  3  : load address for jsr to DYNINSTsave_misc
    // 12  : cost of DYNINSTsave_misc           
    //  3  : jsr to DYNINSTsave_misc (1 + 2 cycle branch penalty)
    //  3  : load address into register for jsr to called function
    //  1  : move argument to argument register (assume 1 arg on average)
    //  3  : issue jsr to called function (1 + 2 cycle branch penalty)
    //  3  : load address for jsr to DYNINSTrestore_misc
    // 12  : cost of DYNINSTrestore_misc           
    //  3  : jsr to DYNINSTrestore_misc (1 + 2 cycle branch penalty)
    //
    // TODO -- what about the cost of the called code ?
    return 43;
  } else if (op ==  trampPreamble) {
    //  2  : load address of observed_cost
    //  1  : load value of observed_cost
    //  1  : add to observed cost
    //  1  : store updated observed_cost
    return(2+1+1+1);
  } else if (op ==  trampTrailer) {
    // ret
    return(1);
  } else if (op == noOp) {
    // noop
    return(1);
  } else {
    switch (op) {
    case eqOp:
      return 1;                 // 1  : cmpeq
    case neOp:
      return 3;                 // cmpeq, negate result, add 1 to result
    case lessOp:          
    case greaterOp:
    case leOp:
    case geOp:
      return 1;                 // cmpXX, arguments may be reversed
    default:                    // other integer operators
      return(1);
      break;
    }
  }
}

// void generateNoOp(process *proc, int addr) {
void generateNoOp(process *proc, unsigned long addr) {
  instruction insn;
  generate_nop(&insn);
  proc->writeTextWord((caddr_t)addr, insn.raw);
}

/*
 * change the insn at fromAddr to be a branch to newAddr.
 *   Used to add multiple tramps to a point.
 */
void generateBranch(process *proc, Address fromAddr, Address newAddr) {
  int disp;
  instruction insn;

  Address a = ABS((long)((fromAddr+4) - newAddr));
  Address b = (Address) MAX_BRANCH;
//  if ((ABS((fromAddr+4) - newAddr)) > (Address) MAX_BRANCH) { 
if (a > b) {
    logLine("a branch too far\n"); assert(0);
  }

  // Note the explicit cast
  // +4 to use updated pc
  disp = newAddr - (fromAddr+4);
  generateBranchInsn(&insn, disp);

  proc->writeTextWord((caddr_t)fromAddr, insn.raw);
}

// The Alpha does not have a divide instruction
// The divide is performed in software by calling __divl
int software_divide(int src1,int src2,int dest,char *i,Address &base,bool,Address divl_addr,bool Imm)
{
  int words;
  int remainder;
  instruction *insn = (instruction *) ((void*)&i[base]);

  assert(!((unsigned long)insn & (unsigned long)3));  
  // or src1,src1,a0
  generate_operate(insn,src1,src1,REG_A0,OP_BIS,FC_BIS);
  insn++;
  base+= sizeof(instruction);
  if (Imm)
    // load constant into register a1
    generate_lit_operate(insn,REG_ZERO,src2,REG_A1, OP_ADDL, FC_ADDL);    
    else
      // or src2,src2,a1
      generate_operate(insn,src2,src2,REG_A1,OP_BIS,FC_BIS);
  insn++;
  base+= sizeof(instruction);

  // jsr __divl
  words = generate_address(insn, REG_PV,divl_addr, remainder);
  if (remainder)
      words += generate_lda(insn+words, REG_PV, REG_PV, remainder, true);
  words += generate_jump(insn+words, REG_PV, MD_JSR, REG_RA, remainder);
  insn += words; base += (words*4);

  // or v0,v0,dest
  generate_operate(insn,REG_V0,REG_V0,dest,OP_BIS,FC_BIS);
  insn++;
  base+= sizeof(instruction);  
  return 0;
}

static inline void
generate_integer_op(instruction *insn, Address src1, Address src2, 
		    Address dest, unsigned long& base, opCode op, bool Imm) {
//		    Address dest, unsigned & base, opCode op) {
  // integer ops
//  unsigned op_code=0, func_code=0, words = 0;
  unsigned op_code=0, func_code=0;
  unsigned long words = 0;

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
    words += genRelOp(insn, OP_CMPEQ, FC_CMPEQ, src1, src2, dest,Imm);
    base += words * 4; return;

  case neOp:
    // last arg == true --> inverts value of comparison		      
    words += genRelOp(insn, OP_CMPEQ, FC_CMPEQ, src1, src2, dest,Imm,true);
    base += words * 4; return;

  case lessOp:
    words += genRelOp(insn, OP_CMPLT, FC_CMPLT, src1, src2, dest,Imm);
    base += words * 4; return;
		      
  case greaterOp:                          
    words += genRelOp(insn, OP_CMPLE, FC_CMPLE, src1, src2, dest,Imm,true);
    base += words * 4; return;

  case leOp:
    words += genRelOp(insn, OP_CMPLE, FC_CMPLE, src1, src2, dest,Imm);
    base += words * 4; return;

  case geOp:                               
    words += genRelOp(insn, OP_CMPLE, FC_CMPLT, src1, src2, dest,Imm,true);
    base += words * 4; return;

  default:
    assert(0);
    break;
  }

  if (Imm) {
      if ((src2 >= 0) && (src2 < MAX_IMM)) {
	words += generate_lit_operate(insn+words, src1, src2, dest, op_code, 
	    func_code);
      } else {
	words += generate_operate(insn+words, src1, src2, dest, op_code, func_code);
      }
  } else {
    words += generate_operate(insn+words, src1, src2, dest, op_code, func_code);
  }
  base += words * 4; return;
}

/*
    ra only has to be saved when a procedure call is added using
    instrumentation.
    the Alpha API.
    ra must be saved because it will be clobbered by a procedure call since 
    it will get the return address. 
 */

/*
   It would be nice to be able to do relative branches to DYNINST funcs from
   here.  Unfortunately, I do not have the base address of the trampoline.
 */
static inline Address
generate_call_code(instruction *insn, Address src1, Address src2, Address dest,
		   unsigned long& base, process *proc) {
//		   unsigned& base, process *proc) {
  // put parameters in argument registers
  // register move is "bis src, src, dest"  (bis is logical or)
  // save and restore the values currently in the argument registers
//  unsigned words = 0;
  unsigned long words = 0;
//  int decrement = -8;

  Symbol info;
  Address baseAddr;

  proc->getSymbolInfo("DYNINSTsave_misc", info, baseAddr);
  function_base *fun_save = proc->findOnlyOneFunction("DYNINSTsave_misc");
  function_base *fun_restore = proc->findOnlyOneFunction("DYNINSTrestore_misc");
  assert(fun_save && fun_restore);
  Address dyn_save = fun_save->addr() + baseAddr;
  Address dyn_restore = fun_restore->addr() + baseAddr;

  // save the current values in v0, a0..a5, pv, at, gp
  // Call to DYNINSTsave_misc
  int remainder;
  words += generate_address(insn+words, REG_T10, dyn_save, remainder);
  if (remainder)
    words += generate_lda(insn+words, REG_T10, REG_T10, remainder, true);
  words += generate_jump(insn+words, REG_T10, MD_JSR, REG_RA, remainder);

  // move args to argument registers
  if (src1 > 0)
    words += generate_operate(insn+words, src1, src1, REG_A0, OP_BIS, FC_BIS);
  if (src2 > 0) 
    words += generate_operate(insn+words, src2, src2, REG_A1, OP_BIS, FC_BIS);

  // Set the value of  pv/t12/r27
  // I have not found any documentation on this
  // But the alpha appears to put the callee's address in this register
  // this register must be saved and restored

  // Jump to the function
  words += generate_address(insn+words, REG_PV, dest, remainder);
  if (remainder)
    words += generate_lda(insn+words, REG_PV, REG_PV, remainder, true);

  // TODO -- clear other argument registers ?
  words += generate_jump(insn+words, REG_PV, MD_JSR, REG_RA, remainder);
  
  // Call to DYNINSTrestore_misc
  words += generate_address(insn+words, REG_T10, dyn_restore, remainder);
  if (remainder)
    words += generate_lda(insn+words, REG_T10, REG_T10, remainder, true);
  words += generate_jump(insn+words, REG_T10, MD_JSR, REG_RA, remainder);

  base += words * 4; return 0;
}

static inline void
generate_tramp_preamble(instruction *insn, Address src1,
			Address dest, unsigned long& base) {
//			Address dest, unsigned& base) {
  // generate code to update the observed cost
  // a4 holds address, a3 gets loaded

//  unsigned words = 0; int obs_rem;
  unsigned long words = 0; int obs_rem;
  // load the current observed cost
  words += generate_address(insn+words, REG_T11, dest, obs_rem);
  words += generate_load(insn+words, REG_T10, REG_T11, obs_rem, dw_long);

  // update the observed cost
  if ((src1 >= 0) && (src1 < MAX_IMM)) {
    // addl literal, REG_T10, REG_T10       (t10 += literal)
    words += generate_lit_operate(insn+words, REG_T10, src1, REG_T10,
				  OP_ADDL, FC_ADDL);
  } else {
    // load literal into REG_T9
    int remainder;
    words += generate_address(insn+words, REG_T9, src1, remainder);
    if (remainder)
      words += generate_lda(insn+words, REG_T9, REG_T9, remainder, true);

    // addl REG_T9, REG_T10, REG_T10       (t10 += t9)
    words += generate_operate(insn+words, REG_T10, REG_T9, REG_T10, OP_ADDL, FC_ADDL);
  }

  // store new observed cost  
  // st REG_T10, obs_rem[REG_T11]
  words += generate_store(insn+words, REG_T10, REG_T11, obs_rem, dw_long);
  base += words * 4; 
  return;
}


// The value returned by emitA can be
// 
// ifOp: address to calculate branch
// 
// TODO -- if an offset is returned, it is the offset to the instruction
//         where the branch occurs.  This offset cannot be used to
//         calculate branches, the offset of the next instruction should
//         be used.  The alpha uses the UPDATED pc for branches.
// 

Address emitA(opCode op, Register src1, Register /*src2*/, Register dest,
	     char *i, Address &base, bool /*noCost*/) {

  //fprintf(stderr,"emitA(op=%d,src1=%d,src2=XX,dest=%d)\n",op,src1,dest);

  instruction *insn = (instruction *) ((void*)&i[base]);
  assert(!((unsigned long)insn & (unsigned long)3));

  switch (op) {
  case ifOp: {
    // beq src1
    // return address used to calculate branch offset
    assert(src1 < Num_Registers);
    // branch calculations use the updated pc
    unsigned long words = generate_branch(insn, src1, dest-4, OP_BEQ);
    // return offset of branch instruction
    base += words * 4; 
    return (base-4);
    }
  case branchOp: {
    generateBranchInsn(insn, dest-4);
    base += sizeof(instruction);
    return(base - sizeof(instruction));
    }
  case trampTrailer: {
    // dest is in words of offset and generateBranchInsn is bytes offset
    assert(!dest);
//    unsigned words = generate_branch(insn, REG_ZERO, dest << 2, OP_BR);
    unsigned long words = generate_branch(insn, REG_ZERO, dest << 2, OP_BR);
    // return the offset of the branch instruction -- not the updated pc
    base += words * 4; 
    return (base-4);
    }
  case trampPreamble: {
    generate_tramp_preamble(insn, src1, dest, base);
    return(0);          // let's hope this is expected!
    }
  default:
    abort();            // unexpected op for this emit!
  }
}

Register emitR(opCode op, Register src1, Register /*src2*/, Register dest,
               char *i, Address &base, bool /*noCost*/,
               const instPoint * /* location */ ) {

  //fprintf(stderr,"emitR(op=%d,src1=%d,src2=XX,dest=%d)\n",op,src1,dest);

  instruction *insn = (instruction *) ((void*)&i[base]);
  assert(!((unsigned long)insn & (unsigned long)3));

  switch (op) {
  case getRetValOp:
    {
      // Return value register v0 is the 12th register saved in the base tramp
      restoreRegister(insn,base,12,dest);
      return(dest);
    }
  case getParamOp:
    {
      if (src1 >5) assert(0);
      /*
       * We don't save the parameter registers unless we make a function call,
       * so we can read the values directly out of the registers.
       */
      unsigned long words =
	  generate_operate(insn,REG_A0+src1,REG_A0+src1,dest,OP_BIS, FC_BIS);
      base += words * sizeof(instruction);
      return(dest);
    }
  default:
    abort();                    // unexpected op for this emit!
  }
  return(Null_Register);        // should never get here!
}


#ifdef BPATCH_LIBRARY
void emitJmpMC(int condition, int offset, char* baseInsn, Address &base)
{
  // Not needed for memory instrumentation, otherwise TBD
}

// VG(11/07/01): Load in destination the effective address given
// by the address descriptor. Used for memory access stuff.
void emitASload(BPatch_addrSpec_NP /*as*/, Register /*dest*/,
		char* /*baseInsn*/, Address &/*base*/, bool /*noCost*/)
{
  // TODO ...
}

void emitCSload(BPatch_addrSpec_NP as, Register dest, char* baseInsn,
		Address &base, bool noCost)
{
  emitASload(as, dest, baseInsn, base, noCost);
}
#endif


void emitVload(opCode op, Address src1, Register, Register dest,
	     char *i, Address &base, bool, int size)
{
  instruction *insn = (instruction *) ((void*)&i[base]);
  assert(!((unsigned long)insn & (unsigned long)3));

  if (op == loadConstOp) {
    // lda dest, src1 (LITERAL)     --> put the literal value of src1 into dest
    // it may take several instructions to do this
    assert(dest < Num_Registers);
    int remainder;
    unsigned long words = 0;
    // if the constant is a zero -- or it into place
    if (!src1) { 
      words = generate_operate(insn+words, REG_ZERO, REG_ZERO, 
			       (unsigned long) dest, OP_BIS, FC_BIS);
//			       (unsigned) dest, OP_BIS, FC_BIS);
    } else {
//      words = generate_address(insn, (unsigned) dest, src1, remainder);
      words = generate_address(insn, (unsigned long) dest, src1, remainder);
      if (remainder)
	words +=
//	  generate_lda(insn+words, (unsigned) dest, (unsigned) dest,
	  generate_lda(insn+words, (unsigned long) dest, (unsigned long) dest,
		       remainder, true);
    }
    base += words * 4; return;

  } else if (op ==  loadOp) {
	// ld? dest, [src1]             --> src1 is a literal
	// src1 = address to load
	// src2 = 
	// dest = register to load
	int remainder;
	unsigned long words = generate_address(insn, (unsigned long) dest, src1, remainder);
	if (size == 4) {
	    words += generate_load(insn+words, (unsigned long) dest, 
		 (unsigned long) dest, remainder, dw_long);
	} else if (size == 8) {
	    words += generate_load(insn+words, (unsigned long) dest, 
		 (unsigned long) dest, remainder, dw_quad);
	} else {
	    abort();
	}
	base += words * 4; return;
    } else if (op ==  loadFrameRelativeOp) {
	unsigned long words = 0;
	// frame offset is negative of actual offset.
	long offset = (long) src1;

	// the saved sp is 16 less than original sp (due to base tramp code)
	offset += 16;		

	assert(ABS(offset) < 32767);
	words += generate_load(insn+words, (unsigned long) dest,  REG_SP,
			       112, dw_quad);
	assert(ABS(offset) < (1<<30));
	if (ABS(offset) > 32767) {
	    Offset low = offset & 0xffff;
	    offset -= SEXT_16(low);
	    Offset high = (offset >> 16) & 0xffff;
	    assert((Address)SEXT_16(low) +
	     ((Address)SEXT_16(high)<<16) == src1);

	    words += generate_lda(insn+words, dest, dest, high, false);
	    words += generate_load(insn+words, (unsigned long) dest,  dest,
			       low, dw_long);
	} else {
	    words += generate_load(insn+words, (unsigned long) dest,  dest,
			       offset, dw_long);
	}
	base += words * 4; 
    } else if (op == loadFrameAddr) {
	unsigned long words = 0;
	// frame offset is signed.
	long offset = (long) src1;

	// the saved sp is 16 less than original sp (due to base tramp code)
	offset += 16;		

	// load fp into dest
	words += generate_load(insn+words, (unsigned long) dest,  REG_SP,
			       112, dw_quad);

	assert(ABS(offset) < (1<<30));
	if (ABS(offset) > 32767) {
	    Offset low = offset & 0xffff;
	    offset -= SEXT_16(low);
	    Offset high = (offset >> 16) & 0xffff;
	    assert((Address)SEXT_16(low) +
	     ((Address)SEXT_16(high)<<16) == src1);

	    // add high bits of offset
	    words += generate_lda(insn+words, dest, dest, high, false);
	    // now addd the low bits of the offset
	    words += generate_lda(insn+words, dest, dest, low, true);
	} else {
	    // addd the offset
	    words += generate_lda(insn+words, dest, dest, offset, true);
	}
	base += words * 4; 
    } else {
	abort();       // unexpected op for this emit!
    }
}

void emitVstore(opCode op, Register src1, Register src2, Address dest,
	     char *i, Address &base, bool /* noCost */, int size)
{
  instruction *insn = (instruction *) ((void*)&i[base]);
  assert(!((unsigned long)insn & (unsigned long)3));

  if (op ==  storeOp) {
    // st? dest, [src1]
    // src1 = value to store
    // src2 = register to hold address
    // dest = address
    int remainder;
//    unsigned words = generate_address(insn, (unsigned) src2, dest, remainder);
    unsigned long words = generate_address(insn, (unsigned long) src2, dest, remainder);
//    words += generate_store(insn+words, (unsigned) src1, (unsigned) src2,
    if (size == 8) {
	words += generate_store(insn+words, (unsigned long) src1, 
	    (unsigned long) src2, remainder, dw_quad);
    } else if (size == 4) {
	words += generate_store(insn+words, (unsigned long) src1, 
	    (unsigned long) src2, remainder, dw_long);
    } else {
	abort();
    }
    base += words * 4; return;
  } else if (op ==  storeFrameRelativeOp) {
    // frame offset is signed.
    long offset = (long) dest;

    // the saved sp is 16 less than original sp (due to base tramp code)
    offset += 16;		

    assert(ABS(offset) < 32767);
    unsigned long words = 0;
    words += generate_load(insn+words, (unsigned long) src2,  REG_SP,
			       112, dw_quad);
    if (size == 8) {
	words += generate_store(insn+words, (unsigned long) src1, 
	    (unsigned long) src2, offset, dw_quad);
    } else if (size == 4) {
	words += generate_store(insn+words, (unsigned long) src1, 
	    (unsigned long) src2, offset, dw_long);
    } else {
	abort();
    }
    base += words * 4; return;
  } else {
      abort();       // unexpected op for this emit!
  }
}

void emitVupdate(opCode op, RegValue /* src1 */, 
			    Register /*src2*/, Address /* dest */,
			    char *i, Address &base, bool /* noCost */)
{
  instruction *insn = (instruction *) ((void*)&i[base]);
  assert(!((unsigned long)insn & (unsigned long)3));

  if (op == updateCostOp) {
      return;
  } else {
      abort();       // unexpected op for this emit!
  }
}

void emitV(opCode op, Register src1, Register src2, Register dest,
	     char *i, Address &base, bool /*noCost*/, int size,
	     const instPoint * /* location */, process * /* proc */,
	     registerSpace * /* rs */ )
{
  //fprintf(stderr,"emitV(op=%d,src1=%d,src2=%d,dest=%d)\n",op,src1,src2,dest);

    assert ((op!=branchOp) && (op!=ifOp) && 
            (op!=trampTrailer) && (op!=trampPreamble));         // !emitA
    assert ((op!=getRetValOp) && (op!=getParamOp));             // !emitR
    assert ((op!=loadOp) && (op!=loadConstOp));                 // !emitVload
    assert ((op!=storeOp));                                     // !emitVstore
    assert ((op!=updateCostOp));                                // !emitVupdate

  instruction *insn = (instruction *) ((void*)&i[base]);
  assert(!((unsigned long)insn & (unsigned long)3));

  data_width width;
  if (size == 4) {
      width = dw_long;
  } else if (size == 8) {
      width = dw_quad;
  } else {
      // should not happen
      abort();
  }

  if (op == noOp) {
    unsigned long words = generate_nop(insn);
    base += words * 4; 
    return;
  } else if (op == loadIndirOp) {
    unsigned long words = generate_load(insn, dest, src1, 0, width);
    base += words * 4;
    return;
  } else if (op == storeIndirOp) {
    unsigned long words = generate_store(insn, src1, dest, 0, width);
    base += words * 4;
    return;
  } else {
    generate_integer_op(insn, src1, src2, dest, base, op);
    return;
  }
}


/************************************************************************
 * void restore_original_instructions(process* p, instPoint* ip)
************************************************************************/

void
restore_original_instructions(process* p, instPoint* ip) {
  Address addr = ip->addr;
  p->writeTextWord((caddr_t)addr, ip->originalInstruction.raw);
  addr += sizeof(instruction);
}

// The only non read only registers are those that are allocated - t0...t7

bool registerSpace::readOnlyRegister(Register reg_number) {
  if ((reg_number >= REG_T0) && (reg_number <= REG_T7))
    return false;
  else 
    return true;
}

//
// return cost in cycles of executing at this point.  This is the cost
//   of the base tramp if it is the first at this point or 0 otherwise.
//
// This was in inst.C, but the base tramp cost is machine dependent
// 
int getPointCost(process *proc, const instPoint *point)
{
  if (proc->baseMap.defines(point)) {
    return(0);
  } else {
    //  1 : decrement stack
    //  1 : push ra onto the stack
    //  3 : bsr to DYNINSTsave_temp (1 for call, 2 cycle branch penalty)
    // 14 : cost of DYNINSTsave_temp
    //  2 : empty nop slots
    //  3 : bsr to DYNINSTrestore_temp (1 for call, 2 cycle branch penalty)
    // 14 : cost of DYNINSTrestore_temp
    //  1 : load ra from stack
    //  1 : increment stack
    //  1 : slot for relocated instruction
    //  1 : decrement stack
    //  1 : push ra onto the stack
    //  3 : bsr to DYNINSTsave_temp (1 for call, 2 cycle branch penalty)
    // 14 : cost of DYNINSTsave_temp
    //  2 : empty nop slots
    //  3 : bsr to DYNINSTrestore_temp (1 for call, 2 cycle branch penalty)
    // 14 : cost of DYNINSTrestore_temp
    //  1 : load ra from stack
    //  1 : increment stack
    //  1 : return
    return(81);
  }
}

dictionary_hash<string, unsigned> funcFrequencyTable(string::hash);

void initDefaultPointFrequencyTable()
{
    FILE *fp;
    float value;
    char name[512];

    funcFrequencyTable["main"] = 1;
    funcFrequencyTable["DYNINSTsampleValues"] = 1;
    funcFrequencyTable[EXIT_NAME] = 1;

    // try to read file.
    fp = fopen("freq.input", "r");
    if (!fp) {
        return;
    } else {
        printf("found freq.input file\n");
    }
    while (!feof(fp)) {
        fscanf(fp, "%s %f\n", name, &value);
        funcFrequencyTable[name] = (int) value;
        printf("adding %s %f\n", name, value);
    }
    fclose(fp);
}



// unsigned findAndInstallBaseTramp(process *proc, 
trampTemplate *findAndInstallBaseTramp(process *proc, 
				       instPoint *&location,
				       returnInstance *&retInstance, 
				       bool /*trampRecursiveDesired*/,
				       bool /* noCost */,
                                       bool& /*deferred*/)
{
  trampTemplate *ret;
  process *globalProc;
  retInstance = NULL;

#ifdef notdef
    if (nodePseudoProcess && (proc->symbols == nodePseudoProcess->symbols)){
	globalProc = nodePseudoProcess;
	// logLine("findAndInstallBaseTramp global\n");
    } else {
	globalProc = proc;
    }
#endif
    globalProc = proc;

    if (!globalProc->baseMap.defines(location)) {
	ret = new trampTemplate;
	installBaseTramp(location, globalProc, *ret);
	instruction *insn = new instruction;

	Address pointAddr = location->addr;
	Address baseAddress;
	if (proc->getBaseAddress(location->image_ptr, baseAddress)) {
	    pointAddr += baseAddress;
	}
	generateBranchInsn(insn, ret->baseAddr - (pointAddr+4));
	globalProc->baseMap[location] = ret;
	retInstance = new returnInstance(1, (instruction *)insn,sizeof(instruction), 
	    pointAddr, sizeof(instruction)); 
    } else {
        ret = globalProc->baseMap[location];
    }
      
    return(ret);
}

/*
 * Install a single tramp.
 *
 */
void installTramp(instInstance *inst, process *proc, char *code, int codeSize,
		  instPoint * /*location*/, callWhen when)
{
    totalMiniTramps++;
    insnGenerated += codeSize/sizeof(int);

    // TODO cast
    proc->writeDataSpace((caddr_t)inst->trampBase, codeSize, code);

    // overwrite branches for skipping instrumentation
    trampTemplate *base = inst->baseInstance;
    if(when == callPreInsn && base->prevInstru == false) {
	base->cost += base->prevBaseCost;
	base->prevInstru = true;
	generateNoOp(proc, base->baseAddr + base->skipPreInsOffset);
    } else if (when == callPostInsn && base->postInstru == false) {
	base->cost += base->postBaseCost;
	base->postInstru = true;
	generateNoOp(proc, base->baseAddr + base->skipPostInsOffset);
    }
}


float getPointFrequency(instPoint *point)
{

    pd_Function *func;

    if (point->callee)
        func = point->callee;
    else
        func = point->func;

    if (!funcFrequencyTable.defines(func->prettyName())) {
      if (0 /*func->isLibTag()*/) {
	return(100);
      } else {
	return(250);
      }
    } else {
      return (funcFrequencyTable[func->prettyName()]);
    }
}

bool isReturn(const instruction insn) {
    return ((insn.mem_jmp.opcode == OP_MEM_BRANCH) &&
	    (insn.mem_jmp.ext == MD_RET));
}

// Borrowed from the POWER version
bool isReturnInsn(const image *owner, Address adr, bool &lastOne)
{
  instruction insn;

  insn.raw = owner->get_instruction(adr);
  // need to know if this is really the last return.
  //    for now assume one return per function - jkh 4/12/96
  lastOne = true;
  return isReturn(insn);
}

bool isCallInsn(const instruction i) {
  return (isBsr(i) || isJsr(i));
}

bool pd_Function::findInstPoints(const image *owner) 
{  
  Address adr = addr();
  instruction instr;
  instruction instr2;
  long gpValue;	// gp needs signed operations
  bool gpKnown = false;
  instPoint *point;
  Address t12Value;
  bool t12Known = false;
  instruction frameRestInsn;

  frame_size = 0;
  // normal linkage on alphas is that the first two instructions load gp.
  //   In this case, many functions jump past these two instructions, so
  //   we put the inst point at the third instruction.
  //   However, system call libs don't always follow this rule, so we
  //   look for a load of gp in the first two instructions.
  instr.raw = owner->get_instruction(adr);
  instr2.raw = owner->get_instruction(adr+4);
  if ((instr.mem.opcode == OP_LDAH) && (instr.mem.ra == REG_GP) &&
      (instr2.mem.opcode == OP_LDA) && (instr2.mem.ra == REG_GP)) {
      // compute the value of the gp
      gpKnown = true;
      gpValue = ((long) adr) + (SEXT_16(instr.mem.disp)<<16) + instr2.mem.disp;
      adr += 8;
  }

  instr.raw = owner->get_instruction(adr);
  if (!IS_VALID_INSN(instr)) 
    goto set_uninstrumentable;

  funcEntry_ = new instPoint(this, instr, owner, adr, true,functionEntry);
  assert(funcEntry_);

  // perform simple data flow tracking on t12 within a basic block.
  
  while (true) {
    instr.raw = owner->get_instruction(adr);

    bool done;

    // check for lda $sp,n($sp) to guess frame size
    if (!frame_size && ((instr.raw & 0xffff0000) == 0x23de0000)) {
	// lda $sp,n($sp)
	frame_size = -((short) (instr.raw & 0xffff));
	if (frame_size < 0) {
	    // we missed the setup and found the cleanup
	    frame_size = 0;
	}
    }

    // check for return insn and as a side affect decide if we are at the
    //   end of the function.
    if (isReturnInsn(owner, adr, done)) {
      // define the return point
      // check to see if adr-8 is ldq fp, xx(sp) or ldq sp, xx(sp), if so 
      // use it as the
      // address since it will ensure the activation record is still active.
      // Gcc uses a frame pointer, on others only sp is used.

      frameRestInsn.raw = owner->get_instruction(adr-8);
      if (((frameRestInsn.raw & 0xffff0000) == 0xa5fe0000) ||
	  ((frameRestInsn.raw & 0xffff0000) == 0x23de0000)) {
	  Address tempAddr = adr - 8;
	  funcReturns.push_back(new instPoint(this,frameRestInsn,
					      owner,tempAddr,false,functionExit));
      } else {
	  funcReturns.push_back(new instPoint(this,instr,owner,adr,false,functionExit));
      }

      // see if this return is the last one 
      if (done) goto set_instrumentable;
    } else if (isCallInsn(instr)) {
      // define a call point
      point = new instPoint(this, instr, owner, adr, false,callSite);

      if (isJsr(instr)) {
	  Address destAddr = 0;
	  if ((instr.mem_jmp.rb == REG_T12) && t12Known) {
	      destAddr = t12Value;
	  }
	  point->callIndirect = true;
	  // this is the indirect address
          point->callee = (pd_Function *) destAddr;		
      } else {
          point->callIndirect = false;
	  point->callee = NULL;
      }
      calls.push_back(point);
      t12Known = false;
    } else if (isJmpType(instr) || isBranchType(instr)) {
      // end basic block, kill t12
      t12Known = false;
    } else if ((instr.mem.opcode == OP_LDQ) && (instr.mem.ra == REG_T12) &&
	       (instr.mem.rb = REG_GP)) {
      // intruction is:  ldq t12, disp(gp)
      if (gpKnown) {
	  t12Value = gpValue + instr.mem.disp;
	  t12Known = true;
      }
    }

    // now do the next instruction
    adr += 4;

  }

 set_instrumentable:
  isInstrumentable_ = 1;
  return true;
  
 set_uninstrumentable:
  isInstrumentable_ = 0;
  return false;
}

//
// Each processor may have a different heap layout.
//   So we make this processor specific.
//
// find all DYNINST symbols that are data symbols
bool process::heapIsOk(const pdvector<sym_data> &find_us) {
  bool err;
  Symbol sym;
  string str;
  Address addr;
  /* Address instHeapStart; */

  // find the main function
  // first look for main or _main
  if (!((mainFunction = findOnlyOneFunction("main")) 
        || (mainFunction = findOnlyOneFunction("_main")))) {
     string msg = "Cannot find main. Exiting.";
     statusLine(msg.c_str());
     showErrorCallback(50, msg);
     return false;
  }
  
  for (unsigned long i=0; i<find_us.size(); i++) {
    addr = findInternalAddress(find_us[i].name, false, err);
    //printf("looking for %s\n", find_us[i].name.c_str());
    if (err) {
	string msg;
        msg = string("Cannot find ") + str + string(". Exiting");
	statusLine(msg.c_str());
	showErrorCallback(50, msg);
	return false;
    }
  }

#if 0
  string ghb = "_DYNINSTtext";
  addr = findInternalAddress(ghb, false, err);
  if (err) {
      string msg;
      msg = string("Cannot find _DYNINSTtext") + string(". Exiting");
      statusLine(msg.c_str());
      showErrorCallback(50, msg);
  }
  instHeapStart = addr;

  string hd = "DYNINSTdata";
  addr = findInternalAddress(hd, true, err);
  if (err) {
      string msg;
      msg = string("Cannot find DYNINSTdata") + string(". Exiting");
      statusLine(msg.c_str());
      showErrorCallback(50, msg);
      return false;
  }
  instHeapStart = addr;
#endif
  return true;
}

void emitImm(opCode op, Register src1, RegValue src2imm, Register dest, 
             char *i, Address &base, bool /* noCost */)
{
  instruction *insn = (instruction *) ((void*)&i[base]);
  assert(!((unsigned long)insn & (unsigned long)3));
  generate_integer_op(insn, src1, src2imm, dest, base, op, true);
  return;
}

Register
emitFuncCall(opCode /* op */, 
	     registerSpace *rs,
	     char *i, Address &base, 
	     const pdvector<AstNode *> &operands,
	     const string &callee, process *proc, bool noCost,
	     const function_base *calleebase,
	     const pdvector<AstNode *> &ifForks,
             const instPoint *location) // FIXME: pass it!
{
  pdvector <Register> srcs;
  
  // First, generate the parameters
  for (unsigned u = 0; u < operands.size(); u++)
    srcs.push_back(operands[u]->generateCode_phase2(proc, rs, i , base, 
						    false, ifForks));

  // put parameters in argument registers
  // register move is "bis src, src, dest"  (bis is logical or)
  // save and restore the values currently in the argument registers
  //  unsigned long words = 0;
  Address addr;
  bool err;
  void cleanUpAndExit(int status);

  if (calleebase)
       addr = calleebase->getEffectiveAddress(proc);
  else {
       addr = proc->findInternalAddress(callee, false, err);
       if (err) {
	    function_base *func_b = proc->findOnlyOneFunction(callee);
	    if (!func_b) {
		 ostrstream os(errorLine, 1024, ios::out);
		 os << "Internal error: unable to find addr of " << callee << endl;
		 logLine(errorLine);
		 showErrorCallback(80, (const char *) errorLine);
		 P_abort();
	    }
	    addr = func_b->addr();
       }
  }

  int remainder;
  instruction *insn = (instruction *) ((void*)&i[base]);
  Address dyn_save;
  Address dyn_restore;

  if (!skipSaveCalls) {
      function_base *fun_save = proc->findOnlyOneFunction("DYNINSTsave_misc");
      function_base *fun_restore = proc->findOnlyOneFunction("DYNINSTrestore_misc");
      assert(fun_save && fun_restore);
      dyn_save = fun_save->addr();
      dyn_restore = fun_restore->addr();

      Symbol info;
      Address baseAddr;
      proc->getSymbolInfo("DYNINSTsave_misc", info, baseAddr);

      dyn_save += baseAddr;
      dyn_restore += baseAddr;

      // save the current values in v0, a0..a5, pv, at, gp
      // Call to DYNINSTsave_misc
      base += (4* generate_address(insn, REG_T10, dyn_save, remainder));
      if (remainder) {
	  insn = (instruction *) ((void*)&i[base]);
	  base += (4 * generate_lda(insn, REG_T10, REG_T10, remainder, true));
      }
      insn = (instruction *) ((void*)&i[base]);
      base += (4 * generate_jump(insn, REG_T10, MD_JSR, REG_RA, remainder));

      //  words += (base - temp_base) / sizeof(insn);
      //  base = temp_base;
  }

  for (unsigned u=0; u<srcs.size(); u++){
    if (u >= 5) {
	 string msg = "Too many arguments to function call in instrumentation code: only 5 arguments can be passed on the sparc architecture.\n";
	 fprintf(stderr, msg.c_str());
	 showErrorCallback(94,msg);
	 cleanUpAndExit(-1);
    }
    insn = (instruction *) ((void*)&i[base]);
    base += (4 * generate_operate(insn, srcs[u], srcs[u], u+REG_A0, 
		OP_BIS, FC_BIS));
  }

  // Set the value of  pv/t12/r27
  // I have not found any documentation on this
  // But the alpha appears to put the callee's address in this register
  // this register must be saved and restored

  // Jump to the function
  insn = (instruction *) ((void*)&i[base]);
  base += (4 *generate_address(insn, REG_PV, addr, remainder));
  if (remainder)
    {
      insn = (instruction *) ((void*)&i[base]);
      base += (4 * generate_lda(insn, REG_PV, REG_PV, remainder, true));
    }

  // TODO -- clear other argument registers ?
  insn = (instruction *) ((void*)&i[base]);
  base += (4 * generate_jump(insn, REG_PV, MD_JSR, REG_RA, remainder));
  
  if (!skipSaveCalls) {
      // Call to DYNINSTrestore_misc
      insn = (instruction *) ((void*)&i[base]);
      base += (4 * generate_address(insn, REG_T10, dyn_restore, remainder));
      if (remainder) {
	  insn = (instruction *) ((void*)&i[base]);
	  base += (4 * generate_lda(insn, REG_T10, REG_T10, remainder, true));
      }
      insn = (instruction *) ((void*)&i[base]);
      base += ( 4 * generate_jump(insn, REG_T10, MD_JSR, REG_RA, remainder));
  }

  Register dest = rs->allocateRegister(i, base, noCost);

  insn = (instruction *) ((void*)&i[base]);

  // or v0,v0,dest
  generate_operate(insn,REG_V0,REG_V0,dest,OP_BIS,FC_BIS);
  base+= sizeof(instruction);

  for (unsigned u=0; u<srcs.size(); u++){
    rs->freeRegister(srcs[u]);
  }

  return dest;
}

bool returnInstance::checkReturnInstance(const pdvector< pdvector<Frame> > &stackWalks)
{
  // Single instruction jumps == we don't have to implement
  return true;
}
 
void returnInstance::installReturnInstance(process *proc) {
    proc->writeTextSpace((caddr_t)addr_, instSeqSize, (caddr_t) instructionSeq); 
	installed = true;
}

void returnInstance::addToReturnWaitingList(Address /* pc */, 
					    process*  /* proc */) 
{
    P_abort();
}

void generateBreakPoint(instruction &insn)
{
  insn.raw = BREAK_POINT_INSN;
}

void generateIllegalInsn(instruction &insn) { // instP.h
   insn.raw = 0;
}

// findCallee: returns false unless callee is already set in instPoint
// dynamic linking not implemented on this platform
bool process::findCallee(instPoint &instr, function_base *&target){

    if((target = const_cast<function_base *>(instr.iPgetCallee()))) {
       return true;
    }
    if (instr.callIndirect && instr.callee) {
	// callee contains the address in the mutatee
	// read the contents of the address
	Address dest;
	if (!readDataSpace((caddr_t)(instr.callee), sizeof(Address),
	    (caddr_t)&(dest),true)) {
	    return false;
	}
	// now lookup the funcation
	target = findFuncByAddr(dest);
	if (target) return true;
    }
    return false;
}

bool process::replaceFunctionCall(const instPoint *point,
				  const function_base *newFunc) {
  // Must be a call site
    if (point->ipType != callSite)
      return false;
    
    // Cannot already be instrumented with a base tramp
    if (baseMap.defines(point))
	return false;
    instruction newInsn;
    if (newFunc == NULL) {	// Replace with a NOOP
      generateNOOP(&newInsn);
    } else {			// Replace with a new call instruction
      generateBSR(&newInsn, newFunc->addr()+sizeof(instruction)-point->addr);
    }
    
    writeTextSpace((caddr_t)point->addr, sizeof(instruction), &newInsn);
    
    return true;
}

static const Address lowest_addr = 0x00400000;
void process::inferiorMallocConstraints(Address near, Address &lo, Address &hi,
					inferiorHeapType /*type*/)
{
  if (near)
    {
      // Avoid wrapping issues
      if (near < lowest_addr + MAX_BRANCH)
	lo = lowest_addr;
      else
	lo = near - MAX_BRANCH;
      hi = near + MAX_BRANCH;
    }
  else // near == 0
    {
      lo = lowest_addr;
    }
}

void process::inferiorMallocAlign(unsigned &size)
{
  // quadword-aligned (stack alignment)
  unsigned align = 16;
  if (size % align) size = ((size/align)+1)*align;
}

// Emit code to jump to function CALLEE without linking.  (I.e., when
// CALLEE returns, it returns to the current caller.)
void emitFuncJump(opCode op, 
		  char *i, Address &base, 
		  const function_base *callee, process *proc,
		  const instPoint *, bool)
{

    Address addr;
    int remainder;
    unsigned long count;

    assert(op == funcJumpOp);

    addr = callee->getEffectiveAddress(proc);
    /* Address addr2 = (const_cast<function_base *>(callee))->getAddress(0); */
    instruction *insn = (instruction *) ((void*)&i[base]);
    count = 0;

    // Cleanup stack state from tramp preamble

    // call DYNINSTrestore_temp
    Symbol info;
    Address baseAddr;
    function_base *fun_restore = proc->findOnlyOneFunction("DYNINSTrestore_temp");
    proc->getSymbolInfo("DYNINSTrestore_temp", info, baseAddr);
    assert(fun_restore);
    Address dyn_restore = fun_restore->addr() + baseAddr;

    callAbsolute(insn, count, dyn_restore);

    // load ra from the stack
    count += generate_load(&insn[count], REG_RA, REG_SP, 0, dw_quad);

    // load GP from the stack
    count += generate_load(&insn[count], REG_GP, REG_SP, 8, dw_quad);

    // increment stack by 16
    count += generate_lda(&insn[count], REG_SP, REG_SP, 16, true);

    // save gp and ra in special location
    // **** Warning this fails in the case of replacing a mutually recursive
    //    function
    Address saveArea = proc->inferiorMalloc(2*sizeof(long), dataHeap, 0);

    count += generate_address(&insn[count], REG_T12, saveArea, remainder);
    count += generate_store(&insn[count], REG_GP, REG_T12, remainder, 
	dw_quad); 

    count += generate_store(&insn[count], REG_RA, REG_T12, 
	remainder+sizeof(long), dw_quad);

    // calling convention seems to expect t12 to contain the address of the
    //    suboutine being called, so we use t12 to build the address
    count += generate_address(&insn[count], REG_T12, addr, remainder);
    if (remainder)
	count += generate_lda(&insn[count], REG_T12, REG_T12, remainder, true);
    count += generate_jump(&insn[count], REG_T12, MD_JSR, REG_RA, remainder);

    count += generate_nop(&insn[count]);

    // back after function, restore everything
    count += generate_address(&insn[count], REG_RA, saveArea, remainder);
    count += generate_load(&insn[count], REG_GP, REG_RA, remainder, 
	dw_quad); 

    count += generate_load(&insn[count], REG_RA, REG_RA, 
	remainder+sizeof(long), dw_quad);
    count += generate_jump(&insn[count], REG_RA, MD_JMP, REG_ZERO, remainder);
    count += generate_nop(&insn[count]);

    base += count * sizeof(instruction);
}

void generateMTpreamble(char *, Address &, process *) {
	assert( 0 );	// We don't yet handle multiple threads.
	} /* end generateMTpreamble() */

void emitLoadPreviousStackFrameRegister(Address, Register,
					char *, Address &, int, bool){
  assert(0);
}
 
#ifndef BPATCH_LIBRARY
bool process::isDynamicCallSite(instPoint *callSite){
  function_base *temp;
  if(!findCallee(*(callSite),temp)){
    return true;
  }
  return false;
}
 
bool process::MonitorCallSite(instPoint *callSite){
  return false;
}
#endif

bool deleteBaseTramp(process * /*proc*/, instPoint* /*location*/,
                     trampTemplate *, instInstance * /*lastMT*/)
{
	cerr << "WARNING : deleteBaseTramp is unimplemented "
	     << "(after the last instrumentation deleted)" << endl;
	return false;
}

#ifdef BPATCH_LIBRARY
/*
 * createInstructionInstPoint
 *
 * Create a BPatch_point instrumentation point at the given address, which
 * is guaranteed not be one of the "standard" inst points.
 *
 * proc         The process in which to create the inst point.
 * address      The address for which to create the point.
 */
BPatch_point *createInstructionInstPoint(process *proc, void *address,
					 BPatch_point** /*alternative*/,
					 BPatch_function* bpf)
{
    function_base *func = NULL;
    if(bpf)
        func = bpf->func;
    else
        func = proc->findFuncByAddr((Address)address);

    if (!isAligned((Address)address))
	return NULL;

    instruction instr;
    proc->readTextSpace(address, sizeof(instruction), &instr.raw);

    pd_Function* pointFunction = (pd_Function*)func;
    Address pointImageBase = 0;
    image* pointImage = pointFunction->file()->exec();
    proc->getBaseAddress((const image*)pointImage,pointImageBase);
    Address pointAddress = (Address)address-pointImageBase;

    instPoint *newpt = new instPoint(pointFunction,
				    (const instructUnion &) instr,
				    (const image *) NULL, // image * - ignored
				    (Address &)pointAddress,
				    false, // bool delayOk - ignored
				    otherPoint);

    pointFunction->addArbitraryPoint(newpt,NULL);


    return proc->findOrCreateBPPoint(NULL, newpt, BPatch_instruction);
}

#include "BPatch_point.h"
#include "BPatch_snippet.h"

int BPatch_point::getDisplacedInstructions(int maxSize, void *insns)
{
    if (static_cast<unsigned>(maxSize) >= sizeof(instruction))
        memcpy(insns, &point->originalInstruction.raw, sizeof(instruction));

    return sizeof(instruction);
}

#endif

// needed in metric.C
bool instPoint::match(instPoint *p)
{
  if (this == p)
    return true;
  
  // should we check anything else?
  if (addr == p->addr)
    return true;
  
  return false;
}
