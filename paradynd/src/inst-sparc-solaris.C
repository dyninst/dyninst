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

/*
 * inst-sparc.C - Identify instrumentation points for a SPARC processors.
 */

#include "util/h/headers.h"
#include "rtinst/h/rtinst.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "inst-sparc.h"
#include "arch-sparc.h"
#include "ast.h"
#include "util.h"
#include "internalMetrics.h"
#include "stats.h"
#include "os.h"
#include "showerror.h"

// NOTE: LOW() and HIGH() can return ugly values if x is negative, because in
// that case, 2's complement has really changed the bitwise representation!
// Perhaps an assert should be done!
#define LOW10(x) ((x) & 0x3ff)
#define LOW13(x) ((x) & 0x1fff)
#define HIGH22(x) ((x) >> 10)

extern bool isPowerOf2(int value, int &result);

class instPoint {
public:
  instPoint(pdFunction *f, const instruction &instr, const image *owner,
	    Address &adr, const bool delayOK, bool isLeaf,
	    instPointType ipt);
  instPoint(pdFunction *f, const instruction &instr, const image *owner,
            Address &adr, const bool delayOK, instPointType ipt,   
	    Address &oldAddr);
  ~instPoint() {  /* TODO */ }

  // can't set this in the constructor because call points can't be classified until
  // all functions have been seen -- this might be cleaned up
  void set_callee(pdFunction *to) { callee = to; }


  Address addr;                       /* address of inst point */
  instruction originalInstruction;    /* original instruction */
  instruction delaySlotInsn;          /* original instruction */
  instruction aggregateInsn;          /* aggregate insn */
  instruction otherInstruction;       
  instruction isDelayedInsn;  
  instruction inDelaySlotInsn;
  instruction extraInsn;   /* if 1st instr is conditional branch this is
			      previous instruction */

  bool inDelaySlot;            /* Is the instruction in a delay slot */
  bool isDelayed;		/* is the instruction a delayed instruction */
  bool callIndirect;		/* is it a call whose target is rt computed ? */
  bool callAggregate;		/* calling a func that returns an aggregate
				   we need to reolcate three insns in this case
				   */
  pdFunction *callee;		/* what function is called */
  pdFunction *func;		/* what function we are inst */

  bool isBranchOut;                /* true if this point is a conditional branch, 
				   that may branch out of the function */
  int branchTarget;                /* the original target of the branch */
  bool leaf;                       /* true if the procedure is a leaf     */
  instPointType ipType;
  int instId;                      /* id of inst in this function */
  int size;                        /* size of multiple instruction sequences */
  const image *image_ptr;	// for finding correct image in process
  bool firstIsConditional;     /* 1st instruction is conditional branch */
  bool relocated_;	// true if this instPoint is from a relocated func
};



#define ABS(x)		((x) > 0 ? x : -x)
#define MAX_BRANCH	(0x1<<23)
//#define MAX_IMM		0x1<<12		/* 11 plus shign == 12 bits */
#define MAX_IMM13       (4095)
#define MIN_IMM13       (-4096)

unsigned getMaxBranch() {
  return MAX_BRANCH;
}

#define	REG_G5		5
#define	REG_G6		6
#define	REG_G7		7

#define REG_O7    	15

#define REG_L0          16
#define REG_L1          17
#define REG_L2          18

#define REG_SP          14
#define REG_FP          30

const char *registerNames[] = { "g0", "g1", "g2", "g3", "g4", "g5", "g6", "g7",
				"o0", "o1", "o2", "o3", "o4", "o5", "sp", "o7",
				"l0", "l1", "l2", "l3", "l4", "l5", "l6", "l7",
				"i0", "i1", "i2", "i3", "i4", "i5", "i6", "i7" };

dictionary_hash<string, unsigned> funcFrequencyTable(string::hash);

string getStrOp(int op)
{
    switch (op) {
	case SetCC: 	return("set");
	case ADDop3:	return("add");
	case ANDop3:	return("and");
	case ORop3:	return("or");
	case SUBop3:	return("sub");
	case SUBop3cc:	return("subcc");
	case SMULop3:	return("smul");
	case SDIVop3:	return("sdiv");
	case XNORop3:	return("xnor");
	case SAVEop3:	return("save");
	case RESTOREop3:	return("restore");
	case JMPLop3:	return("jmpl");
	default: 	return("???");
    }
}

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

