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
 * inst-x86.C - x86 dependent functions and code generator
 *
 * $Log: inst-x86.C,v $
 * Revision 1.1  1996/10/18 23:54:14  mjrg
 * Solaris/X86 port
 *
 *
 *
 */

#include "util/h/headers.h"

#include "rtinst/h/rtinst.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "ast.h"
#include "util.h"
#include "stats.h"
#include "os.h"
#include "showerror.h"

#include "arch-x86.h"

extern bool isPowerOf2(int value, int &result);

// The general machine registers. 
// These values are taken from the Pentium manual and CANNOT be changed.
#define EAX (0)
#define ECX (1)
#define EDX (2)
#define EBX (3)
#define ESP (4)
#define EBP (5)
#define ESI (6)
#define EDI (7)

// Size of a jump rel32 instruction
#define JUMP_REL32_SZ (5)
#define JUMP_SZ (5)

#define PUSH_RM_OPC1 (0xFF)
#define PUSH_RM_OPC2 (6)
#define CALL_RM_OPC1 (0xFF)
#define CALL_RM_OPC2 (2)
#define PUSH_EBP (0x50+EBP)
#define SUB_REG_IMM32 (5)
#define LEAVE (0xC9)

/*
   Function arguments are in the stack and are addressed with a displacement
   from EBP. EBP points to the saved EBP, EBP+4 is the saved return address,
   EBP+8 is the first parameter.
   TODO: what about far calls?
 */
#define PARAM_OFFSET (8)

// offset from EBP of the saved EAX for a tramp
#define SAVED_EAX_OFFSET (-4)



/************************************************************************
 *
 *  class instPoint: representation of instrumentation points
 *
 ***********************************************************************/

class instPoint {

 public:
  instPoint(pdFunction *f, /*const*/ image *im, Address adr, instruction inst) {
    addr = adr;
    jumpAddr = adr;
    func = f;
    owner = (image *)im;
    callee = NULL;
    insnAtPoint = inst;
    insnsBefore = 0;
    insnsAfter = 0;
    tSize = inst.size();
    returnAddr_ = adr + tSize;
    checked = false;
  };

  ~instPoint() {};

  // add an instruction before the point. Instructions should be added in reverse
  // order, the instruction closest to the point first.
  void addInstrBeforePt(instruction inst) {
    assert(insnsBefore < 4);
    insnBeforePt[insnsBefore++] = inst;
  };

  // add an instruction after the point.
  void addInstrAfterPt(instruction inst) {
    assert(insnsAfter < 4);
    insnAfterPt[insnsAfter++] = inst;
  }

  Address returnAddr() const { return returnAddr_; }

  // return the size of all instructions in this point
  // size may change after point is checked
  unsigned size() const { return tSize; }

  // check for jumps to instructions before and/or after this point, and discard
  // instructions when there is a jump.
  // Can only be done after an image has been parsed (that is, all inst. points
  // in the image have been found.
  void checkInstructions();

  // can't set this in the constructor because call points can't be classified until
  // all functions have been seen -- this might be cleaned up
  void set_callee(pdFunction *to) { callee = to; }


// private:

  Address addr;          // the address of this instPoint: this is the address
                         // of the actual point (i.e. a function entry point,
			 // a call or a return instruction)
  Address jumpAddr;      // this is the address where we insert the jump. It may
                         // be an instruction before the point

  pdFunction *func;	 // the function where this instPoint belongs to
  pdFunction *callee;	 // if this point is a call, the function being called
  image *owner;          // the image to which this point belongs to

  instruction insnAtPoint;       // the instruction at this point
  
  instruction insnBeforePt[4];   // additional instructions before the point
  unsigned insnsBefore;          // number of instructions before point

  instruction insnAfterPt[4];    // additional instructions after the point
  unsigned insnsAfter;           // number of instructions after point

private:
  unsigned tSize;                // total size of all instructions at this point
  Address returnAddr_;           // the address to where the basetramp returns

  bool checked;           // true if this point has been checked

};


/*
   checkInstructions: check that there are no known jumps to the instructions
   before and after the point.
*/
void instPoint::checkInstructions() {
  unsigned currAddr = addr;
  unsigned OKinsns = 0;

  if (checked)
    return;
  checked = true;

  tSize = insnAtPoint.size();

  if (!owner->isJumpTarget(currAddr)) {
    // check instructions before point
    for (unsigned u = 0; u < insnsBefore; u++) {
      OKinsns++;
      tSize += insnBeforePt[u].size();
      currAddr -= insnBeforePt[u].size();
      if (owner->isJumpTarget(currAddr)) {
	// must remove instruction from point
	//fprintf(stderr, "check instructions point %x, jmp to %x\n", addr,currAddr);
	break;
      }
    }
  }

  insnsBefore = OKinsns;
  // this is the address where we insert the jump
  jumpAddr = currAddr;

  // check instructions after point
  currAddr = addr + insnAtPoint.size();
  OKinsns = 0;
  for (unsigned u = 0; tSize < 5 && u < insnsAfter; u++) {
    if (owner->isJumpTarget(currAddr))
      break;
    OKinsns++;
    unsigned size = insnAfterPt[u].size();
    currAddr += size;
    returnAddr_ += size;
    tSize += size;
  }
  insnsAfter = OKinsns;

  if (tSize < 5) {
    tSize = insnAtPoint.size();
    jumpAddr = addr;
    returnAddr_ = addr + insnAtPoint.size();
    insnsBefore = 0;
    insnsAfter = 0;
  }

}


/**************************************************************
 *
 *  machine dependent methods of pdFunction
 *
 **************************************************************/

// Determine if the called function is a "library" function or a "user" function
// This cannot be done until all of the functions have been seen, verified, and
// classified
//
void pdFunction::checkCallPoints() {
  unsigned int i;
  instPoint *p;
  Address loc_addr;

  vector<instPoint*> non_lib;

  for (i=0; i<calls.size(); ++i) {
    /* check to see where we are calling */
    p = calls[i];
    assert(p);

    if (!p->insnAtPoint.isCallIndir()) {
      loc_addr = p->insnAtPoint.getTarget(p->addr);
      file()->exec()->addJumpTarget(loc_addr);
      pdFunction *pdf = (file_->exec())->findFunction(loc_addr);

      if (pdf && !pdf->isLibTag()) {
        p->callee = pdf;
        non_lib += p;
      } else {
	delete p;
      }
    } else {
      // Indirect call -- be conservative, assume it is a call to
      // an unnamed user function
      assert(!p->callee);
      p->callee = NULL;
      non_lib += p;
    }
  }
  calls = non_lib;

}

// this function is not needed
Address pdFunction::newCallPoint(const Address, const instruction,
				 const image *, bool &)
{ assert(0); }


