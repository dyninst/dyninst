/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /p/paradyn/CVSROOT/core/paradynd/src/inst.C
,v 1.18 1995/08/24 15:04:05 hollings Exp $";
#endif


/*
 * inst.C - Code to install and remove inst funcs from a running process.
 *
 * $Log: inst.C,v $
 * Revision 1.20  1995/10/26 21:06:37  tamches
 * removed some warnings
 *
 * Revision 1.19  1995/09/26 20:17:48  naim
 * Adding error messages using showErrorCallback function for paradynd
 *
 * Revision 1.18  1995/08/24  15:04:05  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.17  1995/08/05  17:15:28  krisna
 * deleted redundant AND WRONG definition of ipHash
 *
 * Revision 1.16  1995/05/18  10:36:42  markc
 * removed tag dictionary
 *
 * Revision 1.15  1995/03/10  19:33:51  hollings
 * Fixed several aspects realted to the cost model:
 *     track the cost of the base tramp not just mini-tramps
 *     correctly handle inst cost greater than an imm format on sparc
 *     print starts at end of pvm apps.
 *     added option to read a file with more accurate data for predicted cost.
 *
 * Revision 1.14  1995/02/16  08:53:33  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.13  1995/02/16  08:33:30  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.12  1994/11/09  18:40:12  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.11  1994/11/02  11:08:28  markc
 * Moved redundant code to here from inst-< >.C.
 *
 * Revision 1.10  1994/09/30  19:47:07  rbi
 * Basic instrumentation for CMFortran
 *
 * Revision 1.9  1994/09/22  02:00:02  markc
 * Changed *allocs to news
 * cast stringHandles for printing
 * cast args to PCptrace
 *
 * Revision 1.8  1994/08/17  18:13:31  markc
 * Changed variable names in installDefaultInst to quiet compiler warnings.
 * Added reachedFirstBreak check to avoid stopping processes that have yet
 * to reach their initial SIGSTOP.
 *
 * Revision 1.7  1994/07/28  22:40:39  krisna
 * changed definitions/declarations of xalloc functions to conform to alloc.
 *
 * Revision 1.6  1994/07/20  23:23:36  hollings
 * added insn generated metric.
 *
 * Revision 1.5  1994/07/12  19:48:46  jcargill
 * Added warning for functions not found in initialRequests set
 *
 * Revision 1.4  1994/06/29  02:52:30  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 * Revision 1.3  1994/06/27  18:56:51  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.2  1994/03/20  01:53:08  markc
 * Added a buffer to each process structure to allow for multiple writers on the
 * traceStream.  Replaced old inst-pvm.C.  Changed addProcess to return type
 * int.
 *
 * Revision 1.1  1994/01/27  20:31:24  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.13  1993/12/15  21:02:42  hollings
 * added PVM support.
 *
 * Revision 1.12  1993/12/13  19:54:59  hollings
 * count operations.
 *
 * Revision 1.11  1993/10/19  15:27:54  hollings
 * AST based mini-tramp code generator.
 *
 * Revision 1.10  1993/10/04  21:37:10  hollings
 * re-enabled inst ordering directives.
 *
 * Revision 1.10  1993/10/04  21:37:10  hollings
 * re-enabled inst ordering directives.
 *
 * Revision 1.9  1993/10/01  21:29:41  hollings
 * Added resource discovery and filters.
 *
 * Revision 1.8  1993/08/23  23:11:19  hollings
 * fixed removing tramps to work correctly.
 *
 * Revision 1.7  1993/08/11  01:31:57  hollings
 * fixed call noargs & with args to be more general.
 *
 * Revision 1.6  1993/07/13  18:28:19  hollings
 * new include file syntax.
 *
 * Revision 1.5  1993/06/28  23:13:18  hollings
 * fixed process stopping.
 *
 * Revision 1.4  1993/06/24  16:18:06  hollings
 * global fixes.
 *
 * Revision 1.3  1993/06/22  19:00:01  hollings
 * global inst state.
 *
 * Revision 1.2  1993/06/08  20:14:34  hollings
 * state prior to bc net ptrace replacement.
 *
 * Revision 1.1  1993/03/19  22:45:45  hollings
 * Initial revision
 *
 *
 */

#include <assert.h>
#include <sys/signal.h>
#include <sys/param.h>


#include "rtinst/h/rtinst.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "ast.h"
#include "util.h"
#include "internalMetrics.h"
#include <strstream.h>
#include "stats.h"
#include "init.h"
#include "showerror.h"

#define NS_TO_SEC	1000000000.0
dictionary_hash <string, unsigned> primitiveCosts(string::hash);
// dictionary_hash<string, unsigned> tagDict(string::hash);
process *nodePseudoProcess=NULL;

