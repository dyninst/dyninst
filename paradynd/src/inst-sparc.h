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

#if !defined(sparc_sun_sunos4_1_3) && !defined(sparc_sun_solaris2_4) && !defined(sparc_tmc_cmost7_3)
#error "invalid architecture-os inclusion"
#endif

#ifndef INST_SPARC_H
#define INST_SPARC_H

#include "util/h/headers.h"
#include "rtinst/h/rtinst.h"
#include "paradynd/src/symtab.h"
#include "paradynd/src/process.h"
#include "paradynd/src/inst.h"
#include "paradynd/src/ast.h"
#include "paradynd/src/inst-sparc.h"
#include "paradynd/src/arch-sparc.h"
#include "paradynd/src/util.h"
#include "paradynd/src/internalMetrics.h"
#include "paradynd/src/stats.h"
#include "paradynd/src/os.h"
#include "paradynd/src/showerror.h"
#include "paradynd/src/as-sparc.h"
#include "paradynd/src/ast.h"
#include "paradynd/src/instP.h"


#define REG_L7          23        // register saved to keep the address of
                                  // the current vector of counter/timers
                                  // for each thread.
#define NUM_INSN_MT_PREAMBLE 9    // number of instructions required for
                                  // the MT preamble. 

// NOTE: LOW() and HIGH() can return ugly values if x is negative, because in
// that case, 2's complement has really changed the bitwise representation!
// Perhaps an assert should be done!
#define LOW10(x) ((x) & 0x3ff)
#define LOW13(x) ((x) & 0x1fff)
#define HIGH22(x) ((x) >> 10)
#define ABS(x)		((x) > 0 ? x : -x)
#define MAX_BRANCH	(0x1<<23)
#define MAX_IMM13       (4095)
#define MIN_IMM13       (-4096)


#define	REG_G5		5
#define	REG_G6		6
#define	REG_G7		7

#define REG_O7    	15

#define REG_L0          16
#define REG_L1          17
#define REG_L2          18

#define REG_SP          14
#define REG_FP          30

extern "C" void baseTramp();
extern trampTemplate baseTemplate;
extern registerSpace *regSpace;
extern int deadList[];
extern instruction newInstr[1024];

class instPoint {
public:
  instPoint(pdFunction *f, const instruction &instr, const image *owner,
	    Address &adr, const bool delayOK, bool isLeaf,
	    instPointType ipt);
  instPoint(pdFunction *f, const instruction &instr, const image *owner,
            Address &adr, const bool delayOK, instPointType ipt,   
	    Address &oldAddr);
  ~instPoint() {  /* TODO */ }

  // can't set this in the constructor because call points can't be classified
  // until all functions have been seen -- this might be cleaned up
  void set_callee(pdFunction *to) { callee = to; }


  Address addr;                       // address of inst point
  instruction originalInstruction;    // original instruction
  instruction delaySlotInsn;          // original instruction
  instruction aggregateInsn;          // aggregate insn
  instruction otherInstruction;       
  instruction isDelayedInsn;  
  instruction inDelaySlotInsn;
  instruction extraInsn;   	// if 1st instr is conditional branch this is
			        // previous instruction 

  bool inDelaySlot;             // Is the instruction in a delay slot
  bool isDelayed;		// is the instruction a delayed instruction
  bool callIndirect;		// is it a call whose target is rt computed ?
  bool callAggregate;		// calling a func that returns an aggregate
				// we need to reolcate three insns in this case
  pdFunction *callee;		// what function is called
  pdFunction *func;		// what function we are inst
  bool isBranchOut;             // true if this point is a conditional branch, 
				// that may branch out of the function
  int branchTarget;             // the original target of the branch
  bool leaf;                    // true if the procedure is a leaf
  instPointType ipType;
  int instId;                   // id of inst in this function
  int size;                     // size of multiple instruction sequences
  const image *image_ptr;	// for finding correct image in process
  bool firstIsConditional;      // 1st instruction is conditional branch
  bool relocated_;	        // true if instPoint is from a relocated func
};

inline unsigned getMaxBranch() { return MAX_BRANCH; }

inline void generateNOOP(instruction *insn)
{
    insn->raw = 0;
    insn->branch.op = 0;
    insn->branch.op2 = NOOPop2;

    // logLine("nop\n");
}

inline void generateBranchInsn(instruction *insn, int offset)
{
    if (ABS(offset) > MAX_BRANCH) {
	logLine("a Ranch too far\n");
	//showErrorCallback(52, "");
	//abort();
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
    if (ABS(offset) > MAX_BRANCH) {
	logLine("a branch too far\n");
	showErrorCallback(52, "");
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


inline void genSimpleInsn(instruction *insn, int op, reg rs1, reg rs2, reg rd)
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

inline void genImmInsn(instruction *insn, int op, reg rs1, int immd, reg rd)
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

inline void genImmRelOp(instruction *insn, int cond, reg rs1,
		        int immd, reg rd, unsigned &base)
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
    genSimpleInsn(insn, ORop3, 0, 0, rd); insn++;
    base += 4 * sizeof(instruction);
}

inline void genRelOp(instruction *insn, int cond, reg rs1,
		     reg rs2, reg rd, unsigned &base)
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
    genSimpleInsn(insn, ORop3, 0, 0, rd); insn++;
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

extern bool isPowerOf2(int value, int &result);
extern void generateNoOp(process *proc, int addr);
extern void changeBranch(process *proc, unsigned fromAddr, unsigned newAddr,
		  instruction originalBranch);
extern trampTemplate *findAndInstallBaseTramp(process *proc,
				 const instPoint *&location, 
				 returnInstance *&retInstance, bool noCost);

extern void  generateBranch(process *proc, unsigned fromAddr, unsigned newAddr);
extern void generateCall(process *proc, unsigned fromAddr,unsigned newAddr);
extern void genImm(process *proc, Address fromAddr,int op, reg rs1, 
		   int immd, reg rd);
extern int callsTrackedFuncP(instPoint *point);
extern pdFunction *getFunction(instPoint *point);
extern unsigned emitFuncCall(opCode op, registerSpace *rs, char *i, 
			     unsigned &base, const vector<AstNode *> &operands,
			     const string &callee, process *proc, bool noCost);

extern unsigned emitImm(opCode op, reg src1, reg src2, reg dest, char *i,
			unsigned &base, bool noCost);

extern int getInsnCost(opCode op);
extern bool isReturnInsn(const image *owner, Address adr, bool &lastOne);

#endif
