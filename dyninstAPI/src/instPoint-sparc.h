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

// $Id: instPoint-sparc.h,v 1.13 2001/05/12 21:29:38 ning Exp $
// sparc-specific definition of class instPoint

#ifndef _INST_POINT_SPARC_H_
#define _INST_POINT_SPARC_H_

#include "common/h/Types.h" // for "Address" (typedef'd to unsigned)
#include "dyninstAPI/src/symtab.h"
#include "arch-sparc.h" // for union type "instruction"

class process;

typedef enum {
    noneType,
    functionEntry,
    functionExit,
    callSite,
    otherPoint
} instPointType;

class instPoint {
public:
  instPoint(pd_Function *f, const instruction &instr, const image *owner,
	    Address &adr, const bool delayOK,
	    instPointType ipt);
  instPoint(pd_Function *f, const instruction &instr, const image *owner,
            Address &adr, const bool delayOK, instPointType ipt,   
	    Address &oldAddr);
  ~instPoint() {  /* TODO */ }

  // can't set this in the constructor because call points can't be classified
  // until all functions have been seen -- this might be cleaned up
  void set_callee(pd_Function *to) { callee = to; }

  // get instruction at actual inst point (not nec. first instruction, e.g. look
  //  at entry instrumentation below)....
  const instruction &insnAtPoint() const { return originalInstruction;}
  // return instruction directly following originalInstruction (which data member
  //  that corresponds to may vary - see explanation of various instruction data
  //  members below)
  // ALRT ALRT!!!!  FILL IN!!!!
  const instruction insnAfterPoint() const;

  const function_base *iPgetFunction() const { return func;      }
  const function_base *iPgetCallee()   const { return callee;    }
  const image         *iPgetOwner()    const { return image_ptr; }
        Address        iPgetAddress()  const { return addr;      }

  Address getTargetAddress() {
      if(!isCallInsn(originalInstruction)) return 0;
       if (!isInsnType(originalInstruction, CALLmask, CALLmatch)) {
	       return 0;  // can't get target from indirect call instructions
       }

      Address ta = (originalInstruction.call.disp30 << 2); 
      return ( addr+ta ); 
  }

  bool hasNoStackFrame() const {
     // formerly "isLeaf()", but the term leaf fn is confusing since people use it
     // for two different characteristics: (1) has not stack frame, (2) makes no call.
     // By renaming the fn, we make clear what it's returning.
     assert(func);
     return func->hasNoStackFrame();
  }

  // ALERT ALERT - FOR NEW PA CODE........
  // offset (in bytes) from beginning of function at which the FIRST 
  //  instruction of the inst point in located....
  int Size() {return size;}

  // address of 1st instruction to be clobbered by inst point....
  //  Is this always same as iPgetAddress????
  Address firstAddress() {return iPgetAddress();}
  // address of 1st instruction AFTER those clobbered by inst point....
  Address followingAddress() {return firstAddress() + Size();}
  
  // size (in bytes) of a sparc jump instruction 
  int sizeOfInstrumentation() {return 4;}

  bool match(instPoint *p);

  // address of the actual instPoint instruction
  int insnAddress() { return insnAddr; }

  bool getRelocated() { return relocated_; }
  void setRelocated() {relocated_ = true;}

  // These are the instructions corresponding to different instrumentation
  // points (the instr point is always the "originalInstruction" instruction,
  // "addr" has the value of the address of "originalInstruction")
  // 
  //  entry point	   entry point leaf	  call site 
  //  -----------	   ----------------       ---------
  //  saveInstr		   originalInstruction    originalInstruction	
  //  originalInstruction  otherInstruction	  delaySlotInsn
  //  delaySlotInsn 	   delaySlotInsn	  aggregateInsn
  //  isDelayedInsn 	   isDelayedInsn
  //  aggregateInsn 
  // 
  //  return leaf		return non-leaf
  //  ----------		---------------
  //  originalInstruction  	originalInstruction
  //  otherInstruction		delaySlotInsn
  //  delaySlotInsn
  //  inDelaySlotInsn
  //  extraInsn
  // 

// TODO: These should all be private
  Address insnAddr;                   // address of the instPoint
  Address addr;                       // address of the first insn in the 
                                      // instPoints footprint 
  instruction originalInstruction;    // original instruction
  instruction delaySlotInsn;          // original instruction
  instruction aggregateInsn;          // aggregate insn
  instruction otherInstruction;       
  instruction isDelayedInsn;  
  instruction inDelaySlotInsn;
  instruction extraInsn;   	// if 1st instr is conditional branch this is
			        // previous instruction 
  instruction saveInsn;         // valid only with nonLeaf function entry 

  bool inDelaySlot;             // Is the instruction in a delay slot
  bool isDelayed;		// is the instruction a delayed instruction
  bool callIndirect;		// is it a call whose target is rt computed ?
  bool callAggregate;		// calling a func that returns an aggregate
				// we need to reolcate three insns in this case
  pd_Function *callee;		// what function is called
  pd_Function *func;		// what function we are inst
  bool isBranchOut;             // true if this point is a conditional branch, 
				// that may branch out of the function
  int branchTarget;             // the original target of the branch
  instPointType ipType;
  int instId;                   // id of inst in this function
  int size;                     // size of multiple instruction sequences
  const image *image_ptr;	// for finding correct image in process
  bool firstIsConditional;      // 1st instruction is conditional branch
  bool relocated_;	        // true if instPoint is from a relocated func

  bool isLongJump;              // true if it turned out the branch from this 
                                // point to baseTramp needs long jump.   

 private:


};


#endif
