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
    int updateCostOffset;

    int cost;			/* cost in cycles for this basetramp. */
    int costAddr;               /* address of cost in this tramp      */
    bool prevInstru;
    bool postInstru;
    int  prevBaseCost;
    int  postBaseCost;

    void updateTrampCost(process *proc, int c);
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

class image;

class returnInstance {
  public:
    returnInstance() {
	addr_ = 0;
    }

    returnInstance(instruction *instSeq, int seqSize, Address addr, int size) 
		   :instructionSeq(instSeq), instSeqSize(seqSize), 
		   addr_(addr), size_(size) {};

    bool checkReturnInstance(const vector<Address> adr, u_int &index);
    void installReturnInstance(process *proc);
    void addToReturnWaitingList(Address pc, process *proc);

  private: 
    instruction *instructionSeq;     /* instructions to be installed */
    int instSeqSize;
    Address addr_;                  /* beginning address */
    int size_;                      
}; 


class instWaitingList {
  public:
    instWaitingList(instruction *i,int s,Address a,Address pc,
		    instruction r, Address ra, process *wp){
        instructionSeq = i;
	instSeqSize = s;
	addr_ = a;
	pc_ = pc;
#if !defined (i386_unknown_solaris2_5)
	relocatedInstruction.raw = r.raw;
#endif
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

extern vector<instWaitingList*> instWList;

trampTemplate *findAndInstallBaseTramp(process *proc, 
				 const instPoint *&location,
				 returnInstance *&retInstance,
				 bool noCost);
void installTramp(instInstance *inst, char *code, int codeSize);
void modifyTrampReturn(process*, int returnAddr, int newReturnTo);
void generateReturn(process *proc, int currAddr, instPoint *location);
void generateEmulationInsn(process *proc, int addr, instPoint *location);
void generateNoOp(process *proc, int addr);
void initTramps();
void generateBranch(process *proc, unsigned fromAddr,unsigned newAddr);
void removeTramp(process *proc, instPoint *location);

int flushPtrace();






