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

#include "util/h/Vector.h"
#include "util/h/Dictionary.h"
#include "util/h/String.h"

class instInstance;
class process;
class instPoint;
class pdFunction;
class metricDefinitionNode;

typedef enum { callNoArgs, callRecordType, callFullArgs } callOptions;
typedef enum { callPreInsn, callPostInsn } callWhen;
typedef enum { orderFirstAtPoint, orderLastAtPoint } callOrder;

extern void deleteInst(instInstance *old, const vector<unsigned> &pointsToCheck);
   // in inst.C
extern vector<unsigned> getAllTrampsAtPoint(instInstance *);

class AstNode;
class returnInstance;

/*
 * Insert instrumentation a the specificed codeLocation.
 * TODO: make these methods of class process
 */
instInstance *addInstFunc(process *proc,
			  instPoint *&location,
			  AstNode *&ast, // ast could change (sysFlag stuff)
			  callWhen when,
			  callOrder order,
			  bool noCost);

instInstance *addInstFunc(process *proc,
			  instPoint *&location,
			  AstNode *&ast, // ast could change (sysFlag stuff)
			  callWhen when,
			  callOrder order,
			  bool noCost,
			  returnInstance *&retInstance);

void getAllInstInstancesForProcess(const process *,
				   vector<instInstance*> &);

void copyInstInstances(const process *parent, const process *child,
	    dictionary_hash<instInstance *, instInstance *> &instInstanceMapping);
   // writes to "instInstanceMapping[]"


float getPointFrequency(instPoint *point);
int getPointCost(process *proc, const instPoint *point);

/*
 * Test if the inst point is for a call to a tracked function (as opposed to
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

extern AstNode *assignAst(AstNode *);
extern void removeAst(AstNode *&);

class instMapping {
public:
  // instMapping(const string f, const string i, const int w, AstNode *a=NULL)
  //   : func(f), inst(i), where(w) { arg = assignAst(a); };
  // ~instMapping() { removeAst(arg); };

  instMapping(const string f, const string i, const int w, AstNode *a=NULL)
    : func(f), inst(i), where(w) {
    if(a) args += assignAst(a);
  }

  instMapping(const string f, const string i, const int w, 
	      vector<AstNode*> &aList) : func(f), inst(i), where(w) {
    for(unsigned i=0; i < aList.size(); i++) {
      if(aList[i]) args += assignAst(aList[i]);
    }
  };

  ~instMapping() {
    for(unsigned i=0; i < args.size(); i++) {
      if(args[i]) removeAst(args[i]);
    }
  }

public:
  string func;                 /* function to instrument */
  string inst;                 /* inst. function to place at func */
  int where;                   /* FUNC_ENTRY, FUNC_EXIT, FUNC_CALL */
  vector<AstNode *> args;      /* what to pass as arg0 ... n */
  // AstNode *arg;	       /* what to pass as arg0 */
};

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
unsigned getPrimitiveCost(const string &name);

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
	       getRetValOp,
	       getSysRetValOp,
	       getParamOp,
	       getSysParamOp,	   
	       loadIndirOp,
	       storeIndirOp,
	       saveRegOp,
	       updateCostOp } opCode;

/*
 * Generate an instruction.
 *
 */
unsigned emit(opCode op, reg src1, reg src2, reg dest, char *insn, 
              unsigned &base, bool noCost);
unsigned emitImm(opCode op, reg src1, reg src2, reg dest, char *i, 
                 unsigned &base, bool noCost);
int getInsnCost(opCode t);

/*
 * get the requested parameter into a register.
 *
 */
reg getParameter(reg dest, int param);

#define INFERIOR_HEAP_BASE     "DYNINSTdata"
#define UINFERIOR_HEAP_BASE    "_DYNINSTdata"

// The following 2 are NOT USED ANYMORE; LET'S FRY 'EM
//#define GLOBAL_HEAP_BASE        "DYNINSTglobalData"
//#define U_GLOBAL_HEAP_BASE      "_DYNINSTglobalData"

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

extern void initLibraryFunctions();

extern unsigned getMaxBranch();

// find these internal functions before finding any other functions
// extern dictionary_hash<string, unsigned> tagDict;
extern dictionary_hash <string, unsigned> primitiveCosts;

#endif
