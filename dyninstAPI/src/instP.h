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

// $Id: instP.h,v 1.44 2004/05/21 14:14:49 legendre Exp $

#if !defined(instP_h)
#define instP_h
/*
 * Functions that need to be provided by the inst-arch file.
 *
 */

#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/arch.h"  // for type, instruction
#include "dyninstAPI/src/frame.h"

// base tramp template
#include "dyninstAPI/src/trampTemplate.h"
// minitramp data structure
#include "dyninstAPI/src/miniTrampHandle.h"
// minitramp list structure
#include "dyninstAPI/src/installed_miniTramps_list.h"

// class returnInstance: describes how to jmp to the base tramp
class returnInstance {
  public:
    returnInstance() {
	addr_ = 0;
	installed = false;
    }

    returnInstance(unsigned insnsOver, instruction *instSeq, int seqSize, Address addr, int size)
		   :insnsOverwritten(insnsOver), instructionSeq(instSeq), instSeqSize(seqSize), 
		   addr_(addr), size_(size), installed(false) {};

    bool checkReturnInstance(const pdvector<pdvector<Frame> > &stackWalks);
    void installReturnInstance(process *proc);
    void addToReturnWaitingList(Address pc, process *proc);

    bool Installed() const { return installed; }
    Address addr() const { return addr_; }

    // return the number of instructions this instructionSeq is going to overwrite
    // to return true or false: if this instructionSeq will require a stack walk or not
    //   or if the instructionSeq is going to overwrite more than 1 instruction or not
    //
    // 1 for alpha and power, 2 for mips
    // could be 1, 2, 3, ... for sparc, 0 for unknown
    // 1 + insnsBefore + insnsAfter at the inst point (location) for x86
    unsigned InsnsOverwritten() const { return insnsOverwritten; }
    bool needToWalkStack() const {
      return (1 != insnsOverwritten);
    }

  private:
    unsigned insnsOverwritten;      // number of instructions that will be overwritten
                                    // when this instructionSeq is installed; 0 means unknown
    instruction *instructionSeq;    // instructions to be installed
    int instSeqSize;
    Address addr_;                  // beginning address
    int size_;
    bool installed;
}; 


class instWaitingList {
  public:
    instWaitingList(instruction *i,int s,Address a,Address pc,
		    instruction r, Address ra, process *wp){
        instructionSeq = i;
	instSeqSize = s;
	addr_ = a;
	pc_ = pc;
	relocatedInstruction = r;
	relocatedInsnAddr = ra;
	which_proc = wp;
    }
    ~instWaitingList(){} 
    void cleanUp(process *proc, Address pc);

    instruction *instructionSeq;
    int instSeqSize;
    Address addr_;
    Address pc_;
    instruction relocatedInstruction;
    Address relocatedInsnAddr;
    process *which_proc;
};

extern pdvector<instWaitingList*> instWList;

extern trampTemplate *findOrInstallBaseTramp(process *proc, 
                                             instPoint *&location,
                                             returnInstance *&retInstance,
                                             bool trampRecursiveDesired,
                                             bool noCost,
                                             bool &deferred,
                                             bool allowTrap);

extern void installTramp(miniTrampHandle *mtHandle, process *proc, char *code, 
                         int codeSize);
extern void modifyTrampReturn(process*, Address returnAddr, Address newReturnTo);
extern void generateReturn(process *proc, Address currAddr, instPoint *location);
extern void generateEmulationInsn(process *proc, Address addr, instPoint *location);
extern void generateNoOp(process *proc, Address addr);
extern void initTramps(bool is_multithreaded);
extern void generateBranch(process *proc, Address fromAddr, Address newAddr);
extern void removeTramp(process *proc, instPoint *location);
extern int flushPtrace();
extern bool deleteBaseTramp(process *, trampTemplate *);
#endif
