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

// $Id: LocalAlteration-x86.h,v 1.1 2001/02/02 21:26:55 gurari Exp $

#ifndef __LocalAlteration_H__
#define __LocalAlteration_H__


#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/FunctionExpansionRecord-x86.h"
#include "common/h/Vector.h"
#include "dyninstAPI/src/dyninst.h"
#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/util.h"

class pd_Function;

class LocalAlteration {
 protected:
    // function to which alteration is being applied....
    pd_Function *function;
   
    // Offsets into function which specify where alteration applies.
    //  The offsets refer to the ORIGINAL function (not the rewritten
    //  version) and represent BYTES of offset, as opposed to number
    //  of instructions, because architectures may have variables length
    //  instruction sets.... 
    int beginning_offset;  
    
 public:
    // constructor, make new LocalAlteration
    LocalAlteration(pd_Function *f, int offset);

    // update branches around and into footprint....
    virtual bool UpdateExpansions(FunctionExpansionRecord *fer) = 0;

    // virtual function to register changes which alteration makes to locations
    //  of inst points inside the function....
    virtual bool UpdateInstPoints(FunctionExpansionRecord *ips) = 0;
 
   // adr is address (in target address space) of 1st instruction in 
    //  footprint.  It is updated to point to address of 1st instruction
    //  after end of footprint (in origional function).
    // newAdr is address (again in target address space) where 1st 
    //  instruction in rewritten footprint will be written.  It is updated
    //  to point to address of 1st instruction after end of footprint
    //  (in rewritten function).
    // newOffs  t is offset into newInstr at which instructions should start
    //  being written....
    // newBaseAdr is address at which RELOCATED version of function should 
    //  begin....
    virtual bool RewriteFootprint(Address oldBaseAdr, Address &oldAdr, 
                                  Address newBaseAdr, Address &newAdr, 
                                  instruction oldInstr[], 
                                  instruction newInstr[], 
                                  int &oldOffset, int &newOffset,
                                  int newDisp, 
                                  unsigned &codeOffset,
                                  unsigned char *insn) = 0;

    virtual int getOffset() = 0;
    virtual int getShift() = 0;
    virtual LocalAlteration* Copy() = 0;
    virtual void updateOffset(int update) { beginning_offset += update; }
    virtual int numInstrAddedAfter() = 0;
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
class InsertNops : public LocalAlteration {
 protected: 
    int sizeNopRegion;

 public:
    // constructor same as LocalAlteration except for extra field 
    //  specifying how many BYTES of nop....
    InsertNops(pd_Function *f, int offset, int size);

    //  virtual int sizeOfNop();
    virtual int getOffset();
    virtual int getShift();
    virtual int numInstrAddedAfter();
    virtual LocalAlteration* Copy();
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
};

class ExpandInstruction : public LocalAlteration {
 private: 
    int extra_bytes;

 public:
    // constructor same as LocalAlteration except for extra field 
    //  specifying how many BYTES of nop....
    ExpandInstruction(pd_Function *f, int offset, int extra_bytes); 

    virtual int getOffset();
    virtual int getShift();
    virtual int numInstrAddedAfter();
    virtual LocalAlteration* Copy();
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
};


// Set of LocalAlterations to be applied to an function.
//  By definition, none of these wrappers have overlapping footprints,
//  and (because they have contiguous footprints), none completely
//  encloses any other.
//
// Provides the following functionality:
//  records the set of (non-overlapping peephole) alterations to be
//  applied to a function and where they are to be applied....

class LocalAlterationSet {
  protected:
    // list of alterations, sorted by offset....
    vector<LocalAlteration*> alterations;

    // compact representation for set of size changes caused by rewriting
    //  function....
    FunctionExpansionRecord fer;

    // compact representation for changes made to locations of inst points in
    //  function caused by rewriting function....
    FunctionExpansionRecord ips;

    // function to which set of alterations applies....
    pd_Function *func;

    // used for iterator....
    int iterIdx;

    // keep track of whether work has been done to order alterations....
    bool ordered;

    void Order();
  public:
    // add alteration a.  a's data members (e.g. func, offsetBegins, 
    //  offsetEnds) should already be filled in.
    // Does NO copying.  Therefore, a should NOT disappear under
    // AlterationSet disappears....
    void AddAlteration(LocalAlteration *a);
 
    void UpdateAlteration(int update);

    // ....CONSTRUCTOR....
    LocalAlterationSet(pd_Function *f);

    // flush the LocalAlterations out of alterations....
    void Flush();

    // delete the LocalAlterations in alterations 
    void DeleteAlterations();

    // calls FLUSH....
    ~LocalAlterationSet();

    // TOTAL size change (IN BYTES) resulting from application of all alterations
    //  in set....
    int sizeChange();

    //
    // Pass through to fer/ips....
    //
    void Collapse() {fer.Collapse(); ips.Collapse();}
    int getShift(int f) {return fer.GetShift(f);}
    int getInstPointShift(int offset) {return ips.GetShift(offset);}
 
    int numInstrAddedAfter(int offset);
    

    //  
    // ITERATOR CODE
    //  used to iterate over list of known alterations (in order of offset....)....
    //
    // reset iterator (to 1st alteration)....
    void iterReset();
    // return next alteration in iterator....
    LocalAlteration *iterNext();

};

/*  #ifndef __LocalAlteration_H__  */
#endif




