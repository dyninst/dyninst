
#include "util/h/headers.h"

#include "rtinst/h/rtinst.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "arch-alpha.h"
#include "ast.h"
#include "util.h"
#include "dyninstAPI/src/instPoint.h"
#include <sys/procfs.h>
#ifndef BPATCH_LIBRARY
#include "internalMetrics.h"
#endif
#include "stats.h"
#include "os.h"

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
static inline Address generate_integer_op(instruction *insn, Address, Address,
					  Address, unsigned long&, opCode,bool Imm = false);
static inline Address generate_call_code(instruction*, Address, Address,
					 Address, unsigned long&, process *proc);
static inline Address generate_tramp_preamble(instruction*, Address,
					      Address, unsigned long&);

// Register usage conventions:
//   a0-a5 are saved in the base trampoline
//   t0-t10 are scratch and do not have to be saved as long as instrumentation
//      is restricted to function entry/exit, and call points
//      these will be allocated by paradynd
//
//   t0-t7 : allocated by paradynd
//   t8: stores compare results, scratch for paradynd
//   t9,t10 : used as scratch registers by paradynd

// Return value register
#define REG_V0           0
// scratch registers -- not saved across procedure calls
#define REG_T0           1
#define REG_T1           2
#define REG_T2           3
#define REG_T3           4
#define REG_T4           5
#define REG_T5           6
#define REG_T6           7
#define REG_T7           8

// caller save registers
#define REG_S0           9
#define REG_S1          10
#define REG_S2          11
#define REG_S3          12
#define REG_S4          13
#define REG_S5          14

// argument registers
#define REG_A0          16
#define REG_A1          17
#define REG_A2          18
#define REG_A3          19
#define REG_A4          20
#define REG_A5          21

// more scratch registers
#define REG_T8          22
#define REG_T9          23
#define REG_T10         24
#define REG_T11         25

// return address register
#define REG_RA          26

// undocumented pv/t12
#define REG_PV          27

#define REG_GP          29
#define REG_SP          30
#define REG_ZERO        31

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
int regList[] = {1, 2, 3, 4, 5, 6, 7, 8};

trampTemplate baseTemplate;

