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

// $Id: inst-sparc.h,v 1.44 2000/08/01 22:39:52 tikir Exp $

#if !defined(sparc_sun_sunos4_1_3) && !defined(sparc_sun_solaris2_4)
#error "invalid architecture-os inclusion"
#endif

#ifndef INST_SPARC_H
#define INST_SPARC_H

#include "common/h/headers.h"
#ifndef BPATCH_LIBRARY
#include "rtinst/h/rtinst.h"
#endif
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/ast.h"
//#include "dyninstAPI/src/inst-sparc.h"
#include "dyninstAPI/src/arch-sparc.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/stats.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/as-sparc.h"
#include "dyninstAPI/src/instP.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

class NonRecursiveTrampTemplate : public trampTemplate
{

public:

  int guardOnPre_beginOffset;
  int guardOnPre_endOffset;

  int guardOffPre_beginOffset;
  int guardOffPre_endOffset;

  int guardOnPost_beginOffset;
  int guardOnPost_endOffset;

  int guardOffPost_beginOffset;
  int guardOffPost_endOffset;

};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#define INSN_SIZE ( sizeof( instruction ) )

#define REG_MT               23   /* register saved to keep the address of */
                                  /* the current vector of counter/timers  */
                                  /* for each thread.                      */
#define NUM_INSN_MT_PREAMBLE 27   /* number of instructions required for   */
                                  /* the MT preamble.                      */ 

#define RECURSIVE_GUARD_ON_CODE_SIZE   7
#define RECURSIVE_GUARD_OFF_CODE_SIZE  3

// NOTE: LOW() and HIGH() can return ugly values if x is negative, because in
// that case, 2's complement has really changed the bitwise representation!
// Perhaps an assert should be done!
#define LOW10(x) ((x) & 0x3ff)
#define LOW13(x) ((x) & 0x1fff)
#define HIGH22(x) ((x) >> 10)

inline Address ABS(int x) {
   if (x < 0) return -x;
   return x;
}
//#define ABS(x)		((x) > 0 ? x : -x)

//#define MAX_BRANCH	(0x1<<23)

#define MAX_IMM13       (4095)
#define MIN_IMM13       (-4096)


#define REG_G0          0
/* #ifdef REG_G5 */
/* #undef REG_G5 */
/* #endif */
/* #define	REG_G5		5 */
/* #ifdef REG_G6 */
/* #undef REG_G6 */
/* #endif */
/* #define	REG_G6		6 */
/* #ifdef REG_G7 */
/* #undef REG_G7 */
/* #endif */
/* #define	REG_G7		7 */

/* #ifdef REG_O7 */
/* #undef REG_O7 */
/* #endif */
/* #define REG_O7    	15 */
/* #define REG_I7    	31 */

#define REG_L0          16
#define REG_L1          17
#define REG_L2          18

#define REG_SPTR          14
#define REG_FPTR          30

// some macros for helping code which contains register symbolic names
#define REG_I(x) (x + 24)
#define REG_L(x) (x + 16) 
#define REG_O(x) (x + 8)
#define REG_G(x) (x)

extern "C" void baseTramp();
extern "C" void baseTramp_savePreInsn();
extern "C" void baseTramp_restorePreInsn();
extern "C" void baseTramp_savePostInsn();
extern "C" void baseTramp_restorePostInsn();
extern trampTemplate baseTemplate;
extern NonRecursiveTrampTemplate nonRecursiveBaseTemplate;
extern registerSpace *regSpace;
extern Register deadList[];

//additional declarations for the conservative base trampoline
#if defined(sparcv8plus)

extern "C" void conservativeBaseTramp();
extern "C" void conservativeBaseTramp_savePreInsn();
extern "C" void conservativeBaseTramp_restorePreInsn();
extern "C" void conservativeBaseTramp_savePostInsn();
extern "C" void conservativeBaseTramp_restorePostInsn();
extern trampTemplate conservativeBaseTemplate;
extern NonRecursiveTrampTemplate nonRecursiveConservativeBaseTemplate;

#endif

