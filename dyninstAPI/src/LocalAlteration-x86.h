/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: LocalAlteration-x86.h,v 1.9 2005/01/21 23:44:06 bernat Exp $

#ifndef __LocalAlteration_X86_H__
#define __LocalAlteration_X86_H__


#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/LocalAlteration.h"
#include "dyninstAPI/src/FunctionExpansionRecord.h"
#include "common/h/Vector.h"
#include "dyninstAPI/src/dyninst.h"
#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/util.h"


// Expand jump or call instructions so they have larger displacements
class ExpandInstruction : public LocalAlteration {
 private: 
    int extra_bytes;

 public:
    // constructor same as LocalAlteration except for extra field 
    //  specifying how many BYTES of nop....
    ExpandInstruction(int_function *f, int offset, int extra_bytes); 

    virtual int getOffset() const;
    virtual int getShift() const;
    virtual int numInstrAddedAfter();
    virtual bool UpdateExpansions(FunctionExpansionRecord *fer);
    virtual bool UpdateInstPoints(FunctionExpansionRecord *ips);
    virtual bool RewriteFootprint(Address oldBaseAdr, Address &oldAdr, 
                                  Address newBaseAdr, Address &newAdr,
                                  instruction oldInstructions[], 
                                  instruction newInstructions[], 
                                  int &oldOffset, int &newOffset,
                                  int newDisp, 
                                  unsigned &codeOffset,
                                  unsigned char *code);
};

// Rewrite call to adr+5 to push EIP.
class PushEIP : public LocalAlteration {
 public:
    // constructor same as LocalAlteration 
    PushEIP(int_function *f, int offset); 

    virtual int getOffset() const;
    virtual int getShift() const;
    virtual int numInstrAddedAfter();
    virtual bool UpdateExpansions(FunctionExpansionRecord *fer);
    virtual bool UpdateInstPoints(FunctionExpansionRecord *ips);
    virtual bool RewriteFootprint(Address oldBaseAdr, Address &oldAdr, 
                                  Address newBaseAdr, Address &newAdr,
                                  instruction oldInstructions[], 
                                  instruction newInstructions[], 
                                  int &oldOffset, int &newOffset,
                                  int newDisp, 
                                  unsigned &codeOffset,
                                  unsigned char *code);
};

// Rewrite call to mov-ret to push EIP-ret.
class PushEIPmov : public LocalAlteration {
 private: 
    instruction instr;
    int extra_bytes;
    unsigned char dst_reg;

 public:
    // constructor same as LocalAlteration 
    PushEIPmov(int_function *f, int offset, unsigned char reg); 

    virtual int getOffset() const;
    virtual int getShift() const;
    virtual int numInstrAddedAfter();
    virtual bool UpdateExpansions(FunctionExpansionRecord *fer);
    virtual bool UpdateInstPoints(FunctionExpansionRecord *ips);
    virtual bool RewriteFootprint(Address oldBaseAdr, Address &oldAdr, 
                                  Address newBaseAdr, Address &newAdr,
                                  instruction oldInstructions[], 
                                  instruction newInstructions[], 
                                  int &oldOffset, int &newOffset,
                                  int newDisp, 
                                  unsigned &codeOffset,
                                  unsigned char *code);
};

//Handle a function that possibly falls through to the next function
class Fallthrough : public LocalAlteration {
 private:
  int branchsize_;
 public:
    // constructor same as LocalAlteration 
    Fallthrough(int_function *f, int offset); 

    virtual int getOffset() const;
    virtual int getShift() const;
    virtual int numInstrAddedAfter();
    virtual bool UpdateExpansions(FunctionExpansionRecord *fer);
    virtual bool UpdateInstPoints(FunctionExpansionRecord *ips);
    virtual bool RewriteFootprint(Address oldBaseAdr, Address &oldAdr, 
                                  Address newBaseAdr, Address &newAdr,
                                  instruction oldInstructions[], 
                                  instruction newInstructions[], 
                                  int &oldOffset, int &newOffset,
                                  int newDisp, 
                                  unsigned &codeOffset,
                                  unsigned char *code);
};


#endif




