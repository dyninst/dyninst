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

#ifndef INST_HDR
#define INST_HDR

/*
 * inst.h - This file contains the insrumentation functions that are provied
 *   by the instrumentation layer.
 *
 * $Log: inst.h,v $
 * Revision 1.25  1996/08/20 19:13:09  lzheng
 * Implementation of moving multiple instructions sequence and
 * splitting the instrumentation into two phases
 *
 * Revision 1.24  1996/08/16 21:19:07  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.23  1996/08/12 16:27:09  mjrg
 * Code cleanup: removed cm5 kludges and some unused code
 *
 * Revision 1.22  1996/05/08 23:54:49  mjrg
 * added support for handling fork and exec by an application
 * use /proc instead of ptrace on solaris
 * removed warnings
 *
 * Revision 1.21  1996/04/29 03:36:40  tamches
 * declaration of computePauseTimeMetric now has a param
 *
 * Revision 1.20  1996/04/26 19:52:11  lzheng
 * Moved prototype of emitFuncCall to ast.h because the function arguments
 * used is changed.
 *
 * Revision 1.19  1996/04/10 17:58:24  lzheng
 * Two opCodes( loadMemOp, storeMemOp) are added for HP and the prototype of
 * emitFuncCall for HP is changed to support multiple arguments.
 *
 * Revision 1.18  1996/04/08 21:23:40  lzheng
 * HP-specific version of emitFuncCall prototype
 *
 * Revision 1.17  1996/04/03 14:27:41  naim
 * Implementation of deallocation of instrumentation for solaris and sunos - naim
 *
 * Revision 1.16  1996/03/25  20:21:08  tamches
 * the reduce-mem-leaks-in-paradynd commit
 *
 */

#include "rtinst/h/trace.h"
#include "process.h"

class instInstance;
class process;
class instPoint;
class pdFunction;

typedef enum { callNoArgs, callRecordType, callFullArgs } callOptions;
typedef enum { callPreInsn, callPostInsn } callWhen;
typedef enum { orderFirstAtPoint, orderLastAtPoint } callOrder;

class intCounterHandle {
 public:
    intCounterHandle() {
      proc = NULL; sampler = NULL; counterPtr = NULL;
    }
    process *proc;
    intCounter *counterPtr;		/* NOT in our address space !!!! */
    intCounter data;			/* a copy in our address space */
    instInstance *sampler;		/* function to sample value */
};

class timerHandle {
 public:
    timerHandle() {
      proc = NULL; timerPtr = NULL; sampler = NULL;
    }
    process *proc;
    tTimer *timerPtr;			/* NOT in our address space !!!! */
    tTimer data;			/* a copy in our address space !!!! */
    instInstance *sampler;		/* function to sample value */
};

class AstNode;
class returnInstance;

/*
 * Insert instrumentation a the specificed codeLocation.
 *
 */
instInstance *addInstFunc(process *proc,
			  instPoint *location,
			  AstNode &ast, 
			  callWhen when,
			  callOrder order);

instInstance *addInstFunc(process *proc,
			  instPoint *location,
			  AstNode &ast, 
			  callWhen when,
			  callOrder order,
			  returnInstance *&retInstance);

void copyInstInstances(process *parent, process *child,
	    dictionary_hash<instInstance *, instInstance *> &instInstanceMapping);


float getPointFrequency(instPoint *point);
int getPointCost(process *proc, instPoint *point);

void deleteInst(instInstance*, vector<unsigned> pointsToCheck);

intCounterHandle *createIntCounter(process *proc, int value, bool report);
int getIntCounterValue(intCounterHandle*);
void freeIntCounter(intCounterHandle*, vector<unsigVecType>);
void addToIntCounter(intCounterHandle*, int amount);
intCounterHandle *dupIntCounter(intCounterHandle *parent, process *child, int value);

floatCounter *createFloatCounter(int pid);
float getCounterValue(floatCounter*);
void freeFloatCounter(floatCounter*);
void addToFloatCounter(floatCounter*, float amount);

