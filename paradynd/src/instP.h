/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

/*
 * instP.h - interface between inst and the arch specific inst functions.
 *
 * $Log: instP.h,v $
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
 * Revision 1.3  1994/07/14  14:27:51  jcargill
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
class trampTemplate {
 public:
    int size;
    void *trampTemp;		/* template of code to execute */

    /* used only in base tramp */
    int globalPreOffset;
    int globalPostOffset;
    int localPreOffset;
    int localPostOffset;
};

extern trampTemplate baseTemplate;

class instInstance {
 public:
     instInstance() {
       proc = NULL; location = NULL; trampBase=0;
       returnAddr = 0; baseAddr=0; next = NULL;
       prev = NULL; nextAtPoint = NULL; prevAtPoint = NULL; cost=0;
     }
     process *proc;             /* process this inst is for */
     callWhen when;		/* call before or after instruction */
     instPoint *location;       /* where we put the code */
     unsigned trampBase;             /* base of code */
     unsigned returnAddr;            /* address of the return from tramp insn */
     unsigned baseAddr;		/* address of base instance */
     instInstance *next;        /* linked list of installed instances */
     instInstance *prev;        /* linked list of prev. instance */
     instInstance *nextAtPoint; /* next in same addr space at point */
     instInstance *prevAtPoint; /* next in same addr space at point */
     int cost;			/* cost in cycles of this inst req. */
};

unsigned findAndInstallBaseTramp(process *proc, instPoint *location);
void installTramp(instInstance *inst, char *code, int codeSize);
void modifyTrampReturn(process*, int returnAddr, int newReturnTo);
void generateReturn(process *proc, int currAddr, instPoint *location);
void generateEmulationInsn(process *proc, int addr, instPoint *location);
void generateNoOp(process *proc, int addr);

void initTramps();
void generateBranch(process *proc, unsigned fromAddr, unsigned newAddr);
void removeTramp(process *proc, instPoint *location);

int flushPtrace();

