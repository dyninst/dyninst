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

// $Id: instPoint-sparc.h,v 1.19 2003/02/26 21:27:41 schendel Exp $
// sparc-specific definition of class instPoint

#ifndef _INST_POINT_SPARC_H_
#define _INST_POINT_SPARC_H_

#include "common/h/Types.h" // for "Address" (typedef'd to unsigned)
#include "dyninstAPI/src/symtab.h"
#include "arch-sparc.h" // for union type "instruction"

class process;

#ifdef BPATCH_LIBRARY
class BPatch_point;
#endif

typedef enum {
    noneType,
    functionEntry,
    functionExit,
    callSite,
    otherPoint
} instPointType;

class instPoint {
public:

  instPoint(pd_Function *f, const image *owner, Address &adr, 
            const bool delayOK, instPointType ipt, bool noCall=false);

  instPoint(pd_Function *f, const instruction instr[], 
            int instrOffset, const image *owner, Address &adr, 
            bool delayOK, instPointType pointType);

  ~instPoint() {  /* TODO */ }

  // can't set this in the constructor because call points can't be classified
  // until all functions have been seen -- this might be cleaned up
  void set_callee(pd_Function *to) { callee = to; }

  const instruction &insnAtPoint() const { return firstInstruction; }

  const instruction insnAfterPoint() const { return secondInstruction; }

  const function_base *iPgetFunction() const { return func;      }
  const function_base *iPgetCallee()   const { return callee;    }
  const image         *iPgetOwner()    const { return image_ptr; }
        Address        iPgetAddress()  const { return addr;      }

  Address getTargetAddress() {
      if(!isCallInsn(firstInstruction)) return 0;
       if (!isInsnType(firstInstruction, CALLmask, CALLmatch)) {
	       return 0;  // can't get target from indirect call instructions
       }

      Address ta = (firstInstruction.call.disp30 << 2); 
      return ( addr+ta ); 
  }

  // formerly "isLeaf()", but the term leaf fn is confusing since people use it
  // for two different characteristics: 
  // (1) has no stack frame, 
  // (2) makes no call.
  // By renaming the fn, we make clear what it's returning.

  bool hasNoStackFrame() const {
     assert(func);
     return func->hasNoStackFrame();
  }

  // ALERT ALERT - FOR NEW PA CODE........
  // offset (in bytes) from beginning of function at which the FIRST 
  //  instruction of the inst point in located....
  int Size() {return size;}

  // address of 1st instruction to be clobbered by inst point....
  // NOTE: This is not always the same as insnAddr
  Address firstAddress() {return addr;}

  // address of 1st instruction AFTER those clobbered by inst point....
  Address followingAddress() {return addr + Size();}
  
  bool match(instPoint *p);

  // address of the actual instPoint instruction
  int insnAddress() { return insnAddr; }

  bool getRelocated() { return relocated_; }
  void setRelocated() { relocated_ = true; }

  instPointType getPointType() { return ipType; }

// TODO: These should all be private

  Address insnAddr;   // address of the instruction to be instrumented
  Address addr;       // address of the first insn in the instPoint's 
                      // footprint (not counting the prior instructions)

  instruction firstPriorInstruction;   // The three instructions just before
  instruction secondPriorInstruction;  // the instruction to be instrumented
  instruction thirdPriorInstruction;

  instruction firstInstruction;        // The instruction to be instrumented
  instruction secondInstruction;       // and the four instructions that
  instruction thirdInstruction;        // come after it
  instruction fourthInstruction;
  instruction fifthInstruction;

  bool firstPriorIsDCTI;               // indicate whether the named instruction
  bool secondPriorIsDCTI;              // is a Delayed Control Transfer 
  bool thirdPriorIsDCTI;               // Instruction
  bool firstIsDCTI;
  bool secondIsDCTI;
  bool thirdIsDCTI;

  bool firstPriorIsAggregate;          // indicate whether the instruction is a
  bool thirdIsAggregate;               // the aggregate of a call.
  bool fourthIsAggregate;              // (i.e. when a function returns
  bool fifthIsAggregate;               //  a structure, the instruction after 
                                       //  the delay slot of any call to that
                                       //  function is a byte indicating the
                                       //  the size of the structure that will
                                       //  be returned)
  
  bool usesPriorInstructions;  // whether prior instructions need to be claimed
  int  numPriorInstructions;   // how many prior instructions

  bool callIndirect;		// is it a call whose target is rt computed ?
  pd_Function *callee;		// what function is called
  pd_Function *func;		// what function we are inst
  bool isBranchOut;             // true if this point is a conditional branch, 
				// that may branch out of the function
  int branchTarget;             // the original target of the branch
  instPointType ipType;
  int instId;                   // id of inst in this function
  int size;                     // size of multiple instruction sequences
  const image *image_ptr;	// for finding correct image in process
  bool relocated_;	        // true if instPoint is from a relocated func

  bool needsLongJump;              // true if it turned out the branch from this 
                                // point to baseTramp needs long jump.
  // VG(4/25/2002): True if call may not be used to instrument this point
  // because the instruction after this one is an inst point itself
  bool dontUseCall;

  // VG(11/06/01): there is some common stuff amongst instPoint
  // classes on all platforms (like addr and the back pointer to
  // BPatch_point). 
  // TODO: Merge these classes and put ifdefs for platform-specific
  // fields.
#ifdef BPATCH_LIBRARY
 private:
  // We need this here because BPatch_point gets dropped before
  // we get to generate code from the AST, and we glue info needed
  // to generate code for the effective address snippet/node to the
  // BPatch_point rather than here.
  friend class BPatch_point;
  BPatch_point *bppoint; // unfortunately the correspondig BPatch_point
  			 // is created afterwards, so it needs to set this
 public:
  const BPatch_point* getBPatch_point() const { return bppoint; }
  void setBPatch_point(const BPatch_point *p) { bppoint = const_cast<BPatch_point *>(p); }
#endif

};

#endif