int getBaseBranchAddr(process *proc, instInstance *inst)
{
    int fromAddr;

    fromAddr = (int) inst->baseAddr;
    if (inst->when == callPreInsn) {
	fromAddr += proc->aggregate ? baseTemplate.globalPreOffset :
	    baseTemplate.localPreOffset;
    } else {
	fromAddr += proc->aggregate ? baseTemplate.globalPostOffset :
	    baseTemplate.localPostOffset;
    }
    return(fromAddr);
}

void clearBaseBranch(process *proc, instInstance *inst)
{
    int addr;

    addr = (int) inst->baseAddr;
    if (inst->when == callPreInsn) {
	addr += proc->aggregate ? baseTemplate.globalPreOffset :
	    baseTemplate.localPreOffset;
    } else {
	addr += proc->aggregate ? baseTemplate.globalPostOffset :
	    baseTemplate.localPostOffset;
    }
    generateNoOp(proc, addr);
}

// implicit assumption that tramps generate to less than 64K bytes!!!
static char insn[65536];

static dictionary_hash<instPoint*, point*> activePoints(ipHash);

instInstance *addInstFunc(process *proc, instPoint *location, AstNode *ast,
    callWhen when, callOrder order)
{
    int trampCost;
    unsigned count;
    unsigned fromAddr;
    point *thePoint;
    instInstance *ret;
    instInstance *lastAtPoint;
    instInstance *firstAtPoint;

    assert(proc && location);

    initTramps();

    /* check if there are other inst points at this location. for this process
       at the same pre/post mode */
    firstAtPoint = NULL;
    lastAtPoint = NULL;

    if (!activePoints.defines(location)) {
      thePoint = new point;
      activePoints[location] = thePoint;
    } else
      thePoint = activePoints[location];

    assert(thePoint);
    for (ret= thePoint->inst; ret; ret = ret->next) {
	if ((ret->proc == proc) && (ret->when == when)) {
	    if (!ret->nextAtPoint) lastAtPoint = ret;
	    if (!ret->prevAtPoint) firstAtPoint = ret;
	}
    }

    ret = new instInstance;
    assert(ret);
    ret->proc = proc;

    // must do this before findAndInstallBaseTramp, puts the tramp in to
    // get the correct cost.
    trampCost = getPointCost(proc, location);

    /* make sure the base tramp has been installed for this point */
    ret->baseAddr = findAndInstallBaseTramp(proc, location);

    // 
    // Generate the code for this tramp.
    //
    // return value is offset of return stmnt.
    //
    count = 0;
    ret->returnAddr = ast->generateTramp(proc, insn, count, trampCost); 

    ret->trampBase = inferiorMalloc(proc, count, textHeap);
    trampBytes += count;
    ret->returnAddr += ret->trampBase;

    ret->when = when;
    ret->location = location;

    ret->next = thePoint->inst;
    ret->prev = NULL;
    if (thePoint->inst) thePoint->inst->prev = ret;
    thePoint->inst = ret;

    /* first inst. at this point so install the tramp */
    fromAddr = ret->baseAddr;
    if (ret->when == callPreInsn) {
	fromAddr += proc->aggregate ? baseTemplate.globalPreOffset :
	    baseTemplate.localPreOffset;
    } else {
	fromAddr += proc->aggregate ? baseTemplate.globalPostOffset :
	    baseTemplate.localPostOffset;
    }

    /*
     * Now make the call to actually put the code in place.
     *
     */
    installTramp(ret, insn, count);

    if (!lastAtPoint) {

	fromAddr = getBaseBranchAddr(proc, ret);
	generateBranch(proc, fromAddr, ret->trampBase);

	generateBranch(proc, ret->returnAddr, fromAddr+4);

	// just activated this slot.
	activeSlots->value += 1.0;
    } else if (order == orderLastAtPoint) {
	/* patch previous tramp to call us rather than return */
	generateBranch(proc, lastAtPoint->returnAddr, ret->trampBase);
	lastAtPoint->nextAtPoint = ret;
	ret->prevAtPoint = lastAtPoint;

	generateBranch(proc, ret->returnAddr, fromAddr+4);
    } else {
	/* first at point */
	firstAtPoint->prevAtPoint = ret;
	ret->nextAtPoint = firstAtPoint;

	/* branch to the old first one */
	generateBranch(proc, ret->returnAddr, firstAtPoint->trampBase);

	/* base tramp branches to us */
	fromAddr = getBaseBranchAddr(proc, ret);
	generateBranch(proc, fromAddr, ret->trampBase);
    }

    return(ret);
}