// see if we can recognize a jump table and skip it
bool checkJumpTable(image *im, instruction insn, Address addr, 
		    Address funcBegin, Address funcEnd,
		    unsigned &tableSz) {

  unsigned char *instr = insn.ptr();
  tableSz = 0;
  /*
     the instruction usually used for jump tables is 
       jmp dword ptr [REG*4 + ADDR]
     where ADDR is an immediate following the SIB byte.
     The opcode is 0xFF and the MOD/RM byte is 0x24. 
     The SS field (bits 7 and 6) of SIB is 2, and the
     base ( bits 2, 1, 0) is 5. The index bits (5,4,3) 
     select the register.
  */
  if (instr[0] == 0xFF && instr[1] == 0x24 &&
      ((instr[2] & 0xC0)>>6) == 2 && (instr[2] & 0x7) == 5) {
    unsigned tableBase = *(int *)(instr+3);
    //fprintf(stderr, "Found jump table at %x %x\n",addr, tableBase);
    // check if the table is right after the jump and inside the current function
    if (tableBase > funcBegin && tableBase < funcEnd) {
      // table is within function code
      if (tableBase != addr+insn.size()) {
	fprintf(stderr, "bad indirect jump at %x\n", addr);
	return false;
      }
      // skip the jump table
      for (unsigned *ptr = (unsigned *)tableBase; 
	   *ptr >= funcBegin && *ptr <= funcEnd; ptr++) {
	//fprintf(stderr, " jump table entry = %x\n", *(unsigned *)ptr);
	tableSz += sizeof(int);
      }
    }
    else {
      unsigned char *ptr = im->getPtrToInstruction(tableBase);
      for ( ; *(unsigned *)ptr >= funcBegin && *(unsigned *)ptr <= funcEnd; 
	   ptr += sizeof(unsigned)) {
	//fprintf(stderr, " jump table entry = %x\n", *(unsigned *)ptr);
      }
    }
  }
  return true;
}


bool pdFunction::findInstPoints(image *owner) {

   enum { EntryPt, CallPt, ReturnPt };
   class point_ {
   public:
     point_(): point(0), index(0), type(0) {};
     point_(instPoint *p, unsigned i, unsigned t): point(p), index(i), type(t) {};
     instPoint *point;
     unsigned index;
     unsigned type;
   };

   point_ *points = new point_[size()];
   unsigned npoints = 0;

   if (size() == 0) {
     //fprintf(stderr,"Function %s, size = %d\n", prettyName().string_of(), size());
     return false;
   }

// XXXXX kludge: these functions are called by DYNINSTgetCPUtime, 
// they can't be instrumented or we would have an infinite loop
if (prettyName() == "gethrvtime" || prettyName() == "_divdi3")
  return false;

   unsigned char *instr = owner->getPtrToInstruction(addr());
   Address adr = addr();
   unsigned numInsns = 0;

   instruction insn;
   unsigned insnSize;

   // keep a buffer with all the instructions in this function
   instruction *allInstr = new instruction[size()+5];

   // define the entry point
   insnSize = insn.getNextInstruction(instr);
   instPoint *p = new instPoint(this, owner, adr, insn);
   funcEntry_ = p;
   points[npoints++] = point_(p, numInsns, EntryPt);

   // check if the entry point contains another point
   if (insn.isJumpDir()) {
     Address target = insn.getTarget(adr);
     owner->addJumpTarget(target);
     if (target <= addr() && target >= addr() + size()) {
       // jump out of function
       // this is an empty function
       return false;
     }
   } else if (insn.isReturn()) {
     // this is an empty function
     return false;
   } else if (insn.isCall()) {
     // TODO: handle calls at entry point
     // call at entry point
     //instPoint *p = new instPoint(this, owner, adr, insn);
     //calls += p;
     //points[npoints++] = point_(p, numInsns, CallPt);
     //fprintf(stderr,"Function %s, call at entry point\n", prettyName().string_of());
     return false;
   }

   allInstr[numInsns] = insn;
   numInsns++;
   instr += insnSize;
   adr += insnSize;

   // get all the instructions for this function, and define the instrumentation
   // points. For now, we only add one instruction to each point.
   // Additional instructions, for the points that need them, will be added later.

   Address funcEnd = addr() + size();
   for ( ; adr < funcEnd; instr += insnSize, adr += insnSize) {
     insnSize = insn.getNextInstruction(instr);
     assert(insnSize > 0);

     if (adr + insnSize > funcEnd) {
       break;
     }

     if (insn.isJumpIndir()) {
       unsigned jumpTableSz;
       if (!checkJumpTable(owner, insn, adr, addr(), addr()+size(), jumpTableSz)) {
	 delete points;
	 delete allInstr;
         //fprintf(stderr,"Function %s, size = %d, bad jump table\n", 
	 //          prettyName().string_of(), size());
	 return false;
       }
       // process the jump instruction
       allInstr[numInsns] = insn;
       numInsns++;
       // skip the jump table
       // insert an illegal instruction with the size of the jump table
       insn = instruction(instr, ILLEGAL, jumpTableSz);

     } else if (insn.isJumpDir()) {
       // check for jumps out of this function
       Address target = insn.getTarget(adr);
       owner->addJumpTarget(target);
       if (target < addr() || target >= addr() + size()) {
	 // jump out of function
	 instPoint *p = new instPoint(this, owner, adr, insn);
	 funcReturns += p;
	 points[npoints++] = point_(p, numInsns, ReturnPt);
       } 

     } else if (insn.isReturn()) {
       instPoint *p = new instPoint(this, owner, adr, insn);
       funcReturns += p;
       points[npoints++] = point_(p, numInsns, ReturnPt);

     } else if (insn.isCall()) {
       instPoint *p = new instPoint(this, owner, adr, insn);
       calls += p;
       points[npoints++] = point_(p, numInsns, CallPt);
     }

     allInstr[numInsns] = insn;
     numInsns++;
     assert(npoints < size());
     assert(numInsns < size());
   }

   // there are often nops after the end of the function. We get them here,
   // since they may be usefull to instrument the return point
   for (unsigned u = 0; u < 4; u++) {
     if (owner->isValidAddress(adr)) {
       insnSize = insn.getNextInstruction(instr);
       if (insn.isNop()) {
	 allInstr[numInsns] = insn;
	 numInsns++;
	 assert(numInsns < size()+5);
	 instr += insnSize;
	 adr += insnSize;
       }
       else
	 break;
     }
   }

   // add extra instructions to the points that need it.
   unsigned lastPointEnd = 0;
   unsigned thisPointEnd = 0;
   for (unsigned u = 0; u < npoints; u++) {
     instPoint *p = points[u].point;
     unsigned index = points[u].index;
     unsigned type = points[u].type;
     lastPointEnd = thisPointEnd;
     thisPointEnd = index;

     // add instructions before the point
     unsigned size = p->size();
     for (int u1 = index-1; size < JUMP_SZ && u1 >= 0 && u1 > (int)lastPointEnd; u1--) {
       if (!allInstr[u1].isCall()) {
	 p->addInstrBeforePt(allInstr[u1]);
	 size += allInstr[u1].size();
       } else
	 break;
     }

     lastPointEnd = index;
     // add instructions after the point
     if (type == ReturnPt) {
       // normally, we would not add instructions after the return, but the 
       // compilers often add nops after the return, and we can use them if necessary
       for (unsigned u1 = index+1; u1 < index+JUMP_SZ-1 && u1 < numInsns; u1++) {
	 if (allInstr[u1].isNop()) {
	   p->addInstrAfterPt(allInstr[u1]);
	   thisPointEnd = u1;
	 }
	 else 
	   break;
       }
     } else {
       size = p->size();
       for (unsigned u1 = index+1; size < JUMP_SZ && u1 <= numInsns; u1++) {
	 if (u+1 < npoints && points[u+1].index > u1 && !allInstr[u1].isCall()) {
	   p->addInstrAfterPt(allInstr[u1]);
	   size += allInstr[u1].size();
	   thisPointEnd = u1;
	 }
	 else 
	   break;
       }
     }
   }

   delete points;
   delete allInstr;

   return true;
}



