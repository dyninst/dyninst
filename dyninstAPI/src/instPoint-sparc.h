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

// $Id: instPoint-sparc.h,v 1.24 2004/02/28 00:26:30 schendel Exp $
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

class instPoint : public instPointBase {
public:

  instPoint(pd_Function *f, Address &adr, const bool delayOK,
            instPointType ipt, bool noCall=false);

  instPoint(unsigned int id_to_use, pd_Function *f, const instruction instr[], 
            int instrOffset, Address &adr, bool delayOK,
            instPointType pointType);

  ~instPoint() {  /* TODO */ }

  const instruction &insnAtPoint() const { return firstInstruction; }

  const instruction insnAfterPoint() const { return secondInstruction; }

  Address getTargetAddress() {
      if(!isCallInsn(firstInstruction)) return 0;
       if (!isInsnType(firstInstruction, CALLmask, CALLmatch)) {
	       return 0;  // can't get target from indirect call instructions
       }

      Address ta = (firstInstruction.call.disp30 << 2); 
      return ( pointAddr()+ta ); 
  }

  // formerly "isLeaf()", but the term leaf fn is confusing since people use it
  // for two different characteristics: 
  // (1) has no stack frame, 
  // (2) makes no call.
  // By renaming the fn, we make clear what it's returning.

  bool hasNoStackFrame() const {
     assert(pointFunc());
     return pointFunc()->hasNoStackFrame();
  }

  // ALERT ALERT - FOR NEW PA CODE........
  // offset (in bytes) from beginning of function at which the FIRST 
  //  instruction of the inst point in located....
  int Size() {return size;}

  // address of 1st instruction to be clobbered by inst point....
  // NOTE: This is not always the same as insnAddr
  Address firstAddress() {return pointAddr();}

  // address of 1st instruction AFTER those clobbered by inst point....
  Address followingAddress() {return pointAddr() + Size();}

  // address of the actual instPoint instruction
  int insnAddress() { return insnAddr; }

// TODO: These should all be private

  Address insnAddr;   // address of the instruction to be instrumented

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
  bool isBranchOut;             // true if this point is a conditional branch, 
				// that may branch out of the function
  int branchTarget;             // the original target of the branch
  int instId;                   // id of inst in this function
  int size;                     // size of multiple instruction sequences

  bool needsLongJump;              // true if it turned out the branch from this 
                                // point to baseTramp needs long jump.
  // VG(4/25/2002): True if call may not be used to instrument this point
  // because the instruction after this one is an inst point itself
  bool dontUseCall;
};

#endif