/*
 * The tramps are chained together left to right, so we need to find the
 *    tramps to the left anf right of the one to delete, and then patch the
 *    call from the left one to the old to call the right one.
 *    Also we need to patch the return from the right one to go to the left
 *    one.
 *
 */
void deleteInst(instInstance *old)
{
    point *thePoint;
    instInstance *lag;
    instInstance *left;
    instInstance *right;
    instInstance *othersAtPoint;

    /* check if there are other inst points at this location. */
    othersAtPoint = NULL;
    left = right = NULL;

    if (!activePoints.defines(old->location))
      abort();
    thePoint = activePoints[old->location];

    for (lag= thePoint->inst; lag; lag = lag->next) {
	if ((lag->location == old->location) && 
	    (lag->proc == old->proc) &&
	    (lag->when == old->when)) {
	    if (lag != old) {
		othersAtPoint = lag;
		left = old->prevAtPoint;
		right = old->nextAtPoint;
		assert(right || left);
	    }
	}
    }

    if (!othersAtPoint) {
	clearBaseBranch(old->proc, old);
	activeSlots->value -= 1.0;
    } else {
	if (left) {
	    if (right) {
		/* set left's return insn to branch to tramp to the right */
		generateBranch(old->proc, left->returnAddr, right->trampBase);
	    } else {
		/* branch back to the correct point in the base tramp */
		int fromAddr;

		fromAddr = getBaseBranchAddr(old->proc, old);

		// this assumes sizeof(int) == sizeof(instruction)
		generateBranch(old->proc,left->returnAddr,fromAddr+sizeof(int));
	    }
	} else {
	    /* old is first one make code call right tramp */
	    int fromAddr;

	    fromAddr = getBaseBranchAddr(old->proc, right);
	    generateBranch(old->proc, fromAddr, right->trampBase);
	}
    }

    inferiorFree(old->proc, old->trampBase, textHeap);

    /* remove old from atPoint linked list */
    if (right) right->prevAtPoint = left;
    if (left) left->nextAtPoint = right;

    /* remove from doubly linked list for all insts */
    if (old->prev) {
	lag = old->prev;
	lag->next = old->next;
	if (old->next) old->next->prev = lag;
    } else {
	thePoint->inst = old->next;
	if (old->next) old->next->prev = NULL;
    }
    free(old);
}


void installDefaultInst(process *proc, vector<instMapping*>& initialReqs)
{
    AstNode *ast;
    instMapping *item;

    unsigned ir_size = initialReqs.size(); 
    for (unsigned u=0; u<ir_size; u++) {
      item = initialReqs[u];
      // TODO this assumes only one instance of each function (no siblings)
      // TODO - are failures safe here ?
      pdFunction *func = (proc->symbols)->findOneFunction(item->func);
      if (!func) {
//
//  it's ok to fail on an initial inst request if the request 
//  is for a programming model that is not used in this process 
//  (i.e. cmmd, cmf)
//	    sprintf (errorLine, "unable to find %s\n", item->func);
//	    logLine(errorLine);
	continue;
      }
      assert(func);

      if (item->where & FUNC_ARG) {
	ast = new AstNode(item->inst, item->arg, NULL);
      } else {
	ast = new AstNode(item->inst, new AstNode(Constant, 0), NULL);
      }
      if (item->where & FUNC_EXIT) {
	(void) addInstFunc(proc, func->funcReturn(), ast,
			   callPreInsn, orderLastAtPoint);
      }
      if (item->where & FUNC_ENTRY) {
	(void) addInstFunc(proc, func->funcEntry(), ast,
			   callPreInsn, orderLastAtPoint);
      }
      if (item->where & FUNC_CALL) {
	if (!func->calls.size()) {
	  ostrstream os(errorLine, 1024, ios::out);
	  os << "No function calls in procedure " << func->prettyName() <<
	    endl;
	  logLine(errorLine);
	  showErrorCallback(64, (const char *) errorLine);
	} else {
	  for (unsigned i = 0; i < func->calls.size(); i++) {
	    (void) addInstFunc(proc, func->calls[i], ast,
			       callPreInsn, orderLastAtPoint);
	  }
	}
      }
      delete(ast);
    }
    // Supercomputing hack - mdc
    // TODO
    osDependentInst(proc);
}

/*
 * return the time required to execute the passed primitive.
 *
 */
unsigned getPrimitiveCost(const string name)
{

    static bool init=false;

    if (!init) { init = 1; initPrimitiveCost(); }

    if (!primitiveCosts.defines(name)) {
      return 1;
    } else
      return (primitiveCosts[name]);
}


// find any tags to associate semantic meaning to function
unsigned findTags(const string funcName) {
  return 0;
#ifdef notdef
  if (tagDict.defines(funcName))
    return (tagDict[funcName]);
  else
    return 0;
#endif
}