/*
 * Given an instruction, relocate it to a new address, patching up
 *   any relative addressing that is present.
 * The instruction may need to be replaced with a different size instruction
 * or with multiple instructions.
 * Return the size of the new instruction(s)
 */
unsigned relocateInstruction(instruction insn,
			 int origAddr, int newAddr,
			 unsigned char *&newInsn)
{
  /* 
     Relative address instructions need to be modified. The relative address
     can be a 8, 16, or 32-byte displacement relative to the next instruction.
     Since we are relocating the instruction to a different area, we have
     to replace 8 and 16-byte displacements with 32-byte displacements.

     All relative address instructions are one or two-byte opcode followed
     by a displacement relative to the next instruction:

       CALL rel16 / CALL rel32
       Jcc rel8 / Jcc rel16 / Jcc rel32
       JMP rel8 / JMP rel16 / JMP rel32

     The only two-byte opcode instructions are the Jcc rel16/rel32,
     all others have one byte opcode.

     The instruction JCXZ/JECXZ rel8 does not have an equivalent with rel32
     displacement. We must generate code to emulate this instruction:
     
       JCXZ rel8

     becomes

       A0: JCXZ 2 (jump to A4)
       A2: JMP 5  (jump to A9)
       A4: JMP rel32 (relocated displacement)
       A9: ...

  */

  unsigned char *origInsn = insn.ptr();
  unsigned insnType = insn.type();
  unsigned insnSz = insn.size();
  unsigned char *first = newInsn;

  int oldDisp;
  int newDisp;

  if (insnType & REL_B) {
    /* replace with rel32 instruction, opcode is one byte. */
    if (*origInsn == JCXZ) {
      oldDisp = (int)*(char *)(origInsn+1);
      newDisp = (origAddr + 2) + oldDisp - (newAddr + 9);
      *newInsn++ = *origInsn; *(newInsn++) = 2; // jcxz 2
      *newInsn++ = 0xE8; *newInsn++ = 5;        // jmp 5
      *newInsn++ = 0xE9;                        // jmp rel32
      *((int *)newInsn) = newDisp;
      newInsn += sizeof(int);
    }
    else {
      unsigned newSz;
      if (insnType & IS_JCC) {
	/* Change a Jcc rel8 to Jcc rel32. 
	   Must generate a new opcode: a 0x0F followed by (old opcode + 16) */
	unsigned char opcode = *origInsn++;
	*newInsn++ = 0x0F;
	*newInsn++ = opcode + 0x10;
	newSz = 6;
      }
      else if (insnType & IS_JUMP) {
	/* change opcode to 0xE9 */
	origInsn++;
	*newInsn++ = 0xE9;
	newSz = 5;
      }
      oldDisp = (int)*(char *)origInsn;
      newDisp = (origAddr + 2) + oldDisp - (newAddr + newSz);
      *((int *)newInsn) = newDisp;
      newInsn += sizeof(int);
    }
  }
  else if (insnType & REL_W) {
    /* Skip prefix. The only valid prefix here is operand size. */
    if (insnType & PREFIX_OPR)
      origInsn++;
    /* opcode is unchanged, just relocate the displacement */
    if (*origInsn == (unsigned char)0x0F)
      *newInsn++ = *origInsn++;
    *newInsn++ = *origInsn++;
    oldDisp = *((short *)origInsn);
    newDisp = (origAddr + 5) + oldDisp - (newAddr + 3);
    *((int *)newInsn) = newDisp;
    newInsn += sizeof(int);
  } else if (insnType & REL_D) {
    /* opcode is unchanged, just relocate the displacement */
    if (*origInsn == 0x0F)
      *newInsn++ = *origInsn++;
    *newInsn++ = *origInsn++;
    oldDisp = *((int *)origInsn);
    newDisp = (origAddr + 5) + oldDisp - (newAddr + 5);
    *((int *)newInsn) = newDisp;
    newInsn += sizeof(int);
  }
  else {
    /* instruction is unchanged */
    for (unsigned u = 0; u < insnSz; u++)
      *newInsn++ = *origInsn++;
  }

  return (newInsn - first);
}

unsigned getRelocatedInstructionSz(instruction insn)
{
  unsigned char *origInsn = insn.ptr();
  unsigned insnType = insn.type();
  unsigned insnSz = insn.size();

  if (insnType & REL_B) {
    if (*origInsn == JCXZ)
      return 9;
    else {
      if (insnType & IS_JCC)
	return 6;
      else if (insnType & IS_JUMP) {
	return 5;
      }
    }
  }
  else if (insnType & REL_W) {
    return insnSz + 2;
  }
  return insnSz;
}


registerSpace *regSpace;


bool registerSpace::readOnlyRegister(reg) {
  return false;
}

/*
   We don't use the machine registers to store temporaries.
   The "registers" are really locations on the stack.
   The stack frame for a tramp is:

     ebp->   saved ebp (4 bytes)
     ebp-4:  saved registers (8*4 bytes)
     ebp-36: saved flags registers (4 bytes)
     ebp-40: the temporary registers

     The temporaries are assigned numbers from 10 so that it is easier
     to refer to them.

     We are using a fixed number of temporaries now (32), but we could 
     change to using an arbitrary number.

*/
int deadList[32];

void initTramps()
{
    static bool inited=false;

    if (inited) return;
    inited = true;

    for (unsigned u = 0; u < 32; u++) {
      deadList[u] = u+10;
    }

    regSpace = new registerSpace(sizeof(deadList)/sizeof(int), deadList,					 0, NULL);
}

void emitJump(unsigned disp32, unsigned char *&insn);
void emitSimpleInsn(unsigned opcode, unsigned char *&insn);
void emitMovRegToReg(reg dest, reg src, unsigned char *&insn);
void emitAddMemImm(Address dest, int imm, unsigned char *&insn);

/*
 * change the insn at addr to be a branch to newAddr.
 *   Used to add multiple tramps to a point.
 */
void generateBranch(process *proc, unsigned fromAddr, unsigned newAddr)
{
  unsigned char inst[JUMP_REL32_SZ+1];
  unsigned char *insn = inst;
  emitJump(newAddr - (fromAddr + JUMP_REL32_SZ), insn);
  proc->writeTextSpace((caddr_t)fromAddr, JUMP_REL32_SZ, (caddr_t)inst);
}