#define NEW_INSTR_ARRAY_LEN 8192
extern instruction newInstr[NEW_INSTR_ARRAY_LEN];

// amount of expansion for relocated functions....
#define RELOCATED_FUNC_EXTRA_SPACE 36

class instPoint;

inline unsigned getMaxBranch3Insn() {
   // The length we can branch using 3 instructions
   // isn't limited.
   unsigned result = 1;
   result <<= 31;
   return result;

   // probably returning ~0 would be better, since there's no limit
}

//inline unsigned getMaxBranch1Insn() {
//   // The length we can branch using just 1 instruction is dictated by the
//   // sparc instruction set.
//   return (0x1 << 23);
//}

inline bool offsetWithinRangeOfBranchInsn(int offset) {
   // The pc-relative offset range which we can branch to with a single sparc
   // branch instruction is dictated by the sparc instruction set.
   // There are 22 bits available...however, you really get 2 extra bits
   // because the CPU multiplies the 22-bit signed offset by 4.
   // The only downside is that the offset must be a multiple of 4, which we check.
   unsigned abs_offset = ABS(offset);
   assert(abs_offset % 4 == 0);

   // divide by 4.  After the divide, the result must fit in 22 bits.
   offset /= 4;

   const int INT22_MAX = 0x1FFFFF; // low 21 bits all 1's, the high bit (#22) is 0
   const int INT22_MIN = -(INT22_MAX+1); // in 2's comp, negative numbers get 1 extra value
   assert(INT22_MAX > 0);
   assert(INT22_MIN < 0);

   if (offset < INT22_MIN)
      return false;
   else if (offset > INT22_MAX)
      return false;
   else
      return true;
}

inline bool in1BranchInsnRange(Address adr1, Address adr2) 
{
    return (abs((RegValue)(adr1-adr2)) < (0x1 << 23));
}

inline void generateNOOP(instruction *insn)
{
    insn->raw = 0;
    insn->branch.op = 0;
    insn->branch.op2 = NOOPop2;

    // logLine("nop\n");
}

inline void generateTrapRegisterSpill(instruction *insn){
  insn->raw = SPILL_REGISTERS_INSN;
}

inline void generateFlushw(instruction *insn){
  insn->raw = 0;
  insn->rest.op = RESTop;
  insn->rest.op3 = FLUSHWop3;
  insn->rest.i = 0;
}

inline void generateBranchInsn(instruction *insn, int offset)
{
    if (!offsetWithinRangeOfBranchInsn(offset)) {
        char buffer[100];
	sprintf(buffer, "a Branch too far; offset=%d\n", offset);
	logLine(buffer);
	//showErrorCallback(52, buffer);
	assert(false && "a Branch too far");
	return;
    }

    insn->raw = 0;
    insn->branch.op = 0;
    insn->branch.cond = BAcond;
    insn->branch.op2 = BICCop2;
    insn->branch.anneal = true;
    insn->branch.disp22 = offset >> 2;

    // logLine("ba,a %x\n", offset);
}

inline void generateCallInsn(instruction *insn, int fromAddr, int toAddr)
{
    int offset = toAddr - fromAddr;
    insn->call.op = 01;
    insn->call.disp30 = offset >> 2;
}

inline void generateJmplInsn(instruction *insn, int rs1, int offset, int rd)
{
    insn->resti.op = 10;
    insn->resti.rd = rd;
    insn->resti.op3 = JMPLop3;
    insn->resti.rs1 = rs1;
    insn->resti.i = 1;
    assert(offset >= MIN_IMM13 && offset <= MAX_IMM13);
    insn->resti.simm13 = offset;
}    

inline void genBranch(instruction *insn, int offset, unsigned condition, bool annul)
{
//    if (ABS(offset) > getMaxBranch1Insn()) {
    if (!offsetWithinRangeOfBranchInsn(offset)) {
        char buffer[80];
	sprintf(buffer, "a branch too far, offset=%d\n", offset);
	logLine(buffer);
	showErrorCallback(52, buffer);
	abort();
    }

    insn->raw = 0;
    insn->branch.op = 0;
    insn->branch.cond = condition;
    insn->branch.op2 = BICCop2;
    insn->branch.anneal = annul;
    insn->branch.disp22 = offset >> 2;
}

