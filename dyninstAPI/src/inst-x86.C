/*
 * Copyright (c) 1996-2003 Barton P. Miller
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
 * $Id: inst-x86.C,v 1.145 2003/10/22 16:04:57 schendel Exp $
 */

#include <iomanip>

#include <limits.h>
#include "common/h/headers.h"

#ifndef BPATCH_LIBRARY
#include "rtinst/h/rtinst.h"
#endif
#include "common/h/Dictionary.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/stats.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/showerror.h"

#include "dyninstAPI/src/arch-x86.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/instPoint.h" // includes instPoint-x86.h
#include "dyninstAPI/src/instP.h" // class returnInstance
#include "dyninstAPI/src/rpcMgr.h"

// for function relocation
#include "dyninstAPI/src/func-reloc.h" 
#include "dyninstAPI/src/LocalAlteration.h"

#include <sstream>

class ExpandInstruction;
class InsertNops;

extern bool relocateFunction(process *proc, instPoint *&location);
extern void modifyInstPoint(instPoint *&location,process *proc);

extern bool isPowerOf2(int value, int &result);
void BaseTrampTrapHandler(int); //siginfo_t*, ucontext_t*);


// The general machine registers. 
// These values are taken from the Pentium manual and CANNOT be changed.
#undef EAX 
#define EAX (0)
#undef ECX
#define ECX (1)
#undef EDX
#define EDX (2)
#undef EBX
#define EBX (3)
#undef ESP
#define ESP (4)
#undef EBP
#define EBP (5)
#undef ESI
#define ESI (6)
#undef EDI
#define EDI (7)

// Size of a jump rel32 instruction
#define JUMP_REL32_SZ (5)
#define JUMP_SZ (5)
// Size of a call rel32 instruction
#define CALL_REL32_SZ (5)

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

#define FUNC_PARAM_OFFSET (8+(10*4))
#define CALLSITE_PARAM_OFFSET (4+(10*4))


// number of virtual registers
#define NUM_VIRTUAL_REGISTERS (32)

// offset from EBP of the saved EAX for a tramp
#define SAVED_EAX_OFFSET (10*4-4)
#define SAVED_EFLAGS_OFFSET (SAVED_EAX_OFFSET+4)

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