bool insertInTrampTable(process *proc, unsigned key, unsigned val) {
  unsigned u;

  // check for overflow of the tramp table. 
  // stop at 95% capacicty to ensure good performance
  if (proc->trampTableItems == (TRAMPTABLESZ - TRAMPTABLESZ/20))
    return false;
  proc->trampTableItems++;
  for (u = HASH1(key); proc->trampTable[u].key != 0; 
       u = (u + HASH2(key)) % TRAMPTABLESZ)
    ;
  proc->trampTable[u].key = key;
  proc->trampTable[u].val = val;

  internalSym *t = proc->symbols->findInternalSymbol("DYNINSTtrampTable",true);
  assert(t);
  return proc->writeTextSpace((caddr_t)t->getAddr()+u*sizeof(trampTableEntry),
		       sizeof(trampTableEntry),
		       (caddr_t)&(proc->trampTable[u]));
}

// generate a jump to a base tramp or a trap
// return the size of the instruction generated
unsigned generateBranchToTramp(process *proc, instPoint *point, unsigned baseAddr, 
			   Address imageBaseAddr, unsigned char *insn) {
  if (point->size() < JUMP_REL32_SZ) {
    // must use trap
    assert(point->jumpAddr == point->addr);
    sprintf(errorLine, "Warning: unable to insert jump in function %s, address %x. Using trap\n",
	    point->func->prettyName().string_of(), point->addr);
    logLine(errorLine);

    if (!insertInTrampTable(proc, point->addr+imageBaseAddr, baseAddr))
      return 0;

    *insn = 0xCC;
    return 1;
  } else {
    // replace instructions at point with jump to base tramp
    emitJump(baseAddr - (point->jumpAddr + imageBaseAddr + JUMP_REL32_SZ), insn);
    return JUMP_REL32_SZ;
  }
}


/*
 * Install a base tramp, relocating the instructions at location
 * The pre and post jumps are filled with a 'jump 0'
 * Return a descriptor for the base tramp.
 *
 */

trampTemplate *installBaseTramp(instPoint *location, process *proc) 
{

/*
   The base tramp:
   addr   instruction             cost
   0:    <relocated instructions before point>
   a = size of relocated instructions before point
   a+0:   jmp a+30 <skip pre insn>  1
   a+5:   push ebp                  1
   a+6:   mov esp, ebp              1
   a+8:   pushad                    5
   a+9:   pushaf                    9
   a+10:  add costAddr, cost        3
   a+17:  jmp <global pre inst>     1
   a+22:  jmp <local pre inst>      1
   a+27:  popaf                    14
   a+28:  popad                     5
   a+29:  leave                     3
   a+30:  <relocated instructions at point>

   b = a +30 + size of relocated instructions at point
   b+0:   jmp b+30 <skip post insn>
   b+5:   push ebp
   b+6:   mov esp, ebp
   b+8:   pushad
   b+9:   pushaf
   a+10:  add costAddr, cost
   b+17:  jmp <global post inst>
   b+22:  jmp <local post inst>
   b+27:  popaf
   b+28:  popad
   b+29:  leave
   b+30:  <relocated instructions after point>

   c:     jmp <return to user code>

   tramp size = 2*30 + 5 + size of relocated instructions
   Make sure to update the size if the tramp is changed

   cost of the base tramp is 2 * (1+1+1+5+9+3+1+1+14+5+3) + 1 = 89
*/

  trampTemplate *ret = new trampTemplate;
  ret->trampTemp = 0;

  // check instructions at this point to find how many instructions 
  // we should relocate
  location->checkInstructions();

  // compute the tramp size
  // if there are any changes to the tramp, the size must be updated.
  unsigned trampSize = 65;
  for (unsigned u = 0; u < location->insnsBefore; u++) {
    trampSize += getRelocatedInstructionSz(location->insnBeforePt[u]);
  }
  trampSize += getRelocatedInstructionSz(location->insnAtPoint);
  for (unsigned u = 0; u < location->insnsAfter; u++) {
    trampSize += getRelocatedInstructionSz(location->insnAfterPt[u]);
  }

  Address imageBaseAddr;
  if (!proc->getBaseAddress(location->owner, imageBaseAddr)) {
    abort();
  }

  // get address of DYNINSTobsCostLow to update observed cost
  bool err = false;
  Address costAddr = (proc->symbols)->findInternalAddress("DYNINSTobsCostLow",
		true, err);
  assert(costAddr && !err);

  ret->size = trampSize;
  unsigned baseAddr = inferiorMalloc(proc, trampSize, textHeap);
  ret->baseAddr = baseAddr;

  unsigned char *code = new unsigned char[2*trampSize];
  unsigned char *insn = code;
  unsigned currAddr = baseAddr;

  // get the current instruction that is being executed. If the PC is at a
  // instruction that is being relocated, we must change the PC.
  unsigned currentPC = proc->currentPC();

  // emulate the instructions before the point
  unsigned origAddr = location->jumpAddr + imageBaseAddr;
  for (unsigned u = location->insnsBefore; u > 0; ) {
    --u;
    if (currentPC == origAddr) {
      //fprintf(stderr, "changed PC: %x to %x\n", currentPC, currAddr);
      proc->setNewPC(currAddr);
    }

    unsigned newSize = relocateInstruction(location->insnBeforePt[u], origAddr, currAddr, insn);
    assert(newSize == getRelocatedInstructionSz(location->insnBeforePt[u]));
    currAddr += newSize;
    origAddr += location->insnBeforePt[u].size();
  }

  // pre branches
  // skip pre instrumentation
  ret->skipPreInsOffset = insn-code;
  emitJump(25, insn);
  
  // save registers and create a new stack frame for the tramp
  emitSimpleInsn(PUSH_EBP, insn);  // push ebp
  emitMovRegToReg(EBP, ESP, insn); // mov ebp, esp  (2-byte instruction)
  emitSimpleInsn(PUSHAD, insn);    // pushad
  emitSimpleInsn(PUSHFD, insn);    // pushfd
  // update cost
  emitAddMemImm(costAddr, 41, insn);  // add (costAddr), cost

  // global pre branch
  ret->globalPreOffset = insn-code;
  emitJump(0, insn);

  // local pre branch
  ret->localPreOffset = insn-code;
  emitJump(0, insn);

  ret->localPreReturnOffset = insn-code;

  // restore registers
  emitSimpleInsn(POPFD, insn);     // popfd
  emitSimpleInsn(POPAD, insn);     // popad
  emitSimpleInsn(LEAVE, insn);     // leave

  // emulate the instruction at the point 
  ret->emulateInsOffset = insn-code;
  currAddr = baseAddr + (insn - code);
  assert(origAddr == location->addr + imageBaseAddr);
  origAddr = location->addr + imageBaseAddr;
  if (currentPC == origAddr) {
    //fprintf(stderr, "changed PC: %x to %x\n", currentPC, currAddr);
    proc->setNewPC(currAddr);
  }
  unsigned newSize = relocateInstruction(location->insnAtPoint, origAddr, currAddr, insn);
  assert(newSize == getRelocatedInstructionSz(location->insnAtPoint));
  currAddr += newSize;
  origAddr += location->insnAtPoint.size();


  // post branches
  // skip post instrumentation
  ret->skipPostInsOffset = insn-code;
  emitJump(25, insn);

  // save registers and create a new stack frame for the tramp
  emitSimpleInsn(PUSH_EBP, insn);  // push ebp
  emitMovRegToReg(EBP, ESP, insn); // mov ebp, esp
  emitSimpleInsn(PUSHAD, insn);    // pushad
  emitSimpleInsn(PUSHFD, insn);    // pushfd
  // update cost
  emitAddMemImm(costAddr, 44, insn);  // add(costAddr), cost

  // global post branch
  ret->globalPostOffset = insn-code; 
  emitJump(0, insn);

  // local post branch
  ret->localPostOffset = insn-code;
  emitJump(0, insn);

  ret->localPostReturnOffset = insn-code;

  // restore registers
  emitSimpleInsn(POPFD, insn);     // popfd
  emitSimpleInsn(POPAD, insn);     // popad
  emitSimpleInsn(LEAVE, insn);     // leave
  
  // emulate the instructions after the point
  ret->returnInsOffset = insn-code;
  currAddr = baseAddr + (insn - code);
  assert(origAddr == location->addr + imageBaseAddr + location->insnAtPoint.size());
  origAddr = location->addr + imageBaseAddr + location->insnAtPoint.size();
  for (unsigned u = 0; u < location->insnsAfter; u++) {
    if (currentPC == origAddr) {
      //fprintf(stderr, "changed PC: %x to %x\n", currentPC, currAddr);
      proc->setNewPC(currAddr);
    }
    unsigned newSize = relocateInstruction(location->insnAfterPt[u], origAddr, currAddr, insn);
    assert(newSize == getRelocatedInstructionSz(location->insnAfterPt[u]));
    currAddr += newSize;
    origAddr += location->insnAfterPt[u].size();
  }

  // return to user code
  currAddr = baseAddr + (insn - code);
  emitJump((location->returnAddr() + imageBaseAddr) - (currAddr+5), insn);

  assert((unsigned)(insn-code) == trampSize);
  // put the tramp in the application space
  proc->writeDataSpace((caddr_t)baseAddr, insn-code, (caddr_t) code);

  free(code);
  return ret;
}


