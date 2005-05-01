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

// $Id: inst.h,v 1.87 2005/05/01 23:27:32 rutar Exp $

#ifndef INST_HDR
#define INST_HDR

#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/String.h"
#include "ast.h" // enum opCode now defined here.
#include "common/h/Types.h"

#ifdef BPATCH_LIBRARY
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
#endif

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

class instPoint;
class miniTrampHandle;
class trampTemplate;
class process;
class int_function;
class metricFocusNode;
class miniTramps_list;

typedef enum { callNoArgs, callRecordType, callFullArgs } callOptions;
typedef enum { callPreInsn, callPostInsn } callWhen;
typedef enum { orderFirstAtPoint, orderLastAtPoint } callOrder;

extern bool deleteInst(process *proc, miniTrampHandle *&mtHandle);

extern pdvector<Address> getTrampAddressesAtPoint(process *proc, 
                                                  const instPoint *loc,
                                                  callWhen when);

class AstNode;
class returnInstance;

typedef enum loadMiniTramp_result { success_res, failure_res, deferred_res} loadMiniTramp_result;

/*
 * Insert instrumentation at the specified codeLocation.
 * TODO: make these methods of class process
 */
loadMiniTramp_result addInstFunc(process *proc,
                                 miniTrampHandle *&mtHandle, 
                                 instPoint *&location,
                                 AstNode *&ast, // ast could change (sysFlag stuff)
                                 callWhen when,
                                 callOrder order,
                                 bool noCost,
                                 bool trampRecursionDesired,
                                 bool allowTrap);

// writes to (*mtInfo)
loadMiniTramp_result loadMergedTramp(miniTrampHandle *&mtHandle,
                                   process *proc, 
                                   instPoint *&location,
                                   AstNode *&ast, // the ast could be changed 
                                   callWhen when, callOrder order, bool noCost,
                                   returnInstance *&retInstance,
                                   bool trampRecursiveDesired = false,
                                   bool allowTramp = true);

// writes to (*mtInfo)
loadMiniTramp_result loadMiniTramp(miniTrampHandle *&mtHandle,
                                   process *proc, 
                                   instPoint *&location,
                                   AstNode *&ast, // the ast could be changed 
                                   callWhen when, callOrder order, bool noCost,
                                   returnInstance *&retInstance,
                                   bool trampRecursiveDesired = false,
                                   bool allowTramp = true);

void hookupMiniTramp(process *proc,
                     miniTrampHandle *&mtHandle,
                     callOrder order);

/* Utility functions */

bool getInheritedMiniTramp(const miniTrampHandle *parentMT, 
                           miniTrampHandle *&childMT,
                           process *childProc);

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
int_function *getFunction(instPoint *point);

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
  // instMapping(const pdstring f, const pdstring i, const int w, AstNode *a=NULL)
  //   : func(f), inst(i), where(w) { arg = assignAst(a); };
  // ~instMapping() { removeAst(arg); };
  instMapping(const pdstring f, const pdstring i, const int w, 
	      callWhen wn, callOrder o, AstNode *a=NULL)
     : func(f), inst(i), where(w), when(wn), order(o), useTrampGuard(true),
     mt_only(false), allow_trap(false) {
    if(a) args.push_back(assignAst(a));
  }

  instMapping(const pdstring f, const pdstring i, const int w, AstNode *a=NULL)
     : func(f), inst(i), where(w), when(callPreInsn), order(orderLastAtPoint),
       useTrampGuard(true), mt_only(false), allow_trap(false) {
    if(a) args.push_back(assignAst(a));
  }

  instMapping(const pdstring f, const pdstring i, const int w, 
              pdvector<AstNode*> &aList) :
     func(f), inst(i), where(w), when(callPreInsn), order(orderLastAtPoint),
     useTrampGuard(true), mt_only(false), allow_trap(false) {
    for(unsigned u=0; u < aList.size(); u++) {
      if(aList[u]) args.push_back(assignAst(aList[u]));
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
  void dontUseTrampGuard() { useTrampGuard = false; }
  void markAs_MTonly() { mt_only = true; }
  void canUseTrap(bool t) { allow_trap = t; }
  bool is_MTonly() { return mt_only; }

  pdstring func;                 /* function to instrument */
  pdstring inst;                 /* inst. function to place at func */
  int where;                   /* FUNC_ENTRY, FUNC_EXIT, FUNC_CALL */
  callWhen when;               /* callPreInsn, callPostInsn */
  callOrder order;             /* orderFirstAtPoint, orderLastAtPoint */
  pdvector<AstNode *> args;      /* what to pass as arg0 ... n */
  bool useTrampGuard;
  bool mt_only;
  bool allow_trap;
  pdvector<miniTrampHandle *> mtHandles;
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
unsigned getPrimitiveCost(const pdstring &name);

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
               char *insn, Address &base, bool noCost, 
               const instPoint *location, bool for_multithreaded);

// for general arithmetic and logic operations which return nothing
void     emitV(opCode op, Register src1, Register src2, Register dst, 
                char *insn, Address &base, bool noCost, int size = 4,
                const instPoint * location = NULL, process * proc = NULL, registerSpace * rs = NULL);

// for loadOp and loadConstOp (reading from an Address)
void     emitVload(opCode op, Address src1, Register src2, Register dst, 
                char *insn, Address &base, bool noCost, int size = 4, 
                const instPoint * location = NULL, process * proc = NULL, registerSpace * rs = NULL );

// for storeOp (writing to an Address)
void     emitVstore(opCode op, Register src1, Register src2, Address dst, 
                char *insn, Address &base, bool noCost, int size = 4, 
                const instPoint * location = NULL, process * proc = NULL, registerSpace * rs = NULL );

// for updateCostOp
void     emitVupdate(opCode op, RegValue src1, Register src2, Address dst, 
                char *insn, Address &base, bool noCost);

// and the retyped original emitImm companion
void     emitImm(opCode op, Register src, RegValue src2imm, Register dst, 
                char *insn, Address &base, bool noCost);

#ifdef BPATCH_LIBRARY
void emitJmpMC(int condition, int offset, char* baseInsn, Address &base);

void emitASload(BPatch_addrSpec_NP as, Register dest, char* baseInsn,
		Address &base, bool noCost);

void emitCSload(BPatch_addrSpec_NP as, Register dest, char* baseInsn,
		Address &base, bool noCost);
#endif

// VG(11/06/01): moved here and added location
Register emitFuncCall(opCode op, registerSpace *rs, char *i, Address &base, 
		      const pdvector<AstNode *> &operands, 
		      process *proc, bool noCost, 
		      Address callee_addr,
		      const pdvector<AstNode *> &ifForks, // control-flow path
		      const instPoint *location = NULL);

int getInsnCost(opCode t);

/*
 * get the requested parameter into a register.
 *
 */
Register getParameter(Register dest, int param);

extern pdstring getProcessStatus(const process *p);

// TODO - what about mangled names ?
// expects the symbol name advanced past the underscore
extern unsigned findTags(const pdstring funcName);

extern void initLibraryFunctions();

extern Address getMaxBranch();

// find these internal functions before finding any other functions
// extern dictionary_hash<pdstring, unsigned> tagDict;
extern dictionary_hash <pdstring, unsigned> primitiveCosts;

#endif
