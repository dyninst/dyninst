/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

/*
 * instP.h - interface between inst and the arch specific inst functions.
 *
 * $Log: instP.h,v $
 * Revision 1.3  1994/07/14 14:27:51  jcargill
 * Added a couple of new ptrace requests for CM5 nodes
 *
 * Revision 1.2  1994/07/05  03:26:07  hollings
 * observed cost model
 *
 * Revision 1.1  1994/01/27  20:31:26  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.6  1993/10/19  15:27:54  hollings
 * AST based mini-tramp code generator.
 *
 * Revision 1.5  1993/10/01  21:29:41  hollings
 * Added resource discovery and filters.
 *
 * Revision 1.4  1993/08/20  22:00:22  hollings
 * added generateNoOp.
 *
 * Revision 1.3  1993/06/22  19:00:01  hollings
 * global inst state.
 *
 * Revision 1.2  1993/06/08  20:14:34  hollings
 * state prior to bc net ptrace replacement.
 *
 * Revision 1.1  1993/03/19  22:51:05  hollings
 * Initial revision
 *
 *
 */

/*
 * Functions that need to be provided by the inst-arch file.
 *
 */
struct trampTemplateRec {
    int size;
    void *trampTemp;		/* template of code to execute */

    /* used only in base tramp */
    int globalPreOffset;
    int globalPostOffset;
    int localPreOffset;
    int localPostOffset;
};

typedef struct trampTemplateRec trampTemplate;

struct instInstanceRec {
     process *proc;             /* process this inst is for */
     callWhen when;		/* call before or after instruction */
     instPoint *location;       /* where we put the code */
     int trampBase;             /* base of code */
     int returnAddr;            /* address of the return from tramp insn */
     void *baseAddr;		/* address of base instance */
     instInstance *next;        /* linked list of installed instances */
     instInstance *prev;        /* linked list of prev. instance */
     instInstance *nextAtPoint; /* next in same addr space at point */
     instInstance *prevAtPoint; /* next in same addr space at point */
     int cost;			/* cost in cycles of this inst req. */
};

void *findAndInstallBaseTramp(process *proc, instPoint *location);
void installTramp(instInstance *inst, char *code, int codeSize);
void modifyTrampReturn(process*, int returnAddr, int newReturnTo);
void generateReturn(process *proc, int currAddr, instPoint *location);
void generateEmulationInsn(process *proc, int addr, instPoint *location);
void generateNoOp(process *proc, int addr);

void copyToProcess(process *proc, void *from, void *to, int size);
void copyFromProcess(process *proc, void *from, void *to, int size);

void initTramps();
void generateBranch(process *proc, int fromAddr, int newAddr);
void removeTramp(process *proc, instPoint *location);

/* cause a process to dump core */
void dumpCore(process *proc);

/* continue a process */
void continueProcess(process *proc);

/* pause a process */
void pauseProcess(process *proc);

/* do ptrace stuff */
int PCptrace(int request, process *proc, void *addr, int data, void *addr2);

int flushPtrace();

/* we define this to be stop a process on the spot. */
#define PTRACE_INTERRUPT	PTRACE_GETUCODE+1
#define PTRACE_STATUS		PTRACE_GETUCODE+2
#define PTRACE_SNARFBUFFER	PTRACE_GETUCODE+3