// This function is used to clear a jump from base to minitramps
// For the x86 platform, we generate a jump to the next instruction
void generateNoOp(process *proc, int addr) 
{
  static unsigned char jump0[5] = { 0xE9, 0, 0, 0, 0 };
  proc->writeDataSpace((caddr_t) addr, 5, (caddr_t)jump0);
}


trampTemplate *findAndInstallBaseTramp(process *proc, 
				 instPoint *location,
				 returnInstance *&retInstance)
{
    trampTemplate *ret;
    retInstance = NULL;

    if (!proc->baseMap.defines(location)) {
	ret = installBaseTramp(location, proc);
	proc->baseMap[location] = ret;
	// generate branch from instrumentation point to base tramp
	unsigned imageBaseAddr;
	if (!proc->getBaseAddress(location->owner, imageBaseAddr))
	  abort();
	unsigned char *insn = new unsigned char[JUMP_REL32_SZ];
	unsigned size = generateBranchToTramp(proc, location, (int)ret->baseAddr, 
					      imageBaseAddr, insn);
	if (size == 0)
	  return NULL;
	retInstance = new returnInstance(new instruction(insn, 0, size), size,
					 location->jumpAddr + imageBaseAddr, size);
    } else {
        ret = proc->baseMap[location];
    }
    return(ret);
}


/*
 * Install a single mini-tramp.
 *
 */
void installTramp(instInstance *inst, char *code, int codeSize) 
{
    totalMiniTramps++;
    //insnGenerated += codeSize/sizeof(int);
    (inst->proc)->writeDataSpace((caddr_t)inst->trampBase, codeSize, code);
    unsigned atAddr;
    if (inst->when == callPreInsn) {
      atAddr = inst->baseInstance->baseAddr+inst->baseInstance->skipPreInsOffset;
    }
    else {
      atAddr = inst->baseInstance->baseAddr+inst->baseInstance->skipPostInsOffset; 
    }
    generateNoOp(inst->proc, atAddr);
}


/**************************************************************
 *
 *  code generator for x86
 *
 **************************************************************/




#define MAX_BRANCH	0x1<<31

unsigned getMaxBranch() {
  return MAX_BRANCH;
}


bool doNotOverflow(int)
{
  //
  // this should be changed by the correct code. If there isn't any case to
  // be checked here, then the function should return TRUE. If there isn't
  // any immediate code to be generated, then it should return FALSE - naim
  //
  // any int value can be an immediate on the pentium
  return(true);
}



/* build the MOD/RM byte of an instruction */
inline unsigned char makeModRMbyte(unsigned Mod, unsigned Reg, unsigned RM) {
  return ((Mod & 0x3) << 6) + ((Reg & 0x7) << 3) + (RM & 0x7);
}

/* 
   Emit the ModRM byte and displacement for addressing modes.
   base is a register (EAX, ECX, EDX, EBX, EBP, ESI, EDI)
   disp is a displacement
   reg_opcode is either a register or an opcode
 */
void emitAddressingMode(int base, int disp, int reg_opcode, 
                        unsigned char *&insn) {
  assert(base != ESP);
  if (base == -1) {
    *insn++ = makeModRMbyte(0, reg_opcode, 5);
    *((int *)insn) = disp;
    insn += sizeof(int);
  } else if (disp == 0 && base != EBP) {
    *insn++ = makeModRMbyte(0, reg_opcode, base);
  } else if (disp >= -128 && disp <= 127) {
    *insn++ = makeModRMbyte(1, reg_opcode, base);
    *((char *)insn++) = (char) disp;
  } else {
    *insn++ = makeModRMbyte(2, reg_opcode, base);
    *((int *)insn) = disp;
    insn += sizeof(int);
  }
}


/* emit a simple one-byte instruction */
void emitSimpleInsn(unsigned op, unsigned char *&insn) {
  *insn++ = op;
}

// emit a simple register to register instruction: OP dest, src
// opcode is one or two byte
void emitOpRegReg(unsigned opcode, reg dest, reg src, unsigned char *&insn) {
  if (opcode <= 0xFF)
    *insn++ = opcode;
  else {
    *insn++ = opcode >> 8;
    *insn++ = opcode & 0xFF;
  }
  // ModRM byte define the operands: Mod = 3, Reg = dest, RM = src
  *insn++ = makeModRMbyte(3, dest, src);
}

// emit OP reg, r/m
void emitOpRegRM(unsigned opcode, reg dest, reg base, int disp, unsigned char *&insn) {
  *insn++ = opcode;
  emitAddressingMode(base, disp, dest, insn);
}

// emit OP r/m, reg
void emitOpRMReg(unsigned opcode, reg base, int disp, reg src, unsigned char *&insn) {
  *insn++ = opcode;
  emitAddressingMode(base, disp, src, insn);
}

// emit OP reg, imm32
void emitOpRegImm(int opcode, reg dest, int imm, unsigned char *&insn) {
  *insn++ = 0x81;
  *insn++ = makeModRMbyte(3, opcode, dest);
  *((int *)insn) = imm;
  insn+= sizeof(int);
}

