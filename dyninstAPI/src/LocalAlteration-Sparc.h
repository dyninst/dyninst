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

// $Id: LocalAlteration-Sparc.h,v 1.4 2000/07/28 17:20:41 pcroth Exp $

#ifndef __LocalAlteration_SPARC_H__
#define __LocalAlteration_SPARC_H__


#include "dyninstAPI/src/LocalAlteration.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/FunctionExpansionRecord.h"
#include "common/h/Vector.h"
#include "dyninstAPI/src/dyninst.h"
#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/util.h"

// Subclass of LocalAlteration which contains SPARC specific 
//  information....
class SparcLocalAlteration : public LocalAlteration {
 protected:
    // base address at which module is loaded.  0 for statically linked
    //  code, probably non-zero for shared library.
    Address baseAddress;

 public:
    // constructor for SPARC Peephole Alteration.
    //  extra stuff : nBaseAddress (sets baseAddress data member)....
    SparcLocalAlteration(pd_Function *f, int beginning_offset, \
	    int ending_offset, Address nBaseAddress);
};


//
// TAIL-CALL OPTIMIZATION BASE PEEPHOLE ALTERATION (abstract)....
//
// Peephole alteration to unwind tail-call optimization.
// 
class TailCallOptimization : public SparcLocalAlteration {
  public:

    // Constructor - same as SparcLocalAlteration????
    TailCallOptimization(pd_Function *f, int beginning_offset, \
	int ending_offset, Address nBaseAddress);
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
	int ending_offset, Address nBaseAddress, instruction callInsn);
     // update branch targets in/around footprint....
     virtual bool UpdateExpansions(FunctionExpansionRecord *fer);
     // update inst point locations in/around footprint....
     virtual bool UpdateInstPoints(FunctionExpansionRecord *ips);
     virtual bool RewriteFootprint(Address &adr, Address newBaseAdr, \
	Address &newAdr, instruction oldInstr[], instruction newInstr[]);
};

// used to unwind tail-call optimizations which match the pattern:
//  jmp 
//  nop
//
class JmpNopTailCallOptimization : public TailCallOptimization {
 public:
     JmpNopTailCallOptimization(pd_Function *f, int beginning_offset, \
	int ending_offset, Address nBaseAddress);
     // update branch targets and inst point locations in/around footprint....
     virtual bool UpdateExpansions(FunctionExpansionRecord *fer);
     // update inst point locations in/around footprint....
     virtual bool UpdateInstPoints(FunctionExpansionRecord *ips);
     virtual bool RewriteFootprint(Address &adr, Address newBaseAdr, \
	Address &newAdr, instruction oldInstr[], instruction newInstr[]);
};


//
// ..............NOP EXPANSIONS............
//

// Stick a bunch of no-ops into code to expand it so that it can be safely
//  instrumented....
// Nops are stuck in BEFORE the instruction specified by beginning_offset....
// extra fields (beyond SparcLocalAlteration)....
//  size (# BYTES of nop instructions which should be added)
//
// Notes:
//  beginning_offset should be set = ending_offset.
//
class NOPExpansion : public SparcLocalAlteration {
 protected:
    // size (in bytes) of nop region to be added (should translate to integer #
    //  of instructions)....
    int sizeNopRegion;
 public:
    // constructor same as SparcLocalAlteration except for extra field 
    //  specifying how many BYTES of nop....
    // NOTE : as specified above, if the instruction at off
    NOPExpansion(pd_Function *f, int beginning_offset, int ending_offset, \
	Address nBaseAddress, int size);

    virtual bool UpdateExpansions(FunctionExpansionRecord *fer);
    virtual bool UpdateInstPoints(FunctionExpansionRecord *ips);
    virtual bool RewriteFootprint(Address &adr, Address newBaseAdr, \
	Address &newAdr, instruction oldInstr[], instruction newInstr[]);
};

//class SecondInsnCall : public SparcLocalAlteration {
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
class SetO7 : public SparcLocalAlteration {
 public:
    // Set07 constructor - same as SparcLocalAlteration....
    SetO7(pd_Function *f, int beginning_offset, \
	    int ending_offset, Address nBaseAddress);
    virtual bool UpdateExpansions(FunctionExpansionRecord *fer);
    virtual bool UpdateInstPoints(FunctionExpansionRecord *ips);
    virtual bool RewriteFootprint(Address &adr, Address newBaseAdr, \
	Address &newAdr, instruction oldInstr[], instruction newInstr[]);
};



/*  #ifndef __LocalAlteration_SPARC_H__  */
#endif