string getStrOp(int op) {
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
  if ( (value <= 255) && (value >= -255) ) return(true);
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
generate_operate(instruction *insn, unsigned long reg_a, unsigned long reg_b, 
	 unsigned long reg_c, unsigned int opcode, unsigned int func_code) {
  assert(reg_a < 32);
  assert(reg_b < 32);
  assert(reg_c < 32);
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
generate_lit_operate(instruction *insn, unsigned long reg_a, int literal,
	     unsigned long reg_c, unsigned int opcode, unsigned int func_code) {
  assert(reg_a < 32);
  assert(ABS(literal) < MAX_IMM);
  assert(reg_c < 32);
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
generate_jump(instruction *insn, unsigned long dest_reg, unsigned long ext_code,
	      unsigned long ret_reg, int hint) {
  assert(dest_reg < 32); assert(ret_reg < 32); assert((ext_code == MD_JSR));
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
generate_branch(instruction *insn, unsigned long src_reg, int offset, unsigned int opcode) {
  if (ABS(offset >> 2) > MAX_BRANCH) { logLine("a branch too far\n"); assert(0); }
  assert(src_reg < 32);
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
genRelOp(instruction *insn, unsigned opcode, unsigned fcode, unsigned long src1,
	unsigned long src2, unsigned long dest, bool Imm = false,bool do_not=false) {
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
		     const image *owner, Address &adr,
		     const bool delayOK,instPointType pointType)
: addr(adr), originalInstruction(instr), inDelaySlot(false), isDelayed(false),
  callIndirect(false), callAggregate(false), callee(NULL), func(f),ipType(pointType)
{
}



// lda(h) rdest, literal:16(rstart)
// put a literal in a register
// offset is the value before the shift
// unsigned 
// generate_lda(instruction *insn, unsigned rdest, unsigned rstart,
//	     int offset, bool do_low) {
unsigned long
generate_lda(instruction *insn, unsigned long rdest, unsigned long rstart,
	     long offset, bool do_low) {

  assert(ABS(offset) < shifted_16);
  assert(offset >= (- (long) 0xffffffff));
  assert(rdest < 32);
  assert(rstart < 32);

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
// generate_sxl(instruction *insn, unsigned rdest, unsigned amount,
//	     bool left, unsigned rsrc) {
unsigned long
generate_sxl(instruction *insn, unsigned long rdest, unsigned long amount,
	     bool left, unsigned long rsrc) {
  assert(amount < 32);
  insn->raw = 0;
  // insn->oper.sbz = 0;
  // insn->oper.zero = 0;

  insn->oper.opcode = left ? OP_SLL : OP_SRL;
  insn->oper.function = left ? FC_SLL : FC_SRL;
  insn->oper.ra = rsrc;
  insn->oper.rb = amount;
  insn->oper.rc = rdest;
  return 1;
}

#define SEXT_16(x) (((x) & bit_15) ? ((x) | 0xffffffffffff0000) : (x))

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
  Address extra = 0;
  Address third = THIRD_16(addr);
  Address low = 0xffff & addr;
  Address tmp1 = addr - SEXT_16(low);
//  Address high = tmp1 >> 16;
  Address high = (tmp1 >> 16) & 0xffff;
  Address tmp2 = tmp1 - ((SEXT_16(high)) << 16);

  if (tmp2) {
    extra = 0x4000;
    tmp1 -= 0x40000000;
    high = tmp1 >> 16;
  } else
    extra = 0;
  
  bool primed = false;
  if (third) {
    primed = true;
    assert(third < shifted_16);
    generate_lda(insn+words, rdest, REG_ZERO, third, false);
    words++;
    generate_sxl(insn+words, rdest, 16, true, rdest);  // move (31:15) to (47:32)
    words++;
  }
  // generate_lda(insn+words, rdest, rdest, low, true);
  // words++;
  remainder = low;

  if (extra) {
    if (primed) 
      generate_lda(insn+words, rdest, rdest, extra, false);
    else
      generate_lda(insn+words, rdest, REG_ZERO, extra, false);
    words++; primed = true;
  }
  if (high) {
    if (primed)
      generate_lda(insn+words, rdest, rdest, high, false);
    else
      generate_lda(insn+words, rdest, REG_ZERO, high, false);
    words++; primed = true;
  }
  // a better solution is to use register zero as the base
  // clear the base register
  if (!primed) 
    words += generate_operate(insn+words, REG_ZERO, REG_ZERO, rdest, 
			      OP_BIS, FC_BIS);

  assert((Address)SEXT_16(low) +
	 ((Address)SEXT_16(extra)<<16) +
	 ((Address)SEXT_16(high)<<16) +
	 ((Address)SEXT_16(third)<<32) == addr);
  return words;
}

// stx rsrc, [rbase + sign_extend(offset:16)]
// unsigned
// generate_store(instruction* insn, unsigned rsrc, unsigned rbase, int disp,
//	       data_width width) {
unsigned long
generate_store(instruction* insn, unsigned long rsrc, unsigned long rbase, 
	int disp, data_width width) {
  assert((Address)disp < shifted_16); assert(rsrc < 32); assert(rbase < 32);
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
generate_load(instruction *insn, unsigned long rdest, unsigned long rbase, 
	int disp, data_width width,bool aligned = true) {
  assert(disp < shifted_16); assert(rdest < 32); assert(rbase < 32);
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

  vector<instPoint*> non_lib;

  for (i=0; i<calls.size(); ++i) {
    /* check to see where we are calling */
    p = calls[i];
    assert(p);

    if (isJsr(p->originalInstruction)) {
      // assume this is a library function
      // since it is a jump through a register
      // TODO -- should this be deleted here ?
      p->callee = NULL;
      non_lib += p;
      //      delete p;
    } else if (isBsr(p->originalInstruction)) {
      loc_addr = p->addr + (p->originalInstruction.branch.disp << 2)+4;
      non_lib += p;
      pd_Function *pdf = (file_->exec())->findFunction(loc_addr);

      if (pdf == NULL)
	{
	  /* Try alternate entry point in the symbol table */
	  loc_addr = loc_addr - 8;
	  pdf = (file_->exec())->findFunction(loc_addr);
	}

      if (pdf && !pdf->isLibTag())
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
Address pd_Function::newCallPoint(Address adr, const instruction code,
				 const image *owner, bool &err) {
  instPoint *point;
  err = true;
  point = new instPoint(this, code, owner, adr, false,callSite);

  if (isJsr(code)) {
    point->callIndirect = true;
    point->callee = NULL;
  } else
    point->callIndirect = false;

  calls += point;
  err = false;
  return adr;
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

//
// Note -- the code in initTramps and installBaseTramp should be changed
// together.  initTramps generates a "dummy" base trampoline.  
// installBaseTramp generates the real thing.  the dummy base tramp is 
// used to determine offsets and the trampoline size
//
void initTramps() {
  static instruction insn_buffer[50];
  static bool init_done=false;
//  unsigned words = 0;
  unsigned long words = 0;

  if (init_done) return;
  init_done = true;

  regSpace = new registerSpace(sizeof(regList)/sizeof(int), regList, 0, NULL);

  // Pre branch
  baseTemplate.skipPreInsOffset = words*4;
  words += generate_nop(insn_buffer+words);
  // decrement stack by 8
  words += generate_nop(insn_buffer+words);

  // push ra onto the stack
  words += generate_nop(insn_buffer+words);

  // Call to DYNINSTsave
  words += generate_nop(insn_buffer+words);

  // global_pre -- assume a word is 4 bytes
  baseTemplate.globalPreOffset = words * 4;
  words += generate_nop(insn_buffer+words);

  // local_pre
  baseTemplate.localPreOffset = words * 4;
  words += generate_nop(insn_buffer+words);

  // Call to DYNINSTrestore
  baseTemplate.localPreReturnOffset = words * 4;
  words += generate_nop(insn_buffer+words);

  // load ra from the stack
  words += generate_nop(insn_buffer+words);

  // increment stack by 8
  words += generate_nop(insn_buffer+words);

  baseTemplate.emulateInsOffset = words*4;
  // slot for emulate instruction
  words += generate_nop(insn_buffer+words);

  // decrement stack by 8
  words += generate_nop(insn_buffer+words);

  // push ra onto the stack
  words += generate_nop(insn_buffer+words);

  // Call to DYNINSTsave
  words += generate_nop(insn_buffer+words);

  // global_post
  baseTemplate.globalPostOffset = words * 4;
  words += generate_nop(insn_buffer+words);

  // local_post
  baseTemplate.localPostOffset = words * 4;
  words += generate_nop(insn_buffer+words);

  // Call to DYNINSTrestore
  baseTemplate.localPostReturnOffset = words * 4;
  words += generate_nop(insn_buffer+words);

  // load ra from the stack
  words += generate_nop(insn_buffer+words);

  // increment stack by 8
  words += generate_nop(insn_buffer+words);

  words += generate_nop(insn_buffer+words);   // nop
  // slot for return instruction
  words += generate_nop(insn_buffer+words);
  //  words += generate_nop(insn_buffer+words);   //nop padding to 64bit boundary
  words += generate_nop(insn_buffer+words);   // nop
  
  baseTemplate.size = words * 4;
  baseTemplate.trampTemp = (void*) insn_buffer;
}

/*
 * Install a base tramp -- fill calls with nop's for now.
 * An obvious optimization is to turn off the save-restore calls when they 
 * are not needed.
 */
trampTemplate *installBaseTramp(Address baseAddr, 
		      instPoint *location,
		      process *proc) {
//  unsigned words = 0;
  unsigned long words = 0;

  assert(baseTemplate.size > 0);
  instruction *code = new instruction[baseTemplate.size]; assert(code);

  function_base *fun_save = proc->findOneFunction("DYNINSTsave_temp");
  function_base *fun_restore = proc->findOneFunction("DYNINSTrestore_temp");
  assert(fun_save && fun_restore);
  Address dyn_save = fun_save->addr();
  Address dyn_restore = fun_restore->addr();
  Address off_save = dyn_save - baseAddr;
  Address off_restore = dyn_restore - baseAddr;

  // Pre branch
  words += generate_nop(code+words);
  // decrement stack by 8
  words += generate_lda(code+words, REG_SP, REG_SP, -8, true);

  // push ra onto the stack
  words += generate_store(code+words, REG_RA, REG_SP, 0, dw_quad);

  // Call to DYNINSTsave_temp
  words += generate_branch(code+words, REG_RA, off_save-(words*4)-4, OP_BSR);

  // global_pre
  words += generate_nop(code+words);

  // local_pre
  words += generate_nop(code+words);

  // Call to DYNINSTrestore_temp
  words += generate_branch(code+words, REG_RA, off_restore-(words*4)-4, OP_BSR);

  // load ra from the stack
  words += generate_load(code+words, REG_RA, REG_SP, 0, dw_quad);

  // increment stack by 8
  words += generate_lda(code+words, REG_SP, REG_SP, 8, true);

  // slot for emulate instruction
  instruction *reloc = &code[words]; *reloc = location->originalInstruction;
  relocateInstruction(reloc, location->addr, baseAddr + (words * 4));
  words += 1;

  // decrement stack by 8
  words += generate_lda(code+words, REG_SP, REG_SP, -8, true);

  // push ra onto the stack
  words += generate_store(code+words, REG_RA, REG_SP, 0, dw_quad);

  // Call to DYNINSTsave_temp
  words += generate_branch(code+words, REG_RA, off_save-(words*4)-4, OP_BSR);

  // global_post
  words += generate_nop(code+words);

  // local_post
  words += generate_nop(code+words);

  // Call to DYNINSTrestore_temp
  words += generate_branch(code+words, REG_RA, off_restore-(words*4)-4, OP_BSR);

  // load ra from the stack
  words += generate_load(code+words, REG_RA, REG_SP, 0, dw_quad);

  // increment stack by 8
  words += generate_lda(code+words, REG_SP, REG_SP, 8, true);

  // If the relocated insn is a Jsr or Bsr then 
  // appropriately set Register Ra
  if (isCallInsn(location->originalInstruction))
    words += generate_load(code+words,REG_RA,REG_RA,40,dw_quad,true);
  else 
    words += generate_nop(code+words);

  // slot for return (branch) instruction
  reloc = &code[words];
  // location->addr + 4 is address of instruction after relocated instr
  // curr_addr is address of the branch instruction that returns to user code
  // curr_addr + 4 is updated pc when branch instruction executes
  Address curr_addr = baseAddr + (words * 4);
  words += generate_branch(reloc, REG_ZERO,
			   (location->addr+4) - (curr_addr+4), OP_BR);
  //  words += 1;  // skip the padding NOP
  ((instruction *)(code+words))->raw = location->addr+4;
  words += 1;

  proc->writeDataSpace((caddr_t)baseAddr, baseTemplate.size, (caddr_t) code);
  delete (code);
}

//
// move the passed parameter into the passed register, or if it is already
//    in a register return the register number.
//
reg getParameter(reg dest, int param) {
  if (param <= 5) {
    return(16+param);
  }
  assert(0); 
  return((reg)-1);
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
  int sraw = insn.raw;
  unsigned uraw = insn.raw;

  proc->writeTextWord((caddr_t)fromAddr, insn.raw);
}

// The Alpha does not have a divide instruction
// The divide is performed in software by calling __divl
int software_divide(int src1,int src2,int dest,char *i,Address &base,bool noCost,Address divl_addr,bool Imm = false)
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

static inline Address
generate_integer_op(instruction *insn, Address src1, Address src2, 
		    Address dest, unsigned long& base, opCode op,bool Imm = FALSE) {
//		    Address dest, unsigned & base, opCode op) {
  // integer ops
//  unsigned op_code=0, func_code=0, words = 0;
  unsigned op_code=0, func_code=0;
  unsigned long words = 0;

  switch (op) {

  case plusOp:        op_code = OP_ADDL; func_code = FC_ADDL; break;
  case minusOp:       op_code = OP_SUBL; func_code = FC_SUBL; break;
  case timesOp:       op_code = OP_MULL; func_code = FC_MULL; break;
  case divOp:         assert(0); // shouldn't get here. Call software_divide from the ast
  case orOp:          op_code = OP_BIS;  func_code = FC_BIS; break;
  case andOp:         op_code = OP_AND;  func_code = FC_AND; break;

  // The compare instruction leave a one in dest if the compare is true
  // beq branch when dest is 0 --> and the compare is false

  case eqOp:
    words += genRelOp(insn, OP_CMPEQ, FC_CMPEQ, src1, src2, dest,Imm);
    base += words * 4; return(0);

  case neOp:
    // last arg == true --> inverts value of comparison		      
    words += genRelOp(insn, OP_CMPEQ, FC_CMPEQ, src1, src2, dest,Imm,true);
    base += words * 4; return(0);

  case lessOp:
    words += genRelOp(insn, OP_CMPLT, FC_CMPLT, src1, src2, dest,Imm);
    base += words * 4; return(0);
		      
  case greaterOp:                          
    words += genRelOp(insn, OP_CMPLE, FC_CMPLE, src1, src2, dest,Imm,true);
    base += words * 4; return(0);

  case leOp:
    words += genRelOp(insn, OP_CMPLE, FC_CMPLE, src1, src2, dest,Imm);
    base += words * 4; return(0);

  case geOp:                               
    words += genRelOp(insn, OP_CMPLE, FC_CMPLT, src1, src2, dest,Imm,true);
    base += words * 4; return(0);

  default:
    assert(0);
    break;
  }

  if (Imm)
    words += generate_lit_operate(insn+words, src1, src2, dest, op_code, func_code);
  else
    words += generate_operate(insn+words, src1, src2, dest, op_code, func_code);
  base += words * 4; return 0;
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

  function_base *fun_save = proc->findOneFunction("DYNINSTsave_misc");
  function_base *fun_restore = proc->findOneFunction("DYNINSTrestore_misc");
  assert(fun_save && fun_restore);
  Address dyn_save = fun_save->addr();
  Address dyn_restore = fun_restore->addr();

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

static inline Address
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
  if (ABS(src1) < MAX_IMM) {
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
  base += words * 4; return 0;
}


// The value return by emit can be
// 
// ifOp: address to calculate branch
// 
// TODO -- if an offset is returned, it is the offset to the instruction
//         where the branch occurs.  This offset cannot be used to
//         calculate branches, the offset of the next instruction should
//         be used.  The alpha uses the UPDATED pc for branches.
// 
// Address emit(opCode op, reg src1, reg src2, reg dest,
//	     char *i, unsigned &base) {
Address emit(opCode op, reg src1, reg src2, reg dest,
	     char *i, Address &base,bool noCost) {

  instruction *insn = (instruction *) ((void*)&i[base]);
  assert(!((unsigned long)insn & (unsigned long)3));

  if (op == loadConstOp) {
    // lda dest, src1 (LITERAL)     --> put the literal value of src1 into dest
    // it may take several instructions to do this
    assert(dest < 32);
    int remainder;
//    unsigned words = 0;
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
    base += words * 4; return 0;

  } else if (op ==  loadOp) {
    // ld? dest, [src1]             --> src1 is a literal
    // src1 = address to load
    // src2 = 
    // dest = register to load
    int remainder;
//    unsigned words = generate_address(insn, (unsigned) dest, src1, remainder);
    unsigned long words = generate_address(insn, (unsigned long) dest, src1, remainder);
//    words += generate_load(insn+words, (unsigned) dest, (unsigned) dest,
    words += generate_load(insn+words, (unsigned long) dest, (unsigned long) dest,
			   remainder, dw_long);
    base += words * 4; return 0;

  } else if (op ==  storeOp) {
    // st? dest, [src1]
    // src1 = value to store
    // src2 = register to hold address
    // dest = address
    int remainder;
//    unsigned words = generate_address(insn, (unsigned) src2, dest, remainder);
    unsigned long words = generate_address(insn, (unsigned long) src2, dest, remainder);
//    words += generate_store(insn+words, (unsigned) src1, (unsigned) src2,
    words += generate_store(insn+words, (unsigned long) src1, (unsigned long) src2,
			    remainder, dw_long);
    base += words * 4; return 0;

  } else if (op ==  ifOp) {
    // beq src1
    // return address used to calculate branch offset
    assert((src1 < 32) && (src1 >= 0));
    // branch calculations use the udpated pc
//    unsigned words = generate_branch(insn, src1, dest-4, OP_BEQ);
    unsigned long words = generate_branch(insn, src1, dest-4, OP_BEQ);
    // return offset of branch instruction
    base += words * 4; return (base-4);

  } else if (op ==  trampPreamble) {
    return generate_tramp_preamble(insn, src1, dest, base);

  } else if (op ==  trampTrailer) {
    // dest is in words of offset and generateBranchInsn is bytes offset
    assert(!dest);
//    unsigned words = generate_branch(insn, REG_ZERO, dest << 2, OP_BR);
    unsigned long words = generate_branch(insn, REG_ZERO, dest << 2, OP_BR);
    // return the offset of the branch instruction -- not the updated pc
    base += words * 4; return (base - 4);

  } else if (op == noOp) {
//    unsigned words = generate_nop(insn);
    unsigned long words = generate_nop(insn);
    base += words * 4; return 0;

  } else if (op == updateCostOp)
    {
      return 0;
    }
  else if (op == getRetValOp)
    {
      // Return value register v0 is the 9th register saved in the mini tramp
      restoreRegister(insn,base,9,dest);
      return(dest);
    }
  else if (op == getParamOp)
    {
      if (src1 >5) assert(0);
      restoreRegister(insn,base,src1,dest);
      return(dest);
    }
  else if (op == branchOp) {
    generateBranchInsn(insn, dest-4);
    base += sizeof(instruction);
    return(base - sizeof(instruction));
  }
  else {
    return generate_integer_op(insn, src1, src2, dest, base, op);
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

bool registerSpace::readOnlyRegister(reg reg_number) {
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
int getPointCost(process *proc, instPoint *point)
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
				 instPoint *&location,returnInstance *&retInstance,bool noCost)
{
  trampTemplate *ret;
  process *globalProc;
  retInstance = NULL;
  Address baseAddr;

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
	baseAddr = inferiorMalloc(globalProc, baseTemplate.size, textHeap);
	installBaseTramp(baseAddr, location, globalProc);
	instruction *insn = new instruction;
	generateBranchInsn(insn, baseAddr - (location->addr+4));
	globalProc->baseMap[location] = baseAddr;
	retInstance = new returnInstance((instruction *)insn,sizeof(instruction), location->addr, sizeof(instruction)); 
    } else {
        baseAddr = globalProc->baseMap[location];
    }
      
    ret = new trampTemplate;
    *ret = baseTemplate;
    ret->baseAddr = baseAddr;
    return(ret);
}

/*
 * Install a single tramp.
 *
 */
void installTramp(instInstance *inst, char *code, int codeSize)
{
    totalMiniTramps++;
    insnGenerated += codeSize/sizeof(int);

    // TODO cast
    (inst->proc)->writeDataSpace((caddr_t)inst->trampBase, codeSize, code);
}


float getPointFrequency(instPoint *point)
{

    pd_Function *func;

    if (point->callee)
        func = point->callee;
    else
        func = point->func;

    if (!funcFrequencyTable.defines(func->prettyName())) {
      if (func->isLibTag()) {
	return(100);
      } else {
	return(250);
      }
    } else {
      return (funcFrequencyTable[func->prettyName()]);
    }
}

// Borrowed from the POWER version
bool isReturnInsn(const image *owner, Address adr, bool &lastOne)
{
  instruction insn;

  insn.raw = owner->get_instruction(adr);
  // need to know if this is really the last return.
  //    for now assume one return per function - jkh 4/12/96
  lastOne = true;
  return ((insn.mem_jmp.opcode == OP_MEM_BRANCH) &&
          (insn.mem_jmp.ext == MD_RET));
}

bool isCallInsn(const instruction i) {
  return (isBsr(i) || isJsr(i));
}

bool pd_Function::findInstPoints(const image *owner) 
{  
  Address adr = addr();
  instruction instr;
  bool err;

  adr += 8; // On the alphas the actual entry point is two instructions below the advertised entry point

  instr.raw = owner->get_instruction(adr);
  if (!IS_VALID_INSN(instr)) {
    return false;
  }

  funcEntry_ = new instPoint(this, instr, owner, adr, true,functionEntry);
  assert(funcEntry_);

  while (true) {
    instr.raw = owner->get_instruction(adr);

    bool done;

    // check for return insn and as a side affect decide if we are at the
    //   end of the function.
    if (isReturnInsn(owner, adr, done)) {
      // define the return point
      funcReturns += new instPoint(this, instr, owner, adr, false,functionExit);

      // see if this return is the last one 
      if (done) return;
    } else if (isCallInsn(instr)) {
      // define a call point
      // this may update address - sparc - aggregate return value
      // want to skip instructions

      adr = newCallPoint(adr, instr, owner, err);
      if (err)
	return false;
    }

    // now do the next instruction
    adr += 4;

   }
}

//
// Each processor may have a different heap layout.
//   So we make this processor specific.
//
// find all DYNINST symbols that are data symbols
bool process::heapIsOk(const vector<sym_data> &find_us) {
  Address instHeapStart, curr;
  Symbol sym;
  string str;

  // find the main function
  // first look for main or _main
  if (!((mainFunction = findOneFunction("main")) 
        || (mainFunction = findOneFunction("_main")))) {
     string msg = "Cannot find main. Exiting.";
     statusLine(msg.string_of());
     showErrorCallback(50, msg);
     return false;
  }
  
//  for (unsigned i=0; i<find_us.size(); i++) {
  for (unsigned long i=0; i<find_us.size(); i++) {
    str = find_us[i].name;
    if (!symbols->symbol_info(str, sym)) {
      string str1 = string("_") + str.string_of();
      if (!symbols->symbol_info(str1, sym) && find_us[i].must_find) {
	string msg;
        msg = string("Cannot find ") + str + string(". Exiting");
	statusLine(msg.string_of());
	showErrorCallback(50, msg);
	return false;
      }
    }
    //addInternalSymbol(str, sym.addr());
  }

  string ghb = "_DYNINSTtext";
  if (!symbols->symbol_info(ghb, sym)) {
      string msg;
      msg = string("Cannot find ") + ghb + string(". Exiting");
      statusLine(msg.string_of());
      showErrorCallback(50, msg);
      return false;
  }
  instHeapStart = sym.addr();
  //addInternalSymbol(ghb, instHeapStart);

  // check that we can get to our heap.
  if (instHeapStart > getMaxBranch()+symbols->codeOffset+SYN_INST_BUF_SIZE) {
    logLine("*** FATAL ERROR: Program text + data too big for dyninst\n");
    sprintf(errorLine, "    heap starts at %x\n", instHeapStart);
    logLine(errorLine);
    sprintf(errorLine, "    max reachable at %x\n", 
	getMaxBranch());
    logLine(errorLine);
    showErrorCallback(53,(const char *) errorLine);
    return false;
  } 
/*
  else if (instHeapStart + SYN_INST_BUF_SIZE > 
	     getMaxBranch()) {
    logLine("WARNING: Program text + data could be too big for dyninst\n");
    showErrorCallback(54,(const char *) errorLine);
    return false;
  }
*/

  string hd = "DYNINSTdata";
  if (!symbols->symbol_info(hd, sym)) {
      string msg;
      msg = string("Cannot find ") + hd + string(". Exiting");
      statusLine(msg.string_of());
      showErrorCallback(50, msg);
      return false;
  }
  instHeapStart = sym.addr();
  //addInternalSymbol(hd, instHeapStart);

  return true;
}

Address emitImm(opCode op, reg src1, reg src2, reg dest, char *i, 
                 Address &base, bool noCost)
{
  instruction *insn = (instruction *) ((void*)&i[base]);
  assert(!((unsigned long)insn & (unsigned long)3));
  return generate_integer_op(insn, src1, src2, dest, base, op,true);
}

unsigned long
emitFuncCall(opCode op, 
	     registerSpace *rs,
	     char *i, unsigned long &base, 
	     const vector<AstNode *> &operands,
	     const string &func, process *proc,bool noCost)
{
  // put parameters in argument registers
  // register move is "bis src, src, dest"  (bis is logical or)
  // save and restore the values currently in the argument registers
  //  unsigned long words = 0;
  unsigned long addr;
  bool err;
  vector <reg> srcs;
  void cleanUpAndExit(int status);
  addr = proc->findInternalAddress(func, false, err);
  if (err) {
       function_base *func_b = proc->findOneFunction(func);
       if (!func_b) {
          ostrstream os(errorLine, 1024, ios::out);
          os << "Internal error: unable to find addr of " << func << endl;
	  logLine(errorLine);
	  showErrorCallback(80, (const char *) errorLine);
	  P_abort();
       }
       addr = func_b->addr();
  }

  function_base *fun_save = proc->findOneFunction("DYNINSTsave_misc");
  function_base *fun_restore = proc->findOneFunction("DYNINSTrestore_misc");
  assert(fun_save && fun_restore);
  Address dyn_save = fun_save->addr();
  Address dyn_restore = fun_restore->addr();

  // save the current values in v0, a0..a5, pv, at, gp
  // Call to DYNINSTsave_misc
  int remainder;
  instruction *insn = (instruction *) ((void*)&i[base]);
  base += (4* generate_address(insn, REG_T10, dyn_save, remainder));
  if (remainder)
    {
      insn = (instruction *) ((void*)&i[base]);
      base += (4 * generate_lda(insn, REG_T10, REG_T10, remainder, true));
    }
  insn = (instruction *) ((void*)&i[base]);
  base += (4 * generate_jump(insn, REG_T10, MD_JSR, REG_RA, remainder));

    //    srcs += operands[u].generateCode(proc, rs, (char *)(insn + words), base);

  for (unsigned u = 0; u < operands.size(); u++)
    srcs += operands[u]->generateCode(proc, rs, i , base, false, false);

  //  words += (base - temp_base) / sizeof(insn);
  //  base = temp_base;

  for (unsigned u=0; u<srcs.size(); u++){
    if (u >= 5) {
	 string msg = "Too many arguments to function call in instrumentation code: only 5 arguments can be passed on the sparc architecture.\n";
	 fprintf(stderr, msg.string_of());
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
  
  // Call to DYNINSTrestore_misc
  insn = (instruction *) ((void*)&i[base]);
  base += (4 * generate_address(insn, REG_T10, dyn_restore, remainder));
  if (remainder)
    {
      insn = (instruction *) ((void*)&i[base]);
      base += (4 * generate_lda(insn, REG_T10, REG_T10, remainder, true));
      
    }
  insn = (instruction *) ((void*)&i[base]);
  base += ( 4 * generate_jump(insn, REG_T10, MD_JSR, REG_RA, remainder));
  return 0;
}

bool returnInstance::checkReturnInstance(const vector<Address> &adr, u_int &index) {
    return true;
}
 
void returnInstance::installReturnInstance(process *proc) {
    proc->writeTextSpace((caddr_t)addr_, instSeqSize, (caddr_t) instructionSeq); 
}

void returnInstance::addToReturnWaitingList(Address pc,process *proc) {
    P_abort();
}

void generateBreakPoint(instruction &insn){
  insn.raw = BREAK_POINT_INSN;
}

void generateIllegalInsn(instruction &insn) { // instP.h
   insn.raw = 0;
}

// findCallee: returns false unless callee is already set in instPoint
// dynamic linking not implemented on this platform
bool process::findCallee(instPoint &instr, function_base *&target){

    if((target = (function_base *)instr.iPgetCallee())) {
       return true;
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
      generateBSR(&newInsn,
			   newFunc->addr()+sizeof(instruction)-point->addr);
    }
    
    writeTextSpace((caddr_t)point->addr, sizeof(instruction), &newInsn);
    
    return true;
}


#ifdef BPATCH_LIBRARY
/*
   terminate execution of a process
 */
bool process::terminateProc_()
{
    int sig = SIGKILL;
    if (ioctl(proc_fd, PIOCKILL, &sig) == -1)
	return false;
    else
	return true;
}
#endif