/*
// emit OP r/m, imm32
void emitOpRMImm(unsigned opcode, reg base, int disp, int imm, unsigned char *&insn) {
  *insn++ = 0x81;
  emitAddressingMode(base, disp, opcode, insn);
  *((int *)insn) = imm;
  insn += sizeof(int);
}
*/

// emit OP r/m, imm32
void emitOpRMImm(unsigned opcode1, unsigned opcode2,
		 reg base, int disp, int imm, unsigned char *&insn) {
  *insn++ = opcode1;
  emitAddressingMode(base, disp, opcode2, insn);
  *((int *)insn) = imm;
  insn += sizeof(int);
}

// emit OP r/m, imm8
void emitOpRMImm8(unsigned opcode1, unsigned opcode2,
		 reg base, int disp, char imm, unsigned char *&insn) {
  *insn++ = opcode1;
  emitAddressingMode(base, disp, opcode2, insn);
  *insn++ = imm;
}

// emit OP reg, r/m, imm32
void emitOpRegRMImm(unsigned opcode, reg dest,
		 reg base, int disp, int imm, unsigned char *&insn) {
  *insn++ = opcode;
  emitAddressingMode(base, disp, dest, insn);
  *((int *)insn) = imm;
  insn += sizeof(int);
}

// emit MOV reg, reg
void emitMovRegToReg(reg dest, reg src, unsigned char *&insn) {
  *insn++ = 0x8B;
  *insn++ = makeModRMbyte(3, dest, src);
}

// emit MOV reg, r/m
void emitMovRMToReg(reg dest, reg base, int disp, unsigned char *&insn) {
  *insn++ = 0x8B;
  emitAddressingMode(base, disp, dest, insn);
}

// emit MOV r/m, reg
void emitMovRegToRM(reg base, int disp, reg src, unsigned char *&insn) {
  *insn++ = 0x89;
  emitAddressingMode(base, disp, src, insn);
}

// emit MOV m, reg
void emitMovRegToM(int disp, reg src, unsigned char *&insn) {
  *insn++ = 0x89;
  emitAddressingMode(-1, disp, src, insn);
}

// emit MOV reg, m
void emitMovMToReg(reg dest, int disp, unsigned char *&insn) {
  *insn++ = 0x8B;
  emitAddressingMode(-1, disp, dest, insn);
}

// emit MOV reg, imm32
void emitMovImmToReg(reg dest, int imm, unsigned char *&insn) {
  *insn++ = 0xB8 + dest;
  *((int *)insn) = imm;
  insn += sizeof(int);
}

// emit MOV r/m32, imm32
void emitMovImmToRM(reg base, int disp, int imm, unsigned char *&insn) {
  *insn++ = 0xC7;
  emitAddressingMode(base, disp, 0, insn);
  *((int*)insn) = imm;
  insn += sizeof(int);
}

// emit MOV mem32, imm32
void emitMovImmToMem(unsigned maddr, int imm, unsigned char *&insn) {
  *insn++ = 0xC7;
  // emit the ModRM byte: we use a 32-bit displacement for the address,
  // the ModRM value is 0x05
  *insn++ = 0x05;
  *((unsigned *)insn) = maddr;
  insn += sizeof(unsigned);
  *((int*)insn) = imm;
  insn += sizeof(int);
}


// emit Add dword ptr DS:[addr], imm
void emitAddMemImm(Address addr, int imm, unsigned char *&insn) {
  if (imm >= MIN_IMM8 && imm <= MAX_IMM8) {
    *insn++ = 0x83;
    // emit the ModRM byte: we use a 32-bit displacement for the address,
    // the ModRM value is 0x05
    *insn++ = 0x05;
    *((unsigned *)insn) = addr;
    insn += sizeof(unsigned);
    *((char *)insn++) = (char)imm;
  } else {
    *insn++ = 0x81;
    *insn++ = 0x05;
    *((unsigned *)insn) = addr;
    insn += sizeof(unsigned);
    *((int *)insn) = imm;
    insn += sizeof(int);
  }
}

// emit JUMP rel32
void emitJump(unsigned disp32, unsigned char *&insn) {
  *insn++ = 0xE9;
  *((int *)insn) = disp32;
  insn += sizeof(int);
} 


// set dest=1 if src1 op src2, otherwise dest = 0
void emitRelOp(unsigned op, reg dest, reg src1, reg src2, unsigned char *&insn) {
  //fprintf(stderr,"Relop dest = %d, src1 = %d, src2 = %d\n", dest, src1, src2);
  emitOpRegReg(0x29, ECX, ECX, insn);           // clear ECX
  emitMovRMToReg(EAX, EBP, -(src1*4), insn);    // mov eax, -(src1*4)[ebp]
  emitOpRegRM(0x3B, EAX, EBP, -(src2*4), insn); // cmp eax, -(src2*4)[ebp]
  unsigned char opcode;
  switch (op) {
    case eqOp: opcode = JNE_R8; break;
    case neOp: opcode = JE_R8; break;
    case lessOp: opcode = JGE_R8; break;
    case leOp: opcode = JG_R8; break;
    case greaterOp: opcode = JLE_R8; break;
    case geOp: opcode = JL_R8; break;
    default: assert(0);
  }
  *insn++ = opcode; *insn++ = 1;                // jcc 1
  emitSimpleInsn(0x40+ECX, insn);               // inc ECX
  emitMovRegToRM(EBP, -(dest*4), ECX, insn);    // mov -(dest*4)[ebp], ecx

}

// set dest=1 if src1 op src2imm, otherwise dest = 0
void emitRelOpImm(unsigned op, reg dest, reg src1, int src2imm, unsigned char *&insn) {
  //fprintf(stderr,"Relop dest = %d, src1 = %d, src2 = %d\n", dest, src1, src2);
  emitOpRegReg(0x29, ECX, ECX, insn);           // clear ECX
  emitMovRMToReg(EAX, EBP, -(src1*4), insn);    // mov eax, -(src1*4)[ebp]
  emitOpRegImm(0x3D, EAX, src2imm, insn);       // cmp eax, src2
  unsigned char opcode;
  switch (op) {
    case eqOp: opcode = JNE_R8; break;
    case neOp: opcode = JE_R8; break;
    case lessOp: opcode = JGE_R8; break;
    case leOp: opcode = JG_R8; break;
    case greaterOp: opcode = JLE_R8; break;
    case geOp: opcode = JL_R8; break;
    default: assert(0);
  }
  *insn++ = opcode; *insn++ = 1;                // jcc 1
  emitSimpleInsn(0x40+ECX, insn);               // inc ECX
  emitMovRegToRM(EBP, -(dest*4), ECX, insn);    // mov -(dest*4)[ebp], ecx

}

void emitEnter(short imm16, unsigned char *&insn) {
  *insn++ = 0xC8;
  *((short*)insn) = imm16;
  insn += sizeof(short);
  *insn++ = 0;
}