class NonRecursiveTrampTemplate : public trampTemplate
{

public:
    NonRecursiveTrampTemplate(const instPoint *l, process *p)
            :trampTemplate(l,p) {}
    
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

static void emitOpRMImm8( unsigned opcode1, unsigned opcode2, Register base, int disp, char imm,
                          unsigned char * & insn );
static void emitMovImmToMem( Address maddr, int imm,
                             unsigned char * & insn );

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
   checkInstructions: check that there are no known jumps to the instructions
   before and after the point.
*/
void instPoint::checkInstructions() {
  Address currAddr = addr_;
  unsigned OKinsns = 0;

  // if jumpAddr_ is not zero, this point has been checked already
  if (jumpAddr_) 
    return;

  unsigned tSize;
  unsigned maxSize = JUMP_SZ;
  if (address() == func()->getAddress(0)) // entry point
    maxSize = 2*JUMP_SZ;
  tSize = insnAtPoint_.size();

  if (!owner()->isJumpTarget(currAddr)) {
    // check instructions before point
    unsigned insnsBefore_ = insnsBefore();
    for (unsigned u = 0; u < insnsBefore_; u++) {
      OKinsns++;
      tSize += (*insnBeforePt_)[u].size();
      currAddr -= (*insnBeforePt_)[u].size();
      if (owner()->isJumpTarget(currAddr)) {
	// must remove instruction from point
	// fprintf(stderr, "check instructions point 0x%lx, jmp to 0x%lx\n",
        //                addr,currAddr);
	break;
      }
    }
  }
  if (insnBeforePt_)
    (*insnBeforePt_).resize(OKinsns);

  // this is the address where we insert the jump
  jumpAddr_ = currAddr;

  // check instructions after point
  currAddr = addr_ + insnAtPoint_.size();
  OKinsns = 0;
  unsigned insnsAfter_ = insnsAfter();
  for (unsigned u = 0; tSize < maxSize && u < insnsAfter_; u++) {
    if (owner()->isJumpTarget(currAddr))
      break;
    OKinsns++;
    unsigned size = (*insnAfterPt_)[u].size();
    currAddr += size;
    tSize += size;
  }
  if (insnAfterPt_)
    (*insnAfterPt_).resize(OKinsns);
  
#ifdef notdef
  if (tSize < maxSize) {
    tSize = insnAtPoint_.size();
    jumpAddr_ = addr_;
    if (insnBeforePt_) (*insnBeforePt_).resize(0);
    if (insnAfterPt_) (*insnAfterPt_).resize(0);
  }
#endif
}

/* PT is an instrumentation point.  ENTRY is the entry point for the
   same function, and EXITS are the exit instrumentation points for
   the function.  Returns true if this function supports an extra slot
   and PT can use it. */
static bool
_canUseExtraSlot(const instPoint *pt, const instPoint *entry,
		 const pdvector<instPoint*> &exits)
{
     if (entry->size() < 2*JUMP_SZ)
	  return false;

     // We get 10 bytes for the entry points, instead of the usual five,
     // so that we have space for an extra jump. We can then insert a
     // jump to the basetramp in the second slot of the base tramp
     // and use a short 2-byte jump from the point to the second jump.
     // We adopt the following rule: Only one point in the function
     // can use the indirect jump, and this is the first return point
     // with a size that is less than five bytes
     bool canUse = false;
     for (unsigned u = 0; u < exits.size(); u++)
	  if (exits[u] == pt) {
	       canUse = true;
	       break;
	  } else if (exits[u]->size() < JUMP_SZ)
	       return false;
     if (!canUse)
	  return false;

     /* The entry has a slot, the point can be used for a slot,
	now see if the point can reach the slot. */
     int displacement = entry->jumpAddr() + 5 - pt->jumpAddr();
     assert(displacement < 0);
     if (pt->size() >= 2 && (displacement-2) > SCHAR_MIN)
	  return true;
     else
	  return false;
}

/*
   Returns true if we can use the extra slot for a jump at the entry point
   to insert a jump to a base tramp at this point.  */
bool instPoint::canUseExtraSlot(process *proc) const
{
     return _canUseExtraSlot(this,
			     func()->funcEntry(proc),
			     func()->funcExits(proc));
}

/* ENTRY and EXITS are the entry and exit points of a function.  PT
   must be a point in the same function.  Return true if PT requires a
   trap to instrument. */
static bool
_usesTrap(const instPoint *pt,
	  const instPoint *entry,
	  const pdvector<instPoint*> &exits)
{
     /* If this point is big enough to hold a 32-bit jump to any
	basetramp, it doesn't need a trap. */
     if (pt->size() >= JUMP_REL32_SZ)
	  return false;

     /* If it can use the extra slot, it doesn't need a trap. */
     if (_canUseExtraSlot(pt, entry, exits)) {
	  return false;
     }
     /* Otherwise it needs a trap. */
     return true;
}

bool instPoint::usesTrap(process *proc) const
{
     return _usesTrap(this, func()->funcEntry(proc), func()->funcExits(proc));
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
void pd_Function::checkCallPoints() {
  unsigned int i;
  instPoint *p;
  Address loc_addr;
  if (call_points_have_been_checked) return;

  pdvector<instPoint*> non_lib;

  for (i=0; i<calls.size(); ++i) {
    /* check to see where we are calling */
    p = calls[i];
    assert(p);

    if (!p->insnAtPoint().isCallIndir()) {
      loc_addr = p->insnAtPoint().getTarget(p->address());
      file()->exec()->addJumpTarget(loc_addr);
      pd_Function *pdf = (file_->exec())->findFuncByOffset(loc_addr);

      if (pdf) {
        p->set_callee(pdf);
        non_lib.push_back(p);
      } else {
	   // if this is a call outside the fuction, keep it
	   if((loc_addr < getAddress(0))||(loc_addr > (getAddress(0)+size()))){
                non_lib.push_back(p);
	   }
	   else {
	       delete p;
	   }
      } 
    } else {
      // Indirect call -- be conservative, assume it is a call to
      // an unnamed user function
      //assert(!p->callee());
      p->set_callee(NULL);
      non_lib.push_back(p);
    }
  }
  calls = non_lib;
  call_points_have_been_checked = true;
}

// this function is not needed
Address pd_Function::newCallPoint(Address, const instruction,
				 const image *, bool &)
{ assert(0); return 0; }


// see if we can recognize a jump table and skip it
// return the size of the table in tableSz.
bool checkJumpTable(image *im, instruction insn, Address addr, 
		    Address funcBegin, 
		    Address &funcEnd,
		    unsigned &tableSz) {

  const unsigned char *instr = insn.ptr();
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
    const Address tableBase = *(const Address *)(instr+3);
    //fprintf(stderr, "Found jump table at 0x%lx 0x%lx\n",addr, tableBase);
    // check if the table is right after the jump and inside the current function
    if (tableBase > funcBegin && tableBase < funcEnd) {
      // table is within function code
      if (tableBase < addr+insn.size()) {
	fprintf(stderr, "bad indirect jump at 0x%lx\n", addr);
	return false;
      } else if (tableBase > addr+insn.size()) {
	// jump table may be at the end of the function code - adjust funcEnd
	funcEnd = tableBase;
	return true;
      }
      // skip the jump table
      for (const unsigned *ptr = (const unsigned *)im->getPtrToInstruction(tableBase);
	   *ptr >= funcBegin && *ptr <= funcEnd; ptr++) {
	//fprintf(stderr, " jump table entry = 0x%lx\n", *(unsigned *)ptr);
	tableSz += sizeof(int);
      }
    }
    else {
        // fprintf(stderr, "Ignoring external jump table at 0x%lx.\n", tableBase);
    }
  }
  return true;
}

void checkIfRelocatable(instruction insn, bool &canBeRelocated) {
  const unsigned char *instr = insn.ptr();

  // Check if REG bits of ModR/M byte are 100 or 101 (Possible jump 
  // to jump table).
  if (instr[0] == 0xFF && 
     ( ((instr[1] & 0x38)>>3) == 4 || ((instr[1] & 0x38)>>3) == 5 )) {

    // function should not be relocated
    canBeRelocated = false;
  }
}


bool isRealCall(instruction &insn, Address adr, image *owner, bool &validTarget, pd_Function * /* pdf */ ) {

  // calls to adr+5 are not really calls, they are used in 
  // dynamically linked libraries to get the address of the code.
  if (insn.getTarget(adr) == adr + 5) {
    return false;
  }


  // Calls to a mov instruction followed by a ret instruction, where the 
  // source of the mov is the %esp register, are not real calls.
  // These sequences are used to set the destination register of the mov 
  // with the pc of the instruction instruction that follows the call.

  // This sequence accomplishes this because the call instruction has the 
  // side effect of placing the value of the %eip on the stack and setting the
  // %esp register to point to that location on the stack. (The %eip register
  // maintains the address of the next instruction to be executed).
  // Thus, when the value at the location pointed to by the %esp register 
  // is moved, the destination of the mov is set with the pc of the next
  // instruction after the call.   

  //    Here is an example of this sequence:
  //
  //       mov    %esp, %ebx
  //       ret    
  //
  //    These two instructions are specified by the bytes 0xc3241c8b
  //

  int targetAdr = insn.getTarget(adr);
 
  if ( !owner->isValidAddress(targetAdr) ) {
    validTarget = false;
    return false;
  }    

  // Get a pointer to the call target
  const unsigned char *target = (const unsigned char *)owner->getPtrToInstruction(targetAdr);

  // The target instruction is a  mov
  if (*(target) == 0x8b) {

    // The source register of the mov is specified by a SIB byte 
    if (*(target + 1) == 0x1c) {

      // The source register of the mov is the %esp register (0x24) and 
      // the instruction after the mov is a ret instruction (0xc3)
      if ( (*(target + 2) == 0x24) && (*(target + 3) == 0xc3)) {
        return false;
      }
    }
  }

  return true;
}

static bool isStackFramePreamble(instruction& one, instruction& two)
{
  /* test for
     one:  push   %ebp
     two:  mov    %esp,%ebp
  */
  const unsigned char *p, *q;
  p = one.ptr();
  q = two.ptr();
  return (one.size() == 1
	  && p[0] == 0x55
	  && two.size() == 2
	  && q[0] == 0x89
	  && q[1] == 0xe5);
}

/* auxiliary data structures for function findInstPoints */
enum { EntryPt, CallPt, ReturnPt, OtherPt };
class point_ {
  public:
     point_(): point(0), index(0), type(0) {};
     point_(instPoint *p, unsigned i, unsigned t): point(p), index(i), type(t) {};
     instPoint *point;
     unsigned index;
     unsigned type;
};


bool pd_Function::findInstPoints(const image *i_owner) {
   // sorry this this hack, but this routine can modify the image passed in,
   // which doesn't occur on other platforms --ari
   image *owner = const_cast<image *>(i_owner); // const cast
   point_ *points = NULL;
   instruction *allInstr = NULL;
   pdvector<instPoint*> foo;
   unsigned lastPointEnd = 0;
   unsigned thisPointEnd = 0;
   bool canBeRelocated = true;
   instPoint *p;
   unsigned numInsns = 0;
   instruction insn;
   unsigned insnSize;
   unsigned npoints = 0;
   Address adr;
   const unsigned char *instr;
   Address funcEnd;

   if (size() == 0) {
      //fprintf(stderr,"Function %s, size = %d\n", prettyName().c_str(), size());
     goto set_uninstrumentable;
   }

#if defined(i386_unknown_solaris2_5)
   /* On Solaris, this function is called when a signal handler
      returns.  If it requires trap-based instrumentation, it can foul
      the handler return mechanism.  So, better exclude it.  */
   if (prettyName() == "_setcontext" || prettyName() == "setcontext")
     goto set_uninstrumentable;
#endif /* i386_unknown_solaris2_5 */

   // XXXXX kludge: these functions are called by DYNINSTgetCPUtime, 
   // they can't be instrumented or we would have an infinite loop
   if (prettyName() == "gethrvtime" || prettyName() == "_divdi3"
       || prettyName() == "GetProcessTimes")
     goto set_uninstrumentable;

   points = new point_[size()];
   if( points == NULL )
   {
		assert( false );
   }
   //point_ *points = (point_ *)alloca(size()*sizeof(point));

   instr = (const unsigned char *)owner->getPtrToInstruction(getAddress(0));
   adr = getAddress(0);
   numInsns = 0;

   noStackFrame = true; // Initial assumption

   // keep a buffer with all the instructions in this function
   allInstr = new instruction[size()+5];
   //instruction *allInstr = (instruction *)alloca((size()+5)*sizeof(instruction));

   // define the entry point
   insnSize = insn.getNextInstruction(instr);
   p = new instPoint(this, owner, adr, functionEntry, insn);
   funcEntry_ = p;
   points[npoints++] = point_(p, numInsns, EntryPt);

   // check if the entry point contains another point
   if (insn.isJumpDir()) {
      Address target = insn.getTarget(adr);
      owner->addJumpTarget(target);
      if (target < getAddress(0) || target >= getAddress(0) + size()) {
         // jump out of function
         // this is an empty function
	 goto set_uninstrumentable;
      }
   } else if (insn.isReturn()) {
      // this is an empty function
      goto set_uninstrumentable;
   } else if (insn.isCall()) {
      // TODO: handle calls at entry point
      // call at entry point
      //instPoint *p = new instPoint(this, owner, adr, functionEntr, insn);
      //calls += p;
      //points[npoints++] = point_(p, numInsns, CallPt);
      //fprintf(stderr,"Function %s, call at entry point\n", prettyName().c_str());
      goto set_uninstrumentable;
   }

   allInstr[numInsns] = insn;
   numInsns++;
   instr += insnSize;
   adr += insnSize;
   funcEnd = getAddress(0) + size();

   if (adr < funcEnd) {
     instruction next_insn;
     next_insn.getNextInstruction(instr);
     if (isStackFramePreamble(insn, next_insn))
       noStackFrame = false;       
   }

   // get all the instructions for this function, and define the
   // instrumentation points. For now, we only add one instruction to each
   // point.  Additional instructions, for the points that need them, will be
   // added later.

#ifdef BPATCH_LIBRARY
   if (BPatch::bpatch->hasForcedRelocation_NP()) {
      relocatable_ = true;
   }
#endif

   // checkJumpTable will set canBeRelocated = false if their is a jump to a 
   // jump table inside this function. 
   if (prettyName() == "__libc_fork" || prettyName() == "__libc_start_main") {
      canBeRelocated = false;
   }

   for ( ; adr < funcEnd; instr += insnSize, adr += insnSize) {

      insnSize = insn.getNextInstruction(instr);

      assert(insnSize > 0);

      if (adr + insnSize > funcEnd) {
         break;
      }

      if (insn.isJumpIndir()) {
         unsigned jumpTableSz;

         // check if function should be allowed to be relocated
         checkIfRelocatable(insn, canBeRelocated);

         // check for jump table. This may update funcEnd
         if (!checkJumpTable(owner, insn, adr, getAddress(0), funcEnd, jumpTableSz)) {
	   //fprintf(stderr,"Function %s, size = %d, bad jump table\n", 
            //          prettyName().c_str(), size());
	    goto set_uninstrumentable;
	 }

         // process the jump instruction
         allInstr[numInsns] = insn;
         numInsns++;

         if (jumpTableSz > 0) {
            // skip the jump table
            // insert an illegal instruction with the size of the jump table
            insn = instruction(instr, ILLEGAL, jumpTableSz);
            allInstr[numInsns] = insn;
            numInsns++;
            insnSize += jumpTableSz;
         }
      } else if (insn.isJumpDir()) {
         // check for jumps out of this function
         Address target = insn.getTarget(adr);
         owner->addJumpTarget(target);
         if (target < getAddress(0) || target >= getAddress(0) + size()) {
            // jump out of function
            instPoint *p = new instPoint(this, owner, adr, functionExit, insn);
            funcReturns.push_back(p);
            points[npoints++] = point_(p, numInsns, ReturnPt);
         } 
      } else if (insn.isReturn()) {

         instPoint *p = new instPoint(this, owner, adr, functionExit, insn);
         funcReturns.push_back(p);
         points[npoints++] = point_(p, numInsns, ReturnPt);

      } else if (insn.isCall()) {

         // validTarget is set to false if the call target is not a valid 
         // address in the applications process space 
         bool validTarget = true;

         if ( isRealCall(insn, adr, owner, validTarget, this) ) {
            instPoint *p = new instPoint(this, owner, adr, callSite, insn);
            calls.push_back(p);
            points[npoints++] = point_(p, numInsns, CallPt);
         } else {

            // Call was to an invalid address, do not instrument function 
            if (validTarget == false) {
	      goto set_uninstrumentable;
            }

            // Force relocation when instrumenting function
            relocatable_ = true;
         }
      }
      else if (insn.isLeave())
          noStackFrame = false;

      allInstr[numInsns] = insn;
      numInsns++;
      assert(npoints <= size());
      assert(numInsns <= size());
   }


   unsigned u;
   // there are often nops after the end of the function. We get them here,
   // since they may be usefull to instrument the return point
   for (u = 0; u < 4; u++) {
     
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
   for (u = 0; u < npoints; u++) {
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
      if (type == ReturnPt && p->address() == funcEnd-1) {

         /* If an instrumentation point at the end of the function
            does not end on a 4-byte boundary, we claim the bytes up
            to the next 4-byte boundary (or the next symbol, whichever
            comes first) as "bonus bytes" for the point, since the
            next function will likely begin at or past the boundary.
            We do not try to reclaim more than 4 bytes, because this
	    approach is a hack in the first place and may break if some
	    unlabeled data is located after the ret instruction (we have
	    not seen it happening, but it is possible). Remove when
	    a more robust instrumentation mechanism is implemented! */
	 unsigned bonus = 0;
#ifndef i386_unknown_nt4_0 /* VC++ does not align functions by default */
	 Address nextPossibleEntry = funcEnd;
	 while ((nextPossibleEntry & 3) != 0 &&
		!owner->hasSymbolAtPoint(nextPossibleEntry)) {
	     bonus++;
	     nextPossibleEntry++;
	 }
#endif /* i386_unknown_nt4_0 */
       //p->setBonusBytes(bonus);
         unsigned u1;
         for (u1 = index+1 + bonus; u1 < index+JUMP_SZ-1 && u1 < numInsns; u1++) {
            if (allInstr[u1].isNop() || *(allInstr[u1].ptr()) == 0xCC) {
               //p->addInstrAfterPt(allInstr[u1]);
               bonus++;
               thisPointEnd = u1;
            }
            else 
               break;
         }
         p->setBonusBytes(bonus);
      } else if (type == ReturnPt) {
         // normally, we would not add instructions after the return, but the 
         // compilers often add nops after the return, and we can use them if necessary
         for (unsigned u1 = index+1; u1 < index+JUMP_SZ-1 && u1 < numInsns; u1++) {
            if (allInstr[u1].isNop() || *(allInstr[u1].ptr()) == 0xCC) {
               p->addInstrAfterPt(allInstr[u1]);
               thisPointEnd = u1;
            }
            else 
               break;
         }
      } else {
         size = p->size();
         unsigned maxSize = JUMP_SZ;
         if (type == EntryPt) maxSize = 2*JUMP_SZ;
         for (unsigned u1 = index+1; size < maxSize && u1 <= numInsns; u1++) {
            if (((u+1 == npoints) || (u+1 < npoints && points[u+1].index > u1))
                && !allInstr[u1].isCall()) {
               p->addInstrAfterPt(allInstr[u1]);
               size += allInstr[u1].size();
               thisPointEnd = u1;
            }
            else 
               break;
         }
      }
   }


   for (u = 0; u < npoints; u++)
      points[u].point->checkInstructions();

   // create and sort vector of instPoints
   sorted_ips_vector(foo);

   unsigned int i;
   for (i=0;i<foo.size();i++) {

      if (_usesTrap(foo[i], funcEntry_, funcReturns) && size() >= 5) {
         relocatable_ = true;
      }
   }

   // if the function contains a jump to a jump table, we can't relocate
   // the function 
   if ( !canBeRelocated ) {

      // Function would have needed relocation 
      if (relocatable_ == true) {

#ifdef DEBUG_FUNC_RELOC      
         cerr << prettyName() << endl;
         cerr << "Jump Table: Can't relocate function" << endl;
#endif

         relocatable_ = false;

      }
   }

   delete [] points;
   delete [] allInstr;

   isInstrumentable_ = true;
   return true;

 set_uninstrumentable:
   if (points) delete [] points;
   if (allInstr) delete [] allInstr;

   isInstrumentable_ = false;
   return false;
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

  const unsigned char *origInsn = insn.ptr();
  unsigned insnType = insn.type();
  unsigned insnSz = insn.size();
  unsigned char *first = newInsn;

  int oldDisp;
  int newDisp;

  if (insnType & REL_B) {
    /* replace with rel32 instruction, opcode is one byte. */
    if (*origInsn == JCXZ) {
      oldDisp = (int)*(const char *)(origInsn+1);
      newDisp = (origAddr + 2) + oldDisp - (newAddr + 9);
      *newInsn++ = *origInsn; *(newInsn++) = 2; // jcxz 2
      *newInsn++ = 0xEB; *newInsn++ = 5;        // jmp 5
      *newInsn++ = 0xE9;                        // jmp rel32
      *((int *)newInsn) = newDisp;
      newInsn += sizeof(int);
    }
    else {
      unsigned newSz=UINT_MAX;
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
      assert(newSz!=UINT_MAX);
      oldDisp = (int)*(const char *)origInsn;
      newDisp = (origAddr + 2) + oldDisp - (newAddr + newSz);
      *((int *)newInsn) = newDisp;
      newInsn += sizeof(int);
    }
  }
  else if (insnType & REL_W) {
    /* Skip prefixes */
    if (insnType & PREFIX_OPR)
      origInsn++;
    if (insnType & PREFIX_SEG)
      origInsn++;
    /* opcode is unchanged, just relocate the displacement */
    if (*origInsn == (unsigned char)0x0F)
      *newInsn++ = *origInsn++;
    *newInsn++ = *origInsn++;
    oldDisp = *((const short *)origInsn);
    newDisp = (origAddr + 5) + oldDisp - (newAddr + 3);
    *((int *)newInsn) = newDisp;
    newInsn += sizeof(int);
  } else if (insnType & REL_D) {
    // Skip prefixes
    unsigned nPrefixes = 0;
    if (insnType & PREFIX_OPR)
      nPrefixes++;
    if (insnType & PREFIX_SEG)
      nPrefixes++;
    for (unsigned u = 0; u < nPrefixes; u++)
      *newInsn++ = *origInsn++;

    /* opcode is unchanged, just relocate the displacement */
    if (*origInsn == 0x0F)
      *newInsn++ = *origInsn++;
    *newInsn++ = *origInsn++;
    oldDisp = *((const int *)origInsn);
    newDisp = (origAddr + insnSz) + oldDisp - (newAddr + insnSz);
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


/*
 * Relocate a conditional jump and change the target to newTarget.
 * The new target must be within 128 bytes from the new address
 * Size of instruction is unchanged.
 * Returns the old target
 */
unsigned changeConditionalJump(instruction insn,
			 int origAddr, int newAddr, int newTargetAddr,
			 unsigned char *&newInsn)
{

  const unsigned char *origInsn = insn.ptr();
  unsigned insnType = insn.type();
  unsigned insnSz = insn.size();

  int oldDisp=-1;
  int newDisp;

  if (insnType & REL_B) {
    if (insnSz == 2) {
      /* one byte opcode followed by displacement */
      /* opcode is unchanged */
      *newInsn++ = *origInsn++;
      oldDisp = (int)*(const char *)origInsn;
      newDisp = newTargetAddr - (newAddr + insnSz);
      *newInsn++ = (char)newDisp;
    }
    else if (insnSz == 3) {
      /* one byte prefix followed by */
      /* one byte opcode followed by displacement */
      /* opcode and prefix are unchanged */
      *newInsn++ = *origInsn++;
      *newInsn++ = *origInsn++;
      oldDisp = (int)*(const char *)origInsn;
      newDisp = newTargetAddr - (newAddr + insnSz);
      *newInsn++ = (char)newDisp;
    }
    else {
      /* invalid size instruction */
      assert (0);
    }
  }
  else if (insnType & REL_W) {
    /* Skip prefixes */
    if (insnType & PREFIX_OPR)
      *newInsn++ = *origInsn++;
    if (insnType & PREFIX_SEG)
      *newInsn++ = *origInsn++;

    assert(*origInsn==0x0F);
    *newInsn++ = *origInsn++; // copy the 0x0F
    *newInsn++ = *origInsn++; // second opcode byte

    oldDisp = *((const short *)origInsn);
    newDisp = newTargetAddr - (newAddr + insnSz);
    *((short *)newInsn) = (short)newDisp;
    newInsn += sizeof(short);
  }
  else if (insnType & REL_D) {
    // Skip prefixes
    if (insnType & PREFIX_OPR)
      *newInsn++ = *origInsn++;
    if (insnType & PREFIX_SEG)
      *newInsn++ = *origInsn++;

    assert(*origInsn==0x0F);
    *newInsn++ = *origInsn++; // copy the 0x0F
    *newInsn++ = *origInsn++; // second opcode byte

    oldDisp = *((const int *)origInsn);
    newDisp = newTargetAddr - (newAddr + insnSz);
    *((int *)newInsn) = (int)newDisp;
    newInsn += sizeof(int);
  }

  assert (oldDisp!=-1);
  return (origAddr+insnSz+oldDisp);
}



unsigned getRelocatedInstructionSz(instruction insn)
{
  const unsigned char *origInsn = insn.ptr();
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


bool registerSpace::readOnlyRegister(Register) {
  return false;
}

/*
   We don't use the machine registers to store temporaries,
   but "virtual registers" that are located on the stack.
   The stack frame for a tramp is:

     ebp->    saved ebp (4 bytes)
     ebp-4:   128-byte space for 32 virtual registers (32*4 bytes)
     ebp-132: saved registers (8*4 bytes)
     ebp-164: saved flags registers (4 bytes)

     The temporaries are assigned numbers from 1 so that it is easier
     to refer to them: -(reg*4)[ebp]. So the first reg is -4[ebp].

     We are using a fixed number of temporaries now (32), but we could 
     change to using an arbitrary number.

*/
Register deadList[NUM_VIRTUAL_REGISTERS];
int deadListSize = sizeof(deadList);

void initTramps(bool is_multithreaded)
{
   static bool inited = false;

   if (inited) return;
   inited = true;
   
   unsigned regs_to_loop_over;
   if(is_multithreaded)
      regs_to_loop_over = NUM_VIRTUAL_REGISTERS - 1;
   else
      regs_to_loop_over = NUM_VIRTUAL_REGISTERS;

   for (unsigned u = 0; u < regs_to_loop_over; u++) {
      deadList[u] = u+1;
   }

   regSpace = new registerSpace(deadListSize/sizeof(Register), deadList,
                                0, NULL, is_multithreaded);
}


static void emitJump(unsigned disp32, unsigned char *&insn);
static void emitSimpleInsn(unsigned opcode, unsigned char *&insn);
static void emitPushImm(unsigned long imm, unsigned char *&insn); 
static void emitMovRegToReg(Register dest, Register src, unsigned char *&insn);
static void emitAddMemImm32(Address dest, int imm, unsigned char *&insn);
static void emitAddRegImm32(Register dest, int imm, unsigned char *&insn);
static void emitOpRegImm(int opcode, Register dest, int imm, unsigned char *&insn);
static void emitMovRegToRM(Register base, int disp, Register src, unsigned char *&insn);
static void emitMovRMToReg(Register dest, Register base, int disp, unsigned char *&insn);
void emitCallRel32(unsigned disp32, unsigned char *&insn); // XXX used by paradyn
static void emitOpRegRM(unsigned opcode, Register dest, Register base, int disp,
                 unsigned char *&insn);


/*
 * change the insn at addr to be a branch to newAddr.
 *   Used to add multiple tramps to a point.
 */
void generateBranch(process *proc, Address fromAddr, Address newAddr)
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

#if !defined(i386_unknown_nt4_0)
  bool err = false;
  Address addr = proc->findInternalAddress("DYNINSTtrampTable",true, err);
  assert(err==false);
  return proc->writeDataSpace((caddr_t)addr+u*sizeof(trampTableEntry),
		       sizeof(trampTableEntry),
		       (caddr_t)&(proc->trampTable[u]));
#else
  return true;
#endif
}

/* Generate a jump to a base tramp. Return the size of the instruction
   generated at the instrumentation point. */
unsigned generateBranchToTramp(process *proc, const instPoint *point, 
			       Address baseAddr, Address imageBaseAddr,
			       unsigned char *insn, bool &deferred)
{
     /* There are three ways to get to the base tramp:
	1. Ordinary 5-byte jump instruction.
	2. 2-byte jump to the extra slot in the entry point
	3. Trap instruction.
     */

  // VG(7/29/02): Added sequence to pad the rest of the point with nops
  // This is vital if one has to debug the instrumented mutatee...

  int wrote = 0;

  /* Ordinary 5-byte jump */
  if (point->size() >= JUMP_REL32_SZ) {
    // replace instructions at point with jump to base tramp
    emitJump(baseAddr - (point->jumpAddr() + imageBaseAddr + JUMP_REL32_SZ), insn);
    wrote = JUMP_REL32_SZ;
  }
  /* Extra slot */
  else if (point->canUseExtraSlot(proc)) {
    pd_Function *f = point->func();
    const instPoint *the_entry = f->funcEntry(proc);
    
    int displacement = the_entry->jumpAddr() + 5 - point->jumpAddr();
    assert(displacement < 0);
    assert((displacement-2) > SCHAR_MIN);
    assert(point->size() >= 2);
#ifdef INST_TRAP_DEBUG
    cerr << "Using extra slot in entry of " << f->prettyName()
         << " to avoid need for trap @" << (void*)point->address() << endl;
#endif
    
    instPoint *nonConstEntry = const_cast<instPoint *>(the_entry);
    returnInstance *retInstance;
    
    trampTemplate *entryBase =
      findOrInstallBaseTramp(proc, nonConstEntry,
                              retInstance, false, false, deferred);
    assert(entryBase);
    if (retInstance) {
      retInstance->installReturnInstance(proc);
    }
    generateBranch(proc, the_entry->jumpAddr()+imageBaseAddr+5, baseAddr);
    *insn++ = 0xEB;
    *insn++ = (char)(displacement-2);
    wrote = 2;
  }
  else { /* Trap */
#ifdef INST_TRAP_DEBUG
    cerr << "Warning: unable to insert jump in function " 
         << point->func()->prettyName() << " @" << (void*)point->address()
         << ". Using trap!" << endl;
#endif
    if (!insertInTrampTable(proc, point->jumpAddr()+imageBaseAddr, baseAddr))
      return 0;
    *insn++ = 0xCC;
    wrote = 1;
  }
  
  // VG(8/4/2): For reasons yet to be determined, this breaks test1 (on x86)
  // So don't enable it unless you are working on test6/memory instrumentation
#ifdef IA32_NOP_PADDING
   for(int rest = point->size() - wrote; rest > 0; --rest, ++insn)
     *insn = 0x90;
   return point->size();
#else
  return wrote;
#endif
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#if defined(i386_unknown_solaris2_5) || \
    defined(i386_unknown_linux2_0) || \
    defined(i386_unknown_nt4_0)
int guard_code_before_pre_instr_size  = 19; /* size in bytes */
int guard_code_after_pre_instr_size   = 10; /* size in bytes */
int guard_code_before_post_instr_size = 19; /* size in bytes */
int guard_code_after_post_instr_size  = 10; /* size in bytes */
#else
int guard_code_before_pre_instr_size  = 0; /* size in bytes */
int guard_code_after_pre_instr_size   = 0; /* size in bytes */
int guard_code_before_post_instr_size = 0; /* size in bytes */
int guard_code_after_post_instr_size  = 0; /* size in bytes */
#endif

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

unsigned int get_guard_code_size() /* total size in bytes of the four
				      guard code fragments*/
{
  return
    guard_code_before_pre_instr_size +
    guard_code_after_pre_instr_size +
    guard_code_before_post_instr_size +
    guard_code_after_post_instr_size;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void emitJccR8( int condition_code,
		char jump_offset,
		unsigned char * & instruction )
{
  *instruction++ = condition_code;
  *instruction++ = jump_offset;
}

// VG(8/15/02): nicer jcc: condition is the tttn field.
// Because we generate jumps twice, once with bogus 0
// offset, and then with the right offset, the instruction
// may be longer (and overwrite something else) the 2nd time.
// So willRegen defaults to true and always generates jcc near
// (the longer form)

// TODO: generate JEXCZ as well
static inline void emitJcc(int condition, int offset, unsigned char*& instruction, 
                           bool willRegen=true)
{
  unsigned char opcode;

  assert(condition >= 0 && condition <= 0x0F);

  if(!willRegen && (offset >= -128 && offset <= 127)) { // jcc rel8
    opcode = 0x70 | (unsigned char)condition;
    *instruction++ = opcode;
    *instruction++ = (unsigned char) (offset & 0xFF);
  }
  else { // jcc near rel32
    opcode = 0x80 | (unsigned char)condition;
    *instruction++ = 0x0F;
    *instruction++ = opcode;
    *((int*)instruction) = offset;
    instruction += sizeof(int);
  }
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#if defined(i386_unknown_solaris2_5) || \
    defined(i386_unknown_linux2_0) || \
    defined(i386_unknown_nt4_0)
void generate_guard_code( unsigned char * buffer,
			  const NonRecursiveTrampTemplate & base_template,
			  Address /* base_address */,
			  Address guard_flag_address )
{
  unsigned char * instruction;

  /* guard-on code before pre instr */
  /* Code generated:
   * --------------
   * cmpl   $0x0, (guard flag address)
   * je     <after guard-off code>
   * movl   $0x0, (guard flag address)
   */
  instruction = buffer + base_template.guardOnPre_beginOffset;
  /*            CMP_                       (memory address)__  0 */
  emitOpRMImm8( 0x83, 0x07, Null_Register, guard_flag_address, 0, instruction );
  emitJccR8( JE_R8, buffer + base_template.guardOffPre_endOffset - ( instruction + 2 ), instruction );
  emitMovImmToMem( guard_flag_address, 0, instruction );

  /* guard-off code after pre instr */
  /* Code generated:
   * --------------
   * movl   $0x1, (guard flag address)
   */
  instruction = buffer + base_template.guardOffPre_beginOffset;
  emitMovImmToMem( guard_flag_address, 1, instruction );

  /* guard-on code before post instr */
  instruction = buffer + base_template.guardOnPost_beginOffset;
  emitOpRMImm8( 0x83, 0x07, Null_Register, guard_flag_address, 0x00, instruction );
  emitJccR8( JE_R8, buffer + base_template.guardOffPost_endOffset - ( instruction + 2 ), instruction );
  emitMovImmToMem( guard_flag_address, 0, instruction );

  /* guard-off code after post instr */
  instruction = buffer + base_template.guardOffPost_beginOffset;
  emitMovImmToMem( guard_flag_address, 1, instruction );
}
#else
void generate_guard_code( unsigned char * /* buffer */,
			  const NonRecursiveTrampTemplate & /* base_template */,
			  Address /* base_address */,
			  Address /* guard_flag_address */ )
{
}
#endif

void generateMTpreamble(char *insn, Address &base, process *proc) {
   AstNode *threadPOS;
   pdvector<AstNode *> dummy;
   Register src = Null_Register;

   // registers cleanup
   regSpace->resetSpace();

   /* Get the hashed value of the thread */
   if (!proc->multithread_ready()) {
      // Uh oh... we're not ready to build a tramp yet!
      //cerr << "WARNING: tramp constructed without RT multithread support!"
      //     << endl;
      threadPOS = new AstNode("DYNINSTreturnZero", dummy);
   }
   else 
      threadPOS = new AstNode("DYNINSTthreadIndex", dummy);

   src = threadPOS->generateCode(proc, regSpace, (char *)insn,
                                 base, 
                                 false, // noCost 
                                 true); // root node

   if ((src) != REG_MT_POS) {
      // This is always going to happen... we reserve REG_MT_POS, so the
      // code generator will never use it as a destination
      emitV(orOp, src, 0, REG_MT_POS, insn, base, false);
   }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
 * Install a base tramp, relocating the instructions at location
 * The pre and post jumps are filled with a 'jump 0'
 * Return a descriptor for the base tramp.
 *
 */

trampTemplate *installBaseTramp(const instPoint *location, process *proc,
                                bool noCost, bool trampRecursiveDesired = true)
{  
   bool aflag;
  
   /*
     The base tramp:
     addr   instruction             cost
     0:    <relocated instructions before point>
     a = size of relocated instructions before point
     a+0:   jmp a+30 <skip pre insn>  1
     a+5:   pushfd                    5
     a+6:   pushad                    9
     a+7:   push ebp                  1
     a+8:   mov esp, ebp              1
     a+10:  subl esp, 0x80            1
     a+16:  jmp <local pre inst>      1
     a+21:  leave                     3
     a+22:  popad                    14
     a+23:  popfd                     5
     a+24:  add costAddr, cost        3
     a+34:  <relocated instructions at point>

     b = a +30 + size of relocated instructions at point
     b+0:   jmp b+30 <skip post insn>
     b+5:   pushfd
     b+6:   pushad
     b+7:   push ebp
     b+8:   mov esp, ebp
     b+10:  subl esp, 0x80
     b+16:  jmp <local post inst>
     b+21:  leave
     b+22:  popad
     b+23:  popfd
     b+24:  <relocated instructions after point>

     c:     jmp <return to user code>

     tramp size = 2*23 + 10 + 5 + size of relocated instructions
     Make sure to update the size if the tramp is changed

     cost of pre and post instrumentation is (1+1+1+5+9+1+1+15+5+3) = 42
     cost of rest of tramp is (1+3+1+1)

     [mihai Wed Apr 12 00:22:03 CDT 2000]
     Additionally, if a guarded template is generated
     (i.e. trampRecursiveDesired = false), four more code fragments are
     inserted:
     - code to turn the guard on (19 bytes): between a+15 and a+16, and
     between b+15 and b+16
     - code to turn the guard off (10 bytes): between a+21 and a+26, and
     between b+21 and b+26
     A total of 58 bytes are added to the base tramp.
   */

   //pd_Function *f = location->func();

   unsigned u;
   trampTemplate *ret = 0;
   
   if( trampRecursiveDesired )
   {
       ret = new trampTemplate(location, proc);
   }
   else
   {
       // Stores a few more data members
       ret = new NonRecursiveTrampTemplate(location, proc);
   }
   ret->trampTemp = 0;

   unsigned jccTarget = 0; // used when instr. at the point is a cond. jump
   unsigned auxJumpOffset = 0;

   // compute the tramp size
   // if there are any changes to the tramp, the size must be updated.
   unsigned trampSize;
   if(proc->multithread_capable())
      trampSize = 12+63 + 38;  // 38 needs to be verified
   else
      trampSize = 12+63;

   if (location->isConservative()) trampSize += 13*2;

   if( ! trampRecursiveDesired ) {
      trampSize += get_guard_code_size();   // NOTE-131 with Relocation
   }

   for (u = 0; u < location->insnsBefore(); u++) {
      trampSize += getRelocatedInstructionSz(location->insnBeforePt(u));
   }

   if (location->insnAtPoint().type() & IS_JCC) {
      trampSize += location->insnAtPoint().size() + 2 * JUMP_SZ;
   } else {
      trampSize += getRelocatedInstructionSz(location->insnAtPoint());
   }
  
   for (u = 0; u < location->insnsAfter(); u++) {
      trampSize += getRelocatedInstructionSz(location->insnAfterPt(u));
   }


   Address imageBaseAddr;
   if (!proc->getBaseAddress(location->owner(), imageBaseAddr)) {
      abort();
   }

   Address costAddr = 0; // for now...
   if (!noCost) {
#ifdef SHM_SAMPLING
      costAddr = (Address)proc->getObsCostLowAddrInApplicSpace();
      assert(costAddr);
#else
      // get address of DYNINSTobsCostLow to update observed cost
      bool err = false;
      costAddr = proc->findInternalAddress("DYNINSTobsCostLow", true, err);
      assert(costAddr && !err);
#endif
   }

   ret->size = trampSize;
   Address baseAddr = proc->inferiorMalloc(trampSize, textHeap);
   // cout << "installBaseTramp(): trampoline base address = 0x"
   //      << setw( 8 ) << setfill( '0' ) << std::hex << baseAddr
   //      << std::dec <<endl;
   ret->baseAddr = baseAddr;

   unsigned char *code = new unsigned char[2*trampSize];
   unsigned char *insn = code;
   Address currAddr = baseAddr;

   // MT FIXME

   // get the current instruction that is being executed. If the PC is at a
   // instruction that is being relocated, we must change the PC.

   Frame frame = proc->getRepresentativeLWP()->getActiveFrame();
   Address currentPC = frame.getPC();

   // emulate the instructions before the point
   Address origAddr = location->jumpAddr() + imageBaseAddr;

   for (u = location->insnsBefore(); u > 0; ) {
      --u;
      if (currentPC == origAddr) {
         //fprintf(stderr, "changed PC: 0x%lx to 0x%lx\n", currentPC,currAddr);
         proc->getRepresentativeLWP()->changePC(currAddr, NULL);
      }

      unsigned newSize = relocateInstruction(location->insnBeforePt(u),
                                             origAddr, currAddr, insn);
      aflag=(newSize == getRelocatedInstructionSz(location->insnBeforePt(u)));
      assert(aflag);
      currAddr += newSize;
      origAddr += location->insnBeforePt(u).size();
   }


   /***
       If the instruction at the point is a conditional jump, we relocate it
       to the top of the base tramp, and change the code so that the tramp is
       executed only if the branch is taken.

       e.g.
       L1:  jne target
       L2:  ...

       becomes

       jne T1
       jmp T2
       jne
       T1:  ...

       T2:  relocated instructions after point

       then later at the base tramp, at the point where we relocate the
       instruction at the point, we insert a jump to target
   ***/
   if (location->insnAtPoint().type() & IS_JCC) {

      currAddr = baseAddr + (insn - code);
      assert(origAddr == location->address() + imageBaseAddr);
      origAddr = location->address() + imageBaseAddr;
      if (currentPC == origAddr &&
          currentPC != (location->jumpAddr() + imageBaseAddr)) {
         //fprintf(stderr, "changed PC: 0x%lx to 0x%lx\n", currentPC,currAddr);
         proc->getRepresentativeLWP()->changePC(currAddr, NULL);
      }

      jccTarget =
         changeConditionalJump(location->insnAtPoint(), origAddr, currAddr,
                               currAddr+location->insnAtPoint().size()+5,insn);
      currAddr += location->insnAtPoint().size();
      auxJumpOffset = insn-code;
      emitJump(0, insn);
      origAddr += location->insnAtPoint().size();
   }

   if (location->isConservative() && !noCost)
      emitSimpleInsn(PUSHFD, insn);    // pushfd

   // pre branches
   // skip pre instrumentation
   ret->skipPreInsOffset = insn-code;
   emitJump(0, insn);

   // save registers and create a new stack frame for the tramp
   ret->savePreInsOffset = insn-code;
   if (!location->isConservative() || noCost)
      emitSimpleInsn(PUSHFD, insn);    // pushfd
   emitSimpleInsn(PUSHAD, insn);    // pushad
   emitPushImm(location->iPgetAddress(), insn);  // return address for stack frame format
   emitSimpleInsn(PUSH_EBP, insn);  // push ebp (new stack frame)
   emitMovRegToReg(EBP, ESP, insn); // mov ebp, esp  (2-byte instruction)
   if (location->isConservative()) {
      // allocate space for temporaries (virtual registers) and floating
      // point state
      emitOpRegImm(5, ESP, 128 + FSAVE_STATE_SIZE, insn); // sub esp, 128
      // save floating point state
      emitOpRegRM(FSAVE, FSAVE_OP, EBP, -128 - FSAVE_STATE_SIZE, insn);
   } else {
      // allocate space for temporaries (virtual registers)
      emitOpRegImm(5, ESP, 128, insn); // sub esp, 128
   }

   Address base=0;
   if(proc->multithread_capable()) {
      // generate preamble for MT version
      generateMTpreamble((char *)insn, base, proc);
      insn += base;
   }

   if( ! trampRecursiveDesired )
   {
      NonRecursiveTrampTemplate * temp_ret = (NonRecursiveTrampTemplate *)ret;
      temp_ret->guardOnPre_beginOffset = insn - code;
      for( int i = 0; i < guard_code_before_pre_instr_size; i++ )
         emitSimpleInsn( 0x90, insn );
      temp_ret->guardOnPre_endOffset = insn - code;
   }

   // local pre branch
   ret->localPreOffset = insn-code;
   emitJump(0, insn);

   ret->localPreReturnOffset = insn-code;

   if( ! trampRecursiveDesired )
   {
      NonRecursiveTrampTemplate * temp_ret = (NonRecursiveTrampTemplate *)ret;
      temp_ret->guardOffPre_beginOffset = insn - code;
      for( int i = 0; i < guard_code_after_pre_instr_size; i++ )
         emitSimpleInsn( 0x90, insn );
      temp_ret->guardOffPre_endOffset = insn - code;
   }

   // restore registers
   if (location->isConservative())
      emitOpRegRM(FRSTOR, FRSTOR_OP, EBP, -128 - FSAVE_STATE_SIZE, insn);
   emitSimpleInsn(LEAVE, insn);     // leave
   emitSimpleInsn(POP_EAX, insn);   // pop return address
   emitSimpleInsn(POPAD, insn);     // popad
   if (!location->isConservative() || noCost)
      emitSimpleInsn(POPFD, insn);     // popfd
   ret->restorePreInsOffset = insn-code;

   // update cost
   // update cost -- a 10-byte instruction
   ret->updateCostOffset = insn-code;
   currAddr = baseAddr + (insn-code);
   ret->costAddr = currAddr;
   if (!noCost) {
      emitAddMemImm32(costAddr, 88, insn);  // add (costAddr), cost
      if (location->isConservative())
         emitSimpleInsn(POPFD, insn);     // popfd
   }
   else {
      // minor hack: we still need to fill up the rest of the 10 bytes, since
      // assumptions are made about the positioning of instructions that
      // follow.  (This could in theory be fixed) So, 10 NOP instructions
      // (each 1 byte)
      for (unsigned foo=0; foo < 10; foo++)
         emitSimpleInsn(0x90, insn); // NOP
   }
   
   if (!(location->insnAtPoint().type() & IS_JCC)) {
      // emulate the instruction at the point 
      ret->emulateInsOffset = insn-code;
      currAddr = baseAddr + (insn - code);
      assert(origAddr == location->address() + imageBaseAddr);
      origAddr = location->address() + imageBaseAddr;
      if (currentPC == origAddr &&
          currentPC != (location->jumpAddr() + imageBaseAddr)) {
         //fprintf(stderr, "changed PC: 0x%lx to 0x%lx\n", currentPC,currAddr);
         proc->getRepresentativeLWP()->changePC(currAddr, NULL);
      }

      unsigned newSize =
         relocateInstruction(location->insnAtPoint(), origAddr, currAddr,insn);
      aflag=(newSize == getRelocatedInstructionSz(location->insnAtPoint()));
      assert(aflag);
      currAddr += newSize;
      origAddr += location->insnAtPoint().size();
   } else {
      // instruction at point is a conditional jump.  The instruction was
      // relocated to the beggining of the tramp (see comments above) We must
      // generate a jump to the original target here
      assert(jccTarget > 0);
      currAddr = baseAddr + (insn - code);
      emitJump(jccTarget-(currAddr+JUMP_SZ), insn);
      currAddr += JUMP_SZ;
   }

   // post branches
   // skip post instrumentation
   ret->skipPostInsOffset = insn-code;
   emitJump(0, insn);


   // save registers and create a new stack frame for the tramp
   ret->savePostInsOffset = insn-code;
   emitSimpleInsn(PUSHFD, insn);    // pushfd
   emitSimpleInsn(PUSHAD, insn);    // pushad
   emitPushImm(location->iPgetAddress(), insn);
   emitSimpleInsn(PUSH_EBP, insn);  // push ebp
   emitMovRegToReg(EBP, ESP, insn); // mov ebp, esp
   // allocate space for temporaries (virtual registers)
   if (location->isConservative()) {
      // allocate space for temporaries (virtual registers) and floating
      // point state
      emitOpRegImm(5, ESP, 128 + FSAVE_STATE_SIZE, insn); // sub esp, 128
      // save floating point state
      emitOpRegRM(FSAVE, FSAVE_OP, EBP, -128 - FSAVE_STATE_SIZE, insn);
   } else {
      // allocate space for temporaries (virtual registers)
      emitOpRegImm(5, ESP, 128, insn); // sub esp, 128
   }


   if(proc->multithread_capable()) {
      // generate preamble for MT version
      base=0;
      generateMTpreamble((char *)insn, base, proc);
      insn += base;
   }

   if( ! trampRecursiveDesired )
   {
      NonRecursiveTrampTemplate * temp_ret = (NonRecursiveTrampTemplate *)ret;
      temp_ret->guardOnPost_beginOffset = insn - code;
      for( int i = 0; i < guard_code_before_post_instr_size; i++ )
         emitSimpleInsn( 0x90, insn );
      temp_ret->guardOnPost_endOffset = insn - code;
   }

   // local post branch
   ret->localPostOffset = insn-code;
   emitJump(0, insn);

   ret->localPostReturnOffset = insn-code;

   if( ! trampRecursiveDesired )
   {
      NonRecursiveTrampTemplate * temp_ret = (NonRecursiveTrampTemplate *)ret;
      temp_ret->guardOffPost_beginOffset = insn - code;
      for( int i = 0; i < guard_code_after_post_instr_size; i++ )
         emitSimpleInsn( 0x90, insn );
      temp_ret->guardOffPost_endOffset = insn - code;
   }

   // restore registers
   if (location->isConservative())
      emitOpRegRM(FRSTOR, FRSTOR_OP, EBP, -128 - FSAVE_STATE_SIZE, insn);
   emitSimpleInsn(LEAVE, insn);     // leave
   emitSimpleInsn(POP_EAX, insn);   // pop return address
   emitSimpleInsn(POPAD, insn);     // popad
   emitSimpleInsn(POPFD, insn);     // popfd
   ret->restorePostInsOffset = insn-code;
  
   // emulate the instructions after the point
   ret->returnInsOffset = insn-code;
   currAddr = baseAddr + (insn - code);
   assert(origAddr == location->address() + imageBaseAddr + 
                      location->insnAtPoint().size());
   origAddr = location->address() + imageBaseAddr +
              location->insnAtPoint().size();
   for (u = 0; u < location->insnsAfter(); u++) {
      if (currentPC == origAddr) {
         //fprintf(stderr, "changed PC: 0x%lx to 0x%lx\n", currentPC,currAddr);
         proc->getRepresentativeLWP()->changePC(currAddr, NULL);
      }
      unsigned newSize = relocateInstruction(location->insnAfterPt(u), 
                                             origAddr, currAddr, insn);
      aflag=(newSize == getRelocatedInstructionSz(location->insnAfterPt(u)));
      assert(aflag);
      currAddr += newSize;
      origAddr += location->insnAfterPt(u).size();
   }

   // return to user code
   currAddr = baseAddr + (insn - code);
   emitJump(location->returnAddr()+imageBaseAddr - (currAddr+JUMP_SZ), insn);
#ifdef INST_TRAP_DEBUG
   cerr << "installBaseTramp jump back to " <<
      (void*)( location->returnAddr() + imageBaseAddr ) << endl;
#endif
   
   assert((unsigned)(insn-code) == trampSize);

   // update the jumps to skip pre and post instrumentation
   unsigned char *ip = code + ret->skipPreInsOffset;
   emitJump(ret->updateCostOffset - (ret->skipPreInsOffset+JUMP_SZ), ip);
   ip = code + ret->skipPostInsOffset;
   emitJump(ret->returnInsOffset - (ret->skipPostInsOffset+JUMP_SZ), ip);

   if (auxJumpOffset > 0) {
      ip = code + auxJumpOffset;
      emitJump(ret->returnInsOffset - (auxJumpOffset+JUMP_SZ), ip);
   }

   if( ! trampRecursiveDesired )
   {
      /* prepare guard flag memory, if needed */
      Address guardFlagAddress = proc->trampGuardAddr();

      NonRecursiveTrampTemplate * temp_ret = (NonRecursiveTrampTemplate *)ret;
      generate_guard_code( code, * temp_ret, baseAddr, guardFlagAddress );
   }
  
   // put the tramp in the application space
   proc->writeDataSpace((caddr_t)baseAddr, insn-code, (caddr_t) code);
   proc->addCodeRange(baseAddr, ret);

   delete [] code;

   ret->cost = 6;
   //
   // The cost for generateMTpreamble is 25 for pre and post instrumentation:
   // movl   $0x80570ec,%eax                1
   // call   *%ea                           5
   // movl   %eax,0xfffffffc(%ebp)          1
   // shll   $0x2,0xfffffffc(%ebp)         12
   // addl   $0x84ac670,0xfffffffc(%ebp)    4
   // movl   0xfffffffc(%ebp),%eax          1
   // movl   %eax,0xffffff80(%ebp)          1
   //
   ret->prevBaseCost = 42+25;
   ret->postBaseCost = 42+25;
   ret->prevInstru = false;
   ret->postInstru = false;
   return ret;
}


// This function is used to clear a jump from base to minitramps
// For the x86 platform, we generate a jump to the next instruction
void generateNoOp(process *proc, Address addr) 
{
  static unsigned char jump0[5] = { 0xE9, 0, 0, 0, 0 };
  proc->writeDataSpace((caddr_t) addr, 5, (caddr_t)jump0);
}


trampTemplate* findOrInstallBaseTramp(process *proc, 
			   	       instPoint *&location, 
				       returnInstance *&retInstance,
				       bool trampRecursiveDesired,
				       bool noCost,
                                       bool &deferred)
{
    trampTemplate *ret;
    retInstance = NULL;

    pd_Function *f = location->func();

    // location may not have been updated since relocation of function
    if (f->needsRelocation() && f->isInstalled(proc)) {
      f->modifyInstPoint(const_cast<const instPoint *&>(location), proc);
    }

    if (!proc->baseMap.defines(location)) {

        // if function needs relocation  
        if (f->needsRelocation()) {
	
          // if function has not already been relocated
	  if (!f->isInstalled(proc)) {
            bool relocated = f->relocateFunction(proc, location, deferred);
     
            // Silence warnings
            assert(relocated || true);

#ifndef BPATCH_LIBRARY
            if (!relocated) return NULL;
#endif
	  }
	}

	ret = installBaseTramp(location, proc, noCost, trampRecursiveDesired);
	proc->baseMap[location] = ret;

	// generate branch from instrumentation point to base tramp
	Address imageBaseAddr;
	if (!proc->getBaseAddress(location->owner(), imageBaseAddr))
	  abort();
	unsigned char *insn = new unsigned char[JUMP_REL32_SZ];
	unsigned size = generateBranchToTramp(proc, location, ret->baseAddr, 
					      imageBaseAddr, insn, deferred);
	if (size == 0)
	  return NULL;
	retInstance = new returnInstance(location->insns(), 
                                         new instruction(insn, 0, size), size,
					 location->jumpAddr() + imageBaseAddr, 
                                         size);

    } else {
        ret = proc->baseMap[location];
    }
    return(ret);
}


/*
 * Install a single mini-tramp.
 *
 */
void installTramp(miniTrampHandle *mt, process *proc, 
                  char *code, int codeSize)
{
    totalMiniTramps++;
    //insnGenerated += codeSize/sizeof(int);
    proc->writeDataSpace((caddr_t)mt->miniTrampBase, codeSize, code);
    Address atAddr;
    if (mt->when == callPreInsn) {
        if (mt->baseTramp->prevInstru == false) {
	    atAddr = mt->baseTramp->baseAddr+mt->baseTramp->skipPreInsOffset;
	    mt->baseTramp->cost += mt->baseTramp->prevBaseCost;
	    mt->baseTramp->prevInstru = true;
	    generateNoOp(proc, atAddr);
	}
    }
    else {
	if (mt->baseTramp->postInstru == false) {
	    atAddr = mt->baseTramp->baseAddr+mt->baseTramp->skipPostInsOffset; 
	    mt->baseTramp->cost += mt->baseTramp->postBaseCost;
	    mt->baseTramp->postInstru = true;
	    generateNoOp(proc, atAddr);
	}
    }
}


/**************************************************************
 *
 *  code generator for x86
 *
 **************************************************************/




#define MAX_BRANCH	(0x1<<31)

Address getMaxBranch() {
  return (Address)MAX_BRANCH;
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
static inline unsigned char makeModRMbyte(unsigned Mod, unsigned Reg, unsigned RM)
{
  return ((Mod & 0x3) << 6) + ((Reg & 0x7) << 3) + (RM & 0x7);
}

// VG(7/30/02): Build the SIB byte of an instruction */
static inline unsigned char makeSIBbyte(unsigned Scale, unsigned Index, unsigned Base)
{
  return ((Scale & 0x3) << 6) + ((Index & 0x7) << 3) + (Base & 0x7);
}

/* 
   Emit the ModRM byte and displacement for addressing modes.
   base is a register (EAX, ECX, EDX, EBX, EBP, ESI, EDI)
   disp is a displacement
   reg_opcode is either a register or an opcode
*/
static inline void emitAddressingMode(Register base, RegValue disp, int reg_opcode, 
                                      unsigned char *&insn)
{
  assert(base != ESP);
  if (base == Null_Register) {
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

// VG(7/30/02): emit a fully fledged addressing mode: base+index<<scale+disp
static inline void emitAddressingMode(Register base, Register index, unsigned int scale,
                                      RegValue disp, int reg_opcode, unsigned char *&insn)
{
  bool needSIB = (base == ESP) || (index != Null_Register);

  if(!needSIB) {
    emitAddressingMode(base, disp, reg_opcode, insn);
    return;
  }

  assert(index != ESP);

  if(index == Null_Register) {
    assert(base == ESP); // not necessary, but sane
    index = 4;           // (==ESP) which actually means no index in SIB
  }

  if(base == Null_Register) { // we have to emit [index<<scale+disp32]
    *insn++ = makeModRMbyte(0, reg_opcode, 4);
    *insn++ = makeSIBbyte(scale, index, 5);
    *((int *)insn) = disp;
    insn += sizeof(int);
  }
  else if(disp == 0 && base != EBP) { // EBP must have 0 disp8; emit [base+index<<scale]
    *insn++ = makeModRMbyte(0, reg_opcode, 4);
    *insn++ = makeSIBbyte(scale, index, base);
  }
  else if (disp >= -128 && disp <= 127) { // emit [base+index<<scale+disp8]
    *insn++ = makeModRMbyte(1, reg_opcode, 4);
    *insn++ = makeSIBbyte(scale, index, base);
    *((char *)insn++) = (char) disp;
  }
  else { // emit [base+index<<scale+disp32]
    *insn++ = makeModRMbyte(2, reg_opcode, 4);
    *insn++ = makeSIBbyte(scale, index, base);
    *((int *)insn) = disp;
    insn += sizeof(int);
  }
}


/* emit a simple one-byte instruction */
static inline void emitSimpleInsn(unsigned op, unsigned char *&insn) {
  *insn++ = op;
}

static inline void emitPushImm(unsigned long imm, unsigned char *&insn)
{
	unsigned i;
	unsigned char *p = (unsigned char*)&imm;
	*insn++ = 0x68;
	for (i = 0; i < sizeof(imm); i++)
		*insn++ = p[i];
}

// emit a simple register to register instruction: OP dest, src
// opcode is one or two byte
static inline void emitOpRegReg(unsigned opcode, Register dest, Register src,
                  unsigned char *&insn) {
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
static inline void emitOpRegRM(unsigned opcode, Register dest, Register base, int disp,
                 unsigned char *&insn) {
  if (opcode <= 0xff) {
    *insn++ = opcode;
  } else {
    *insn++ = opcode >> 8;
    *insn++ = opcode & 0xff;
  }
  emitAddressingMode(base, disp, dest, insn);
}

// emit OP r/m, reg
static inline void emitOpRMReg(unsigned opcode, Register base, int disp, Register src,
                 unsigned char *&insn) {
  *insn++ = opcode;
  emitAddressingMode(base, disp, src, insn);
}

// emit OP reg, imm32
static inline void emitOpRegImm(int opcode, Register dest, int imm, unsigned char *&insn) {
  *insn++ = 0x81;
  *insn++ = makeModRMbyte(3, opcode, dest);
  *((int *)insn) = imm;
  insn+= sizeof(int);
}

/*
// emit OP r/m, imm32
void emitOpRMImm(unsigned opcode, Register base, int disp, int imm,
                 unsigned char *&insn) {
  *insn++ = 0x81;
  emitAddressingMode(base, disp, opcode, insn);
  *((int *)insn) = imm;
  insn += sizeof(int);
}
*/

// emit OP r/m, imm32
static inline void emitOpRMImm(unsigned opcode1, unsigned opcode2,
                               Register base, int disp, int imm, unsigned char *&insn) {
  *insn++ = opcode1;
  emitAddressingMode(base, disp, opcode2, insn);
  *((int *)insn) = imm;
  insn += sizeof(int);
}

// emit OP r/m, imm8
static inline void emitOpRMImm8(unsigned opcode1, unsigned opcode2,
                                Register base, int disp, char imm, unsigned char *&insn) {
  *insn++ = opcode1;
  emitAddressingMode(base, disp, opcode2, insn);
  *insn++ = imm;
}

// emit OP reg, r/m, imm32
static inline void emitOpRegRMImm(unsigned opcode, Register dest,
                                  Register base, int disp, int imm, unsigned char *&insn) {
  *insn++ = opcode;
  emitAddressingMode(base, disp, dest, insn);
  *((int *)insn) = imm;
  insn += sizeof(int);
}

// emit MOV reg, reg
static inline void emitMovRegToReg(Register dest, Register src, unsigned char *&insn) {
  *insn++ = 0x8B;
  *insn++ = makeModRMbyte(3, dest, src);
}

// emit MOV reg, r/m
static inline void emitMovRMToReg(Register dest, Register base, int disp, unsigned char *&insn) {
  *insn++ = 0x8B;
  emitAddressingMode(base, disp, dest, insn);
}

// emit MOV r/m, reg
static inline void emitMovRegToRM(Register base, int disp, Register src, unsigned char *&insn) {
  *insn++ = 0x89;
  emitAddressingMode(base, disp, src, insn);
}

// emit MOV m, reg
static inline void emitMovRegToM(int disp, Register src, unsigned char *&insn) {
  *insn++ = 0x89;
  emitAddressingMode(Null_Register, disp, src, insn);
}

// emit MOV reg, m
static inline void emitMovMToReg(Register dest, int disp, unsigned char *&insn) {
  *insn++ = 0x8B;
  emitAddressingMode(Null_Register, disp, dest, insn);
}

// emit MOV reg, imm32
static inline void emitMovImmToReg(Register dest, int imm, unsigned char *&insn) {
  *insn++ = 0xB8 + dest;
  *((int *)insn) = imm;
  insn += sizeof(int);
}

// emit MOV r/m32, imm32
static inline void emitMovImmToRM(Register base, int disp, int imm, unsigned char *&insn) {
  *insn++ = 0xC7;
  emitAddressingMode(base, disp, 0, insn);
  *((int*)insn) = imm;
  insn += sizeof(int);
}

// emit MOV mem32, imm32
static inline void emitMovImmToMem(Address maddr, int imm, unsigned char *&insn) {
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
static inline void emitAddMemImm32(Address addr, int imm, unsigned char *&insn) {
  *insn++ = 0x81;
  *insn++ = 0x05;
  *((unsigned *)insn) = addr;
  insn += sizeof(unsigned);
  *((int *)insn) = imm;
  insn += sizeof(int);
}

// emit Add reg, imm32
static inline void emitAddRegImm32(Register reg, int imm, unsigned char *&insn) {
  *insn++ = 0x81;
  *insn++ = makeModRMbyte(3, 0, reg);
  *((int *)insn) = imm;
  insn += sizeof(int);
}

// emit Sub reg, reg
static inline void emitSubRegReg(Register dest, Register src, unsigned char *&insn)
{
  *insn++ = 0x2B;
  *insn++ = makeModRMbyte(3, dest, src);
}

// emit JUMP rel32
static inline void emitJump(unsigned disp32, unsigned char *&insn) {
  if ((signed)disp32 >= 0)
    assert (disp32 < unsigned(1<<31));
  else
    assert ((unsigned)(-(signed)disp32) < unsigned(1<<31));
  *insn++ = 0xE9;
  *((int *)insn) = disp32;
  insn += sizeof(int);
} 

// emit CALL rel32
void emitCallRel32(unsigned disp32, unsigned char *&insn) {
  *insn++ = 0xE8;
  *((int *)insn) = disp32;
  insn += sizeof(int);
}

// set dest=1 if src1 op src2, otherwise dest = 0
void emitRelOp(unsigned op, Register dest, Register src1, Register src2,
               unsigned char *&insn) {
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
void emitRelOpImm(unsigned op, Register dest, Register src1, int src2imm,
                  unsigned char *&insn) {
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

static inline void emitEnter(short imm16, unsigned char *&insn) {
  *insn++ = 0xC8;
  *((short*)insn) = imm16;
  insn += sizeof(short);
  *insn++ = 0;
}



Register emitFuncCall(opCode op, 
		      registerSpace *rs,
		      char *ibuf, Address &base,
		      const pdvector<AstNode *> &operands, 
		      const pdstring &callee, process *proc,
	              bool noCost, const function_base *calleefunc,
		      const pdvector<AstNode *> &ifForks,
		      const instPoint *location)
{
  assert(op == callOp);
  Address addr;
  bool err;
  pdvector <Register> srcs;

  if (calleefunc)
       addr = calleefunc->getEffectiveAddress(proc);
  else {
       addr = proc->findInternalAddress(callee, false, err);
       if (err) {
	    function_base *func = proc->findOnlyOneFunction(callee);
	    if (!func) {
	      cerr << __FILE__ << ":" <<__LINE__
		   <<": Internal error: unable to function " << callee << endl;

		 std::ostringstream os(std::ios::out);
		 os << __FILE__ << ":" <<__LINE__
		    <<": Internal error: unable to find addr of " << callee << endl;
		 logLine(os.str().c_str());
		 showErrorCallback(80, os.str().c_str());
		 P_abort();
	    }
	    addr = func->getEffectiveAddress(proc);
       }
  }
  for (unsigned u = 0; u < operands.size(); u++)
      srcs.push_back((Register)operands[u]->generateCode_phase2(proc, rs, ibuf,
								base, noCost, 
								ifForks, location));

  unsigned char *insn = (unsigned char *) ((void*)&ibuf[base]);
  unsigned char *first = insn;

  // push arguments in reverse order, last argument first
  // must use int instead of unsigned to avoid nasty underflow problem:
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
  if (srcs.size() > 0)
     emitOpRegImm(0, ESP, srcs.size()*4, insn); // add esp, srcs.size()*4

  // allocate a (virtual) register to store the return value
  Register ret = rs->allocateRegister((char *)insn, base, noCost);
  emitMovRegToRM(EBP, -(ret*4), EAX, insn);

  base += insn - first;
  return ret;
}



/*
 * emit code for op(src1,src2, dest)
 * ibuf is an instruction buffer where instructions are generated
 * base is the next free position on ibuf where code is to be generated
 */

Address emitA(opCode op, Register src1, Register /*src2*/, Register dest,
	     char *ibuf, Address &base, bool /*noCost*/)
{
    //fprintf(stderr,"emitA(op=%d,src1=%d,src2=XX,dest=%d)\n",op,src1,dest);

    unsigned char *insn = (unsigned char *) (&ibuf[base]);
    unsigned char *first = insn;

    switch (op) {
    case ifOp: {
      // if src1 == 0 jump to dest
      // src1 is a temporary
      // dest is a target address
      emitOpRegReg(0x29, EAX, EAX, insn);           // sub EAX, EAX ; clear EAX
      emitOpRegRM(0x3B, EAX, EBP, -(src1*4), insn); // cmp -(src1*4)[EBP], EAX
      // je dest
      *insn++ = 0x0F;
      *insn++ = 0x84;
      *((int *)insn) = dest;
      insn += sizeof(int);
      base += insn-first;
      return base;
      }
    case branchOp: { 
        emitJump(dest - JUMP_REL32_SZ, insn); 
        base += JUMP_REL32_SZ;
        return(base - JUMP_REL32_SZ);
      }
    case trampTrailer: {
      // generate the template for a jump -- actual jump is generated elsewhere
      emitJump(0, insn); // jump xxxx
      // return the offset of the previous jump
      base += insn - first;
      return(base - JUMP_REL32_SZ);
      }
    case trampPreamble: {
        base += insn - first;
        return(0);      // let's hope this is expected!
      }
    default:
        abort();        // unexpected op for this emit!
    }
  return(0);            // should never reach here!
}

Register emitR(opCode op, Register src1, Register /*src2*/, Register dest,
               char *ibuf, Address &base, bool /*noCost*/,
               const instPoint *location, bool /*for_multithreaded*/)
{
    //fprintf(stderr,"emitR(op=%d,src1=%d,src2=XX,dest=%d)\n",op,src1,dest);

    unsigned char *insn = (unsigned char *) (&ibuf[base]);
    unsigned char *first = insn;

    switch (op) {
      case getRetValOp: {
         // dest is a register where we can store the value
         // the return value is in the saved EAX
         emitMovRMToReg(EAX, EBP, SAVED_EAX_OFFSET, insn);
         emitMovRegToRM(EBP, -(dest*4), EAX, insn);
         base += insn - first;

         return dest;
      }
      case getParamOp: {
         // src1 is the number of the argument
         // dest is a register where we can store the value
         // Parameters are addressed by a positive offset from ebp,
         // the first is PARAM_OFFSET[ebp]
         instPointType ptType = location->getPointType();
         if(ptType == callSite) {
            emitMovRMToReg(EAX, EBP, CALLSITE_PARAM_OFFSET + src1*4, insn);
            emitMovRegToRM(EBP, -(dest*4), EAX, insn);
            base += insn - first;
            return dest;
         } else {
            // assert(ptType == functionEntry)
            emitMovRMToReg(EAX, EBP, FUNC_PARAM_OFFSET + src1*4, insn);
            emitMovRegToRM(EBP, -(dest*4), EAX, insn);
            base += insn - first;
            return dest;
         }
      }
      default:
         abort();                  // unexpected op for this emit!
    }
    return(Null_Register);        // should never be reached!
}

#ifdef BPATCH_LIBRARY
static inline void emitSHL(Register dest, unsigned char pos, unsigned char *&insn)
{
  //fprintf(stderr, "Emiting SHL\n");
  *insn++ = 0xC1;
  *insn++ = makeModRMbyte(3 /* rm gives register */, 4 /* opcode ext. */, dest);
  *insn++ = pos;
}

// VG(8/15/02): Emit the jcc over a conditional snippet
void emitJmpMC(int condition, int offset, char* baseInsn, Address &base)
{
  // What we want: 
  //   mov eax, [original EFLAGS]
  //   push eax
  //   popfd
  //   jCC target   ; CC = !condition (we jump on the negated condition)

  assert(condition >= 0 && condition <= 0x0F);

  unsigned char *insn = (unsigned char *) (&baseInsn[base]);
  unsigned char *first = insn;

  //fprintf(stderr, "OC: %x, NC: %x\n", condition, condition ^ 0x01);
  condition ^= 0x01; // flip last bit to negate the tttn condition

  emitMovRMToReg(EAX, EBP, SAVED_EFLAGS_OFFSET, insn); // mov eax, offset[ebp]
  emitSimpleInsn(0x50, insn);  // push eax
  emitSimpleInsn(POPFD, insn); // popfd
  emitJcc(condition, offset, insn);
  
  base += insn-first;
}

// VG(07/30/02): Restore mutatee value of GPR reg to dest (real) GPR
static inline void restoreGPRtoGPR(Register reg, Register dest, unsigned char *&insn)
{
  // NOTE: I don't use emitLoadPreviousStackFrameRegister because it saves
  // the value to a virtual (stack based) register, which is not what I want!
  emitMovRMToReg(dest, EBP, SAVED_EAX_OFFSET-(reg<<2), insn); //mov dest, offset[ebp]
}

// VG(07/30/02): Emit a lea dest, [base + index * scale + disp]; dest is a real GPR
static inline void emitLEA(Register base, Register index, unsigned int scale,
                           RegValue disp, Register dest, unsigned char *&insn)
{
  *insn++ = 0x8D;
  emitAddressingMode(base, index, scale, disp, (int)dest, insn);
}

// VG(11/07/01): Load in destination the effective address given
// by the address descriptor. Used for memory access stuff.
void emitASload(BPatch_addrSpec_NP as, Register dest, char* baseInsn,
		Address &base, bool /* noCost */)
{
  unsigned char *insn = (unsigned char *) (&baseInsn[base]);
  unsigned char *first = insn;

  // TODO 16-bit registers, rep hacks
  int imm = as.getImm();
  int ra  = as.getReg(0);
  int rb  = as.getReg(1);
  int sc  = as.getScale();

  bool havera = ra > -1, haverb = rb > -1;

  // VG(7/30/02): given that we use virtual (stack allocated) registers for
  // our inter-snippet temporaries, I assume all real registers to be fair game.
  // So, we restore the original registers in EAX and EDX - this allows us to
  // generate a lea (load effective address instruction) that will make the cpu
  // do the math for us.

  // assuming 32-bit addressing (for now)

  //fprintf(stderr, "ASLOAD: ra=%d rb=%d sc=%d imm=%d\n", ra, rb, sc, imm);

  if(havera)
    restoreGPRtoGPR(ra, EAX, insn);        // mov eax, [saved_ra]

  if(haverb)
    restoreGPRtoGPR(rb, EDX, insn);        // mov edx, [saved_rb]

  // Emit the lea to do the math for us:
  // e.g. lea eax, [eax + edx * sc + imm] if both ra and rb had to be restored
  emitLEA((havera ? EAX : Null_Register), (haverb ? EDX : Null_Register), sc, imm, EAX, insn);

  emitMovRegToRM(EBP, -(dest<<2), EAX, insn); // mov (virtual reg) dest, eax
  
  base += insn - first;
}

void emitCSload(BPatch_addrSpec_NP as, Register dest, char* baseInsn,
		Address &base, bool /* noCost */ )
{
  // VG(7/30/02): different from ASload on this platform, no LEA business
  unsigned char *insn = (unsigned char *) (&baseInsn[base]);
  unsigned char *first = insn;

  int imm = as.getImm();
  int ra  = as.getReg(0);
  int rb  = as.getReg(1);
  int sc  = as.getScale();

  // count is at most 1 register or constant or hack (aka pseudoregister)
  assert((ra == -1) && ((rb == -1) || ((imm == 0) && (rb == 1 /*ECX */ || rb >= IA32_EMULATE))));

  if(rb >= IA32_EMULATE) {
    // TODO: firewall code to ensure that direction is up
    bool neg = false;
    //fprintf(stderr, "!!!In case rb >= IA32_EMULATE!!!\n");
    switch(rb) {
    case IA32_NESCAS:
      neg = true;
    case IA32_ESCAS:
      // plan: restore flags, edi, eax, ecx; do rep(n)e scas(b/w); compute (saved_ecx - ecx) << sc;
      emitMovRMToReg(EAX, EBP, SAVED_EFLAGS_OFFSET, insn); // mov eax, offset[ebp]
      emitSimpleInsn(0x50, insn);  // push eax
      emitSimpleInsn(POPFD, insn); // popfd
      restoreGPRtoGPR(EAX, EAX, insn);
      restoreGPRtoGPR(ECX, ECX, insn);
      restoreGPRtoGPR(EDI, EDI, insn);
      emitSimpleInsn(neg ? 0xF2 : 0xF3, insn); // rep(n)e
      switch(sc) {
      case 0:
        emitSimpleInsn(0xAE, insn); // scasb
        break;
      case 1:
        emitSimpleInsn(0x66, insn); // operand size override for scasw;
      case 2:
        emitSimpleInsn(0xAF, insn); // scasw/d
        break;
      default:
        assert(!"Wrong scale!");
      }
      restoreGPRtoGPR(ECX, EAX, insn); // old ecx -> eax
      emitSubRegReg(EAX, ECX, insn); // eax = eax - ecx
      if(sc > 0)
        emitSHL(EAX, sc, insn);              // shl eax, scale
      emitMovRegToRM(EBP, -(dest<<2), EAX, insn); // mov (virtual reg) dest, eax
      break;
    case IA32_NECMPS:
      neg = true;
    case IA32_ECMPS:
      // plan: restore flags, esi, edi, ecx; do rep(n)e cmps(b/w); compute (saved_ecx - ecx) << sc;
      emitMovRMToReg(EAX, EBP, SAVED_EFLAGS_OFFSET, insn); // mov eax, offset[ebp]
      emitSimpleInsn(0x50, insn);  // push eax
      emitSimpleInsn(POPFD, insn); // popfd
      restoreGPRtoGPR(ECX, ECX, insn);
      restoreGPRtoGPR(ESI, ESI, insn);
      restoreGPRtoGPR(EDI, EDI, insn);
      emitSimpleInsn(neg ? 0xF2 : 0xF3, insn); // rep(n)e
      switch(sc) {
      case 0:
        emitSimpleInsn(0xA6, insn); // cmpsb
        break;
      case 1:
        emitSimpleInsn(0x66, insn); // operand size override for cmpsw;
      case 2:
        emitSimpleInsn(0xA7, insn); // cmpsw/d
        break;
      default:
        assert(!"Wrong scale!");
      }
      restoreGPRtoGPR(ECX, EAX, insn); // old ecx -> eax
      emitSubRegReg(EAX, ECX, insn); // eax = eax - ecx
      if(sc > 0)
        emitSHL(EAX, sc, insn);              // shl eax, scale
      emitMovRegToRM(EBP, -(dest<<2), EAX, insn); // mov (virtual reg) dest, eax
      break;
    default:
      assert(!"Wrong emulation!");
    }
  }
  else if(rb > -1) {
    //fprintf(stderr, "!!!In case rb > -1!!!\n");
    // TODO: 16-bit pseudoregisters
    assert(rb < 8); 
    restoreGPRtoGPR(rb, EAX, insn);        // mov eax, [saved_rb]
    if(sc > 0)
      emitSHL(EAX, sc, insn);              // shl eax, scale
    emitMovRegToRM(EBP, -(dest<<2), EAX, insn); // mov (virtual reg) dest, eax
  }
  else
    emitMovImmToRM(EBP, -(dest<<2), imm, insn);

  base += insn - first;
}
#endif


void emitVload(opCode op, Address src1, Register /*src2*/, Register dest, 
             char *ibuf, Address &base, bool /*noCost*/, int /* size */)
{
    unsigned char *insn = (unsigned char *) (&ibuf[base]);
    unsigned char *first = insn;

    if (op == loadConstOp) {
      // dest is a temporary
      // src1 is an immediate value 
      // dest = src1:imm32
      emitMovImmToRM(EBP, -(dest*4), src1, insn);
      base += insn - first;
      return;
    } else if (op ==  loadOp) {
      // dest is a temporary
      // src1 is the address of the operand
      // dest = [src1]
      emitMovMToReg(EAX, src1, insn);               // mov eax, src1
      emitMovRegToRM(EBP, -(dest*4), EAX, insn);    // mov -(dest*4)[ebp], eax
      base += insn - first;
      return;
    } else if (op == loadFrameRelativeOp) {
      // dest is a temporary
      // src1 is the offset of the from the frame of the variable
      // eax = [eax]	- saved sp
      // dest = [eax](src1)
      emitMovRMToReg(EAX, EBP, 0, insn);       // mov (%ebp), %eax 
      emitMovRMToReg(EAX, EAX, src1, insn);    // mov <offset>(%eax), %eax 
      emitMovRegToRM(EBP, -(dest*4), EAX, insn);    // mov -(dest*4)[ebp], eax
      base += insn - first;
      return;
    } else if (op == loadFrameAddr) {
      emitMovRMToReg(EAX, EBP, 0, insn);       // mov (%ebp), %eax 
      emitAddRegImm32(EAX, src1, insn);        // add #<offset>, %eax
      emitMovRegToRM(EBP, -(dest*4), EAX, insn);    // mov -(dest*4)[ebp], eax
      base += insn - first;
      return;
    } else {
        abort();                // unexpected op for this emit!
    }
}

void emitVstore(opCode op, Register src1, Register src2, Address dest,
             char *ibuf, Address &base, bool /*noCost*/, int /* size */)
{
    unsigned char *insn = (unsigned char *) (&ibuf[base]);
    unsigned char *first = insn;

    if (op ==  storeOp) {
      // [dest] = src1
      // dest has the address where src1 is to be stored
      // src1 is a temporary
      // src2 is a "scratch" register, we don't need it in this architecture
      emitMovRMToReg(EAX, EBP, -(src1*4), insn);    // mov eax, -(src1*4)[ebp]

      emitMovRegToM(dest, EAX, insn);               // mov dest, eax
      base += insn - first;
      return;
    } else if (op == storeFrameRelativeOp) {
      // src1 is a temporary
      // src2 is a "scratch" register, we don't need it in this architecture
      // dest is the frame offset 
      //
      // src2 = [ebp]	- saved sp
      // (dest)[src2] = src1
      emitMovRMToReg(src2, EBP, 0, insn);    	    // mov src2, (ebp)
      emitMovRMToReg(EAX, EBP, -(src1*4), insn);    // mov eax, -(src1*4)[ebp]
      emitMovRegToRM(src2, dest, EAX, insn);        // mov (dest)[src2], eax
      base += insn - first;
      return;
    } else {
        abort();                // unexpected op for this emit!
    }
}

void emitVupdate(opCode op, RegValue src1, Register /*src2*/, Address dest, 
             char *ibuf, Address &base, bool noCost)
{
    unsigned char *insn = (unsigned char *) (&ibuf[base]);
    unsigned char *first = insn;

    if (op == updateCostOp) {
      // src1 is the cost value
      // src2 is not used
      // dest is the address of observed cost

      if (!noCost) {
         // update observed cost
         // dest = address of DYNINSTobsCostLow
         // src1 = cost
         emitAddMemImm32(dest, src1, insn);  // ADD (dest), src1
      }
      base += insn-first;
      return;           //return base;    // never seem to ever need this
    } else {
        abort();                // unexpected op for this emit!
    }
}

void emitV(opCode op, Register src1, Register src2, Register dest, 
             char *ibuf, Address &base, bool /*noCost*/, int /* size */,
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

    unsigned char *insn = (unsigned char *) (&ibuf[base]);
    unsigned char *first = insn;

    if (op ==  loadIndirOp) {
      // same as loadOp, but the value to load is already in a register
      emitMovRMToReg(EAX, EBP, -(src1*4), insn); // mov eax, -(src1*4)[ebp]
      emitMovRMToReg(EAX, EAX, 0, insn);         // mov eax, [eax]
      emitMovRegToRM(EBP, -(dest*4), EAX, insn); // mov -(dest*4)[ebp], eax

    } 
    else if (op ==  storeIndirOp) {
      // same as storeOp, but the address where to store is already in a
      // register
      emitMovRMToReg(EAX, EBP, -(src1*4), insn);   // mov eax, -(src1*4)[ebp]
      emitMovRMToReg(ECX, EBP, -(dest*4), insn);   // mov ecx, -(dest*4)[ebp]
      emitMovRegToRM(ECX, 0, EAX, insn);           // mov [ecx], eax

    } else if (op == noOp) {
       emitSimpleInsn(NOP, insn); // nop

    } else if (op == saveRegOp) {
      // should not be used on this platform
      assert(0);

    } else {
      unsigned opcode = 0;//initialize to placate gcc warnings
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
		return;
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
		return;
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
    return;
}


void emitImm(opCode op, Register src1, RegValue src2imm, Register dest, 
             char *ibuf, Address &base, bool)
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
	        int result=-1;
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
		return;
	      }
		break;

	    case divOp: {
	        int result=-1;
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
		return;
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
		return;
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
    return;
}



int getInsnCost(opCode op)
{
    if (op == loadConstOp) {
	return(1);
    } else if (op ==  loadOp) {
	return(1+1);
    } else if (op ==  loadIndirOp) {
	return(3);
    } else if (op ==  storeOp) {
	return(1+1); 
    } else if (op ==  storeIndirOp) {
	return(3);
    } else if (op ==  ifOp) {
	return(1+2+1);
    } else if (op ==  ifMCOp) { // VG(8/15/02): No clue if this is right or not
	return(1+2+1);
    } else if (op ==  whileOp) {
	return(1+2+1+1); /* Need to find out about this */
    } else if (op == branchOp) {
	return(1);	/* XXX Need to find out what value this should be. */
    } else if (op ==  callOp) {
        // cost of call only
        return(1+2+1+1);
    } else if (op == funcJumpOp) {
        // copy callOp
        return(1+2+1+1);
    } else if (op == updateCostOp) {
        return(3);
    } else if (op ==  trampPreamble) {
        return(0);
    } else if (op ==  trampTrailer) {
        return(1);
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
	    case getAddrOp:
		return(0);	// doesn't add anything to operand
	    default:
		assert(0);
		return 0;
		break;
	}
    }
}



bool process::heapIsOk(const pdvector<sym_data> &find_us) {
  Symbol sym;
  pdstring str;
  Address baseAddr;
  pdvector<pd_Function *> *pdfv=NULL;
 


  // find the main function
  // first look for main or _main
#if !defined(i386_unknown_nt4_0)
  if (NULL == (pdfv = symbols->findFuncVectorByPretty("main")) || !pdfv->size()) {
    if (NULL == (pdfv = symbols->findFuncVectorByPretty("_main")) || !pdfv->size()) {
      cerr << __FILE__ << __LINE__ << ":  findFuncVectorByPretty(main) failed!" << endl;
     pdstring msg = "Cannot find main. Exiting.";
     statusLine(msg.c_str());
     showErrorCallback(50, msg);
     return false;
    }
  }

  if (pdfv->size() > 1)
    cerr << __FILE__ << __LINE__ << ":  Found more than one main!  using the first" << endl;

  mainFunction = (function_base *) (*pdfv)[0];
#else

  if (!((mainFunction = findOnlyOneFunction("main")) 
        || (mainFunction = findOnlyOneFunction("_main"))
	|| (mainFunction = findOnlyOneFunction("WinMain"))
	|| (mainFunction = findOnlyOneFunction("_WinMain"))
	|| (mainFunction = findOnlyOneFunction("wWinMain"))
	|| (mainFunction = findOnlyOneFunction("_wWinMain")))) {
     pdstring msg = "Cannot find main or WinMain. Exiting.";
     statusLine(msg.c_str());
     showErrorCallback(50, msg);
     return false;
  }
#endif

  for (unsigned i=0; i<find_us.size(); i++) {
    str = find_us[i].name;
    if (!getSymbolInfo(str, sym, baseAddr)) {
      pdstring str1 = pdstring("_") + str.c_str();
      if (!getSymbolInfo(str1, sym, baseAddr) && find_us[i].must_find) {
         pdstring msg;
         msg = pdstring("Cannot find ") + str + pdstring(". Exiting");
         statusLine(msg.c_str());
         showErrorCallback(50, msg);
         return false;
      }
    }
  }

//  pdstring ghb = GLOBAL_HEAP_BASE;
//  if (!getSymbolInfo(ghb, sym, baseAddr)) {
//    ghb = U_GLOBAL_HEAP_BASE;
//    if (!getSymbolInfo(ghb, symm baseAddr)) {
//      pdstring msg;
//      msg = pdstring("Cannot find ") + str + pdstring(". Exiting");
//      statusLine(msg.c_str());
//      showErrorCallback(50, msg);
//      return false;
//    }
//  }
//  Address instHeapEnd = sym.addr()+baseAddr;
//  addInternalSymbol(ghb, instHeapEnd);

#if !defined(i386_unknown_nt4_0)
  pdstring tt = "DYNINSTtrampTable";
  if (!getSymbolInfo(tt, sym, baseAddr)) {
      pdstring msg;
      msg = pdstring("Cannot find ") + tt + pdstring(". Cannot use this application");
      statusLine(msg.c_str());
      showErrorCallback(50, msg);
      return false;
  }
#endif

  return true;
}



dictionary_hash<pdstring, unsigned> funcFrequencyTable(pdstring::hash);

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
 * Get an estimate of the frequency for the passed instPoint.  
 *    This is not (always) the same as the function that contains the point.
 * 
 *  The function is selected as follows:
 *
 *  If the point is an entry or an exit return the function name.
 *  If the point is a call and the callee can be determined, return the called
 *     function.
 *  else return the funcation containing the point.
 *
 *  WARNING: This code contains arbitrary values for func frequency (both user 
 *     and system).  This should be refined over time.
 *
 * Using 1000 calls sec to be one SD from the mean for most FPSPEC apps.
 *	-- jkh 6/24/94
 *
 */
float getPointFrequency(instPoint *point)
{

    pd_Function *func = point->callee();

    if (!func)
      func = point->func();

    if (!funcFrequencyTable.defines(func->prettyName())) {
      // Changing this value from 250 to 100 because predictedCost was
      // too high - naim 07/18/96
      return(100.0);       
    } else {
      return ((float)funcFrequencyTable[func->prettyName()]);
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
        if (point->usesTrap(proc))
	  return 9000; // estimated number of cycles for traps
        else
	  return(83);
    }
}



bool returnInstance::checkReturnInstance(const pdvector<pdvector<Frame> >&stackWalks)
{
    for (unsigned walk_iter = 0; walk_iter < stackWalks.size(); walk_iter++)
        for (u_int i=0; i < stackWalks[walk_iter].size(); i++) {
            // 27FEB03: we no longer return true if we are at the 
            // exact same address as the return instance. In this case
            // writing a jump is safe. -- bernat
            if ((stackWalks[walk_iter][i].getPC() > addr_) && 
                (stackWalks[walk_iter][i].getPC() < addr_+size_)) 
            {
                fprintf(stderr, "PC at 0x%lx (thread %d, frame %d) conflicts with inst point 0x%lx\n",
                        stackWalks[walk_iter][i].getPC(), walk_iter, i, addr_);
                return false;
            }
        }  
    return true;
}
 
void returnInstance::installReturnInstance(process *proc) {
    assert(instructionSeq);
    proc->writeTextSpace((void *)addr_, instSeqSize, instructionSeq->ptr());
    delete instructionSeq;
    instructionSeq = 0;
	installed = true;
}

void returnInstance::addToReturnWaitingList(Address , process *) {
    //P_abort();
	assert(0);
}

void generateBreakPoint(instruction &insn) {
  insn = instruction ((const unsigned char*)"\017\013", ILLEGAL, 2);
}

void instWaitingList::cleanUp(process *, Address ) {
  P_abort();
/*
  proc->writeTextSpace((caddr_t)pc, relocatedInstruction.size(),
            (caddr_t&)(relocatedInstruction.ptr()));
  proc->writeTextSpace((caddr_t)addr_, instSeqSize,
            (caddr_t)instructionSeq);
*/
}

/* ***************************************************** */

bool rpcMgr::emitInferiorRPCheader(void *void_insnPtr, Address &baseBytes) 
{
   unsigned char *insnPtr = (unsigned char *)void_insnPtr;
   unsigned char *origInsnPtr = insnPtr;
   insnPtr += baseBytes;

   // We emit the following here (to set up a fresh stack frame):
   // pushl %ebp        (0x55)
   // movl  %esp, %ebp  (0x89 0xe5)
   // pushad
   // pushfd

   emitSimpleInsn(PUSHFD, insnPtr);
   emitSimpleInsn(PUSHAD, insnPtr);
   emitSimpleInsn(PUSH_EBP, insnPtr);
   emitMovRegToReg(EBP, ESP, insnPtr);

   // allocate space for temporaries (virtual registers)
   emitOpRegImm(5, ESP, 128, insnPtr); // sub esp, 128

   baseBytes += (insnPtr - origInsnPtr);

   return true;
}

bool rpcMgr::emitInferiorRPCtrailer(void *void_insnPtr, Address &baseBytes,
                                    unsigned &breakOffset,
                                    bool shouldStopForResult,
                                    unsigned &stopForResultOffset,
                                    unsigned &justAfter_stopForResultOffset) {
   unsigned char *insnPtr = (unsigned char *)void_insnPtr;

   // unsigned char * is the most natural to work with on x86, since 
   // instructions are always an integral # of bytes.  Besides, it makes 
   // the following line easy:
   insnPtr += baseBytes; // start off in the right spot

   if (shouldStopForResult) {
      // illegal insn: 0x0f0b does the trick.
      stopForResultOffset = baseBytes;
      *insnPtr++ = 0x0f;
      *insnPtr++ = 0x0b;
      baseBytes += 2;

      justAfter_stopForResultOffset = baseBytes;
   }

   // Sequence: popfd, popad, leave (0xc9), call DYNINSTbreakPoint(), illegal

   emitSimpleInsn(LEAVE, insnPtr); // leave
   emitSimpleInsn(POPAD, insnPtr); // popad
   emitSimpleInsn(POPFD, insnPtr); // popfd
   baseBytes += 3; // all simple insns are 1 byte

   // We can't do a SIGTRAP since SIGTRAP is reserved in x86.
   // So we do a SIGILL instead.
   breakOffset = baseBytes;
   *insnPtr++ = 0x0f;
   *insnPtr++ = 0x0b;
   baseBytes += 2;

   // Here, we should generate an illegal insn, or something.
   // A two-byte insn, 0x0f0b, should do the trick.  The idea is that
   // the code should never be executed.
   *insnPtr++ = 0x0f;
   *insnPtr++ = 0x0b;
   baseBytes += 2;

   return true;
}

// process::replaceFunctionCall
//
// Replace the function call at the given instrumentation point with a call to
// a different function, or with a NOOP.  In order to replace the call with a
// NOOP, pass NULL as the parameter "func."
// Returns true if sucessful, false if not.  Fails if the site is not a call
// site, or if the site has already been instrumented using a base tramp.
//
// Note that right now we can only replace a call instruction that is five
// bytes long (like a call to a 32-bit relative address).
bool process::replaceFunctionCall(const instPoint *point,
                                  const function_base *func) {
    // Must be a call site
    if (!point->insnAtPoint().isCall())
	return false;

    // Cannot already be instrumented with a base tramp
    if (baseMap.defines(point))
	return false;

    // Replace the call
    if (func == NULL) {	// Replace with NOOPs
	unsigned char *newInsn = new unsigned char[point->insnAtPoint().size()];
	unsigned char *p = newInsn;
	for (unsigned i = 0; i < point->insnAtPoint().size(); i++)
	    emitSimpleInsn(NOP, p);
	writeTextSpace((void *)point->iPgetAddress(),
		       point->insnAtPoint().size(), newInsn);
    } else { // Replace with a call to a different function
	// XXX Right only, call has to be 5 bytes -- sometime, we should make
	// it work for other calls as well.
	assert(point->insnAtPoint().size() == CALL_REL32_SZ);
	unsigned char *newInsn = new unsigned char[CALL_REL32_SZ];
	unsigned char *p = newInsn;
	emitCallRel32(func->addr() - (point->iPgetAddress()+CALL_REL32_SZ), p);
	writeTextSpace((void *)point->iPgetAddress(), CALL_REL32_SZ, newInsn);
    }

    return true;
}

// Emit code to jump to function CALLEE without linking.  (I.e., when
// CALLEE returns, it returns to the current caller.)
void emitFuncJump(opCode op, 
		  char *i, Address &base, 
		  const function_base *callee, process *proc,
		  const instPoint *loc, bool noCost)
{
       assert(op == funcJumpOp);

       Address addr = callee->getEffectiveAddress(proc);
       unsigned char *insn = (unsigned char *) &i[base];
       unsigned char *first = insn;

       if (loc->isConservative())
          emitOpRegRM(FRSTOR, FRSTOR_OP, EBP, -128 - FSAVE_STATE_SIZE, insn);
       emitSimpleInsn(LEAVE, insn);     // leave
       emitSimpleInsn(POP_EAX, insn);
       emitSimpleInsn(POPAD, insn);     // popad
       if (!loc->isConservative() || noCost)
          emitSimpleInsn(POPFD, insn);     // popfd

       *insn++ = 0x68; /* push 32 bit immediate */
       *((int *)insn) = addr; /* the immediate */
       insn += 4;
       *insn++ = 0xc3; /* ret */

       base += (insn-first);
}

void emitLoadPreviousStackFrameRegister(Address register_num,
                                        Register dest,
                                        char *insn,
                                        Address &base,
                                        int,
                                        bool){
  //Previous stack frame register is stored on the stack,
  //it was stored there at the begining of the base tramp.

  //Calculate the register's offset from the frame pointer in EBP
  unsigned offset = SAVED_EAX_OFFSET - (register_num * 4);
  
  unsigned char *in = (unsigned char *) (&insn[base]);
  unsigned char *first = in;
  
  emitMovRMToReg(EAX, EBP, offset, in); //mov eax, offset[ebp]
  emitMovRegToRM(EBP, -(dest*4), EAX, in); //mov dest, 0[eax]
  base += in - first;
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
  Register base_reg, index_reg;
  int displacement;
  unsigned scale;
  int addr_mode;
  unsigned Mod;

  AstNode *func;
  instruction i = callSite->insnAtPoint();
  pdvector<AstNode *> the_args(2);
  if(i.isCallIndir()){
    addr_mode = get_instruction_operand(i.ptr(), base_reg, index_reg,
					 displacement, scale, Mod);
    switch(addr_mode){
      
    case REGISTER_DIRECT:
      {
      the_args[0] = new AstNode(AstNode::PreviousStackFrameDataReg,
				(void *) base_reg);
      the_args[1] = new AstNode(AstNode::Constant,
				(void *) callSite->iPgetAddress());
      func = new AstNode("DYNINSTRegisterCallee", the_args);
      miniTrampHandle *mtHandle;
      addInstFunc(this, mtHandle, callSite, func, callPreInsn,
		  orderFirstAtPoint, true,false);
      break;
      }
    case REGISTER_INDIRECT:
      {
	AstNode *prevReg = new AstNode(AstNode::PreviousStackFrameDataReg,
				       (void *) base_reg);
	the_args[0] = new AstNode(AstNode::DataIndir, prevReg);
	the_args[1] = new AstNode(AstNode::Constant,
				  (void *) callSite->iPgetAddress());
	func = new AstNode("DYNINSTRegisterCallee", the_args);
	miniTrampHandle *mtHandle;
	addInstFunc(this, mtHandle, callSite, func, callPreInsn,
		    orderFirstAtPoint, true,false);
	break;
      }
    case REGISTER_INDIRECT_DISPLACED:
      {
	AstNode *prevReg = new AstNode(AstNode::PreviousStackFrameDataReg,
				       (void *) base_reg);
 	AstNode *offset = new AstNode(AstNode::Constant, 
	 			      (void *) displacement);
 	AstNode *sum = new AstNode(plusOp, prevReg, offset);
	
 	the_args[0] = new AstNode(AstNode::DataIndir, sum);
 	the_args[1] = new AstNode(AstNode::Constant,
 				  (void *) callSite->iPgetAddress());
  	func = new AstNode("DYNINSTRegisterCallee", the_args);
	miniTrampHandle *mtHandle;
 	addInstFunc(this, mtHandle, callSite, func, callPreInsn,
		    orderFirstAtPoint, true,false);
	break;
      }
    case DISPLACED: 
      {
	AstNode *offset = new AstNode(AstNode::Constant, 
				      (void *) displacement);
	the_args[0] = new AstNode(AstNode::DataIndir, offset);
	the_args[1] = new AstNode(AstNode::Constant,
				  (void *) callSite->iPgetAddress());
	func = new AstNode("DYNINSTRegisterCallee", the_args);
	miniTrampHandle *mtHandle;
	addInstFunc(this, mtHandle, callSite, func, callPreInsn,
		    orderFirstAtPoint, true, false);
	break;
      }
    case SIB:
      {
	AstNode *effective_address;
	if(index_reg != 4) { //We use a scaled index
	  bool useBaseReg = true;
	  if(Mod == 0 && base_reg == 5){
	    cerr << "Inserting untested call site monitoring "
            << "instrumentation at address " << std::hex
            << callSite->iPgetAddress() << std::dec << endl;
	    useBaseReg = false;
	  }
	  
	  AstNode *index = new AstNode(AstNode::PreviousStackFrameDataReg,
				       (void *) index_reg);
	  AstNode *base = new AstNode(AstNode::PreviousStackFrameDataReg,
				      (void *) base_reg);
	  
	  AstNode *disp = new AstNode(AstNode::Constant, 
				      (void *) displacement);
	  
	  if(scale == 1){ //No need to do the multiplication
	    if(useBaseReg){
	      AstNode *base_index_sum = new AstNode(plusOp, index, base);
	      effective_address = new AstNode(plusOp, base_index_sum,
					      disp);
	    }
	    else 
	      effective_address = new AstNode(plusOp, index, disp); 
	    
	    the_args[0] = new AstNode(AstNode::DataIndir, effective_address);
	    
	    the_args[1] = new AstNode(AstNode::Constant,
				      (void *) callSite->iPgetAddress());
	    func = new AstNode("DYNINSTRegisterCallee", the_args);
	    miniTrampHandle *mtHandle;
	    addInstFunc(this, mtHandle, callSite, func, callPreInsn,
			orderFirstAtPoint, true, false);
	  }
	  else {
	    AstNode *scale_factor
	      = new AstNode(AstNode::Constant, (void *) scale);
	    AstNode *index_scale_product = new AstNode(timesOp, index, 
						       scale_factor);
	    if(useBaseReg){
	      AstNode *base_index_sum = new AstNode(plusOp, 
						    index_scale_product,
						    base);
	      effective_address = new AstNode(plusOp, base_index_sum,
					      disp);
	    }
	    else 
	      effective_address = new AstNode(plusOp, 
					       index_scale_product,
					       disp);
	    the_args[0] = new AstNode(AstNode::DataIndir, effective_address);
	    
	    the_args[1] = new AstNode(AstNode::Constant,
				      (void *) callSite->iPgetAddress());
	    func = new AstNode("DYNINSTRegisterCallee", the_args);
	    miniTrampHandle *mtHandle;	    
	    addInstFunc(this, mtHandle, callSite, func, callPreInsn,
			orderFirstAtPoint, true,false);
	  }
	}
	else { //We do not use a scaled index. 
	  cerr << "Inserting untested call site monitoring "
          << "instrumentation at address " << std::hex
          << callSite->iPgetAddress() << std::dec << endl;
	  AstNode *base = new AstNode(AstNode::PreviousStackFrameDataReg,
				      (void *) base_reg);
	  AstNode *disp = new AstNode(AstNode::Constant, 
				      (void *) displacement);
	  AstNode *effective_address =  new AstNode(plusOp, base,
						    disp);
	  the_args[0] = new AstNode(AstNode::DataIndir, effective_address);
	  
	  the_args[1] = new AstNode(AstNode::Constant,
				    (void *) callSite->iPgetAddress());
	  func = new AstNode("DYNINSTRegisterCallee", the_args);
	  miniTrampHandle *mtHandle;
	  addInstFunc(this, mtHandle, callSite, func, callPreInsn,
		      orderFirstAtPoint, true, false);
	}
      }
      break;
      
    default:
      cerr << "Unexpected addressing type in MonitorCallSite at addr:" 
           << std::hex << callSite->iPgetAddress() << std::dec 
           << "The daemon declines the monitoring request of this call site." 
           << endl; 
      break;
    }
  }
  else if(i.isCall()){
    //Regular callees are statically determinable, so no need to 
    //instrument them
    return true;
  }
  else {
    cerr << "Unexpected instruction in MonitorCallSite()!!!\n";
  }
  return true;
}
#endif

#if (defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0))
#include <sys/signal.h>
//#include <sys/ucontext.h>

void
BaseTrampTrapHandler (int)//, siginfo_t*, ucontext_t*)
{
  cout << "In BaseTrampTrapHandler()" << endl;
  // unset trap handler, so that DYNINSTtrapHandler can take place
  if (sigaction(SIGTRAP, NULL, NULL) != 0) {
    perror("sigaction(SIGTRAP)");
    assert(0);
    abort();
  }
}
#endif

bool deleteBaseTramp(process *, trampTemplate *)
{
	cerr << "WARNING : deleteBaseTramp is unimplemented "
	     << "(after the last instrumentation deleted)" << endl;
	return false;
}


/*
 * createInstructionInstPoint
 *
 * Create a BPatch_point instrumentation point at the given address, which
 * is guaranteed not be one of the "standard" inst points.
 *
 * proc         The process in which to create the inst point.
 * address      The address for which to create the point.
 */
BPatch_point *createInstructionInstPoint(process* proc, void *address,
                                         BPatch_point** alternative,
					 BPatch_function* bpf)
{
    /*
     * Get some objects we need, such as the enclosing function, image, etc.
     */
    pd_Function *func = NULL;
    if(bpf)
        func = (pd_Function*)bpf->func;
    else
        func = (pd_Function*)proc->findFuncByAddr((Address)address);

    if (func == NULL) // Should make an error callback here?
	return NULL;

    const image *image = func->file()->exec();

    Address imageBase;
    proc->getBaseAddress(image, imageBase);
    Address relAddr = (Address)address - imageBase;

    BPatch_function *bpfunc = proc->findOrCreateBPFunc(func);
	
    /*
     * Check if the address points to the beginning of an instruction.
     */
    bool isInstructionStart = false;

    const unsigned char *ptr =
	(const unsigned char *)image->getPtrToInstruction(func->getAddress(0));

    instruction insn;
    unsigned insnSize;

    Address funcEnd = func->getAddress(proc) + func->size();
    for (Address checkAddr = func->getAddress(proc);
	checkAddr < funcEnd; ptr += insnSize, checkAddr += insnSize) {

	if (checkAddr == (Address)address) {
	    isInstructionStart = true;
	    break;
	} else if (checkAddr > (Address)address) {
	    break;
	}

	insnSize = insn.getNextInstruction(ptr);
	assert(insnSize > 0);
    }

    if (!isInstructionStart)
	return NULL; // Should make an error callback as well?
    
    /*
     * Check if the point overlaps an existing instrumentation point.
     */
    Address begin_addr, end_addr, curr_addr;
    unsigned int i;

    curr_addr = (Address)address;

    instPoint *entry = const_cast<instPoint *>(func->funcEntry(NULL));
    assert(entry);

    begin_addr = entry->iPgetAddress();
    end_addr = begin_addr + entry->size();

    if (curr_addr >= begin_addr && curr_addr < end_addr) { 
	BPatch_reportError(BPatchSerious, 117,
			   "instrumentation point conflict (1)");
	if(alternative)
	    *alternative = proc->findOrCreateBPPoint(bpfunc, entry, BPatch_entry);
	return NULL;
    }

    const pdvector<instPoint*> &exits = func->funcExits(NULL);
    for (i = 0; i < exits.size(); i++) {
	assert(exits[i]);

	begin_addr = exits[i]->iPgetAddress();
	end_addr = begin_addr + exits[i]->size();

	if (curr_addr >= begin_addr && curr_addr < end_addr) {
	    BPatch_reportError(BPatchSerious, 117,
			       "instrumentation point conflict (2)");
	    if(alternative)
		    *alternative = proc->findOrCreateBPPoint(bpfunc,exits[i],BPatch_exit);
	    return NULL;
	}
    }

    const pdvector<instPoint*> &calls = func->funcCalls(NULL);
    for (i = 0; i < calls.size(); i++) {
	assert(calls[i]);

	begin_addr = calls[i]->iPgetAddress();
	end_addr = begin_addr + calls[i]->size();

	if (curr_addr >= begin_addr && curr_addr < end_addr) {
	    BPatch_reportError(BPatchSerious, 117,
			       "instrumentation point conflict (3)");
	    if(alternative)
		    *alternative = proc->findOrCreateBPPoint(bpfunc,calls[i],BPatch_subroutine);
	    return NULL;
	}
    }

    /*
     * Create the new inst point.
     */

    ptr = (const unsigned char *)image->getPtrToInstruction(relAddr);
    insnSize = insn.getNextInstruction(ptr);

    instPoint *newpt = new instPoint(func, image, relAddr, otherPoint, 
                                     insn, true);

    newpt->checkInstructions();

    func->addArbitraryPoint(newpt, proc);

    return proc->findOrCreateBPPoint(bpfunc, newpt, BPatch_arbitrary);
}

#ifdef BPATCH_LIBRARY
/*
 * BPatch_point::getDisplacedInstructions
 *
 * Returns the instructions to be relocated when instrumentation is inserted
 * at this point.  Returns the number of bytes taken up by these instructions.
 *
 * maxSize      The maximum number of bytes of instructions to return.
 * insns        A pointer to a buffer in which to return the instructions.
 */

int BPatch_point::getDisplacedInstructions(int maxSize, void* insns)
{
    void *code;
    char copyOut[1024];
    unsigned int count = 0;

    if (point->insnsBefore()) {
	for (unsigned int i=0; i < point->insnsBefore(); i++) {
	    code = const_cast<unsigned char *>(point->insnBeforePt(i).ptr());
	    memcpy(&copyOut[count], code, point->insnBeforePt(i).size());
	    count += point->insnBeforePt(i).size();
	    assert(count < sizeof(copyOut));
	}
    }

    code = const_cast<unsigned char *>(point->insnAtPoint().ptr());
    memcpy(&copyOut[count], code, point->insnAtPoint().size());
    count += point->insnAtPoint().size();
    assert(count < sizeof(copyOut));

    if (point->insnsAfter()) {
	for (unsigned int i=0; i < point->insnsAfter(); i++) {
	    code = const_cast<unsigned char *>(point->insnAfterPt(i).ptr());
	    memcpy(&copyOut[count], code, point->insnAfterPt(i).size());
	    count += point->insnAfterPt(i).size();
	    assert(count < sizeof(copyOut));
	}
    }

    if (count <= (unsigned) maxSize) {
	memcpy(insns, copyOut, count);
	return(count);
    } else {
	return -1;
    }
}

#endif

/****************************************************************************/
/****************************************************************************/

/* pd_Function Code for function relocation */

// Check if an instruction is a relative addressed jump instruction

bool pd_Function::isNearBranchInsn(const instruction insn) {
  if (insn.isJumpDir())
    return true;
  return false;
}

// Check if an instruction is a relative addressed call instruction 

bool pd_Function::isTrueCallInsn(const instruction insn) {
  if (insn.isCall() && !insn.isCallIndir())
    return true;
  return false;
}

/****************************************************************************/
/****************************************************************************/

/* x86 */

// Create a buffer of x86 instructon objects. These x86 instructions will
// contain pointers to the machine code 

bool pd_Function::loadCode(const image* /* owner */, process *proc, 
                           instruction *&oldCode, 
                           unsigned &numberOfInstructions, 
                           Address &firstAddress) {

  pdvector<instruction> insnVec;
  unsigned type, insnSize, totalSize = 0; 
  instruction *insn = 0;

#ifdef DEBUG_FUNC_RELOC 
    cerr << "pd_Function::loadCode" << endl;
#endif

  originalCode = new unsigned char[size()];

  // copy function to be relocated from application into instructions
  proc->readDataSpace((caddr_t)firstAddress, size(), originalCode, true);

  // first address of function
  unsigned char *p = originalCode;

  // last address of function
  unsigned end_of_function = (unsigned)(p + size());

  // iterate over all instructions in function
  while ( (unsigned) p < end_of_function ) {

    // new instruction object 
    insnSize = get_instruction(p, type);
    insn = new instruction(p, type, insnSize);   
    insnVec.push_back(*insn);

    /*
    // check for the following instruction sequence:
    //
    //  call (0)       (PC relative address, where the target of call (0)
    //                  is the next instruction
    //  pop  %ebx      (pops return address of call instruction off of the
    //                  stack and places it in the ebx reg. The value in
    //                  ebx becomes the address of the pop instruction
    //
    // This sequence is used to get the address of the currently 
    // executing instruction. Presently we don't relocate a function 
    // with this sequence of instructions 

    // A call instruction whose target is the next instruction, is generally
    // used to obtain the address of the next instruction. 
    // Presently we don't relocate a functions with such calls
    if ( isTrueCallInsn((const instruction)(*insn)) && 
         get_disp(insn) == 0 && *(p + insnSize) == 0x5b ) {

         relocatable_ = false;

         delete insn;
         return false;
    }
    */

    // update p so it points to the next machine code instruction
    p = p + insnSize; 
    totalSize += insnSize;

    delete insn;
  }

  // Occasionally a function's size is not calculated correctly for 
  // pd_Function. In such cases, the last few bytes in the function
  // are "garbage", and the parsing done in the above while loop
  // interprets those bytes as an instruction. If the last byte of that 
  // "garbage" instruction is outside the bounds of the function, 
  // the sum of the insnSizes of the insn's that were parsed above will be 
  // greater than the size of the function being relocating. To keep the
  // sum of the insnSizes equal to the size of the pd_Function, we replace 
  // the "garbage" bytes with nop instructions, and ignore drop those bytes
  // that are outside of the function.   

  // # bytes of "garbage" at the end of the function 
  int garbage = (unsigned)p - end_of_function;

  // if "garbage" bytes are found
  if (garbage) { 
  
    // create a nop machine instruction
    unsigned char *nop = new unsigned char;
    emitSimpleInsn(0x90, nop);
    nop--;

    // create an x86 instruction for a nop
    insn = new instruction(nop, 0, 1);

    // replace "garbage" x86 instruction with a nop instruction
    insnVec[insnVec.size() - 1] = *insn;

    // replace all "garbage" bytes up to the end of the function with nops
    for (int i = 0; i < garbage; i++) {   
      insnVec.push_back(*insn);
    }  
  }

  // buffer of x86 instructions
  unsigned bufSize = insnVec.size();
  oldCode = new instruction[bufSize];

  // if unable to allocate array, dump warn and return false.... 
  if (oldCode == NULL) {
    cerr << "WARN : unable to allocate array (" << insnVec.size() << " bytes) to read in " \
         << "instructions for function" << prettyName().c_str() << " unable to instrument" \
         << endl;
    return false;
  }

  // copy vector of instructions into buffer of instructions
  for (unsigned i = 0; i < insnVec.size(); i++) {
    oldCode[i] = insnVec[i];
  }

  numberOfInstructions = insnVec.size();
  setNumInstructions(numberOfInstructions);  
  
  return true;
} 

/****************************************************************************/
/****************************************************************************/

// Copy machine code from one location (in mutator) to another location
// (also in the mutator)
// Also updates the corresponding buffer of x86 instructions 

void pd_Function::copyInstruction(instruction &newInsn, instruction &oldInsn, 
                                                      unsigned &codeOffset) {
 
  unsigned insnSize = oldInsn.size(); 
  const unsigned char *oldPtr = oldInsn.ptr();
  unsigned tmp = codeOffset;

  // iterate over each byte of the machine instruction, copying it
  for (unsigned i = 0; i < insnSize; i++) {     
    relocatedCode[codeOffset] = *(oldPtr + i);
    codeOffset++;
  }

  // update x86 instruction corresponding to machine code instruction
  newInsn = *(new instruction(&relocatedCode[tmp], oldInsn.type(), insnSize));
}   

/****************************************************************************/
/****************************************************************************/

// update displacement of expanded instruction

int pd_Function::expandInstructions(LocalAlterationSet &alteration_set, 
                                    instruction &insn, 
                                    Address offset,
                                    instruction &newCodeInsn) {

 
  int oldDisp = 0, newDisp = 0, extra_offset = 0;
  unsigned char *oldInsn = 0, *newInsn = 0;
 
  unsigned insnType = insn.type(); 

  // location (in mutator) instruction was originally located at
  oldInsn = const_cast<unsigned char *> (insn.ptr());

  // location (in mutator) instruction is being relocated to (temporarily)
  newInsn = const_cast<unsigned char *> (newCodeInsn.ptr());

  // old displacement from instruction to target
  oldDisp = get_disp(&insn);

  // change in displacement of target  
  extra_offset = alteration_set.getShift(offset + oldDisp) - 
                 alteration_set.getShift(offset);

  if (insnType & REL_B) {
    /* replace with rel32 instruction, opcode is one byte. */
    if (*oldInsn == JCXZ) {
      *newInsn++ = *oldInsn; *newInsn++ = 2; // jcxz 2
      *newInsn++ = 0xEB; *newInsn++ = 5;        // jmp 5
      *newInsn++ = 0xE9;                        // jmp rel32
      *((int *)newInsn) = oldDisp + extra_offset - 7; // change in insn size is 7
      newInsn += sizeof(int);
      return true;
    } else {
        unsigned newSz=UINT_MAX;
        if (insnType & IS_JCC) {
	  /* Change a Jcc rel8 to Jcc rel32. 
	     Must generate a new opcode: a 0x0F followed by (old opcode + 16) */
	  unsigned char opcode = *oldInsn++;
	  *newInsn++ = 0x0F;
	  *newInsn++ = opcode + 0x10;
          newDisp = oldDisp + extra_offset - 4;   // change in insn size is 4
	  newSz = 6;
        } else {
            if (insnType & IS_JUMP) {
	      /* change opcode to 0xE9 */
	      oldInsn++;
	      *newInsn++ = 0xE9;
              newDisp = oldDisp + extra_offset - 3;  // change in insn size is 3
	      newSz = 5;
	    }
            assert(newSz!=UINT_MAX);
            *((int *)newInsn) = newDisp;
            newInsn += sizeof(int); 
            return true;
	}
    }
  } else {
      if (insnType & REL_W) {
        assert(insnType & PREFIX_OPR);
        if (insnType & PREFIX_SEG)
          *newInsn++ = *oldInsn++;
	
           /* opcode is unchanged, just relocate the displacement */
   
          if (*oldInsn == (unsigned char)0x0F)
            *newInsn++ = *oldInsn++;
          *newInsn++ = *oldInsn++;
          newDisp = oldDisp + extra_offset - 1;  // change in insn size is 1
          *((int *)newInsn) = newDisp;
          newInsn += sizeof(int);
          return true;
      } else {
	  // should never get here
          assert (insnType & REL_D);
          assert (0); 
      }
  }
   return false;   
}


/****************************************************************************/
/****************************************************************************/

// given the Address adr, calculate the offset in the buffer code[], 
// of the x86 instruction that begins at adr. Return -1 if adr is 
// a byte in the middle of an instruction and not the first byte of 
// the instruction

int pd_Function::getArrayOffset(Address adr, instruction code[]) {
  
  unsigned i;
  Address insnAdr = addressOfMachineInsn(&code[0]);  

  assert(adr >= insnAdr && adr <= insnAdr + size()); 

  // find the instruction that contains the byte at Address adr
  for (i = 0; insnAdr < adr; i++) {
    insnAdr += ((instruction)code[i]).size();
  }

  // if adr is the first byte of the instruction, return the offset in
  // the buffer of the instruction
  if (insnAdr == adr) return i;
  else return -1;
}

/****************************************************************************/
/****************************************************************************/

// update the before and after insns of x86 instPoint p

void pd_Function::instrAroundPt(instPoint *p, instruction allInstr[], 
                                int numBefore, int numAfter, 
                                unsigned type, int index) {   

  Address newJumpAddr = p->address();

  // add instructions before the point
  unsigned size = (p->insnAtPoint()).size();
  for (int u1 = index-1; size < JUMP_SZ && u1 >= 0 && 
                           u1 > (index - 1) - numBefore; u1--) {
    if (!allInstr[u1].isCall()) {
        p->addInstrBeforePt(allInstr[u1]);
        size += allInstr[u1].size();
	newJumpAddr -= allInstr[u1].size();
    } else {
        break;
    }
  }

  p->setJumpAddr(newJumpAddr);

  // add instructions after the point
  if (type == ReturnPt) {

    for (int u1 = index+1; size < JUMP_SZ && u1 < (index + 1) + numAfter; u1++) {

      if (allInstr[u1].isNop() || *(allInstr[u1].ptr()) == 0xCC) {
          p->addInstrAfterPt(allInstr[u1]);
          size += allInstr[u1].size();
      } else {
          break;
      }
    }

  } else {

      unsigned maxSize = JUMP_SZ;
      if (type == EntryPt) maxSize = 2*JUMP_SZ;
      for (int u1 = index+1; size < maxSize && u1 < index+1+numAfter; u1++) {
        if (!allInstr[u1].isCall()) {
            p->addInstrAfterPt(allInstr[u1]);
            size += allInstr[u1].size();
        } else { 
            break;
	}
      }
  }
}

/****************************************************************************/
/****************************************************************************/
// originalOffset:      offset (in bytes) of the machine insn from the 
//                      beginning of the original function
//
// newOffset:           offset (in bytes) of the machine insn from the 
//                      beginning of the relocated and expanded function 
//
// originalArrayOffset: offset (in # of instructions) of the x86 instruction 
//                      corresponding to the instPoint, from the beginning of 
//                      the buffer corresponding to the original function.
//
// newArrayOffset:      offset (in # of instructions) of the x86 instruction 
//                      corresponding to the instPoint, from the beginning of 
//                      the buffer corresponding to the expanded and relocate
//                      function.
//
// adr:                 absolute Address of the instruction in the expanded 
//                      and relocated function.


#define CALC_OFFSETS(ip)						      \
     originalOffset = ((ip->iPgetAddress() + imageBaseAddr) - mutatee);	      \
     originalArrayOffset = getArrayOffset(originalOffset + mutator, oldCode); \
     if (originalArrayOffset < 0) return false;				      \
     newOffset = originalOffset + alteration_set.getShift(originalOffset);    \
     newArrayOffset = originalArrayOffset +				      \
                      alteration_set.getInstPointShift(originalOffset);	      \
     adr = newAdr + newOffset;
 
/****************************************************************************/
/****************************************************************************/

// update info about instrumentation points

bool pd_Function::fillInRelocInstPoints(
                            const image *owner, process *proc,
                            instPoint *&location, 
                            relocatedFuncInfo *&reloc_info, Address mutatee, 
                            Address mutator,instruction oldCode[], 
                            Address newAdr,instruction newCode[],
                            LocalAlterationSet &alteration_set) {
   
  unsigned retId = 0, callId = 0,arbitraryId = 0;
  int originalOffset, newOffset, originalArrayOffset, newArrayOffset;
  Address adr;

  if (!call_points_have_been_checked)
    checkCallPoints();

  instPoint *point = 0; 

  assert(newAdr);

#ifdef DEBUG_FUNC_RELOC    
  cerr << "fillInRelocInstPoints called for " << prettyName() << endl;
  cerr << std::hex << " mutator = 0x" << mutator << " mutatee = 0x" << mutatee 
       << " newAdr = 0x" << std::hex << newAdr << endl;
#endif

  Address imageBaseAddr;
  if (!proc->getBaseAddress(owner, imageBaseAddr))
        abort();

  alteration_set.Collapse();

  //  Add inst point corresponding to func entry....
  //   Assumes function has single entry point  
  if (funcEntry_ != NULL) {

    //  figure out how far entry inst point is from beginning of function..
    CALC_OFFSETS(funcEntry_)

    point = new instPoint(this, owner, adr-imageBaseAddr, functionEntry,
                          newCode[newArrayOffset]);

#ifdef DEBUG_FUNC_RELOC    
    cerr << std::dec << " added entry point at originalOffset = " 
         << originalOffset << ", newOffset = " << newOffset << endl;
#endif
      
    assert(point != NULL);

    point->setRelocated();

    instrAroundPt(point, newCode, funcEntry_->insnsBefore(), 
                  funcEntry_->insnsAfter() + 
                  alteration_set.numInstrAddedAfter(originalOffset), 
                  EntryPt, newArrayOffset);  

    if (location == funcEntry_) {
      location = point;
    }

    // update reloc_info with new instPoint
    reloc_info->addFuncEntry(point);
    assert(reloc_info->funcEntry());
  }


    // Add inst points corresponding to func exits....
  for(retId=0;retId < funcReturns.size(); retId++) {

    CALC_OFFSETS(funcReturns[retId])

    point = new instPoint(this, owner, adr-imageBaseAddr, functionExit, 
                          newCode[newArrayOffset]);

#ifdef DEBUG_FUNC_RELOC
    cerr << std::dec << " added return point at originalOffset = " 
         << originalOffset << ", newOffset = " << newOffset << endl;
#endif

    assert(point != NULL);
 
    point->setRelocated();

    instrAroundPt(point, newCode,funcReturns[retId]->insnsBefore(), 
                  funcReturns[retId]->insnsAfter() + 
                  alteration_set.numInstrAddedAfter(originalOffset), 
                  ReturnPt, newArrayOffset);

    if (location == funcReturns[retId]) {
      location = point;
    } 

    // update reloc_info with new instPoint
    reloc_info->addFuncReturn(point);
  } 

  // Add inst points corresponding to func call sites....
  for(callId=0;callId<calls.size();callId++) {

    CALC_OFFSETS(calls[callId])

    point = new instPoint(this, owner, adr-imageBaseAddr, callSite,
                          newCode[newArrayOffset]);

#ifdef DEBUG_FUNC_RELOC
    cerr << std::dec << " added call site at originalOffset = "
         << originalOffset << ", newOffset = " << newOffset << endl;
#endif

    assert(point != NULL);
 
    point->setRelocated();

    instrAroundPt(point, newCode, calls[callId]->insnsBefore(), 
                  calls[callId]->insnsAfter() + 
                  alteration_set.numInstrAddedAfter(originalOffset),
                  CallPt, newArrayOffset);

    if (location == calls[callId]) {
      location = point;
    }

    // update reloc_info with new instPoint
    reloc_info->addFuncCall(point);
  }

  for(arbitraryId=0;arbitraryId < arbitraryPoints.size();arbitraryId++){

    CALC_OFFSETS(arbitraryPoints[arbitraryId]);

    point = new instPoint(this, owner, adr-imageBaseAddr, otherPoint,
                          newCode[newArrayOffset], true);

    assert(point != NULL);

    point->setRelocated();

    instrAroundPt(point, newCode,funcReturns[arbitraryId]->insnsBefore(),
                  funcReturns[arbitraryId]->insnsAfter() +
                  alteration_set.numInstrAddedAfter(originalOffset),
                  ReturnPt, newArrayOffset);

    if (location == arbitraryPoints[arbitraryId])
	location = point;

    reloc_info->addArbitraryPoint(point);
  }

  return true;    
}

/****************************************************************************/
/****************************************************************************/

// returns the number of instructions that function rewriting will insert 
int InsertNops::numInstrAddedAfter() {
    return sizeNopRegion;
}

// size (in bytes) of x86 nop instruction
int InsertNops::sizeOfNop() {
    return 1;
}

/****************************************************************************/
/****************************************************************************/

// Insert nops after the machine instruction pointed to by oldInstructions[oldOffset]

bool InsertNops::RewriteFootprint(Address /* oldBaseAdr */, Address &oldAdr, 
                                  Address /* newBaseAdr */, Address &newAdr, 
                                  instruction oldInstructions[], 
                                  instruction newInstructions[], 
                                  int &oldOffset, int &newOffset,  
                                  int /* newDisp */, unsigned &codeOffset,
                                  unsigned char *code) {

  unsigned char *insn = 0;

  // copy the instruction we are inserting nops after into relocatedCode 
  function->copyInstruction(newInstructions[newOffset], 
                            oldInstructions[oldOffset], 
                            codeOffset);
  newOffset++;  

  // add nops
  for (int i=0;i<sizeNopRegion;i++) {

    // pointer to the machine code
    insn = (unsigned char *)(&code[codeOffset]);

    // write nop to relocatedCode
    emitSimpleInsn(0x90, insn);
    // emit simple insn increments insn, so we need to decrement insn
    insn--;  

    // add instruction corresponding to nop to buffer of instructions
    newInstructions[newOffset] = *(new instruction ((const unsigned char *)insn, 0, 1));

    newOffset++;
    codeOffset++;
  }
  oldAdr += oldInstructions[oldOffset].size();
  newAdr += oldInstructions[oldOffset].size() + sizeNopRegion;
  oldOffset++;

  return true;
}

/****************************************************************************/
/****************************************************************************/

// expand and relocate a call or jump instruction that uses relative addressing


// oldOffset (offset into old buffer of x6 instructions)
// newOffset (offset into new buffer of x86 instructions)

bool ExpandInstruction::RewriteFootprint(Address /* oldBaseAdr */, 
                                         Address &oldAdr, 
                                         Address /* newBaseAdr */, 
                                         Address &newAdr, 
                                         instruction oldInstructions[], 
                                         instruction newInstructions[], 
                                         int &oldOffset, int &newOffset, 
                                         int newDisp, unsigned &codeOffset,
                                         unsigned char* code) {

#ifdef DEBUG_FUNC_RELOC
    cerr << "ExpandInstruction::RewriteFootprint" <<endl;
    cerr << " newDisp = " << newDisp << endl;
    cerr << " oldOffset = " << oldOffset << endl;
#endif 

 
  unsigned char *oldInsn = 0; 
  unsigned char *newInsn = (unsigned char *)(&code[codeOffset]);
  const unsigned char *tmpInsn = const_cast<const unsigned char *> (newInsn);  

  instruction insn = oldInstructions[oldOffset];
  unsigned oldInsnType = insn.type();
  int oldInsnSize = insn.size();

  int sizeChange = 0;
  bool rtn = false;

  oldInsn = const_cast<unsigned char *> (insn.ptr());

  if (oldInsnType & REL_B) {

    /* replace with rel32 instruction, opcode is one byte. */
    if (*oldInsn == JCXZ) {
      *newInsn++ = *oldInsn; *newInsn++ = 2; // jcxz 2
      *newInsn++ = 0xEB; *newInsn++ = 5;        // jmp 5
      *newInsn++ = 0xE9;                        // jmp rel32

      *((int *)newInsn) = newDisp;   
      newInsn+= sizeof(int);  // move pointer to end of insn
      sizeChange = 7;
      rtn = true;
    } else {
        unsigned newSz=UINT_MAX;
        if (oldInsnType & IS_JCC) {
	  /* Change a Jcc rel8 to Jcc rel32. 
	     Must generate a new opcode: a 0x0F followed by (old opcode + 16) */
	  unsigned char opcode = *oldInsn++;
	  *newInsn++ = 0x0F;
	  *newInsn++ = opcode + 0x10;

          sizeChange = 4;
	  newSz = 6;
        } else {
            if (oldInsnType & IS_JUMP) {
	      /* change opcode to 0xE9 */
	      oldInsn++;
	      *newInsn++ = 0xE9;

	      sizeChange = 3;
              newSz = 5;
	    }
	}     
        assert(newSz!=UINT_MAX);
        *((int *)newInsn) = newDisp; 
        newInsn += sizeof(int);
        rtn = true;
    }
  } else {
      if (oldInsnType & REL_W) {
        assert(oldInsnType & PREFIX_OPR);
        if (oldInsnType & PREFIX_SEG)
          *newInsn++ = *oldInsn++;
	
           /* opcode is unchanged, just relocate the displacement */
   
          if (*oldInsn == (unsigned char)0x0F)
            *newInsn++ = *oldInsn++;
          *newInsn++ = *oldInsn++;

          *((int *)newInsn) = newDisp;
          newInsn += sizeof(int);
          sizeChange = 1;
          rtn = true;
      } else {
	  // should never get here
          assert (oldInsnType & REL_D);
          assert (0); 
      }
  }

  oldAdr += oldInsnSize;
  newAdr += (oldInsnSize + sizeChange);
  oldOffset++;
  newOffset++;
  codeOffset += (oldInsnSize + sizeChange);

#ifdef DEBUG_FUNC_RELOC
  cerr << "rewrote footprint from " << oldInsnSize << " to "
       << oldInsnSize + sizeChange << endl;  
#endif 

  unsigned newInsnType, newInsnSize;
  newInsnSize = get_instruction(tmpInsn, newInsnType);
  newInstructions[newOffset - 1] = *(new instruction(tmpInsn, newInsnType, newInsnSize));

  return rtn;   
}
  
/****************************************************************************/
/****************************************************************************/

// Change call to adr+5 to 
//   push  %eip

bool PushEIP::RewriteFootprint(Address /* oldBaseAdr */, Address &oldAdr, 
                               Address /* newBaseAdr */, Address &newAdr, 
                               instruction oldInstructions[], 
                               instruction newInstructions[], 
                               int &oldInsnOffset, int &newInsnOffset,  
                               int /* newDisp */, unsigned &codeOffset,
                               unsigned char *code) 
{

#ifdef DEBUG_FUNC_RELOC
    cerr << "PushEIP::RewriteFootprint" <<endl;
#endif 

  instruction oldInsn = oldInstructions[oldInsnOffset]; 
  unsigned char *insn = (unsigned char *)(&code[codeOffset]);

  // Push OpCode
  *insn = 0x68;

  // address of instruction following call, in original function location
  *((unsigned int *)(insn+1)) = oldAdr + 5;

  oldAdr += oldInsn.size();  // size of call instruction
  newAdr += 5;               // push is 5 bytes
  oldInsnOffset++;
  newInsnOffset++;
  codeOffset += 5;
 
  // Generate x86 instruction object, and place it in buffer of instructions
  unsigned newInsnType, newInsnSize;
  newInsnSize = get_instruction(const_cast<const unsigned char *> (insn), 
                                                            newInsnType);
  newInstructions[newInsnOffset - 1] = *(new instruction(
                                      const_cast<const unsigned char *> (insn),
                                      newInsnType, newInsnSize));

  return true;
}

/****************************************************************************/
/****************************************************************************/

// Change call to adr+5 to 
//   push  (adr+5)
//   mov   (adr+5), %ebx

// This places the address of the instruction after the call on the stack, 
// and then copies that address into the %ebx register
bool PushEIPmov::RewriteFootprint(Address /* oldBaseAdr */, Address &oldAdr, 
                                  Address /* newBaseAdr */, Address &newAdr, 
                                  instruction oldInstructions[], 
                                  instruction newInstructions[], 
                                  int &oldInsnOffset, int &newInsnOffset,  
                                  int /* newDisp */, unsigned &codeOffset,
                                  unsigned char *code) 
{

#ifdef DEBUG_FUNC_RELOC
    cerr << "PushEIPmov::RewriteFootprint" <<endl;
#endif 

  instruction oldInsn = oldInstructions[oldInsnOffset]; 
  unsigned oldInsnSize = oldInsn.size();

  unsigned char *movInsn = (unsigned char *)(&code[codeOffset]);
  unsigned int movSize = 5;


  // Generate mov (adr + 5), %ebx instruction
  // mov OpCode
  *movInsn++ = 0xbb;

  // address of instruction following call
  *((unsigned int *)movInsn) = (unsigned int)(oldAdr + 5);   
  movInsn--;
 


  // Generate x86 instruction object for the mov and place it in the new 
  // buffer of x86 instruction objects
  unsigned newInsnType, newInsnSize;

  newInsnSize = get_instruction(const_cast<const unsigned char *> (movInsn), 
                                                               newInsnType);

  newInstructions[newInsnOffset] = *(new instruction(movInsn, newInsnType,
                                                             newInsnSize));


  // Update offsets and addresses
  oldAdr        += oldInsnSize;        // read oldInsnSize bytes
  newAdr        += movSize;            // wrote movSize bytes
  oldInsnOffset += 1;                  // one old instruction
  newInsnOffset += 1;                  // one new instruction
  codeOffset    += movSize;            // wrote movSize bytes
  
  return true;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// Find overlapping instrumentation points 

bool pd_Function::PA_attachOverlappingInstPoints(
                              LocalAlterationSet *temp_alteration_set, 
                              Address baseAddress, Address firstAddress,
                              instruction* /* loadedCode */, int /* codeSize */) {

  int overlap = 0, offset = 0;

#ifdef DEBUG_FUNC_RELOC
    cerr << "pd_Function::PA_attachOverlappingInstPoints called" <<endl;
#endif

    // create and sort vector of instPoints
    pdvector<instPoint*> foo;
    sorted_ips_vector(foo);

    // loop over all consecutive pairs of instPoints
    for (unsigned i=0;i<foo.size()-1;i++) {
        instPoint *this_inst_point = foo[i];
        instPoint *next_inst_point = foo[i+1];

        overlap = ((this_inst_point->iPgetAddress() + 
                    this_inst_point->sizeOfInstrumentation()) - 
                    next_inst_point->iPgetAddress()); 

        // check if inst point overlaps with next inst point
        if (overlap > 0) {
            offset = ((this_inst_point->iPgetAddress() + baseAddress) - firstAddress);

            // LocalAlteration inserting nops after this_inst_point
       	    InsertNops *nops = new InsertNops(this, offset, overlap);
	    temp_alteration_set->AddAlteration(nops);

#ifdef DEBUG_FUNC_RELOC
    cerr << " detected overlapping inst points: "  << endl;
    cerr << " adding LocalAlteration of size: " << overlap 
         << " at offset: " << offset << endl; 
#endif

	}
    }
    return true;
}

/****************************************************************************/
/****************************************************************************/

// Locate jumps with targets inside the footprint of an inst points. 

bool pd_Function::PA_attachBranchOverlaps(
                           LocalAlterationSet *temp_alteration_set, 
                           Address /* baseAddress */, Address firstAddress, 
                           instruction loadedCode[],
                           unsigned numberOfInstructions, int codeSize)  {

#ifdef DEBUG_FUNC_RELOC
    cerr << "pd_Function::PA_attachBranchOverlaps called" <<endl;
    cerr << " codeSize = " << codeSize << endl;
    cerr << " numberOfInstructions = " << numberOfInstructions << endl;
#endif

  int instr_address;
  int disp, offset;
  instruction instr;
   
  // create and sort vector of instPoints
  pdvector<instPoint*> foo;
  sorted_ips_vector(foo);

  // Iterate over function instruction by instruction....
  for(unsigned i = 0; i < numberOfInstructions; i++) {       
    instr = loadedCode[i];
    instr_address = addressOfMachineInsn(&instr);
     
    // look for branch and call insns whose targets are inside the function.
    if (!branchInsideRange(instr, instr_address, firstAddress, 
                                        firstAddress + codeSize) &&
        !trueCallInsideRange(instr, instr_address, firstAddress, 
                                        firstAddress + codeSize)) {
      continue;
    } 

#ifdef DEBUG_FUNC_RELOC
    cerr << " branch at " << std::hex << (unsigned) instr_address 
         << " insn offset = "  << instr_address - firstAddress
         << " has target inside range of function" << endl;
#endif  

    disp = get_disp(&instr);

    // target of branch or call instruction 
    Address target = instr_address + disp;

    // Check if target is in the footprint of an inst point....
    instPoint *overlap = find_overlap(foo, target);
    if (overlap == NULL) continue;

    // offset of instruction from the beginning of the function 
    offset = overlap->iPgetAddress() - firstAddress;

    temp_alteration_set->iterReset();

    // If multiple jumps have their target address within the same 
    // instPoint, we only want to add nops once. To do this we
    // iterate over the known LocalAlterations, checking if any already 
    // are already planning on inserting nops at this instPoint.
    LocalAlteration *alteration = temp_alteration_set->iterNext();
    while (alteration != NULL && alteration->getOffset() < offset) {
      alteration = temp_alteration_set->iterNext();
    }

    if (alteration == NULL || alteration->getOffset() != offset) {
 
      int shift = overlap->followingAddress() - target;

      InsertNops *nops = new InsertNops(this, offset, shift);
      temp_alteration_set->AddAlteration(nops);

#ifdef DEBUG_FUNC_RELOC
   cerr << " detected overlap between branch target and inst point : offset "
        << target - firstAddress << " # bytes " 
        << overlap->firstAddress() - target << endl;
   cerr << " adding LocalAlteration" << endl;        
#endif

    }
  }
  return true;
}

/****************************************************************************/
/****************************************************************************/

bool pd_Function::PA_attachGeneralRewrites(
			      const image* owner,
                              LocalAlterationSet *temp_alteration_set, 
                              Address baseAddress, Address firstAddress,
                              instruction* loadedCode,
                              unsigned numInstructions, 
                              int /* codeSize */ ) {

#ifdef DEBUG_FUNC_RELOC
    cerr << "pd_Function::PA_attachGeneralRewrites" << endl;
    cerr << " baseAddress = " << std::hex << baseAddress << endl;
    cerr << " firstAddress = " << std::hex << firstAddress << endl;
#endif

    int size;
    int offset;
    instruction instr;
    Address instr_address;

    // create and sort vector of instPoints
    pdvector<instPoint*> foo;
    sorted_ips_vector(foo);

    // loop over all consecutive pairs of instPoints
    for (unsigned i=0;i<foo.size();i++) {

      // check if instPoint has enough space for jump
      instPoint *ip = foo[i];
      if (ip->size() < JUMP_REL32_SZ) {
        offset = (ip->iPgetAddress() + baseAddress) - firstAddress; 

        InsertNops *nops = new InsertNops(this, offset, ip->extraBytes());
	temp_alteration_set->AddAlteration(nops);

#ifdef DEBUG_FUNC_RELOC
    cerr << "adding LocalAlteration for inserting nops" << endl;
    cerr << "ipAddress = " << std::hex << ip->iPgetAddress() + baseAddress
         << endl;
    cerr << "offset = " << (ip->iPgetAddress() + baseAddress) - firstAddress
         << endl;
#endif 
      }
    }

    // address of first instruction in function    
    instr_address = getAddress(0) + baseAddress;
 
    // offset of instruction in function
    offset = 0;

    // size of previous instruction in function
    size = 0;

    // Iterate over all instructions looking for calls to adr+5
    for(unsigned j = 0; j < numInstructions; j++) {       
      instr = loadedCode[j];
      instr_address += size;
            // check if instruction is a relative addressed call
      if ( instr.isCall() && !instr.isCallIndir() ) {

        // Look for call to adr+5
        if (instr.getTarget(instr_address) == instr_address + 5) {

#ifdef DEBUG_FUNC_RELOC
    cerr << "adding localAlteration for call to next instruction" 
         << " at offset " << offset << endl;
#endif              
          PushEIP *eip = new PushEIP(this, offset); 
	  temp_alteration_set->AddAlteration(eip);
	}

        
        // Look for call to:
        //   mov   %esp, %ebx
        //   ret

        int targetAdr = instr.getTarget(instr_address-baseAddress);

        const unsigned char *insnPtr = instr.ptr();      
 
        if (*(insnPtr) == 0xe8) {

          if ( !owner->isValidAddress(targetAdr) ) {
            cerr << "ERROR: " << prettyName() << " has a call at " 
                 << instr_address - baseAddress << " with target "
                 << " outside the application's address space" << endl;
            return false;
          }

          // Get a pointer to the call target
          const unsigned char *target = (const unsigned char *)owner->getPtrToInstruction(targetAdr);

          // The target instruction is a mov
          if (*(target) == 0x8b) {

            // The source register of the mov is specified by a SIB byte 
            if (*(target + 1) == 0x1c) {
 
              // The source register of the mov is the %esp register (0x24) 
              // and the instruction after the mov is a ret instruction (0xc3)
              if ( (*(target + 2) == 0x24) && (*(target + 3) == 0xc3)) {

#ifdef DEBUG_FUNC_RELOC
                cerr << std::hex
                     << "Adding PushEIPmov LocalAlteration at offset " 
                     << offset << " of " << prettyName() << endl;
#endif
                PushEIPmov *eipMov = new PushEIPmov(this, offset); 
  	        temp_alteration_set->AddAlteration(eipMov);
	      }
	    }
	  }
	}
      }

      // iterated over another instruction
      size = instr.size();
      offset += size;
    }

    return true;
}

// modifyInstPoint: if the function associated with the process was 
// recently relocated, then the instPoint may have the old pre-relocated
// address (this can occur because we are getting instPoints in mdl routines 
// and passing these to routines that do the instrumentation, it would
// be better to let the routines that do the instrumenting find the points)

void pd_Function::modifyInstPoint(const instPoint *&location, process *proc) {
    
    unsigned retId = 0, callId = 0,arbitraryID = 0;
    bool found = false;
    if (!call_points_have_been_checked)
      checkCallPoints();

    if(relocatable_ && !(const_cast<instPoint *>(location)->getRelocated())){
        for(u_int i=0; i < relocatedByProcess.size(); i++){
           if((relocatedByProcess[i])->getProcess() == proc){
               if(location == funcEntry_){
                 const instPoint *new_entry =
                                        ((relocatedByProcess[i])->funcEntry());
                 location = new_entry;
               } 
               else {
                 for(retId=0;retId < funcReturns.size(); retId++) {
                    if(location == funcReturns[retId]){
                       const pdvector<instPoint *> new_returns = 
                          (relocatedByProcess[i])->funcReturns(); 
                       location = (new_returns[retId]);
                       found = true;
                       break;
		    }
		 }
                 if (found) break;
         
                 for(callId=0;callId < calls.size(); callId++) {
                    if(location == calls[callId]){
                       pdvector<instPoint *> new_calls = 
                          (relocatedByProcess[i])->funcCallSites(); 
                       location = (new_calls[callId]);
                       found = true;
                       break;
                    }
		 }
                 if (found) break;

                 for(arbitraryID=0;
		     arbitraryID < arbitraryPoints.size(); 
		     arbitraryID++) 
		 {
                    if(location == arbitraryPoints[arbitraryID]){
                       const pdvector<instPoint *> new_arbitrary = 
                          (relocatedByProcess[i])->funcArbitraryPoints(); 
                       location = (new_arbitrary[arbitraryID]);
                       break;
		    }
		 }

               break;
	       }
	   }
	}
    }
}

/****************************************************************************/
/****************************************************************************/

// Check if an ExpandInstruction alteration has already been created 
// at an offset, with size, shift.
// This allows us to avoid adding a new ExpandInstruction LocalAlterations for
// an instruction each time we go through discoverAlterations. This prevents
// an infinite looping 

bool alreadyExpanded(int offset, int shift, LocalAlterationSet *alteration_set) {
  bool already_expanded;
  LocalAlteration *alteration = 0;

  alteration_set->iterReset();

  // find the LocalAlteration at offset  
  do {
    alteration = alteration_set->iterNext();
  } while (alteration != NULL && alteration->getOffset() < offset);
  
  if ((alteration != NULL) && (alteration->getOffset() == offset)) {
 
    alteration = dynamic_cast<ExpandInstruction *> (alteration);

    if (alteration == NULL || alteration->getShift() > shift) {
      already_expanded = false;
    }
    else {
      already_expanded = true;
    }
  } else {
      already_expanded = false;
  }
  
  alteration_set->iterReset();
  return already_expanded;
}

/****************************************************************************/
/****************************************************************************/

// It may be that in two different passes over a function, we note 
// two different LocalAlterations at the same offset. This function 
// reconciles any conflict between the LocalAlterations and merges them into 
// a single LocalAlteration, or ignores one of them,

LocalAlteration *fixOverlappingAlterations(LocalAlteration *alteration, 
                                           LocalAlteration *tempAlteration) {

  LocalAlteration *casted_alteration = 0, *casted_tempAlteration = 0; 

#ifdef DEBUG_FUNC_RELOC 
	   cerr << "Function fixOverlappingAlterations called" << endl;
#endif

  // assert that there is indeed a conflict  
  assert (alteration->getOffset() == tempAlteration->getOffset()); 

  casted_alteration = dynamic_cast<ExpandInstruction *> (alteration); 
  casted_tempAlteration = dynamic_cast<ExpandInstruction *> (tempAlteration);
  if (casted_alteration != NULL) {
    return alteration;
  } else {
      if (casted_tempAlteration != NULL) {
        return tempAlteration;
      }
  }  
  
  casted_alteration = dynamic_cast<InsertNops *> (alteration); 
  casted_tempAlteration = dynamic_cast<InsertNops *> (tempAlteration); 
  if (casted_alteration != NULL) {
    if (casted_tempAlteration != NULL) {
      if (alteration->getShift() >= tempAlteration->getShift()) {
        return alteration;
      } else {  
          return tempAlteration;
      }
    }
  }

  return NULL;
}


// needed in metric.C
bool instPoint::match(instPoint *p)
{
  if (this == p)
    return true;
  
  // should we check anything else?
  if (addr_ == p->addr_)
    return true;
  
  return false;
}

// Get the absolute address of an instPoint
Address instPoint::iPgetAddress(process *p) const {
    if (!p) return addr_;
    Address baseAddr;
    p->getBaseAddress(iPgetOwner(), baseAddr);
    return addr_ + baseAddr;
}

void pd_Function::addArbitraryPoint(instPoint* location,
				    process* proc,
				    relocatedFuncInfo* reloc_info)
{
    if(!isInstalled(proc))
	return;

    instPoint *point;
    int originalOffset, newOffset;
    //int arrayOffset, 
    int originalArrayOffset, newArrayOffset;

    Address mutatee,newAdr,mutator;
    LocalAlterationSet alteration_set(this);
    Address adr;

    instruction *oldCode = NULL, *newCode = NULL;

    const image* owner = location->iPgetOwner();

    findAlterations(owner,proc,oldCode,alteration_set,
                                    mutator, mutatee);
    Address imageBaseAddr;
    if (!proc->getBaseAddress(owner,imageBaseAddr))
        abort();

    newAdr = reloc_info->address();

    originalOffset = ((location->iPgetAddress() + imageBaseAddr) - mutatee);
    originalArrayOffset = getArrayOffset(originalOffset + mutator, oldCode);
    assert(originalArrayOffset >= 0);
    newOffset = originalOffset + alteration_set.getShift(originalOffset);
    newArrayOffset = originalArrayOffset +
		  alteration_set.getInstPointShift(originalOffset);
    adr = newAdr + newOffset;

    newCode  = reinterpret_cast<instruction *> (relocatedCode);
 
    point = new instPoint(this, owner, newAdr-imageBaseAddr, otherPoint,
                          newCode[newArrayOffset], true);

    point->setRelocated();

    instrAroundPt(point, newCode, location->insnsBefore(), 
                  location->insnsAfter() + 
                  alteration_set.numInstrAddedAfter(originalOffset), 
                  OtherPt, newArrayOffset);

    reloc_info->addArbitraryPoint(point);

    delete[] oldCode;
}