inline void generateAnnulledBranchInsn(instruction *insn, int offset)
{
    genBranch(insn, offset, BAcond, true);
}


inline void genSimpleInsn(instruction *insn, int op,
        Register rs1, Register rs2, Register rd)
{
    insn->raw = 0;
    insn->rest.op = RESTop;
    insn->rest.rd = rd;
    insn->rest.op3 = op;
    insn->rest.rs1 = rs1;
    insn->rest.rs2 = rs2;
}

inline void genBreakpointTrap(instruction *insn) {
   insn->raw = BREAK_POINT_INSN; // ta (trap always) 1
}

inline void genUnimplementedInsn(instruction *insn) {
   insn->raw = 0; // UNIMP 0
}

inline void genImmInsn(instruction *insn, int op,
        Register rs1, int immd, Register rd)
{
    insn->raw = 0;
    insn->resti.op = RESTop;
    insn->resti.rd = rd;
    insn->resti.op3 = op;
    insn->resti.rs1 = rs1;
    insn->resti.i = 1;
    assert(immd >= MIN_IMM13 && immd <= MAX_IMM13);
    insn->resti.simm13 = immd;
}

inline void genImmRelOp(instruction *insn, int cond, Register rs1,
		        int immd, Register rd, Address &base)
{
    // cmp rs1, rs2
    genImmInsn(insn, SUBop3cc, rs1, immd, 0); insn++;
    // mov 1, rd
    genImmInsn(insn, ORop3, 0, 1, rd); insn++;

    // b??,a +2
    insn->branch.op = 0;
    insn->branch.cond = cond;
    insn->branch.op2 = BICCop2;
    insn->branch.anneal = true;
    insn->branch.disp22 = 2;
    insn++;

    // clr rd
    genImmInsn(insn, ORop3, 0, 0, rd); insn++; 
    //genSimpleInsn(insn, ORop3, 0, 0, rd); insn++;
    base += 4 * sizeof(instruction);
}

inline void genRelOp(instruction *insn, int cond, Register rs1,
		     Register rs2, Register rd, Address &base)
{
    // cmp rs1, rs2
    genSimpleInsn(insn, SUBop3cc, rs1, rs2, 0); insn++;
    // mov 1, rd
    genImmInsn(insn, ORop3, 0, 1, rd); insn++;

    // b??,a +2
    insn->branch.op = 0;
    insn->branch.cond = cond;
    insn->branch.op2 = BICCop2;
    insn->branch.anneal = true;
    insn->branch.disp22 = 2;
    insn++;

    // clr rd
    genImmInsn(insn, ORop3, 0, 0, rd); insn++;
    //genSimpleInsn(insn, ORop3, 0, 0, rd); insn++;
    base += 4 * sizeof(instruction);
}

inline void generateSetHi(instruction *insn, int src1, int dest)
{
    insn->raw = 0;
    insn->sethi.op = FMT2op;
    insn->sethi.rd = dest;
    insn->sethi.op2 = SETHIop2;
    insn->sethi.imm22 = HIGH22(src1);
}

// st rd, [rs1 + offset]
inline void generateStore(instruction *insn, int rd, int rs1, int offset)
{
    insn->resti.op = STop;
    insn->resti.rd = rd;
    insn->resti.op3 = STop3;
    insn->resti.rs1 = rs1;
    insn->resti.i = 1;
    assert(offset >= MIN_IMM13 && offset <= MAX_IMM13);
    insn->resti.simm13 = offset;
}

// sll rs1,rs2,rd
inline void generateLShift(instruction *insn, int rs1, int offset, int rd)
{
    insn->restix.op = SLLop;
    insn->restix.op3 = SLLop3;
    insn->restix.rd = rd;
    insn->restix.rs1 = rs1;
    insn->restix.i = 1;
    insn->restix.x = 0;
    insn->restix.rs2 = offset;
}