unsigned emitFuncCall(opCode op, 
		      registerSpace *rs,
		      char *i, unsigned &base,
		      vector<AstNode> operands, 
		      string callee, process *proc)
{
  assert(op == callOp);
  unsigned addr;
  bool err;
  vector <reg> srcs;

  addr = (proc->symbols)->findInternalAddress(callee, false, err);
  if (err) {
    pdFunction *func = (proc->symbols)->findOneFunction(callee);
    if (!func) {
      ostrstream os(errorLine, 1024, ios::out);
      os << "Internal error: unable to find addr of " << callee << endl;
      logLine(errorLine);
      showErrorCallback(80, (const char *) errorLine);
      P_abort();
    }
    addr = func->addr();
  }

  for (unsigned u = 0; u < operands.size(); u++)
    srcs += operands[u].generateCode(proc, rs, i, base);

  unsigned char *insn = (unsigned char *) ((void*)&i[base]);
  unsigned char *first = insn;

  // push arguments in reverse order, last argument first
  for (int i=srcs.size() - 1 ; i >= 0; i--) {
    emitOpRMReg(PUSH_RM_OPC1, EBP, -(srcs[i]*4), PUSH_RM_OPC2, insn);
    rs->freeRegister(srcs[i]);
  }

  // make the call
  // we are using an indirect call here because we don't know the
  // address of this instruction, so we can't use a relative call.
  // TODO: change this to use a direct call
  emitMovImmToReg(EAX, addr, insn);       // mov eax, addr
  emitOpRegReg(CALL_RM_OPC1, CALL_RM_OPC2, EAX, insn);   // call *(eax)
  // reset the stack pointer
  emitOpRegImm(0, ESP, srcs.size()*4, insn); // add esp, srcs.size()*4

  // allocate a register to store the return value
  reg ret = rs->allocateRegister((char *)insn, base);
  emitMovRegToRM(EBP, -(ret*4), EAX, insn);

  base += insn - first;
  return ret;
}



/*
 * emit code for op(sr1,src2, dest)
 * ibuf is an instruction buffer where instructions are generated
 * base is the next free position on ibuf where code is to be generated
 */
unsigned emit(opCode op, reg src1, reg src2, reg dest, char *ibuf, unsigned &base)
{
    unsigned char *insn = (unsigned char *) (&ibuf[base]);
    unsigned char *first = insn;

    if (op == loadConstOp) {
      // dest is a temporary
      // src1 is an immediate value 
      // dest = src1:imm32
      emitMovImmToRM(EBP, -(dest*4), src1, insn);

    } else if (op ==  loadOp) {
      // dest is a temporary
      // src1 is the address of the operand
      // dest = [src1]
      emitMovMToReg(EAX, src1, insn);               // mov eax, src1
      emitMovRegToRM(EBP, -(dest*4), EAX, insn);    // mov -(dest*4)[ebp], eax

    } else if (op ==  storeOp) {
      // [dest] = src1
      // dest has the address where src1 is to be stored
      // src1 is a temporary
      // src2 is a "scratch" register, we don't need it in this architecture
      emitMovRMToReg(EAX, EBP, -(src1*4), insn);    // mov eax, -(src1*4)[ebp]
      emitMovRegToM(dest, EAX, insn);               // mov dest, eax

    } else if (op ==  ifOp) {
      // if src1 == 0 jump to dest
      // src1 is a temporary
      // dest is a target address
      emitOpRegReg(0x29, EAX, EAX, insn);            // sub EAX, EAX ; clear EAX
      emitOpRegRM(0x3B, EAX, EBP, -(src1*4), insn);  // cmp -(src1*4)[EBP], EAX
      // je dest
      *insn++ = 0x0F;
      *insn++ = 0x84;
      *((int *)insn) = dest;
      insn += sizeof(int);
      base += insn-first;
      return base;

    } else if (op ==  trampPreamble) {
      // allocate space for temporaries
      emitOpRegImm(5, ESP, 128, insn); // sub esp, 128
      
      // update observed cost
      // dest = address of DYNINSTobsCostLow
      // src1 = cost
      emitAddMemImm(dest, src1, insn);  // ADD (dest), src1

    } else if (op ==  trampTrailer) {
      // reset the stack pointer
      emitOpRegImm(0, ESP, 128, insn); // add esp, 128

      // generate the template for a jump -- actual jump is generated elsewhere
      emitJump(0, insn); // jump xxxx
      // return the offset of the previous jump
      base += insn - first;
      return(base - JUMP_REL32_SZ);

    } else if (op == noOp) {
       emitSimpleInsn(NOP, insn); // nop

    } else if (op == getRetValOp) {
      // dest is a register were we can store the value
      // the return value is in the saved EAX
      emitMovRMToReg(EAX, EBP, SAVED_EAX_OFFSET, insn);
      emitMovRegToRM(EBP, -(dest*4), EAX, insn);
      base += insn - first;
      return dest;

    } else if (op == getParamOp) {
      // src1 is the number of the argument
      // dest is a register were we can store the value
      // Parameters are addressed by a positive offset from ebp,
      // the first is PARAM_OFFSET[ebp]
      emitMovRMToReg(EAX, EBP, PARAM_OFFSET + src1*4, insn);
      emitMovRegToRM(EBP, -(dest*4), EAX, insn);
      base += insn - first;
      return dest;

    } else if (op == saveRegOp) {
      // should not be used on this platform
      assert(0);

    } else {
        unsigned opcode;
	switch (op) {
	    // integer ops
	    case plusOp:
 	        // dest = src1 + src2
	        // mv eax, src1
	        // add eax, src2
	        // mov dest, eax
	        opcode = 0x03; // ADD
		break;

	    case minusOp:
		opcode = 0x2B; // SUB
		break;

	    case timesOp:
		opcode = 0x0FAF; // IMUL
		break;

	    case divOp:
		// dest = src1 div src2
		// mov eax, src1
		// cdq   ; edx = sign extend of eax
		// idiv eax, src2 ; eax = edx:eax div src2, edx = edx:eax mod src2
		// mov dest, eax
		emitMovRMToReg(EAX, EBP, -(src1*4), insn);
		emitSimpleInsn(0x99, insn);
		emitOpRegRM(0xF7, 0x7 /*opcode extension*/, EBP, -(src2*4), insn);
		emitMovRegToRM(EBP, -(dest*4), EAX, insn);
		base += insn-first;
		return 0;
		break;

	    // Bool ops
	    case orOp:
		opcode = 0x0B; // OR 
		break;

	    case andOp:
		opcode = 0x23; // AND
		break;

	    // rel ops
	    // dest = src1 relop src2
	    case eqOp:
	    case neOp:
	    case lessOp:
	    case leOp:
	    case greaterOp:
	    case geOp:
		emitRelOp(op, dest, src1, src2, insn);
		base += insn-first;
		return 0;
		break;

	    default:
		abort();
		break;
	}

	emitMovRMToReg(EAX, EBP, -(src1*4), insn);
	emitOpRegRM(opcode, EAX, EBP, -(src2*4), insn);
	emitMovRegToRM(EBP, -(dest*4), EAX, insn);
      }
    base += insn - first;
    return(0);
}