/*
 * Create a triggered timer.  A triggered timer is one that only will start/stop
 *   when the value of the trigger counter is positive.
 *
 *   type indicates if the timer is based on wall or process (virtual) time.
 *
 *   trigger indicates the trigger counter to use.  A null trigger implies no
 *     trigger is required.
 *
 *   pid is the thread that the timer will run in.
 */
timerHandle *createTimer(process*proc,timerType type, bool report);

/*
 * return the current value of the timer.
 *
 */
float getTimerValue(timerHandle *timer);

/*
 * dispose of a timer. 
 *
 */
void freeTimer(timerHandle*, vector<unsigVecType>);
timerHandle *dupTimer(timerHandle *parent, process *child);

/*
 * Test if the inst point is for a call to a tahracked function (as opposed to
 *   a function that has been either implictly or explictly suppressed from
 *   instrumentation (i.e. library functions or user suppression of user 
 *   functions).
 */
int callsTrackedFuncP(instPoint *);

/* return the function asociated with a point. */
pdFunction *getFunction(instPoint *point);

/*
 * struct to define a list of inst requests 
 *
 */
#define FUNC_ENTRY      0x1             /* entry to the function */
#define FUNC_EXIT       0x2             /* exit from function */
#define FUNC_CALL       0x4             /* subroutines called from func */
#define FUNC_ARG  	0x8             /* use arg as argument */

class instMapping {
 public:
  instMapping(const string f, const string i, const int w, AstNode *a=NULL)
	: func(f), inst(i), where(w), arg(a) { }

  string func;         /* function to instrument */
  string inst;         /* inst. function to place at func */
  int where;          /* FUNC_ENTRY, FUNC_EXIT, FUNC_CALL */
  AstNode *arg;	      /* what to pass as arg0 */
};

/*
 * Install a list of inst requests in a process.
 *
 */
void installDefaultInst(process *proc, vector<instMapping*>& initialReq);

/*
 * get information about the cost of pimitives.
 *
 */
void initPrimitiveCost();
void initDefaultPointFrequencyTable();

//
// Return the expected runtime of the passed function in instruction
//   times.
//
unsigned getPrimitiveCost(const string name);

// a register.
typedef int reg;

typedef enum { plusOp,
               minusOp,
               timesOp,
               divOp,
               lessOp,
               leOp,
               greaterOp,
               geOp,
               eqOp,
               neOp,
               loadOp,           
               loadConstOp,
               storeOp,
	       storeMemOp,
	       loadMemOp,
               ifOp,
	       callOp,
	       trampPreamble,
	       trampTrailer,
	       noOp,
	       orOp,
	       andOp,
	       getParamOp,
	       getSysParamOp,	   
	       saveRegOp} opCode;

/*
 * Generate an instruction.
 *
 */
unsigned emit(opCode op, reg src1, reg src2, reg dest, char *insn, unsigned &base);
//unsigned emitFuncCall(opCode op, registerSpace *rs, char *i,unsigned &base, 
//		      vector<AstNode> operands, string func, process *proc);

int getInsnCost(opCode t);

/*
 * get the requested parameter into a register.
 *
 */
reg getParameter(reg dest, int param);

#define INFERIOR_HEAP_BASE     "DYNINSTdata"
#define UINFERIOR_HEAP_BASE    "_DYNINSTdata"
#define GLOBAL_HEAP_BASE        "DYNINSTglobalData"
#define U_GLOBAL_HEAP_BASE      "_DYNINSTglobalData"

class point {
 public:
  point() { inst = NULL; }
  instInstance *inst;
};

extern string getProcessStatus(const process *p);

// TODO - what about mangled names ?
// expects the symbol name advanced past the underscore
extern unsigned findTags(const string funcName);

extern float computePauseTimeMetric(const metricDefinitionNode *);

extern process *nodePseudoProcess;

extern void initLibraryFunctions();

extern unsigned getMaxBranch();

// find these internal functions before finding any other functions
// extern dictionary_hash<string, unsigned> tagDict;
extern dictionary_hash <string, unsigned> primitiveCosts;

extern process *nodePseudoProcess;

#endif
