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

// $Id: instP.h,v 1.48 2005/07/29 19:18:45 bernat Exp $

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
//#include "dyninstAPI/src/baseTramp.h"
// minitramp data structure
//#include "dyninstAPI/src/miniTramp.h"

class baseTramp;
class miniTramp;

#if 0
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

#endif

#if 0
extern baseTramp *installMergedTramp(process *proc, 
					 instPoint *&location,
					 char * insn, Address count,
					 registerSpace * regS,
					 callWhen when,
					 returnInstance *&retInstance,
					 bool trampRecursiveDesired,
					 bool noCost,
					 bool &deferred,
					 bool allowTrap);
#endif

extern void generateReturn(process *proc, Address currAddr, instPoint *location);
extern void initTramps(bool is_multithreaded);
extern void generateBranch(unsigned char *buffer, unsigned &offset,
                           Address fromAddr, Address toAddr);
extern unsigned generateAndWriteBranch(process *proc, Address fromAddr, Address toAddr, unsigned fillSize = 0);
extern void removeTramp(process *proc, instPoint *location);
extern int flushPtrace();

extern unsigned saveGPRegister(char *baseInsn, Address &base, Register reg);
extern unsigned saveRestoreRegistersInBaseTramp(process *proc, baseTramp * bt, 
						registerSpace * rs);

extern void generateNoopField(unsigned size,
                              unsigned char *buffer);

#endif