unsigned emitImm(opCode op, reg src1, int src2imm, reg dest, char *ibuf, unsigned &base)
{
    unsigned char *insn = (unsigned char *) (&ibuf[base]);
    unsigned char *first = insn;

    if (op ==  storeOp) {
      // [dest] = src1
      // dest has the address where src1 is to be stored
      // src1 is an immediate value
      // src2 is a "scratch" register, we don't need it in this architecture
      emitMovImmToReg(EAX, dest, insn);
      emitMovImmToRM(EAX, 0, src1, insn);
    } else {
        unsigned opcode1;
	unsigned opcode2;
	switch (op) {
	    // integer ops
	    case plusOp:
	        opcode1 = 0x81;
	        opcode2 = 0x0; // ADD
		break;

	    case minusOp:
		opcode1 = 0x81;
		opcode2 = 0x5; // SUB
		break;

	    case timesOp: {
	        int result;
		if (isPowerOf2(src2imm, result) && result <= MAX_IMM8) {
		  if (src1 != dest) {
		    emitMovRMToReg(EAX, EBP, -(src1*4), insn);
		    emitMovRegToRM(EBP, -(dest*4), EAX, insn);
		  }
		  // sal dest, result
		  emitOpRMImm8(0xC1, 4, EBP, -(dest*4), result, insn);
		}
		else {
		  // imul EAX, -(src1*4)[ebp], src2imm
		  emitOpRegRMImm(0x69, EAX, EBP, -(src1*4), src2imm, insn);
		  emitMovRegToRM(EBP, -(dest*4), EAX, insn);
		} 
		base += insn-first;
		return 0;
	      }
		break;

	    case divOp: {
	        int result;
		if (isPowerOf2(src2imm, result) && result <= MAX_IMM8) {
		  if (src1 != dest) {
		    emitMovRMToReg(EAX, EBP, -(src1*4), insn);
		    emitMovRegToRM(EBP, -(dest*4), EAX, insn);
		  }
		  // sar dest, result
		  emitOpRMImm8(0xC1, 7, EBP, -(dest*4), result, insn);
		}
		else {
		  // dest = src1 div src2imm
		  // mov eax, src1
		  // cdq   ; edx = sign extend of eax
		  // mov ebx, src2imm
		  // idiv eax, ebx ; eax = edx:eax div src2, edx = edx:eax mod src2
		  // mov dest, eax
		  emitMovRMToReg(EAX, EBP, -(src1*4), insn);
		  emitSimpleInsn(0x99, insn);
		  emitMovImmToReg(EBX, src2imm, insn);
		  // idiv eax, ebx
		  emitOpRegReg(0xF7, 0x7 /*opcode extension*/, EBX, insn); 
		  emitMovRegToRM(EBP, -(dest*4), EAX, insn);
		}
	      }
		base += insn-first;
		return 0;
		break;

	    // Bool ops
	    case orOp:
		opcode1 = 0x81;
		opcode2 = 0x1; // OR 
		break;

	    case andOp:
		opcode1 = 0x81;
		opcode2 = 0x4; // AND
		break;

	    // rel ops
	    // dest = src1 relop src2
	    case eqOp:
	    case neOp:
	    case lessOp:
	    case leOp:
	    case greaterOp:
	    case geOp:
		emitRelOpImm(op, dest, src1, src2imm, insn);
		base += insn-first;
		return 0;
		break;

	    default:
		abort();
		break;
	}
	if (src1 != dest) {
	  emitMovRMToReg(EAX, EBP, -(src1*4), insn);
	  emitMovRegToRM(EBP, -(dest*4), EAX, insn);
	}
	emitOpRMImm(opcode1, opcode2, EBP, -(dest*4), src2imm, insn);
      }
    base += insn - first;
    return(0);
}



int getInsnCost(opCode op)
{
    if (op == loadConstOp) {
	return(1);
    } else if (op ==  loadOp) {
	return(1+1);
    } else if (op ==  storeOp) {
	return(1+1); 
    } else if (op ==  ifOp) {
	return(1+2+1);
    } else if (op ==  callOp) {
        // cost of call only
        return(1+2+1+1);
    } else if (op ==  trampPreamble) {
        return(1+3);
    } else if (op ==  trampTrailer) {
        return(1+1);
    } else if (op == noOp) {
	return(1);
    } else if (op == getRetValOp) {
	return (1+1);
    } else if (op == getParamOp) {
        return(1+1);
    } else {
	switch (op) {
	    // rel ops
	    case eqOp:
            case neOp:
	    case lessOp:
            case leOp:
            case greaterOp:
	    case geOp:
	        return(1+1+2+1+1+1);
	        break;
	    case divOp:
		return(1+2+46+1);
	    case timesOp:
		return(1+10+1);
	    case plusOp:
	    case minusOp:
	    case orOp:
	    case andOp:
		return(1+2+1);
	    default:
		assert(0);
		break;
	}
    }
}



// The exact semantics of the heap are processor specific.
//
// find all DYNINST symbols that are data symbols
//
bool image::heapIsOk(const vector<sym_data> &find_us) {
  Address curr, instHeapEnd;
  Symbol sym;
  string str;

  for (unsigned i=0; i<find_us.size(); i++) {
    str = find_us[i].name;
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
      msg = string("Cannot find ") + str + string(". Exiting");
      statusLine(msg.string_of());
      showErrorCallback(50, msg);
      return false;
    }
  }
  instHeapEnd = sym.addr();
  addInternalSymbol(ghb, instHeapEnd);
  ghb = INFERIOR_HEAP_BASE;

  if (!linkedFile.get_symbol(ghb, sym)) {
    ghb = UINFERIOR_HEAP_BASE;
    if (!linkedFile.get_symbol(ghb, sym)) {
      string msg;
      msg = string("Cannot find ") + str + string(". Cannot use this application");
      statusLine(msg.string_of());
      showErrorCallback(50, msg);
      return false;
    }
  }
  curr = sym.addr();
  addInternalSymbol(ghb, curr);
  if (curr > instHeapEnd) instHeapEnd = curr;

  // check that we can get to our heap.
  if (instHeapEnd > getMaxBranch()) {
    logLine("*** FATAL ERROR: Program text + data too big for dyninst\n");
    sprintf(errorLine, "    heap ends at %x\n", instHeapEnd);
    logLine(errorLine);
    return false;
  } else if (instHeapEnd + SYN_INST_BUF_SIZE > getMaxBranch()) {
    logLine("WARNING: Program text + data could be too big for dyninst\n");
    showErrorCallback(54, "");
    return false;
  }
  return true;
}


dictionary_hash<string, unsigned> funcFrequencyTable(string::hash);

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
int getPointCost(process *proc, instPoint *point)
{
    point->checkInstructions();

    if (proc->baseMap.defines(point)) {
        return(0);
    } else {
        if (point->size() == 1)
	  return 9000; // estimated number of cycles for traps
        else
	  return(83);
    }
}



bool returnInstance::checkReturnInstance(const Address ) {
    return true;
}
 
void returnInstance::installReturnInstance(process *proc) {
    assert(instructionSeq);
    proc->writeTextSpace((caddr_t)addr_, instSeqSize, (caddr_t) instructionSeq->ptr());
    delete instructionSeq;
    instructionSeq = 0;
}

void returnInstance::addToReturnWaitingList(Address , process *) {
    P_abort();
}

void generateBreakPoint(unsigned char &) {
  P_abort();
}

void instWaitingList::cleanUp(process *, Address ) {
  P_abort();
}