// sll rs1,rs2,rd
inline void generateRShift(instruction *insn, int rs1, int offset, int rd)
{
    insn->restix.op = SRLop;
    insn->restix.op3 = SRLop3;
    insn->restix.rd = rd;
    insn->restix.rs1 = rs1;
    insn->restix.i = 1;
    insn->restix.x = 0;
    insn->restix.rs2 = offset;
}

// load [rs1 + offset], rd
inline void generateLoad(instruction *insn, int rs1, int offset, int rd)
{
    insn->resti.op = LOADop;
    insn->resti.op3 = LDop3;
    insn->resti.rd = rd;
    insn->resti.rs1 = rs1;
    insn->resti.i = 1;
    assert(offset >= MIN_IMM13 && offset <= MAX_IMM13);
    insn->resti.simm13 = offset;
}

// swap [rs1 + offset], rd
inline void generateSwap(instruction *insn, int rs1, int offset, int rd)
{
    insn->resti.op = SWAPop;
    insn->resti.rd = rd;
    insn->resti.op3 = SWAPop3;
    insn->resti.rs1 = rs1;
    insn->resti.i = 0;
    assert(offset >= MIN_IMM13 && offset <= MAX_IMM13);
    insn->resti.simm13 = offset;
}    

// std rd, [rs1 + offset]
inline void genStoreD(instruction *insn, int rd, int rs1, int offset)
{
    insn->resti.op = STop;
    insn->resti.rd = rd;
    insn->resti.op3 = STDop3;
    insn->resti.rs1 = rs1;
    insn->resti.i = 1;
    assert(offset >= MIN_IMM13 && offset <= MAX_IMM13);
    insn->resti.simm13 = offset;
}

// ldd [rs1 + offset], rd
inline void genLoadD(instruction *insn, int rs1, int offset, int rd)
{
    insn->resti.op = LOADop;
    insn->resti.op3 = LDDop3;
    insn->resti.rd = rd;
    insn->resti.rs1 = rs1;
    insn->resti.i = 1;
    assert(offset >= MIN_IMM13 && offset <= MAX_IMM13);
    insn->resti.simm13 = offset;
}

bool processOptimaRet(instPoint *location, AstNode *&ast);

extern bool isPowerOf2(int value, int &result);
extern void generateNoOp(process *proc, Address addr);
extern void changeBranch(process *proc, Address fromAddr, Address newAddr,
		  instruction originalBranch);
extern trampTemplate *findAndInstallBaseTramp(process *proc,
				 instPoint *&location, 
				 returnInstance *&retInstance,
				 bool trampRecursiveDesired,
				 bool noCost);

extern void generateBranch(process *proc, Address fromAddr, Address newAddr);
extern void generateCall(process *proc, Address fromAddr, Address newAddr);
extern void genImm(process *proc, Address fromAddr, int op, Register rs1, 
		   int immd, Register rd);
extern int callsTrackedFuncP(instPoint *point);
extern pd_Function *getFunction(instPoint *point);
extern Register emitFuncCall(opCode op, registerSpace *rs, char *i, 
			     Address &base, const vector<AstNode *> &operands,
			     const string &callee, process *proc, bool noCost,
			     const function_base *calleefunc);

extern void emitImm(opCode op, Register src1, RegValue src2imm, Register dest,
                    char *i, Address &base, bool noCost);

extern Register emitOptReturn(instruction, Register, char *, Address &, bool);

extern int getInsnCost(opCode op);
extern bool isReturnInsn(const image *owner, Address adr, bool &lastOne, 
			 string name); 
extern bool isReturnInsn(instruction i, Address adr, string name);
extern bool isBranchInsn(instruction i);
extern bool branchInsideRange(instruction i, Address branchAddress, 
      Address firstAddress, Address lastAddress); 
extern bool trueCallInsideRange(instruction instr, Address callAddress, 
      Address firstAddress, Address lastAddress);
#if defined(SHM_SAMPLING) && defined(MT_THREAD)
extern void generateMTpreamble(char *insn, Address &base, process *proc);
#endif

#endif
