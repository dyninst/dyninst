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

// $Id: LocalAlteration-Sparc.h,v 1.5 2001/02/20 21:40:50 gurari Exp $

#ifndef __LocalAlteration_SPARC_H__
#define __LocalAlteration_SPARC_H__


#include "dyninstAPI/src/LocalAlteration.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/FunctionExpansionRecord.h"
#include "common/h/Vector.h"
#include "dyninstAPI/src/dyninst.h"
#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/util.h"

//
// TAIL-CALL OPTIMIZATION BASE PEEPHOLE ALTERATION (abstract)....
//
// Peephole alteration to unwind tail-call optimization.
// 
class TailCallOptimization : public LocalAlteration {
  public:

    // Constructor - same as LocalAlteration????
    TailCallOptimization(pd_Function *f, int offsetBegins, \
	int offsetEnds);

  protected:
    int ending_offset;
};

//
// TAIL-CALL OPTIMIZATION PA VARIANTS....
//

// used to unwind tail-call optimizations which match the pattern:
//  call PC_REL_ADDRESS
//  restore
//  
class CallRestoreTailCallOptimization : public TailCallOptimization {
 protected:
     // is the call a "true" call - meaning call to address (really PC + offset)
     //  or a"jump and link" call - meaning call through a register 
     //  (really register + offset) 
     bool true_call, jmpl_call;

     void SetCallType(instruction callInsn);
 public:
     CallRestoreTailCallOptimization(pd_Function *f, int beginning_offset, \
	int ending_offset, instruction callInsn);
     // update branch targets in/around footprint....
     virtual bool UpdateExpansions(FunctionExpansionRecord *fer);
     // update inst point locations in/around footprint....
     virtual bool UpdateInstPoints(FunctionExpansionRecord *ips);
     virtual bool RewriteFootprint(Address oldBaseAdr, Address &oldAdr, 
                                   Address newBaseAdr, Address &newAdr,
                                   instruction oldInstr[], 
                                   instruction newInstr[], 
                                   int &oldOffset, int &newOffset,
                                   int newDisp,
                                   unsigned &codeOffset, 
                                   unsigned char *code);
     virtual int getShift();
     virtual int numInstrAddedAfter();
     virtual int getOffset() { return beginning_offset; }
};

// used to unwind tail-call optimizations which match the pattern:
//  jmp 
//  nop
//
class JmpNopTailCallOptimization : public TailCallOptimization {
 public:
     JmpNopTailCallOptimization(pd_Function *f, int beginning_offset, \
	int ending_offset);
     // update branch targets and inst point locations in/around footprint....
     virtual bool UpdateExpansions(FunctionExpansionRecord *fer);
     // update inst point locations in/around footprint....
     virtual bool UpdateInstPoints(FunctionExpansionRecord *ips);
     virtual bool RewriteFootprint(Address oldBaseAdr, Address &oldAdr, 
                                   Address newBaseAdr, Address &newAdr,
                                   instruction oldInstr[], 
                                   instruction newInstr[], 
                                   int &oldOffset, int &newOffset,
                                   int newDisp,
                                   unsigned &codeOffset, 
                                   unsigned char *code);
     virtual int getShift();
     virtual int numInstrAddedAfter();
     virtual int getOffset() { return beginning_offset; }
};

//class SecondInsnCall : public LocalAlteration {
//    
//};


// Sparc code sequences in shared libraries often contain something like:
//  call PC + 8
//  nop
// The function of such a sequence is to set the 07 register (a side effect 
//  of the call insn on the sparc arch) - this can be used e.g. to figure
//  out where the executing function has been loaded.
// When code is relocated, paradyn often wants the 07 the be set to value 
//  which it would have been set to if the code had NOT been relocated.
// One method (used in older paradyn releases) is to insert a call to the old
//  code, and also a call back from it (with an add in its delay slot).
// However, given that we know the locatiosn (virtual addresss) at which
//  both the origional code was loaded, and at which the relcoated code 
//  will be placed when we do the alterations, it seems that
//  a simpler solution is just to replace the original 
//    call PC + 8
//    nop
//  with a sequence which explicitly sets 07 to the address of the original
//   call:
//    setlo %07
//    sethi %07
//    
class SetO7 : public LocalAlteration {
 public:
    // Set07 constructor - same as LocalAlteration....
    SetO7(pd_Function *f, int offset);
    virtual bool UpdateExpansions(FunctionExpansionRecord *fer);
    virtual bool UpdateInstPoints(FunctionExpansionRecord *ips);
    virtual bool RewriteFootprint(Address oldBaseAdr, Address &oldAdr, 
                                   Address newBaseAdr, Address &newAdr,
                                   instruction oldInstr[], 
                                   instruction newInstr[], 
                                   int &oldOffset, int &newOffset,
                                   int newDisp,
                                   unsigned &codeOffset, 
                                   unsigned char *code);

    
    virtual int getOffset() { return beginning_offset; }
    virtual int getShift() { return sizeof(instruction); }
    virtual int numInstrAddedAfter();
};



/*  #ifndef __LocalAlteration_SPARC_H__  */
#endif