inline void genSimpleInsn(instruction *insn, int op, reg rs1, reg rs2, reg rd)
{
    insn->raw = 0;
    insn->rest.op = RESTop;
    insn->rest.rd = rd;
    insn->rest.op3 = op;
    insn->rest.rs1 = rs1;
    insn->rest.rs2 = rs2;

    // logLine("%s %%%s,%%%s,%%%s\n", getStrOp(op), registerNames[rs1], 
    // 	registerNames[rs2], registerNames[rd]);
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

    // logLine("%s %%%s,%d,%%%s\n", getStrOp(op), registerNames[rs1], immd,
    // 	registerNames[rd]);
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

    // logLine("sethi  %%hi(0x%x), %%%s\n", HIGH(src1)*1024, 
    // 	registerNames[dest]);
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


// Constructor for the class instPoint. This one defines the 
// instPoints for the relocated function. Since the function reloated
// to the heap don't need to worry about that jump could be out of
// reach, the instructions to be moved are the one instruction at that
// point plus others if necessary(i.e. instruction in the delayed
// slot and maybe the aggregate instuction ).    
instPoint::instPoint(pdFunction *f, const instruction &instr, 
		     const image *owner, Address &adr, bool delayOK, 
		     instPointType pointType, Address &oldAddr)
: addr(adr), originalInstruction(instr), inDelaySlot(false), isDelayed(false),
  callIndirect(false), callAggregate(false), callee(NULL), func(f), 
  ipType(pointType), image_ptr(owner), firstIsConditional(false), 
  relocated_(false)
{
  assert(f->isTrapFunc() == true);  

  isBranchOut = false;
  delaySlotInsn.raw = owner->get_instruction(oldAddr+4);
  aggregateInsn.raw = owner->get_instruction(oldAddr+8);

  // If the instruction is a DCTI, the instruction in the delayed 
  // slot is to be moved.
  if (IS_DELAYED_INST(instr))
    isDelayed = true;

  // If this function call another function which return an aggregate
  // value, move the aggregate instruction, too. 
  if (ipType == callSite) {
      if (!IS_VALID_INSN(aggregateInsn) && aggregateInsn.raw != 0) {
	  callAggregate = true;
	  adr += 8;
	  oldAddr += 8;
      }
  }

  if (owner->isValidAddress(oldAddr-4)) {
    instruction iplus1;
    iplus1.raw = owner->get_instruction(oldAddr-4);
    if (IS_DELAYED_INST(iplus1) && !delayOK) {
      // ostrstream os(errorLine, 1024, ios::out);
      // os << "** inst point " << func->file->fullName << "/"
      //  << func->prettyName() << " at addr " << addr <<
      //        " in a delay slot\n";
      // logLine(errorLine);
      inDelaySlot = true;
    }
  }
}

// Another constructor for the class instPoint. This one is called
// for the define the instPoints for regular functions which means
// multiple instructions is going to be moved to based trampoline.
// Since we will use the instruction CALL to branch to the base
// tramp(so it doesn't have any code size restriction), things are
// a little more complicated because instruction CALL changes the 
// value in the link register.
instPoint::instPoint(pdFunction *f, const instruction &instr, 
		     const image *owner, Address &adr,
		     bool delayOK, bool isLeaf, instPointType pointType)
: addr(adr), originalInstruction(instr), inDelaySlot(false), isDelayed(false),
  callIndirect(false), callAggregate(false), callee(NULL), func(f),
  leaf(isLeaf), ipType(pointType), image_ptr(owner), firstIsConditional(false),
  relocated_(false)
{

  isBranchOut = false;
  size = 0;

  // When the function is not a leaf function 
  if (!leaf) {

      // we will treat the first instruction after the SAVE instruction
      // in the nonleaf procedure as the function entry.  
      if (ipType == functionEntry) {

	  assert(isInsnType(instr, SAVEmask, SAVEmatch));
	  addr += 4;
	  originalInstruction.raw = owner->get_instruction(addr);
	  delaySlotInsn.raw = owner->get_instruction(addr+4);
	  size += 2*sizeof(instruction);

	  // If the second instruction is DCTI, we need to move the
	  // the instruction in the delayed slot.
	  if (IS_DELAYED_INST(delaySlotInsn)) {
	      isDelayed = true; 
	      isDelayedInsn.raw = owner->get_instruction(addr+8);
	      size += 1*sizeof(instruction);

	      // Life is hard. If the second instruction is actually
	      // an CALL instruction, we need to move the instruction
	      // after the instruction in the delayed slot if the 
	      // return value of this function is a aggregate value.
	      aggregateInsn.raw = owner->get_instruction(addr+12);
	      if (isCallInsn(delaySlotInsn)) {
		  if (!IS_VALID_INSN(aggregateInsn) && aggregateInsn.raw != 0) {
		      callAggregate = true;
		      size += 1*sizeof(instruction);
		  }
	      }
	  }

      // The following are easier.	  
      } else if (ipType == callSite) {
	  delaySlotInsn.raw = owner->get_instruction(addr+4);
	  size += 2*sizeof(instruction);

	  aggregateInsn.raw = owner->get_instruction(addr+8);
	  if (!IS_VALID_INSN(aggregateInsn) && aggregateInsn.raw != 0) {
	      callAggregate = true;
	      size += 1*sizeof(instruction);
	  }
      } else {
	  delaySlotInsn.raw = owner->get_instruction(addr+4);
	  size += 2*sizeof(instruction);
      }
  }

  // When the function is a leaf function
  else {

      // For the leaf procedure, there are no function calls in
      // this procdure. So we don't need to consider the 
      // aggregate instuction.
      if (ipType == functionEntry) {

	  otherInstruction.raw = owner->get_instruction(addr+4);
	  delaySlotInsn.raw = owner->get_instruction(addr+8);
	  size += 2*sizeof(instruction);

	  if (IS_DELAYED_INST(delaySlotInsn)) {
	      isDelayed = true;
	      isDelayedInsn.raw = owner->get_instruction(addr+12);
	      size += 2*sizeof(instruction);
	  }

      } else if (ipType == functionExit) {
	  
	  addr -= 4;

	  if (owner->isValidAddress(addr-4)) {
	      instruction iplus1;
	      iplus1.raw = owner->get_instruction(addr-4);
	      if (IS_DELAYED_INST(iplus1) && !delayOK) {
		  addr -= 4;
		  inDelaySlot = true;
		  size += 1*sizeof(instruction);
		  if(isCondBranch(iplus1)){
		      instruction previous_inst; 
		      previous_inst.raw = owner->get_instruction(addr-4);
                      firstIsConditional = true;
		      addr -= sizeof(instruction);
		      size += 1*sizeof(instruction);
		  }
	      }
	  }

	  originalInstruction.raw = owner->get_instruction(addr);
	  otherInstruction.raw = owner->get_instruction(addr+4);
	  delaySlotInsn.raw = owner->get_instruction(addr+8);
	  size += 3*sizeof(instruction);

	  if (inDelaySlot) {
	      inDelaySlotInsn.raw = owner->get_instruction(addr+12);
	      if(firstIsConditional) {
		  extraInsn.raw = owner->get_instruction(addr+16);
	      }
	  }

      } else {
	  // Of course, the leaf function could not have call sites. 
	  logLine("Internal Error: in inst-sparc.C.");
	  abort();
      }
  }

  // return the address in the code segment after this instruction
  // sequence. (there's a -1 here because one will be added up later in
  // the function findInstPoints)  
  adr = addr + (size - 1*sizeof(instruction));
}

// Determine if the called function is a "library" function or a "user" function
// This cannot be done until all of the functions have been seen, verified, and
// classified
//
void pdFunction::checkCallPoints() {
  instPoint *p;
  Address loc_addr;

  vector<instPoint*> non_lib;

  for (unsigned i=0; i<calls.size(); ++i) {
    /* check to see where we are calling */
    p = calls[i];
    assert(p);

    if (isInsnType(p->originalInstruction, CALLmask, CALLmatch)) {
      // Direct call
      loc_addr = p->addr + (p->originalInstruction.call.disp30 << 2);
      pdFunction *pdf = (file_->exec())->findFunction(loc_addr);
      if (pdf && !pdf->isLibTag()) {
	p->callee = pdf;
	non_lib += p;
      } else if(!pdf){
	   // if this is a call outside the fuction, keep it
	   if((loc_addr < getAddress(0))||(loc_addr > (getAddress(0)+size()))){
	        p->callIndirect = true;
                p->callee = NULL;
                non_lib += p;
	   }
	   else {
	       delete p;
	   }
      } else {
          delete p;
      }
    } else {
      // Indirect call -- be conservative, assume it is a call to 
      // an unnamed user function
      assert(!p->callee); assert(p->callIndirect);
      p->callee = NULL;
      non_lib += p;
    }
  }
  calls = non_lib;
}

// TODO we cannot find the called function by address at this point in time
// because the called function may not have been seen.
// reloc_info is 0 if the function is not currently being relocated
Address pdFunction::newCallPoint(Address &adr, const instruction instr,
				 const image *owner, bool &err, 
				 int &callId, Address &oldAddr,
				 relocatedFuncInfo *reloc_info,
				 const instPoint *&location)
{
    Address ret=adr;
    instPoint *point;

    err = true;
    if (isTrap) {
	point = new instPoint(this, instr, owner, adr, false, callSite, oldAddr);
    } else {
	point = new instPoint(this, instr, owner, adr, false, false, callSite);
    }

    if (!isInsnType(instr, CALLmask, CALLmatch)) {
      point->callIndirect = true;
      point->callee = NULL;
    } else{
      point->callIndirect = false;
    }

    if (isTrap) {
	if (!reloc_info) {
	    calls += point;
	    calls[callId] -> instId = callId++;
	} else {
	    // calls to a location within the function are not
	    // kept in the calls vector
	    assert(callId >= 0);
	    assert(((u_int)callId) < calls.size());
	    point->relocated_ = true;
	    // if the location was this call site, then change its value
	    if(location && (calls[callId] == location)) { 
		assert(calls[callId]->instId  == location->instId);
		location = point; 
	    } 
	    point->instId = callId++;
	    reloc_info->addFuncCall(point);
	}
    } else {
	if (!reloc_info) {
	    calls += point;
	}
	else {
	    point->relocated_ = true;
	    reloc_info->addFuncCall(point);
	}
    }
    err = false;
    return ret;
}

//
// initDefaultPointFrequencyTable - define the expected call frequency of
//    procedures.  Currently we just define several one shots with a
//    frequency of one, and provide a hook to read a file with more accurate
//    information.
//
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

/*
 * Get an etimate of the frequency for the passed instPoint.  
 *    This is not (always) the same as the function that contains the point.
 * 
 *  The function is selected as follows:
 *
 *  If the point is an entry or an exit return the function name.
 *  If the point is a call and the callee can be determined, return the called
 *     function.
 *  else return the funcation containing the point.
 *
 *  WARNING: This code contins arbitray values for func frequency (both user 
 *     and system).  This should be refined over time.
 *
 * Using 1000 calls sec to be one SD from the mean for most FPSPEC apps.
 *	-- jkh 6/24/94
 *
 */
float getPointFrequency(instPoint *point)
{

    pdFunction *func;

    if (point->callee)
        func = point->callee;
    else
        func = point->func;

    if (!funcFrequencyTable.defines(func->prettyName())) {
      if (func->isLibTag()) {
	return(100);
      } else {
        // Changing this value from 250 to 100 because predictedCost was
        // too high - naim 07/18/96
	return(100); 
      }
    } else {
      return (funcFrequencyTable[func->prettyName()]);
    }
}

//
// return cost in cycles of executing at this point.  This is the cost
//   of the base tramp if it is the first at this point or 0 otherwise.
//
int getPointCost(process *proc, const instPoint *point)
{
    if (proc->baseMap.defines(point)) {
        return(0);
    } else {
        // 70 cycles for base tramp (worst case)
        return(70);
    }
}

/*
 * Given and instruction, relocate it to a new address, patching up
 *   any relative addressing that is present.
 *
 */
void relocateInstruction(instruction *insn, u_int origAddr, u_int targetAddr,
			 process *proc)
{
    int newOffset;

    // If the instruction is a CALL instruction, calculate the new
    // offset
    if (isInsnType(*insn, CALLmask, CALLmatch)) {
	newOffset = origAddr  - targetAddr + (insn->call.disp30 << 2);
	insn->call.disp30 = newOffset >> 2;
    } else if (isInsnType(*insn, BRNCHmask, BRNCHmatch)||
	       isInsnType(*insn, FBRNCHmask, FBRNCHmatch)) {

	// If the instruction is a Branch instruction, calculate the 
        // new offset. If the new offset is out of reach after the 
        // instruction is moved to the base Trampoline, we would do
        // the following:
	//    b  address  ......    address: save
	//                                   call new_offset             
	//                                   restore 
	newOffset = origAddr - targetAddr + (insn->branch.disp22 << 2);

	// if the branch is too far, then allocate more space in inferior
	// heap for a call instruction to branch target.  The base tramp 
	// will branch to this new inferior heap code, which will call the
	// target of the branch
	if (ABS(newOffset) > MAX_BRANCH) {
	    int ret = inferiorMalloc(proc,3*sizeof(instruction), textHeap);
	    u_int old_offset = insn->branch.disp22 << 2;
	    insn->branch.disp22  = (ret - targetAddr)>>2;
	    instruction insnPlus[3];
	    genImmInsn(insnPlus, SAVEop3, REG_SP, -112, REG_SP);
	    generateCallInsn(insnPlus+1, ret+sizeof(instruction), 
			     origAddr+old_offset);
	    genSimpleInsn(insnPlus+2, RESTOREop3, 0, 0, 0); 
	    proc->writeDataSpace((caddr_t)ret, sizeof(insnPlus), 
			 (caddr_t) insnPlus);
	} else {
	    insn->branch.disp22 = newOffset >> 2;
	}
    } else if (isInsnType(*insn, TRAPmask, TRAPmatch)) {
	// There should be no probelm for moving trap instruction
	// logLine("attempt to relocate trap\n");
    } 
    /* The rest of the instructions should be fine as is */
}

trampTemplate baseTemplate;

extern "C" void baseTramp();

void initATramp(trampTemplate *thisTemp, instruction *tramp)
{
    instruction *temp;

    // TODO - are these offset always positive?
    thisTemp->trampTemp = (void *) tramp;
    for (temp = tramp; temp->raw != END_TRAMP; temp++) {
	switch (temp->raw) {
	    case LOCAL_PRE_BRANCH:
		thisTemp->localPreOffset = ((void*)temp - (void*)tramp);
		thisTemp->localPreReturnOffset = thisTemp->localPreOffset 
		                                 + sizeof(temp->raw);
		break;
	    case GLOBAL_PRE_BRANCH:
		thisTemp->globalPreOffset = ((void*)temp - (void*)tramp);
		break;
	    case LOCAL_POST_BRANCH:
		thisTemp->localPostOffset = ((void*)temp - (void*)tramp);
		thisTemp->localPostReturnOffset = thisTemp->localPostOffset
		                                  + sizeof(temp->raw);
		break;
	    case GLOBAL_POST_BRANCH:
		thisTemp->globalPostOffset = ((void*)temp - (void*)tramp);
		break;
	    case SKIP_PRE_INSN:
                thisTemp->skipPreInsOffset = ((void*)temp - (void*)tramp);
                break;
	    case UPDATE_COST_INSN:
		thisTemp->updateCostOffset = ((void*)temp - (void*)tramp);
		break;
	    case SKIP_POST_INSN:
                thisTemp->skipPostInsOffset = ((void*)temp - (void*)tramp);
                break;
	    case RETURN_INSN:
                thisTemp->returnInsOffset = ((void*)temp - (void*)tramp);
                break;
	    case EMULATE_INSN:
                thisTemp->emulateInsOffset = ((void*)temp - (void*)tramp);
                break;
  	}	
    }

    // Cost with the skip branchs.
    thisTemp->cost = 14;  
    thisTemp->prevBaseCost = 20;
    thisTemp->postBaseCost = 22;
    thisTemp->prevInstru = thisTemp->postInstru = false;
    thisTemp->size = (int) temp - (int) tramp;
}

registerSpace *regSpace;

int deadList[] = {16, 17, 18, 19, 20, 21, 22, 23 };

void initTramps()
{
    static bool inited=false;

    if (inited) return;
    inited = true;

    initATramp(&baseTemplate, (instruction *) baseTramp);

    regSpace = new registerSpace(sizeof(deadList)/sizeof(int), deadList,					 0, NULL);
    assert(regSpace);
}

void generateMTpreamble(char *insn, unsigned &base, process *proc)
{
  AstNode *t1,*t2,*t3;
  vector<AstNode *> dummy;
  unsigned tableAddr;
  int value; 
  bool err;
  reg src = -1;

  /* t3=DYNINSTthreadTable[thr_self()] */
  t1 = new AstNode(string("DYNINSTthreadPos"), dummy);
  value = sizeof(unsigned);
  t2 = new AstNode(timesOp, t1, new AstNode(AstNode::Constant,(void *)value));

  tableAddr = proc->findInternalAddress("DYNINSTthreadTable",true,err);
  assert(!err);
  t3 = new AstNode(plusOp, t2, new AstNode(AstNode::Constant, (void *)tableAddr));
  src = t3->generateCode(proc, regSpace, insn, base, false);
  removeAst(t3);
  (void) emit(orOp, src, 0, REG_L7, insn, base, false);
  regSpace->freeRegister(src);
}

/*
 * Install a base tramp -- fill calls with nop's for now.
 *
 * This one install the base tramp for the regular functions.
 *
 */
trampTemplate *installBaseTramp(const instPoint *&location, process *proc)
{
    unsigned baseAddr = inferiorMalloc(proc, baseTemplate.size, textHeap);

    instruction *code = new instruction[baseTemplate.size];
    assert(code);

    memcpy((char *) code, (char*) baseTemplate.trampTemp, baseTemplate.size);

    instruction *temp;
    unsigned currAddr;
    for (temp = code, currAddr = baseAddr; 
	(currAddr - baseAddr) < (unsigned) baseTemplate.size;
	temp++, currAddr += sizeof(instruction)) {

	if (temp->raw == EMULATE_INSN) {

	    // Load the value of link register from stack 
	    // If it is a leaf function, genereate a RESTORE instruction
            // since there's an instruction SAVE generated and put in the
            // code segment.
	    if (location -> leaf) {
		genImmInsn(temp, RESTOREop3, 0, 0, 0);
		temp++;
		currAddr += sizeof(instruction);
	    } 

	    // Same for the leaf and nonleaf functions.
            // First, relocate the "FIRST instruction" in the sequence;  
	    *temp = location->originalInstruction;
	    Address fromAddr = location->addr;

	    // compute the real from address if this instrumentation
	    // point is from a shared object image
	    Address baseAddress = 0;
	    if(proc->getBaseAddress(location->image_ptr,baseAddress)){
		fromAddr += baseAddress;		
            }
            // If the instruction is a call instruction to a location somewhere 
	    // within the function, then the 07 regester must be saved and 
	    // resored around the relocated call from the base tramp...the call
	    // instruction changes the value of 07 to be the PC value, and if
	    // we move the call instruction to the base tramp, its value will
	    // be incorrect when we use it in the function.  We generate the
	    // following base tramp code:
	    //		original delay slot instruction 
	    //		save
	    // 		original call instruction
	    //		restore
            // This case should only occur for function entry points in
	    // functions from shared objects, and there should be no append
	    // trampolene code because the relocated call instruction will
	    // not return to the base tramp
	    if (isInsnType(*temp, CALLmask, CALLmatch)) {
		Address offset = fromAddr + (temp->call.disp30 << 2);
		if ((offset >= (location->func->getAddress(0)+ baseAddress)) && 
		    (offset <= ((location->func->getAddress(0)+ baseAddress)+
				 location->func->size()))) {
		    // TODO: this assumes that the delay slot instruction is not
		    // a call instruction....is this okay?
		    *temp = location->delaySlotInsn;  
		    temp++; 
		    currAddr += sizeof(instruction);
		    genImmInsn(temp, SAVEop3, REG_SP, -112, REG_SP); 
		    temp++; 
		    currAddr += sizeof(instruction);  
		    *temp = location->originalInstruction;
		    relocateInstruction(temp,fromAddr,currAddr,(process *)proc);
		    temp++; 
		    fromAddr += sizeof(instruction); 
		    currAddr += sizeof(instruction);
		    genImmInsn(temp, RESTOREop3, 0, 0, 0);
		    continue;
		}
	    }   

	    relocateInstruction(temp,fromAddr,currAddr,(process *)proc);

	    // Again, for leaf function, one more is needed to move for one
	    // more spot;
	    if (location->leaf) {
		fromAddr += sizeof(instruction);
		currAddr += sizeof(instruction);
		*++temp = location->otherInstruction;
		relocateInstruction(temp, fromAddr, currAddr, 
				    (process *)proc);
	    }	  
	    
	    // Second, relocate the "NEXT instruction";
	    fromAddr += sizeof(instruction);
	    currAddr += sizeof(instruction);
	    *++temp = location->delaySlotInsn;
	    relocateInstruction(temp, fromAddr, currAddr,
				(process *)proc);
	    
	    // Third, if the "NEXT instruction" is a DCTI, 
	    if (location->isDelayed) {
		fromAddr += sizeof(instruction);
		currAddr += sizeof(instruction);
		*++temp = location->isDelayedInsn;
		relocateInstruction(temp, fromAddr, currAddr,
				    (process *)proc);
		
		// Then, possibly, there's an callAggregate instruction
		// after this. 
		if (location->callAggregate) {
		    currAddr += sizeof(instruction);
		    *++temp = location->aggregateInsn;
		    continue;
		}	
	    }
	    
	    // If the "FIRST instruction" is a DCTI, then our so called 
	    // "NEXT instruction" is in the delayed Slot and this might
	    // happen. (actullay, it happened)
	    if (location->callAggregate) {
		currAddr += sizeof(instruction);
		*++temp = location->aggregateInsn;
		continue;
	    }	
	    
	    // For the leaf function, if there's an inDelaySlot instruction,
            // move this one to the base Tramp.(i.e. at the function exit,
            // if the first instruction is in the delayed slot the previous
            // instruction, we have to move that one too, so we count from 
            // that one and the last one is this sequence is called inDelaySlot
	    // instruction.)
	    // Well, after all these, another SAVE instruction is generated
	    // so we are prepared to handle the returning to our application's
            // code segment. 
	    if (location->leaf) {
		if (location->inDelaySlot) {
		    fromAddr += sizeof(instruction);
		    currAddr += sizeof(instruction);
		    *++temp = location->inDelaySlotInsn;
		    relocateInstruction(temp,fromAddr,currAddr,(process *)proc);
                    if(location->firstIsConditional){
		        fromAddr += sizeof(instruction);
		        currAddr += sizeof(instruction);
		        *++temp = location->extraInsn;
		        relocateInstruction(temp, fromAddr, currAddr, proc);
		    }
		} 
		
		genImmInsn(temp+1, SAVEop3, REG_SP, -112, REG_SP);
	    }
	    
	} else if (temp->raw == RETURN_INSN) {
	    // compute the real from address if this instrumentation
	    // point is from a shared object image
	    Address baseAddress = 0;
	    if(proc->getBaseAddress(location->image_ptr,baseAddress)){
            }
	    // Back to the code segement of the application.
            // If the location is in the leaf procedure, generate an RESTORE
	    // instruction right after the CALL instruction to restore all
	    // the values in the registers.
	    if (location -> leaf) {
		generateCallInsn(temp, currAddr, 
				(baseAddress + location->addr)+location->size);
		genImmInsn(temp+1, RESTOREop3, 0, 0, 0);
	    } else {
		generateCallInsn(temp, currAddr, 
				(baseAddress + location->addr)+location->size);
	    }
        } else if (temp->raw == SKIP_PRE_INSN) {
	    unsigned offset;
	    offset = baseAddr+baseTemplate.updateCostOffset-currAddr;
	    generateBranchInsn(temp,offset);

        } else if (temp->raw == SKIP_POST_INSN) {
	    unsigned offset;
	    offset = baseAddr+baseTemplate.returnInsOffset-currAddr;
	    generateBranchInsn(temp,offset);

        } else if (temp->raw == UPDATE_COST_INSN) {
	    
	    baseTemplate.costAddr = currAddr;
	    generateNOOP(temp);
	} else if ((temp->raw == LOCAL_PRE_BRANCH) ||
                   (temp->raw == GLOBAL_PRE_BRANCH) ||
                   (temp->raw == LOCAL_POST_BRANCH) ||
		   (temp->raw == GLOBAL_POST_BRANCH)) {
	    /* fill with no-op */
	    generateNOOP(temp);
	}
    }
    // TODO cast
    proc->writeDataSpace((caddr_t)baseAddr, baseTemplate.size,(caddr_t) code);

    delete [] code;

    trampTemplate *baseInst = new trampTemplate;
    *baseInst = baseTemplate;
    baseInst->baseAddr = baseAddr;
    return baseInst;
}

/*
 * Install the base Tramp for the function relocated.
 * (it means the base tramp that don't need to bother with long jump and
 *  is the one we used before for all the functions(since there's no
 *  long jumps)
 *  for system calls
 */ 
trampTemplate *installBaseTrampSpecial(const instPoint *&location,
			     process *proc,
			     vector<instruction> &extra_instrs) 
{
    unsigned currAddr;
    instruction *code;
    instruction *temp;

    unsigned baseAddr = inferiorMalloc(proc, baseTemplate.size, textHeap);

    if(!(location->func->isInstalled(proc))) {
        location->func->relocateFunction(proc,location,extra_instrs);
    }
    else if(!location->relocated_){
	// need to find new instPoint for location...it has the pre-relocated
	// address of the instPoint
        location->func->modifyInstPoint(location,proc);	     
    }

    code = new instruction[baseTemplate.size];
    memcpy((char *) code, (char*) baseTemplate.trampTemp, baseTemplate.size);

    for (temp = code, currAddr = baseAddr; 
        (currAddr - baseAddr) < (unsigned) baseTemplate.size;
        temp++, currAddr += sizeof(instruction)) {

        if (temp->raw == EMULATE_INSN) {
	    if (location->isBranchOut) {
                // the original instruction is a branch that goes out of a 
		// function.  We don't relocate the original instruction. We 
		// only get to the tramp is the branch is taken, so we generate
		// a unconditional branch to the target of the original 
		// instruction here 
                assert(location->branchTarget);
                int disp = location->branchTarget - currAddr;
                generateBranchInsn(temp, disp);
                disp = temp->branch.disp22;
                continue;
            }
	    else {
		*temp = location->originalInstruction;
		Address fromAddress = location->addr;
		relocateInstruction(temp, fromAddress, currAddr, proc);
		if (location->isDelayed) {
		    /* copy delay slot instruction into tramp instance */
		    currAddr += sizeof(instruction);  
		    *++temp = location->delaySlotInsn;
		}
		if (location->callAggregate) {
		    /* copy invalid insn with aggregate size in it */
		    currAddr += sizeof(instruction);  
		    *++temp = location->aggregateInsn;
		}
	    }
        } else if (temp->raw == RETURN_INSN) {
            generateBranchInsn(temp, 
		(location->addr+ sizeof(instruction) - currAddr));
            if (location->isDelayed) {
                /* skip the delay slot instruction */
                temp->branch.disp22 += 1;
            }
            if (location->callAggregate) {
                /* skip the aggregate size slot */
                temp->branch.disp22 += 1;
            }
        } else if (temp->raw == SKIP_PRE_INSN) {
          unsigned offset;
          offset = baseAddr+baseTemplate.updateCostOffset-currAddr;
          generateBranchInsn(temp,offset);
        } else if (temp->raw == SKIP_POST_INSN) {
          unsigned offset;
          offset = baseAddr+baseTemplate.returnInsOffset-currAddr;
          generateBranchInsn(temp,offset);
        } else if (temp->raw == UPDATE_COST_INSN) {
	    
	    baseTemplate.costAddr = currAddr;
	    generateNOOP(temp);
	} else if ((temp->raw == LOCAL_PRE_BRANCH) ||
                   (temp->raw == GLOBAL_PRE_BRANCH) ||
                   (temp->raw == LOCAL_POST_BRANCH) ||
		   (temp->raw == GLOBAL_POST_BRANCH)) {
            /* fill with no-op */
            generateNOOP(temp);
        }
    }
    // TODO cast
    proc->writeDataSpace((caddr_t)baseAddr, baseTemplate.size,(caddr_t) code);

    delete [] code;

    trampTemplate *baseInst = new trampTemplate;
    *baseInst = baseTemplate;
    baseInst->baseAddr = baseAddr;
    return baseInst;
}

void generateNoOp(process *proc, int addr)
{
    instruction insn;

    /* fill with no-op */
    insn.raw = 0;
    insn.branch.op = 0;
    insn.branch.op2 = NOOPop2;

    // TODO cast
    proc->writeTextWord((caddr_t)addr, insn.raw);
}

void changeBranch(process *proc, unsigned fromAddr, unsigned newAddr, 
		  instruction originalBranch);

void generateCall(process *proc, unsigned fromAddr, unsigned newAddr);
void genImm(process *proc, unsigned fromAddr, int op, reg rs1, int immd,reg rd);


void AstNode::sysFlag(instPoint *location)
{
    astFlag = location->func->isTrapFunc();
    if (loperand)
	loperand->sysFlag(location);
    if (roperand)
	roperand->sysFlag(location); 

    for (unsigned u = 0; u < operands.size(); u++) {
	operands[u]->sysFlag(location);
    }
}


/*
 * Allocate the space for the base Trampoline, and generate the instruction
 * we need for modifying the code segment
 *
 */
trampTemplate *findAndInstallBaseTramp(process *proc, 
				 const instPoint *&location,
				 returnInstance *&retInstance,
				 bool noCost)
{
    Address adr = location->addr;
    trampTemplate *ret;
    retInstance = NULL;
    if (!proc->baseMap.defines((const instPoint *)location)) {
	if (location->func->isTrapFunc()) {
	    // get the base Address of this function if it is a 
	    // shared object
	    Address baseAddress = 0;
  	    if(!(proc->getBaseAddress(location->image_ptr,baseAddress))){
	        // TODO: what should be done here? 	
		logLine("Error:findAndInstallBaseTramp call getBaseAddress\n"); 
	    }
	    // Install Base Tramp for the functions which are 
	    // relocated to the heap.
            vector<instruction> extra_instrs;
	    ret = installBaseTrampSpecial(location, proc,extra_instrs);

            // add a branch from relocated function to the base tramp
	    // if function was just relocated than location has old address
	    // otherwise location will have address in already relocated func
            if(!(location->func->isInstalled(proc))){
	        if (location->isBranchOut){
		    changeBranch(proc, location->addr, 
		          (int) ret->baseAddr, location->originalInstruction);
                } else {
		    generateBranch(proc, location->addr, (int)ret->baseAddr);
                }
	    }
	    else {  // location's address is correct...it is in the heap
	        if (location->isBranchOut){
		    changeBranch(proc, location->addr, 
		          (int) ret->baseAddr, location->originalInstruction);
                } else {
		    generateBranch(proc, location->addr, (int)ret->baseAddr);
                }
	    }

	    // If for this process, a call to the relocated function has not
	    // yet be installed in its original location, then genterate the
	    // following instructions at the begining of the function:
	    //   SAVE;             CALL;         RESTORE.
	    // so that it would jump the start of the relocated function
	    // which is in heap.
	    if(!(location->func->isInstalled(proc))){
	    	location->func->setInstalled(proc);
		u_int e_size = extra_instrs.size();
		instruction *insn = new instruction[3 + e_size];
		Address adr = location-> func -> getAddress(0);
		genImmInsn(insn, SAVEop3, REG_SP, -112, REG_SP);
		generateCallInsn(insn+1, adr+baseAddress+4, 
				 location->func->getAddress(proc));
		genSimpleInsn(insn+2, RESTOREop3, 0, 0, 0); 
		for(u_int i=0; i < e_size; i++){
		    insn[3+i] = extra_instrs[i];
		}
		retInstance = new returnInstance((instructUnion *)insn, 
					 (3+e_size)*sizeof(instruction), 
					 adr+baseAddress, 
					 location->func->size());
                assert(retInstance);

                //cerr << "created a new return instance (relocated fn)!" << endl;
	    }

	} else {
	    // Install base tramp for all the other regular functions. 
	    ret = installBaseTramp(location, proc);
	    // compute the real from address if this instrumentation
	    // point is from a shared object image
	    Address baseAddress = 0;
	    if(proc->getBaseAddress(location->image_ptr,baseAddress)){
		adr += baseAddress;		
            }
	    if (location->leaf) {
		// if it is the leaf function, we need to generate
		// the following instruction sequence:
		//     SAVE;      CALL;      NOP.
		instruction *insn = new instruction[3];
		genImmInsn(insn, SAVEop3, REG_SP, -112, REG_SP);
		generateCallInsn(insn+1, adr+4, (int) ret->baseAddr);
		generateNOOP(insn+2);
		retInstance = new returnInstance((instructUnion *)insn, 
						 3*sizeof(instruction), adr, 
						 3*sizeof(instruction));
                assert(retInstance);

                //cerr << "created a new return instance (leaf)!" << endl;
	    } else {
		// Otherwise,
		// Generate branch instruction from the application to the
		// base trampoline and no SAVE instruction is needed
		instruction *insn = new instruction[2];	
		generateCallInsn(insn, adr, (int) ret->baseAddr);
		generateNOOP(insn+1);
		retInstance = new returnInstance((instructUnion *)insn, 
						 2*sizeof(instruction), adr, 
						 2*sizeof(instruction));
                assert(retInstance);

                //cerr << "created a new return instance (normal)!" << endl;
	    }
	}

	proc->baseMap[(const instPoint *)location] = ret;

    } else {
        ret = proc->baseMap[(const instPoint *)location];
    }
      
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

    unsigned atAddr;
    if (inst->when == callPreInsn) {
	if (inst->baseInstance->prevInstru == false) {
	    atAddr = inst->baseInstance->baseAddr+baseTemplate.skipPreInsOffset;
	    inst->baseInstance->cost += inst->baseInstance->prevBaseCost;
	    inst->baseInstance->prevInstru = true;
	    generateNoOp(inst->proc, atAddr);
	}
    } else {
	if (inst->baseInstance->postInstru == false) {
	    atAddr = inst->baseInstance->baseAddr+baseTemplate.skipPostInsOffset; 
	    inst->baseInstance->cost += inst->baseInstance->postBaseCost;
	    inst->baseInstance->postInstru = true;
	    generateNoOp(inst->proc, atAddr);
	}
    }
}

/*
 * change the insn at addr to be a branch to newAddr.
 *   Used to add multiple tramps to a point.
 */
void generateBranch(process *proc, unsigned fromAddr, unsigned newAddr)
{
    int disp;
    instruction insn;

    disp = newAddr-fromAddr;
    generateBranchInsn(&insn, disp);

    // TODO cast
    proc->writeTextWord((caddr_t)fromAddr, insn.raw);
}

void generateCall(process *proc, unsigned fromAddr,unsigned newAddr)
{
    instruction insn; 
    generateCallInsn(&insn, fromAddr, newAddr);

    proc->writeTextWord((caddr_t)fromAddr, insn.raw);

}

void genImm(process *proc, Address fromAddr,int op, reg rs1, int immd, reg rd)
{
    instruction insn;
    genImmInsn(&insn, op, rs1, immd, rd);

    proc->writeTextWord((caddr_t)fromAddr, insn.raw);
}

/*
 *  change the target of the branch at fromAddr, to be newAddr.
 */
void changeBranch(process *proc, unsigned fromAddr, unsigned newAddr, 
		  instruction originalBranch) {
    int disp = newAddr-fromAddr;
    instruction insn;
    insn.raw = originalBranch.raw;
    insn.branch.disp22 = disp >> 2;
    proc->writeTextWord((caddr_t)fromAddr, insn.raw);
}

int callsTrackedFuncP(instPoint *point)
{
    if (point->callIndirect) {
#ifdef notdef
        // TODO this won't compile now
	// it's rare to call a library function as a parameter.
        sprintf(errorLine, "*** Warning call indirect\n from %s %s (addr %d)\n",point->func->file->fullName, point->func->prettyName, point->addr);
	logLine(errorLine);
#endif
        return(true);
    } else {
	if (point->callee && !(point->callee->isLibTag())) {
	    return(true);
	} else {
	    return(false);
	}
    }
}

/*
 * return the function asociated with a point.
 *
 *     If the point is a funcation call, and we know the function being called,
 *          then we use that.  Otherwise it is the function that contains the
 *          point.
 *  
 *   This is done to return a better idea of which function we are using.
 */
pdFunction *getFunction(instPoint *point)
{
    return(point->callee ? point->callee : point->func);
}

unsigned emitFuncCall(opCode op, 
		      registerSpace *rs,
		      char *i, unsigned &base, 
		      const vector<AstNode *> &operands, 
		      const string &callee, process *proc,
		      bool noCost)
{
        assert(op == callOp);
        unsigned addr;
	bool err;
	vector <reg> srcs;

        addr = proc->findInternalAddress(callee, false, err);

        if (err) {
	    pdFunction *func = proc->findOneFunction(callee);
	    if (!func) {
		ostrstream os(errorLine, 1024, ios::out);
		os << "Internal error: unable to find addr of " << callee << endl;
		logLine(errorLine);
		showErrorCallback(80, (const char *) errorLine);
		P_abort();
	    }
	    // TODO: is this correct or should we get relocated address?
	    addr = func->getAddress(0);
	}
	
	for (unsigned u = 0; u < operands.size(); u++)
	    srcs += operands[u]->generateCode(proc, rs, i, base, noCost);

	// TODO cast
	instruction *insn = (instruction *) ((void*)&i[base]);

        for (unsigned u=0; u<srcs.size(); u++){
            if (u >= 5) {
	         string msg = "Too many arguments to function call in instrumentation code: only 5 arguments can be passed on the sparc architecture.\n";
		 fprintf(stderr, msg.string_of());
	         showErrorCallback(94,msg);
		 cleanUpAndExit(-1);
            }
            genSimpleInsn(insn, ORop3, 0, srcs[u], u+8); insn++;
            base += sizeof(instruction);
	    rs->freeRegister(srcs[u]);
        }

        // As Ling pointed out to me, the following is rather inefficient.  It does:
        //   sethi %hi(addr), %o5
        //   jmpl %o5 + %lo(addr), %o7   ('call' pseudo-instr)
        //   nop
        // We can do better:
        //   call <addr>    (but note that the call true-instr is pc-relative jump)
        //   nop
        generateSetHi(insn, addr, 13); insn++;
        genImmInsn(insn, JMPLop3, 13, LOW10(addr), 15); insn++;
        generateNOOP(insn);

        base += 3 * sizeof(instruction);

        // return value is the register with the return value from the
        //   function.
        // This needs to be %o0 since it is back in the callers scope.
        return(8);
}
 
bool process::emitInferiorRPCheader(void *insnPtr, unsigned &baseBytes) {
   instruction *insn = (instruction *)insnPtr;
   unsigned baseInstruc = baseBytes / sizeof(instruction);

   genImmInsn(&insn[baseInstruc++], SAVEop3, 14, -112, 14);

   baseBytes = baseInstruc * sizeof(instruction); // convert back
   return true;
}


bool process::emitInferiorRPCtrailer(void *insnPtr, unsigned &baseBytes,
				     unsigned &firstPossibBreakOffset,
				     unsigned &lastPossibBreakOffset) {
   // Sequence: restore, trap, illegal

   instruction *insn = (instruction *)insnPtr;

   unsigned baseInstruc = baseBytes / sizeof(instruction);

   genSimpleInsn(&insn[baseInstruc++], RESTOREop3, 0, 0, 0);

   // Now that the inferior has executed the 'restore' instruction, the %in and
   // %local registers have been restored.  We mustn't modify them after this point!!
   // (reminder: the %in and %local registers aren't saved and set with ptrace
   //  GETREGS/SETREGS call)

   // Trap instruction:
   genBreakpointTrap(&insn[baseInstruc]); // ta 1
   firstPossibBreakOffset = lastPossibBreakOffset = baseInstruc * sizeof(instruction);
   baseInstruc++;

   // And just to make sure that we don't continue from the trap:
   genUnimplementedInsn(&insn[baseInstruc++]); // UNIMP 0

   baseBytes = baseInstruc * sizeof(instruction); // convert back

   return true; // success
}

unsigned emitImm(opCode op, reg src1, reg src2, reg dest, char *i, 
                 unsigned &base, bool noCost)
{
        instruction *insn = (instruction *) ((void*)&i[base]);
        int op3=-1;
        int result;
	switch (op) {
	    // integer ops
	    case plusOp:
		op3 = ADDop3;
                genImmInsn(insn, op3, src1, src2, dest);
		break;

	    case minusOp:
		op3 = SUBop3;
                genImmInsn(insn, op3, src1, src2, dest);
		break;

	    case timesOp:
		op3 = SMULop3;
                if (isPowerOf2(src2,result) && (result<32))
                  generateLShift(insn, src1, (reg)result, dest);           
                else 
                  genImmInsn(insn, op3, src1, src2, dest);
		break;

	    case divOp:
		op3 = SDIVop3;
                if (isPowerOf2(src2,result) && (result<32))
                  generateRShift(insn, src1, (reg)result, dest);           
                else 
                  genImmInsn(insn, op3, src1, src2, dest);
		break;

	    // Bool ops
	    case orOp:
		op3 = ORop3;
                genImmInsn(insn, op3, src1, src2, dest);
		break;

	    case andOp:
		op3 = ANDop3;
                genImmInsn(insn, op3, src1, src2, dest);
		break;

	    // rel ops
	    // For a particular condition (e.g. <=) we need to use the
            // the opposite in order to get the right value (e.g. for >=
            // we need BLTcond) - naim
	    case eqOp:
		genImmRelOp(insn, BNEcond, src1, src2, dest, base);
		return(0);
		break;

            case neOp:
                genImmRelOp(insn, BEcond, src1, src2, dest, base);
                return(0);
                break;

	    case lessOp:
                genImmRelOp(insn, BGEcond, src1, src2, dest, base);
                return(0);
                break;

            case leOp:
                genImmRelOp(insn, BGTcond, src1, src2, dest, base);
                return(0);
                break;

            case greaterOp:
                genImmRelOp(insn, BLEcond, src1, src2, dest, base);
                return(0);
                break;

            case geOp:
                genImmRelOp(insn, BLTcond, src1, src2, dest, base);
                return(0);
                break;

	    default:
                reg dest2 = regSpace->allocateRegister(i, base, noCost);
                (void) emit(loadConstOp, src2, dest2, dest2, i, base, noCost);
                (void) emit(op, src1, dest2, dest, i, base, noCost);
                regSpace->freeRegister(dest2);
                return(0);
		break;
	}
	base += sizeof(instruction);
        return(0);
}

unsigned emit(opCode op, reg src1, reg src2, reg dest, char *i, unsigned &base,
	      bool noCost)
{
    // TODO cast
    instruction *insn = (instruction *) ((void*)&i[base]);

    if (op == loadConstOp) {
      // dest = src1:imm    TODO
      if (src1 > MAX_IMM13 || src1 < MIN_IMM13) {
            // src1 is out of range of imm13, so we need an extra instruction
	    generateSetHi(insn, src1, dest);
	    base += sizeof(instruction);
	    insn++;

	    // or regd,imm,regd

            // Chance for optimization: we should check for LOW10(src1)==0, and
            // if so, don't generate the following bitwise-or instruction, since
            // in that case nothing would be done.

	    genImmInsn(insn, ORop3, dest, LOW10(src1), dest);
	    base += sizeof(instruction);
	} else {
	    // really or %g0,imm,regd
	    genImmInsn(insn, ORop3, 0, src1, dest);

	    base += sizeof(instruction);
	}
    } else if (op ==  loadOp) {
	// dest = [src1]   TODO
	generateSetHi(insn, src1, dest);
	insn++;

	generateLoad(insn, dest, LOW10(src1), dest);

	base += sizeof(instruction)*2;
    } else if (op ==  loadIndirOp) {
	generateLoad(insn, src1, 0, dest);
	base += sizeof(instruction);
    } else if (op ==  storeOp) {
	insn->sethi.op = FMT2op;
	insn->sethi.rd = src2;
	insn->sethi.op2 = SETHIop2;
	insn->sethi.imm22 = HIGH22(dest);
	insn++;

	generateStore(insn, src1, src2, LOW10(dest));

	base += sizeof(instruction)*2;
    } else if (op ==  storeIndirOp) {
	generateStore(insn, src1, dest, 0);
	base += sizeof(instruction);
    } else if (op ==  ifOp) {
	// cmp src1,0
	genSimpleInsn(insn, SUBop3cc, src1, 0, 0); insn++;

	insn->branch.op = 0;
	insn->branch.cond = BEcond;
	insn->branch.op2 = BICCop2;
	insn->branch.anneal = false;
	insn->branch.disp22 = dest/4;
	insn++;

	generateNOOP(insn);
	base += sizeof(instruction)*3;
	return(base - 2*sizeof(instruction));
    } else if (op ==  updateCostOp) {
        // generate code to update the observed cost.
	if (!noCost) {
	   // sethi %hi(dest), %l0
	   generateSetHi(insn, dest, REG_L0);
	   base += sizeof(instruction);
	   insn++;
  
	   // ld [%l0+ lo(dest)], %l1
	   generateLoad(insn, REG_L0, LOW10(dest), REG_L1);
	   base += sizeof(instruction);
	   insn++;
  
	   // update value (src1 holds the cost, in cycles; e.g. 19)
	   if (src1 <= MAX_IMM13) {
	      genImmInsn(insn, ADDop3, REG_L1, src1, REG_L1);
	      base += sizeof(instruction);
	      insn++;

	      generateNOOP(insn);
	      base += sizeof(instruction);
	      insn++;

	      generateNOOP(insn);
	      base += sizeof(instruction);
	      insn++;
	   } else {
	      // load in two parts
	      generateSetHi(insn, src1, REG_L2);
	      base += sizeof(instruction);
	      insn++;

	      // or regd,imm,regd
	      genImmInsn(insn, ORop3, REG_L2, LOW10(src1), REG_L2);
	      base += sizeof(instruction);
	      insn++;

	      // now add it
	      genSimpleInsn(insn, ADDop3, REG_L1, REG_L2, REG_L1);
	      base += sizeof(instruction);
	      insn++;
	   }
  
	   // store result st %l1, [%l0+ lo(dest)];
	   generateStore(insn, REG_L1, REG_L0, LOW10(dest));
	   base += sizeof(instruction);
	   insn++;
	} // if (!noCost)
    } else if (op ==  trampPreamble) {
#ifdef ndef
        // save and restore are done inthe base tramp now
        genImmInsn(insn, SAVEop3, REG_SP, -112, REG_SP);
	base += sizeof(instruction);
        insn++;

	// generate code to save global registers
	for (unsigned u = 0; u < 4; u++) {
	  genStoreD(insn, 2*u, REG_FP, - (8 + 8*u));
	  base += sizeof(instruction);
	  insn++;
	}
#endif
    } else if (op ==  trampTrailer) {
#ifdef ndef
        // save and restore are done inthe base tramp now
	// generate code to restore global registers
	for (unsigned u = 0; u < 4; u++) {
	  genLoadD(insn, REG_FP, - (8 + 8*u), 2*u);
	  base += sizeof(instruction);
	  insn++;
	}

        // sequence: restore; nop; b,a back to base tramp; nop
        // we can do better.  How about putting the restore in
        // the delay slot of the branch instruction, as in:
        // b <back to base tramp>; restore
        genSimpleInsn(insn, RESTOREop3, 0, 0, 0); 
	base += sizeof(instruction);
	insn++;

	generateNOOP(insn);
	base += sizeof(instruction);
	insn++;
#endif
	// dest is in words of offset and generateBranchInsn is bytes offset
	generateBranchInsn(insn, dest << 2);
	base += sizeof(instruction);
	insn++;

        // add no-op, SS-5 sometimes seems to try to decode this insn - jkh 2/14
        generateNOOP(insn);
        insn++;
        base += sizeof(instruction);

	return(base -  2 * sizeof(instruction));
    } else if (op == noOp) {
	generateNOOP(insn);
	base += sizeof(instruction);
    } else if (op == getParamOp) {
	// first 8 parameters are in register 24 ....
	genSimpleInsn(insn, RESTOREop3, 0, 0, 0);
	insn++;

	generateStore(insn, 24+src1, REG_SP, 68+4*src1); 
	insn++;
	      
	genImmInsn(insn, SAVEop3, REG_SP, -112, REG_SP);
	insn++;

	generateLoad(insn, REG_SP, 112+68+4*src1, 24+src1); 
	insn++;

	base += 4*sizeof(instruction);
	
	if (src1 <= 8) {
	    return(24+src1);
	}
	
	abort();
    } else if (op == getSysParamOp) {
	
	if (src1 <= 8) {
	    return(24+src1);
	}	
    } else if (op == getRetValOp) {
	// return value is in register 24
	genSimpleInsn(insn, RESTOREop3, 0, 0, 0);
	insn++;

	generateStore(insn, 24, REG_SP, 68); 
	insn++;
	      
	genImmInsn(insn, SAVEop3, REG_SP, -112, REG_SP);
	insn++;

	generateLoad(insn, REG_SP, 112+68, 24); 
	insn++;

	base += 4*sizeof(instruction);

	return(24);

    } else if (op == getSysRetValOp) {
	return(24);
    } else if (op == saveRegOp) {
	// should never be called for this platform.
	abort();
    } else {
      int op3=-1;
	switch (op) {
	    // integer ops
	    case plusOp:
		op3 = ADDop3;
		break;

	    case minusOp:
		op3 = SUBop3;
		break;

	    case timesOp:
		op3 = SMULop3;
		break;

	    case divOp:
		op3 = SDIVop3;
		break;

	    // Bool ops
	    case orOp:
		op3 = ORop3;
		break;

	    case andOp:
		op3 = ANDop3;
		break;

	    // rel ops
	    // For a particular condition (e.g. <=) we need to use the
            // the opposite in order to get the right value (e.g. for >=
            // we need BLTcond) - naim
	    case eqOp:
		genRelOp(insn, BNEcond, src1, src2, dest, base);
		return(0);
		break;

            case neOp:
                genRelOp(insn, BEcond, src1, src2, dest, base);
                return(0);
                break;

	    case lessOp:
                genRelOp(insn, BGEcond, src1, src2, dest, base);
                return(0);
                break;

            case leOp:
                genRelOp(insn, BGTcond, src1, src2, dest, base);
                return(0);
                break;

            case greaterOp:
                genRelOp(insn, BLEcond, src1, src2, dest, base);
                return(0);
                break;

            case geOp:
                genRelOp(insn, BLTcond, src1, src2, dest, base);
                return(0);
                break;

	    default:
		abort();
		break;
	}
	genSimpleInsn(insn, op3, src1, src2, dest);

	base += sizeof(instruction);
      }
    return(0);
}

//
// All values based on Cypress0 && Cypress1 implementations as documented in
//   SPARC v.8 manual p. 291
//
int getInsnCost(opCode op)
{
    if (op == loadConstOp) {
	return(1);
    } else if (op ==  loadOp) {
	// sethi + load single
	return(1+1);
    } else if (op ==  loadIndirOp) {
	return(1);
    } else if (op ==  storeOp) {
	// sethi + store single
	// return(1+3); 
	// for SS-5 ?
	return(1+2); 
    } else if (op ==  storeIndirOp) {
	return(2); 
    } else if (op ==  ifOp) {
	// subcc
	// be
	// nop
	return(1+1+1);
    } else if (op ==  callOp) {
	int count = 0;

	// mov src1, %o0
	count += 1;

	// mov src2, %o1
	count += 1;

	// clr i2
	count += 1;

	// clr i3
	count += 1;

	// sethi
	count += 1;

	// jmpl
	count += 1;

	// noop
	count += 1;

	return(count);
    } else if (op ==  updateCostOp) {
        // sethi %hi(obsCost), %l0
        // ld [%lo + %lo(obsCost)], %l1
        // add %l1, <cost>, %l1
        // st %l1, [%lo + %lo(obsCost)]
        return(1+2+1+3);
    } else if (op ==  trampPreamble) {
	return(0);
    } else if (op ==  trampTrailer) {
	// retl
	return(2);
    } else if (op == noOp) {
	// noop
	return(1);
    } else if (op == getParamOp) {
        return(0);
    } else {
	switch (op) {
	    // rel ops
	    case eqOp:
            case neOp:
	    case lessOp:
            case leOp:
            case greaterOp:
	    case geOp:
	        // bne -- assume taken
	        return(2);
	        break;
	    default:
		return(1);
		break;
	}
    }
}

bool isReturnInsn(const image *owner, Address adr, bool &lastOne)
{
    instruction instr;
    
    instr.raw = owner->get_instruction(adr);
    lastOne = false;

    if (isInsnType(instr, RETmask, RETmatch) ||
        isInsnType(instr, RETLmask, RETLmatch)) {
        if ((instr.resti.simm13 != 8) && (instr.resti.simm13 != 12)) {
	    logLine("*** FATAL Error:");
	    sprintf(errorLine, " unsupported return\n");
	    logLine(errorLine);
	    showErrorCallback(55, "");
	    P_abort();
        }
	return true;
    }
    return false;
}

/*
 * Find the instPoints of this function.
 */
bool pdFunction::findInstPoints(const image *owner) {

   if (size() == 0) {
     return false;
   }

   leaf = true;
   Address adr;
   Address adr1 = getAddress(0);
   instruction instr;
   instr.raw = owner->get_instruction(adr1);
   if (!IS_VALID_INSN(instr))
     return false;

   // If it contains an instruction, I assume it would be s system call
   // which will be treat differently. 
   isTrap = false;
   bool func_entry_found = false;

   for ( ; adr1 < getAddress(0) + size(); adr1 += 4) {
       instr.raw = owner->get_instruction(adr1);

       // If there's an TRAP instruction in the function, we 
       // assume that it is an system call and will relocate it 
       // to the heap
       if (isInsnType(instr, TRAPmask, TRAPmatch)) {
	   isTrap = true;
	   return findInstPoints(owner, getAddress(0), 0);
       } 

       // TODO: This is a hacking for the solaris(solaris2.5 actually)
       // We will relocate that function if the function has been 
       // tail-call optimazed.
       // (Actully, the reason of this is that the system calls like 
       //  read, write, etc have the tail-call optimazation to call
       //  the _read, _write etc. which contain the TRAP instruction 
       //  This is only done if libc is statically linked...if the
       //  libTag is set, otherwise we instrument read and _read
       //  both for the dynamically linked case
       if (isLibTag()) {
	   if (isCallInsn(instr)) {
	       instruction nexti; 
	       nexti.raw = owner->get_instruction(adr1+4);
	       
	       if (nexti.rest.op == 2 
		   && ((nexti.rest.op3 == ORop3 && nexti.rest.rd == 15)
		       || nexti.rest.op3 == RESTOREop3)) {
		   isTrap = true;
		   return findInstPoints(owner, getAddress(0), 0);
	       }
	   }   
       }

       // The function Entry is defined as the first SAVE instruction plus
       // the instructions after this.
       // ( The first instruction for the nonleaf function is not 
       //   necessarily a SAVE instruction. ) 
       if (isInsnType(instr, SAVEmask, SAVEmatch) && !func_entry_found) {

	   leaf = false;
	   func_entry_found = true;
	   funcEntry_ = new instPoint(this, instr, owner, adr1, true, leaf, 
				      functionEntry);
	   adr = adr1;
	   assert(funcEntry_);
       }
   }

   // If there's no SAVE instruction found, this is a leaf function and
   // and function Entry will be defined from the first instruction
   if (leaf) {
       adr = getAddress(0);
       instr.raw = owner->get_instruction(adr);
       funcEntry_ = new instPoint(this, instr, owner, adr, true, leaf, functionEntry);
       assert(funcEntry_);
   }

   for ( ; adr < getAddress(0) + size(); adr += sizeof(instruction)) {

     instr.raw = owner->get_instruction(adr);

     bool done;

     // check for return insn and as a side affect decide if we are at the
     //   end of the function.
     if (isReturnInsn(owner, adr, done)) {
       // define the return point
       funcReturns += new instPoint(this, instr, owner, adr, false, leaf, 
				    functionExit);

     } else if (instr.branch.op == 0 
		&& (instr.branch.op2 == 2 || instr.branch.op2 == 6) 
		&& (instr.branch.cond == 0 ||instr.branch.cond == 8)) {
       // find if this branch is going out of the function
       int disp = instr.branch.disp22;
       Address target = adr +  (disp << 2);
       if ((target < (getAddress(0)))  
	   || (target >= (getAddress(0) + size()))) {
	 instPoint *point = new instPoint(this, instr, owner, adr, false, leaf, 
					  functionExit);
	 funcReturns += point;
       }

     } else if (isCallInsn(instr)) {

       // if the call target is the address of the call instruction
       // then this is not something that we can instrument...
       // this occurs in functions with code that is modifined when 
       // they are loaded by the run-time linker, or when the .init
       // section is executed.  In this case the instructions in the
       // parsed image file are different from the ones in the executable
       // process.
       if(instr.call.op == CALLop) { 
           Address call_target = adr + (instr.call.disp30 << 2);
           if(call_target == adr){ 
	        return false;
       }}
       // first, check for tail-call optimization: a call where the instruction 
       // in the delay slot write to register %o7(15), usually just moving
       // the caller's return address, or doing a restore
       // Tail calls are instrumented as return points, not call points.


       instruction nexti; 
       nexti.raw = owner->get_instruction(adr+4);

       if (nexti.rest.op == 2 
	   && ((nexti.rest.op3 == ORop3 && nexti.rest.rd == 15)
	      || nexti.rest.op3 == RESTOREop3)) {
	 //fprintf(stderr, "#### Tail-call optimization in function %s, addr %x\n",
	 //	prettyName().string_of(), adr);
	 funcReturns += new instPoint(this, instr, owner, adr, false, leaf, 
				      functionExit);

       } else {
	 // define a call point
	 // this may update address - sparc - aggregate return value
	 // want to skip instructions
	 bool err;
	 int dummyId;
	 instPoint *blah = 0;
	 adr = newCallPoint(adr, instr, owner, err, dummyId, adr,0,blah);
       }
     }

     else if (isInsnType(instr, JMPLmask, JMPLmatch)) {
       /* A register indirect jump. Some jumps may exit the function 
          (e.g. read/write on SunOS). In general, the only way to 
	  know if a jump is exiting the function is to instrument
	  the jump to test if the target is outside the current 
	  function. Instead of doing this, we just check the 
	  previous two instructions, to see if they are loading
	  an address that is out of the current function.
	  This should catch the most common cases (e.g. read/write).
	  For other cases, we would miss a return point.

	  This is the case considered:

	     sethi addr_hi, r
	     or addr_lo, r, r
	     jump r
	*/

       reg jumpreg = instr.rest.rs1;
       instruction prev1;
       instruction prev2;

       prev1.raw = owner->get_instruction(adr-4);
       prev2.raw = owner->get_instruction(adr-8);

       unsigned targetAddr;

       if (instr.rest.rd == 0 && (instr.rest.i == 1 || instr.rest.rs2 == 0)
	   && prev2.sethi.op == FMT2op && prev2.sethi.op2 == SETHIop2 
	   && prev2.sethi.rd == (unsigned)jumpreg
	   && prev1.rest.op == RESTop 
	   && prev1.rest.rd == (unsigned)jumpreg && prev1.rest.i == 1
	   && prev1.rest.op3 == ORop3 && prev1.rest.rs1 == (unsigned)jumpreg) {

	 targetAddr = (prev2.sethi.imm22 << 10) & 0xfffffc00;
	 targetAddr |= prev1.resti.simm13;
	 if ((targetAddr<getAddress(0))||(targetAddr>=(getAddress(0)+size()))){
	   instPoint *point = new instPoint(this, instr, owner, adr, false, 
					    leaf, functionExit);
	   funcReturns += point;
	 }
       }

     }
 }

 return (checkInstPoints(owner)); 
}

/*
 * Check all the instPoints within this function to see if there's 
 * any conficts happen.
 */
bool pdFunction::checkInstPoints(const image *owner) {

    // Our own library function, skip the test.
    if (prettyName_.prefixed_by("DYNINST")) 
	return true;

    // The function is too small to be worthing instrumenting.
    if (size() <= 12)
	return false;

    // No function return! return false;
    if (sizeof(funcReturns) == 0)
	return false;

    instruction instr;
    Address adr = getAddress(0);

    bool retl_inst = false;
    // Check if there's any branch instruction jump to the middle
    // of the instruction sequence in the function entry point
    // and function exit point.
    for ( ; adr < getAddress(0) + size(); adr += sizeof(instruction)) {

	instr.raw = owner->get_instruction(adr);
	if(isInsnType(instr, RETLmask, RETLmatch)) retl_inst = true;

	if (isInsnType(instr, BRNCHmask, BRNCHmatch)||
	    isInsnType(instr, FBRNCHmask, FBRNCHmatch)) {

	    int disp = instr.branch.disp22;
	    Address target = adr + (disp << 2);

	    if ((target > funcEntry_->addr)&&
		(target < (funcEntry_->addr + funcEntry_->size))) {
		if (adr > (funcEntry_->addr+funcEntry_->size))
		    //cout << "Function " << prettyName_ <<" entry" << endl;
		    return false;
	    }

	    for (u_int i = 0; i < funcReturns.size(); i++) {
		if ((target > funcReturns[i]->addr)&&
		    (target < (funcReturns[i]->addr + funcReturns[i]->size))) {
		    if ((adr < funcReturns[i]->addr)||
			(adr > (funcReturns[i]->addr + funcReturns[i]->size)))
			return false;
		}
	    }
	}
    }

    // if there is a retl instruction and we don't think this is a leaf
    // function then this is a way messed up function...well, at least we
    // we can't deal with this...the only example I can find is _cerror
    // and _cerror64 in libc.so.1
    if(retl_inst && !leaf){
	 return false;
    }

    // check that no instrumentation points could overlap
    Address func_entry = funcEntry_->addr + funcEntry_->size; 
    for (u_int i = 0; i < funcReturns.size(); i++) {
	if(func_entry >= funcReturns[i]->addr){
	   return false;
        }
	if(i > 1){ // check if return points overlap
	    Address prev_exit = funcReturns[i-1]->addr+funcReturns[i-1]->size;  
	    if(funcReturns[i]->addr >= prev_exit) {
		return false;
	    } 
	}
    }

    return true;	
}


/* The maximum length of relocatable function is 1k instructions */  
static instruction newInstr[1024];

// This function is to find the inst Points for a function
// that will be relocated if it is instrumented. 
bool pdFunction::findInstPoints(const image *owner, Address newAdr, process*){

   int i;
   if (size() == 0) {
     return false;
   }
   relocatable_ = true;

   Address adr = getAddress(0);
   instruction instr;
   instr.raw = owner->get_instruction(adr);
   if (!IS_VALID_INSN(instr))
     return false;
   
   if (size() <= 3*sizeof(instruction)) 
       return false;

   instPoint *point = new instPoint(this, instr, owner, newAdr, true, 
				    functionEntry, adr);

   funcEntry_ = point;

   // if the second instruction in a relocated function is a call instruction
   // or a branch instruction, then we can't deal with this 
   if(size() > sizeof(instruction)){
       Address second_adr = adr + sizeof(instruction);
       instruction second_instr;
       second_instr.raw =  owner->get_instruction(second_adr); 
       if ((isCallInsn(second_instr)) || 
		      (second_instr.branch.op == 0 && 
		      (second_instr.branch.op2 == 2 || 
		      second_instr.branch.op2 == 6))) {
	   return false;
       }
   }
   
   assert(funcEntry_);
   int retId = 0;
   int callsId = 0; 

   for (i = 0; adr < getAddress(0) + size(); adr += sizeof(instruction),  
	newAdr += sizeof(instruction), i++) {

     instr.raw = owner->get_instruction(adr);
     newInstr[i] = instr;
     bool done;

     // check for return insn and as a side affect decide if we are at the
     //   end of the function.
     if (isReturnInsn(owner, adr, done)) {
       // define the return point
       instPoint *point	= new instPoint(this, instr, owner, newAdr, false, 
					functionExit, adr);
       funcReturns += point;
       funcReturns[retId] -> instId = retId++;
     } else if (instr.branch.op == 0 
		&& (instr.branch.op2 == 2 || instr.branch.op2 == 6)) {
       // find if this branch is going out of the function
       int disp = instr.branch.disp22;
       Address target = adr + (disp << 2);
       if (target < getAddress(0) || target >= getAddress(0) + size()) {
	   instPoint *point = new instPoint(this, newInstr[i], owner, 
					    newAdr, false, 
					    functionExit, adr);
	   if ((instr.branch.cond != 0) && (instr.branch.cond != 8)) {  
	       point->isBranchOut = true;
	       point->branchTarget = target;
	   }
	   funcReturns += point;
	   funcReturns[retId] -> instId = retId++;
       }

     } else if (isCallInsn(instr)) {

       // first, check for tail-call optimization: a call where the instruction 
       // in the delay slot write to register %o7(15), usually just moving
       // the caller's return address, or doing a restore
       // Tail calls are instrumented as return points, not call points.
       instruction nexti; 
       nexti.raw = owner->get_instruction(adr+4);

       if (nexti.rest.op == 2 
	   && ((nexti.rest.op3 == ORop3 && nexti.rest.rd == 15)
	      || nexti.rest.op3 == RESTOREop3)) {

           instPoint *point = new instPoint(this, instr, owner, newAdr, false,
				      functionExit, adr);
           funcReturns += point;
           funcReturns[retId] -> instId = retId++;

       } else {
         // if this is a call instr to a location within the function, and if 
         // the offest is not 8 then do not define this function 
         if(instr.call.op == CALLop){ 
           Address call_target = adr + (instr.call.disp30 << 2);
           if((call_target >= getAddress(0)) 
	      && (call_target <= (getAddress(0)+size()))){ 
	      if((instr.call.disp30 << 2) != 2*sizeof(instruction)) {
	        return false;
	      }
           }
	 }
	 // define a call point
	 // this may update address - sparc - aggregate return value
	 // want to skip instructions
	 bool err;
	 instPoint *blah = 0;
	 adr = newCallPoint(newAdr, instr, owner, err, callsId, adr,0,blah);
	 if (err)
	   return false;
       }
     }

     else if (isInsnType(instr, JMPLmask, JMPLmatch)) {
       /* A register indirect jump. Some jumps may exit the function 
          (e.g. read/write on SunOS). In general, the only way to 
	  know if a jump is exiting the function is to instrument
	  the jump to test if the target is outside the current 
	  function. Instead of doing this, we just check the 
	  previous two instructions, to see if they are loading
	  an address that is out of the current function.
	  This should catch the most common cases (e.g. read/write).
	  For other cases, we would miss a return point.

	  This is the case considered:

	     sethi addr_hi, r
	     or addr_lo, r, r
	     jump r
	*/

	 reg jumpreg = instr.rest.rs1;
	 instruction prev1;
	 instruction prev2;
	 
	 prev1.raw = owner->get_instruction(adr-4);
	 prev2.raw = owner->get_instruction(adr-8);

	 unsigned targetAddr;

	 if (instr.rest.rd == 0 && (instr.rest.i == 1 || instr.rest.rs2 == 0)
	     && prev2.sethi.op == FMT2op && prev2.sethi.op2 == SETHIop2 
	     && prev2.sethi.rd == (unsigned)jumpreg
	     && prev1.rest.op == RESTop 
	     && prev1.rest.rd == (unsigned)jumpreg && prev1.rest.i == 1
	     && prev1.rest.op3 == ORop3 && prev1.rest.rs1 == (unsigned)jumpreg){
	     
	     targetAddr = (prev2.sethi.imm22 << 10) & 0xfffffc00;
	     targetAddr |= prev1.resti.simm13;
	     if ((targetAddr < getAddress(0)) 
		 || (targetAddr >= (getAddress(0)+size()))) {
		 instPoint *point = new instPoint(this, instr, owner, 
						  newAdr, false,
						  functionExit, adr);
		 funcReturns += point;
		 funcReturns[retId] -> instId = retId++;
	     }
	 }
     }
 }
 return true;
}

// This function assigns new address to instrumentation points of  
// a function that has been relocated
bool pdFunction::findNewInstPoints(const image *owner, 
				const instPoint *&location,
				Address newAdr,
				process *proc,
				vector<instruction> &callInstrs,
				relocatedFuncInfo *reloc_info) {

   int i;
   if (size() == 0) {
     return false;
   }
   assert(reloc_info);

   Address adr = getAddress(0);
   instruction instr;
   instr.raw = owner->get_instruction(adr);
   if (!IS_VALID_INSN(instr))
     return false;

   instPoint *point = new instPoint(this, instr, owner, newAdr, true, 
				    functionEntry, adr);
   point->relocated_ = true;
   // if location was the entry point then change location's value to new pt
   if(location == funcEntry_) { 
	location = point; 
   }

   reloc_info->addFuncEntry(point);
   assert(reloc_info->funcEntry());
   int retId = 0;
   int callsId = 0; 

   // get baseAddress if this is a shared object
   Address baseAddress = 0;
   if(!(proc->getBaseAddress(owner,baseAddress))){
	baseAddress =0;
   }

   // if we have call instructions that need to be added after the instrs
   // to call the relocated instruction, the first address we can use is
   // the address of the 4th instruction in the function
   Address call_start_addr = getAddress(0)+baseAddress + 3*sizeof(instruction);

   for (i = 0; adr < getAddress(0) + size(); adr += 4,  newAdr += 4, i++) {
    
     instr.raw = owner->get_instruction(adr);
     newInstr[i] = instr;

     bool done;

     // check for return insn and as a side affect decide if we are at the
     //   end of the function.
     if (isReturnInsn(owner, adr, done)) {
       // define the return point
       instPoint *point	= new instPoint(this, instr, owner, newAdr, false, 
					functionExit, adr);
       point->relocated_ = true;
       // if location was this point, change it to new point
       if(location == funcReturns[retId]) { 
	   location = point; 
       }
       retId++;
       reloc_info->addFuncReturn(point);
     } else if (instr.branch.op == 0 
		&& (instr.branch.op2 == 2 || instr.branch.op2 == 6)) {
       // find if this branch is going out of the function
       int disp = instr.branch.disp22;
       Address target = adr + baseAddress + (disp << 2);
       if ((target < (getAddress(0) + baseAddress)) 
	   || (target >= (getAddress(0) + baseAddress + size()))) {
	   relocateInstruction(&newInstr[i],adr+baseAddress,newAdr,proc);
	   instPoint *point = new instPoint(this, newInstr[i], owner, 
					    newAdr, false, 
					    functionExit, adr);
           point->relocated_ = true;
	   disp = newInstr[i].branch.disp22;
	   if ((instr.branch.cond != 0) && (instr.branch.cond != 8)) {  
	       point->isBranchOut = true;
	       point->branchTarget = adr + (disp<<2);
	   }
           // if location was this point, change it to new point
           if(location == funcReturns[retId]) { 
	       location = point;
           }
           retId++;
           reloc_info->addFuncReturn(point);
       }

     } else if (isCallInsn(instr)) {

       // first, check for tail-call optimization: a call where the instruction 
       // in the delay slot write to register %o7(15), usually just moving
       // the caller's return address, or doing a restore
       // Tail calls are instrumented as return points, not call points.
       instruction nexti; 
       nexti.raw = owner->get_instruction(adr+4);

       if (nexti.rest.op == 2 
	   && ((nexti.rest.op3 == ORop3 && nexti.rest.rd == 15)
	      || nexti.rest.op3 == RESTOREop3)) {

	    // Undoing the tail-call optimazation when the function
	    // is relocated. Here is an example:
	    //   before:          --->             after
	    // ---------------------------------------------------
	    //   call  %g1                        restore    
	    //   restore                          st  %i0, [ %fp + 0x44 ]
	    //                                    mov %o7 %i0
	    //                                    call %g1 
	    //                                    nop
	    //                                    mov %i0,%o7
	    //                                    st  [ %fp + 0x44 ], %i0
	    //         			    retl
            //                                    nop
	    // Q: Here the assumption that register i1 is available 
	    //    might be an question, is it?
	    // A: I think it is appropriate because:
	    //      (in situation A calls B and B calls C)
	    //      The procedure C called via tail call is a leaf 
	    //      procedure, the value arguments and return value between
	    //      A and C are passed by register (o1...o5, o7)
	    //      So even If B mess up the value of i0, it won't affect the
	    //      commnucation between A and C. Also, we saved the value of
	    //      i0 on stack and when we return from B, the value of i0
	    //      won't be affected.
	    //      If C is not a leaf procedure, it should be fine
	    //      as it is.
	    //    ( If you could give an counter-example, please
	    //      let me know.                         --ling )

	    genSimpleInsn(&newInstr[i++], RESTOREop3, 0, 0, 0);
	    generateStore(&newInstr[i++], 24, REG_FP, 0x44); 
	    genImmInsn(&newInstr[i++], ORop3, 15, 0, 24); 
	    newInstr[i++].raw = owner->get_instruction(adr);
	    generateNOOP(&newInstr[i++]);
	    genImmInsn(&newInstr[i++], ORop3, 24, 0, 15);
	    generateLoad(&newInstr[i++], REG_FP, 0x44, 24);  	    
	    generateJmplInsn(&newInstr[i++], 15, 8 ,0);
	    newAdr += 28;
	    generateNOOP(&newInstr[i]);
	    instPoint *point = new instPoint(this, instr, owner, newAdr, false,
				      functionExit, adr);
	    point-> originalInstruction = newInstr[i-1];
	    point-> delaySlotInsn = newInstr[i];
	    point-> isDelayed = true;
            point->relocated_ = true;
            // if location was this point, change it to new point
            if(location == funcReturns[retId]) { 
		location = point;
            }
            retId++;
            reloc_info->addFuncReturn(point);
       } else {
	 // if the second instruction in the function is a call instruction
         // then this cannot go in the delay slot of the branch to the
         // base tramp, so add a noop between first and second instructions
	 // in the relocated function (check out write in libc.so.1 for
	 // and example of this):
	 //
	 //     save  %sp, -96, %sp             brach to base tramp
	 //     call  0x73b70                   nop
	 //                                     call 0x73b70
	 if(adr == (getAddress(0)+4)){
	     newInstr[i+1] = instr;
	     generateNOOP(&newInstr[i]);
	     i++;
	     newAdr += 4;
	 }

	 // if this is a call to an address within the same function, then
	 // we need to set the 07 register to have the same value as it
	 // would before the function was relocated
	 // to do this we generate a call instruction back to the original
	 // function location, and then at this location we generate a call 
	 // instruction back to the relocated instruction.  In the delay 
	 // slot of the second instruction the value of 07 is changed by 
	 // the difference between the origninal call instruction, and 
	 // the location of the call instruction back to the relocated
	 // function.  This way the 07 register will contain the address
	 // of the original call instruction
	 Address call_target = adr + (instr.call.disp30 << 2);
         if((call_target >= getAddress(0)) 
		&& (call_target <= (getAddress(0) + size()))){ 
	    assert((newInstr[i].call.disp30 << 2) == 8);

	    // generating call instruction to orginal function address
	    // after the SAVE call RESTORE instr.s that call the relocated
	    // function 
            newInstr[i].call.disp30 = ((call_start_addr -newAdr) >> 2); 

	    // generate call to relocated function from original function 
	    // (this will get almost correct value for register 07)
            instruction new_inst;
	    generateCallInsn(&new_inst,call_start_addr,
			     newAdr+sizeof(instruction));
            callInstrs += new_inst;
	   
	    // generate add isntruction to get correct value for 07 register 
	    // this will go in delay slot of previous call instr.
	    genImmInsn(&new_inst,ADDop3,REG_O7,
		       (adr+baseAddress-call_start_addr),REG_O7);
            callInstrs += new_inst;
	    call_start_addr += 2*sizeof(instruction);
         }
	 else {
	    // otherwise, this is a call instruction to a location
	    // outside the function
	    bool err;
	    relocateInstruction(&newInstr[i],adr+baseAddress,newAdr,proc);
	    Address temp = newCallPoint(newAdr, newInstr[i], owner, err, 
					callsId, adr,reloc_info,location);
	    if (err) return false;
         }
       }
     }

     else if (isInsnType(instr, JMPLmask, JMPLmatch)) {
       /* A register indirect jump. Some jumps may exit the function 
          (e.g. read/write on SunOS). In general, the only way to 
	  know if a jump is exiting the function is to instrument
	  the jump to test if the target is outside the current 
	  function. Instead of doing this, we just check the 
	  previous two instructions, to see if they are loading
	  an address that is out of the current function.
	  This should catch the most common cases (e.g. read/write).
	  For other cases, we would miss a return point.

	  This is the case considered:

	     sethi addr_hi, r
	     or addr_lo, r, r
	     jump r
	*/

	 reg jumpreg = instr.rest.rs1;
	 instruction prev1;
	 instruction prev2;
	 
	 prev1.raw = owner->get_instruction(adr-4);
	 prev2.raw = owner->get_instruction(adr-8);

	 unsigned targetAddr;

	 if (instr.rest.rd == 0 && (instr.rest.i == 1 || instr.rest.rs2 == 0)
	     && prev2.sethi.op == FMT2op && prev2.sethi.op2 == SETHIop2 
	     && prev2.sethi.rd == (unsigned)jumpreg
	     && prev1.rest.op == RESTop 
	     && prev1.rest.rd == (unsigned)jumpreg && prev1.rest.i == 1
	     && prev1.rest.op3 == ORop3 && prev1.rest.rs1 == (unsigned)jumpreg){
	     
	     targetAddr = (prev2.sethi.imm22 << 10) & 0xfffffc00;
	     targetAddr |= prev1.resti.simm13;
	     if ((targetAddr < getAddress(0)) 
		 || (targetAddr >= (getAddress(0)+size()))) {
		 instPoint *point = new instPoint(this, instr, owner, 
						  newAdr, false,
						  functionExit, adr);
                 point->relocated_ = true;
                 // if location was this point, change it to new point
                 if(location == funcReturns[retId]) { 
		     location = point;
                 }
                 retId++;
                 reloc_info->addFuncReturn(point);
	     }
	 }
     }
 }
   
   return true;
}

//
// called to relocate a function: when a request is made to instrument
// a system call we relocate the entire function into the heap
//
bool pdFunction::relocateFunction(process *proc, 
				 const instPoint *&location,
				 vector<instruction> &extra_instrs) {

    relocatedFuncInfo *reloc_info = 0;
    // check to see if this process already has a relocation record 
    // for this function
    for(u_int i=0; i < relocatedByProcess.size(); i++){
	if((relocatedByProcess[i])->getProcess() == proc){
	    reloc_info = relocatedByProcess[i];
	}
    }
    u_int ret = 0;
    if(!reloc_info){
        //Allocate the heap for the function to be relocated
        ret = inferiorMalloc(proc, size()+32, textHeap);
	if(!ret)  return false;
        reloc_info = new relocatedFuncInfo(proc,ret);
	relocatedByProcess += reloc_info;
    }
    if(!(findNewInstPoints(location->image_ptr, location, ret, proc, 
			   extra_instrs, reloc_info))){
    }
    proc->writeDataSpace((caddr_t)ret, size()+32,(caddr_t) newInstr);
    return true;
}

// modifyInstPoint: if the function associated with the process was 
// recently relocated, then the instPoint may have the old pre-relocated
// address (this can occur because we are getting instPoints in mdl routines 
// and passing these to routines that do the instrumentation, it would
// be better to let the routines that do the instrumenting find the points)
void pdFunction::modifyInstPoint(const instPoint *&location,process *proc)
{

    if(relocatable_ && !(location->relocated_)){
        for(u_int i=0; i < relocatedByProcess.size(); i++){
	    if((relocatedByProcess[i])->getProcess() == proc){
		if(location->ipType == functionEntry){
		    const instPoint *new_entry = 
				(relocatedByProcess[i])->funcEntry();
		    location = new_entry;
		} 
		else if(location->ipType == functionExit){
		    const vector<instPoint *> new_returns = 
			(relocatedByProcess[i])->funcReturns(); 
                    assert(funcReturns.size() == new_returns.size());
                    for(u_int j=0; j < new_returns.size(); j++){
			if(funcReturns[j] == location){
			    location = (new_returns[j]);
			    break;
			}
		    }
		}
		else {
		    const vector<instPoint *> new_calls = 
				(relocatedByProcess[i])->funcCallSites(); 
                    assert(calls.size() == new_calls.size());
                    for(u_int j=0; j < new_calls.size(); j++){
			if(calls[j] == location){
			    location = (new_calls[j]);
			    break;
			}
		    }

		}
 		break;
    } } }
}


// The exact semantics of the heap are processor specific.
//
// find all DYNINST symbols that are data symbols
//
bool image::heapIsOk(const vector<sym_data> &find_us) {
  Symbol sym;

  for (unsigned i=0; i<find_us.size(); i++) {
    const string &str = find_us[i].name;
    if (!linkedFile.get_symbol(str, sym)) {
      string str1 = string("_") + str.string_of();
      if (!linkedFile.get_symbol(str1, sym) && find_us[i].must_find) {
	string msg;
        msg = string("Cannot find ") + str + string(". Exiting");
	statusLine(msg.string_of());
	showErrorCallback(50, msg);
	return false;
      }
    }
    addInternalSymbol(str, sym.addr());
  }

  string ghb = GLOBAL_HEAP_BASE;
  if (!linkedFile.get_symbol(ghb, sym)) {
    ghb = U_GLOBAL_HEAP_BASE;
    if (!linkedFile.get_symbol(ghb, sym)) {
      string msg;
      msg = string("Cannot find ") + ghb + string(". Exiting");
      statusLine(msg.string_of());
      showErrorCallback(50, msg);
      return false;
    }
  }
  Address instHeapEnd = sym.addr();
  addInternalSymbol(ghb, instHeapEnd);
  string ihb = INFERIOR_HEAP_BASE;

  if (!linkedFile.get_symbol(ihb, sym)) {
    ihb = UINFERIOR_HEAP_BASE;
    if (!linkedFile.get_symbol(ihb, sym)) {
      string msg;
      msg = string("Cannot find ") + ihb + string(". Cannot use this application");
      statusLine(msg.string_of());
      showErrorCallback(50, msg);
      return false;
    }
  }
  Address curr = sym.addr();
  addInternalSymbol(ihb, curr);
  if (curr > instHeapEnd) instHeapEnd = curr;

  // check that we can get to our heap.
  //if (instHeapEnd > getMaxBranch()) {
  //  logLine("*** FATAL ERROR: Program text + data too big for dyninst\n");
  //  sprintf(errorLine, "    heap ends at %x\n", instHeapEnd);
  //  logLine(errorLine);
  //  return false;
  //} else if (instHeapEnd + SYN_INST_BUF_SIZE > getMaxBranch()) {
  //  logLine("WARNING: Program text + data could be too big for dyninst\n");
  //  showErrorCallback(54, "");
  //  return false;
  //}
  return true;
}

// Certain registers (i0-i7 on a SPARC) may be available to be read
// as an operand, but cannot be written.
bool registerSpace::readOnlyRegister(reg reg_number) {
  if ((reg_number < 16) || (reg_number > 23))
      return true;
  else
      return false;
}

bool returnInstance::checkReturnInstance(const vector<Address> adr,u_int &index)
{
    for(u_int i=0; i < adr.size(); i++){
	index = i;
        if ((adr[i] > addr_) && ( adr[i] <= addr_+size_)){
	    return false;
        }
    }
    return true;
}

void returnInstance::installReturnInstance(process *proc) {
    proc->writeTextSpace((caddr_t)addr_, instSeqSize, 
			 (caddr_t) instructionSeq); 
}

void generateBreakPoint(instruction &insn) {
    insn.raw = BREAK_POINT_INSN;
}

void returnInstance::addToReturnWaitingList(Address pc, process *proc) {

    // if there already is a TRAP set at this pc for this process don't
    // generate a trap instruction again...you will get the wrong original
    // instruction if you do a readDataSpace
    bool found = false;
    instruction insn;
    for(u_int i=0; i < instWList.size(); i++){
         if(((instWList[i])->pc_ == pc)&&((instWList[i])->which_proc == proc)){
	     found = true;
	     insn = (instWList[i])->relocatedInstruction;
	     break;
    } }
    if(!found) {
        instruction insnTrap;
        generateBreakPoint(insnTrap);
        proc->readDataSpace((caddr_t)pc, sizeof(insn), (char *)&insn, true);
        proc->writeTextSpace((caddr_t)pc, sizeof(insnTrap), (caddr_t)&insnTrap);
    }
    else {
    }

    instWaitingList *instW = new instWaitingList(instructionSeq,instSeqSize,
				     addr_,pc,insn,pc,proc);
    instWList += instW;

}


bool doNotOverflow(int value)
{
  // we are assuming that we have 13 bits to store the immediate operand.
  if ( (value <= 16383) && (value >= -16384) ) return(true);
  else return(false);
}

void instWaitingList::cleanUp(process *proc, Address pc) {
    proc->writeTextSpace((caddr_t)pc, sizeof(relocatedInstruction),
		    (caddr_t)&relocatedInstruction);
    proc->writeTextSpace((caddr_t)addr_, instSeqSize, (caddr_t)instructionSeq);
}
