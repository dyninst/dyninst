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

// $Id: inst.h,v 1.59 2000/10/17 17:42:18 schendel Exp $

#ifndef INST_HDR
#define INST_HDR

#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/String.h"
#include "ast.h" // enum opCode now defined here.
#include "common/h/Types.h"
#include "common/h/Time.h"
#ifndef BPATCH_LIBRARY
#include "pdutil/h/pdSample.h"
#endif

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

class instPoint;
class instInstance;
class trampTemplate;
class process;
class pd_Function;
class metricDefinitionNode;

typedef enum { callNoArgs, callRecordType, callFullArgs } callOptions;
typedef enum { callPreInsn, callPostInsn } callWhen;
typedef enum { orderFirstAtPoint, orderLastAtPoint } callOrder;

extern void deleteInst(instInstance *old, const vector<Address> &pointsToCheck);
extern vector<Address> getAllTrampsAtPoint(instInstance *);

class AstNode;
class returnInstance;

/*
 * Insert instrumentation at the specified codeLocation.
 * TODO: make these methods of class process
 */
instInstance *addInstFunc(process *proc,
			  instPoint *&location,
			  AstNode *&ast, // ast could change (sysFlag stuff)
			  callWhen when,
			  callOrder order,
			  bool noCost,
			  bool trampRecursionDesired);

instInstance *addInstFunc(process *proc,
			  instPoint *&location,
			  AstNode *&ast, // ast could change (sysFlag stuff)
			  callWhen when,
			  callOrder order,
			  bool noCost,
			  returnInstance *&retInstance,
			  bool trampRecursionDesired);


/* Utility functions */

void getAllInstInstancesForProcess(const process *,
				   vector<instInstance*> &);

instPoint * findInstPointFromAddress(const process *, Address);
instInstance * findMiniTramps( const instPoint * );
trampTemplate * findBaseTramp( const instPoint *, const process * );

// findAddressInFuncsAndTramps: returns the function which contains
// this address.  This checks the a.out image and shared object images
// for this function, as well as checking base- and mini-tramps which 
// correspond to this function.  If the address was in a tramp, the 
// trampTemplate is returned as well.
pd_Function *findAddressInFuncsAndTramps(process *, Address,
					 instPoint *&,
					 trampTemplate *&,
					 instInstance *&);



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
pd_Function *getFunction(instPoint *point);

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
  instMapping(const string f, const string i, const int w, 
	      callWhen wn, callOrder o, AstNode *a=NULL)
    : func(f), inst(i), where(w), when(wn), order(o)  {
    if(a) args += assignAst(a);
  }

  instMapping(const string f, const string i, const int w, AstNode *a=NULL)
    : func(f), inst(i), where(w), when(callPreInsn), order(orderLastAtPoint)  {
    if(a) args += assignAst(a);
  }

  instMapping(const string f, const string i, const int w, 
	      vector<AstNode*> &aList) : func(f), inst(i), where(w),
	      when(callPreInsn), order(orderLastAtPoint) {
    for(unsigned u=0; u < aList.size(); u++) {
      if(aList[u]) args += assignAst(aList[u]);
    }
  };

  ~instMapping() {
    // an AstNode has referenceCount = 1 when first created, 
    // we perform removeAst when we installInitialRequests in process.C
    for(unsigned i=0; i < args.size(); i++) {
      if(args[i]) removeAst(args[i]);
    }
    args.resize(0);
  }

public:
  string func;                 /* function to instrument */
  string inst;                 /* inst. function to place at func */
  int where;                   /* FUNC_ENTRY, FUNC_EXIT, FUNC_CALL */
  callWhen when;               /* callPreInsn, callPostInsn */
  callOrder order;             /* orderFirstAtPoint, orderLastAtPoint */
  vector<AstNode *> args;      /* what to pass as arg0 ... n */
  // AstNode *arg;            /* what to pass as arg0 */
};

/*
 * get information about the cost of primitives.
 *
 */
void initPrimitiveCost();
void initDefaultPointFrequencyTable();

//
// Return the expected runtime of the passed function in instruction times.
//
unsigned getPrimitiveCost(const string &name);

/*
 * Generate an instruction.
 * Previously this was handled by the polymorphic "emit" function, which
 * took a variety of argument types and variously returned either an
 * Address or a Register or nothing of value.  The following family of
 * functions replace "emit" with more strongly typed versions.
 */

// for operations requiring an Address to be returned
// (e.g., ifOp/branchOp, trampPreambleOp/trampTrailerOp)
Address  emitA(opCode op, Register src1, Register src2, Register dst, 
                char *insn, Address &base, bool noCost);

// for operations requiring a Register to be returned
// (e.g., getRetValOp, getParamOp, getSysRetValOp, getSysParamOp)
Register emitR(opCode op, Register src1, Register src2, Register dst, 
                char *insn, Address &base, bool noCost);

// for general arithmetic and logic operations which return nothing
void     emitV(opCode op, Register src1, Register src2, Register dst, 
                char *insn, Address &base, bool noCost, int size = 4);

// for loadOp and loadConstOp (reading from an Address)
void     emitVload(opCode op, Address src1, Register src2, Register dst, 
                char *insn, Address &base, bool noCost, int size = 4);

// for storeOp (writing to an Address)
void     emitVstore(opCode op, Register src1, Register src2, Address dst, 
                char *insn, Address &base, bool noCost, int size = 4);

// for updateCostOp
void     emitVupdate(opCode op, RegValue src1, Register src2, Address dst, 
                char *insn, Address &base, bool noCost);

// and the retyped original emitImm companion
void     emitImm(opCode op, Register src, RegValue src2imm, Register dst, 
                char *insn, Address &base, bool noCost);

int getInsnCost(opCode t);

/*
 * get the requested parameter into a register.
 *
 */
Register getParameter(Register dest, int param);

class point {
 public:
  point() { inst = NULL; }
  instInstance *inst;
};

extern string getProcessStatus(const process *p);

// TODO - what about mangled names ?
// expects the symbol name advanced past the underscore
extern unsigned findTags(const string funcName);

#ifndef BPATCH_LIBRARY
extern pdSample computePauseTimeMetric(const metricDefinitionNode *);

extern pdSample computeStackwalkTimeMetric(const metricDefinitionNode *);
#endif

extern void initLibraryFunctions();

extern Address getMaxBranch();

// find these internal functions before finding any other functions
// extern dictionary_hash<string, unsigned> tagDict;
extern dictionary_hash <string, unsigned> primitiveCosts;

void cleanInstFromActivePoints(process *proc);

#endif
