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

/*
 * instP.h - interface between inst and the arch specific inst functions.
 *
 * $Log: instP.h,v $
 * Revision 1.16  1996/09/13 21:41:58  mjrg
 * Implemented opcode ReturnVal for ast's to get the return value of functions.
 * Added missing calls to free registers in Ast.generateCode and emitFuncCall.
 * Removed architecture dependencies from inst.C.
 * Changed code to allow base tramps of variable size.
 *
 * Revision 1.15  1996/09/12 15:08:25  naim
 * This commit move all saves and restores from the mini-tramps to the base
 * tramp. It also add jumps to skip instrumentation in the base-tramp when
 * it isn't required - naim
 *
 * Revision 1.14  1996/09/05 16:31:35  lzheng
 * Move the defination of BREAK_POINT_INSN to the machine dependent file
 *
 * Revision 1.13  1996/08/20 19:05:41  lzheng
 * Implementation of moving multiple instructions sequence and splitting
 * the instrumentation into two phases.
 *
 * Revision 1.12  1996/08/16 21:19:09  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.11  1996/04/26 19:49:21  lzheng
 * remove a #ifdef for hpux
 *
 * Revision 1.10  1996/04/08 21:24:08  lzheng
 * added generateToBranch, an HP-specific routine
 *
 * Revision 1.9  1995/08/24 15:04:10  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.8  1995/02/16  08:53:36  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.7  1995/02/16  08:33:33  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.6  1994/11/02  11:09:27  markc
 * Changed PCptrace prototype.
 *
 * Revision 1.5  1994/10/13  07:24:47  krisna
 * solaris porting and updates
 *
 * Revision 1.4  1994/09/22  02:01:19  markc
 * change instInstanceRec struct to a class
 * change signature to PCptrace
 * changed #defines for cust PTRACE_
 *
 */

/*
 * Functions that need to be provided by the inst-arch file.
 *
 */

class trampTemplate {
 public:
    int size;
    void *trampTemp;		/* template of code to execute */
    unsigned baseAddr;          /* the base address of this tramp */
    /* used only in base tramp */
    int globalPreOffset;
    int globalPostOffset;
    int localPreOffset;
    int localPostOffset;

    int localPreReturnOffset;     /* return offset for local pre tramps */
    int localPostReturnOffset;      /* offset of the return instruction */

    int returnInsOffset;
    int skipPreInsOffset;
    int skipPostInsOffset;
    int emulateInsOffset;
};

extern trampTemplate baseTemplate;

class instInstance {
 public:
     instInstance() {
       proc = NULL; location = NULL; trampBase=0;
       returnAddr = 0; baseInstance = NULL; next = NULL;
       prev = NULL; nextAtPoint = NULL; prevAtPoint = NULL; cost=0;

     }
     process *proc;             /* process this inst is for */
     callWhen when;		/* call before or after instruction */
     instPoint *location;       /* where we put the code */
     unsigned trampBase;             /* base of code */
     unsigned returnAddr;            /* address of the return from tramp insn */
     trampTemplate *baseInstance;  /* the base trampoline instance */
     instInstance *next;        /* linked list of installed instances */
     instInstance *prev;        /* linked list of prev. instance */
     instInstance *nextAtPoint; /* next in same addr space at point */
     instInstance *prevAtPoint; /* next in same addr space at point */
     int cost;			/* cost in cycles of this inst req. */
};

class returnInstance {
  public:
    returnInstance() {
	addr_ = 0;
    }

    returnInstance(instruction *instSeq, int seqSize, Address addr, int size) 
	:instructionSeq(instSeq), instSeqSize(seqSize), addr_(addr), size_(size)
	    {};

    bool checkReturnInstance(const Address adr);
    void installReturnInstance(process *proc);
    void addToReturnWaitingList(instruction insn, Address pc);

  private: 
    instruction *instructionSeq;     /* instructions to be installed */
    int instSeqSize;
    Address addr_;                  /* beginning address */
    int size_;                      
}; 


class instWaitingList {
  public:
    instruction *instructionSeq;
    int instSeqSize;
    Address addr_;

    instruction relocatedInstruction;
    Address relocatedInsnAddr;

    instWaitingList *next;
};

extern List<instWaitingList *> instWList;

trampTemplate *findAndInstallBaseTramp(process *proc, instPoint *location,
				 returnInstance *&retInstance);
void installTramp(instInstance *inst, char *code, int codeSize);
void modifyTrampReturn(process*, int returnAddr, int newReturnTo);
void generateReturn(process *proc, int currAddr, instPoint *location);
void generateEmulationInsn(process *proc, int addr, instPoint *location);
void generateNoOp(process *proc, int addr);
void generateBreakPoint(instruction &insn);

void initTramps();
void generateBranch(process *proc, unsigned fromAddr, unsigned newAddr);
void removeTramp(process *proc, instPoint *location);

int flushPtrace();


/*inline bool returnInstance::checkReturnInstance(const Address adr) {
    if ((adr > addr_) && ( adr <= addr_+size_))
	return false;
    else 
	return true;
}
 
inline void returnInstance::installReturnInstance(process *proc) {
    proc->writeTextSpace((caddr_t)addr_, instSeqSize, (caddr_t) instructionSeq); 
}

inline void returnInstance::addToReturnWaitingList(instruction insn, Address pc) {

    instWaitingList *instW = new instWaitingList; 
    
    instW->instructionSeq = instructionSeq;
    instW->instSeqSize = instSeqSize;
    instW->addr_ = addr_;

    instW->relocatedInstruction = insn;
    instW->relocatedInsnAddr = pc;

    instWList.add(instW, (void *)pc);
}*/





