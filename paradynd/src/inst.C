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
 * inst.C - Code to install and remove inst funcs from a running process.
 *
 * inst.C,v
 * Revision 1.29  1996/05/16  15:03:03  naim
 * Checking that instInstance pointer is not NULL - naim
 *
 * Revision 1.28  1996/05/15  18:32:45  naim
 * Fixing bug in inferiorMalloc and adding some debugging information - naim
 *
 * Revision 1.27  1996/05/10  22:36:31  naim
 * Bug fix and some improvements passing a reference instead of copying a
 * structure - naim
 *
 * Revision 1.26  1996/05/08  23:54:47  mjrg
 * added support for handling fork and exec by an application
 * use /proc instead of ptrace on solaris
 * removed warnings
 *
 * Revision 1.25  1996/04/03 14:27:39  naim
 * Implementation of deallocation of instrumentation for solaris and sunos - naim
 *
 * Revision 1.24  1996/03/25  22:58:07  hollings
 * Support functions that have multiple exit points.
 *
 * Revision 1.23  1996/03/25  20:21:10  tamches
 * the reduce-mem-leaks-in-paradynd commit
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

instInstance *addInstFunc(process *proc, instPoint *location,
			  const AstNode &ast,
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
    if (!ret->baseAddr) return(NULL);

    // 
    // Generate the code for this tramp.
    //
    // return value is offset of return stmnt.
    //

    // clear out old stuff - for debugging.
    memset(insn, 0x00, 65536);

    count = 0;
    ret->returnAddr = ast.generateTramp(proc, insn, count, trampCost); 

    ret->trampBase = inferiorMalloc(proc, count, textHeap);

#ifdef FREEDEBUG1
    static vector<unsigned> TESTaddrs;
    for (unsigned i=0;i<TESTaddrs.size();i++) {
      if (TESTaddrs[i] == ret->trampBase) {
        sprintf(errorLine,"=====> inferiorMalloc returned same address 0x%x\n",ret->trampBase);
        logLine(errorLine);
      }
    }
    TESTaddrs += (unsigned)ret->trampBase;
#endif

    if (!ret->trampBase) return(NULL);
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
	//activeSlots->value += 1.0;
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

//
// copyInstInstances: called when a process we are tracing forks.
// The child will have a copy of all instrumentation in the parent, so we
// must duplicate all instInstances of the parent for the child.
// 
// On return, for each active instInstance parentInst in the parent
// instInstanceMapping[parentInst] will be the corresponding instInstance for the
// child.
//
void copyInstInstances(process *parent, process *child, 
	    dictionary_hash<instInstance *, instInstance *> &instInstanceMapping)
{
    vector<point*> allPoints = activePoints.values();
    vector<instInstance*> instsToCopy;
 
    // find all instInstances of the parent process
    for (unsigned u = 0; u < allPoints.size(); u++) {
      for (instInstance *inst = allPoints[u]->inst; inst; inst = inst->next) {
	if (inst->proc == parent)
	  instsToCopy += inst;
      }
    }

    // duplicate the parent instance for the child, and define instMapping
    vector<instInstance *>newInsts;
    for (unsigned u = 0; u < instsToCopy.size(); u++) {
      instInstance *old = instsToCopy[u];
      instInstance *newInst = new instInstance;
      newInst->proc = child;
      newInst->when = old->when;
      newInst->location = old->location;
      newInst->trampBase = old->trampBase;
      newInst->returnAddr = old->returnAddr;
      newInst->baseAddr = old->baseAddr;
      newInst->cost = old->cost;
      instInstanceMapping[old] = newInst;
      newInsts += newInst;
    }

    // update nextAtPoint and prevAtPoint
    for (unsigned u = 0; u < newInsts.size(); u++) {
      instInstance *newInst = newInsts[u];
      instInstance *old = instsToCopy[u];
      newInst->nextAtPoint = instInstanceMapping[old->nextAtPoint];
      newInst->prevAtPoint = instInstanceMapping[old->prevAtPoint];

      assert(activePoints.defines(newInst->location));
      point *thePoint = activePoints[newInst->location];
      newInst->next = thePoint->inst;
      newInst->prev = NULL;
      if (thePoint->inst) thePoint->inst->prev = newInst;
      thePoint->inst = newInst;
    }
}

// This procedure assumes that any mini-tramp for an inst request could refer 
// to any data pointer for that request. A more complex analysis could check 
// what data pointers each mini-tramp really used, but I don't think it is 
// worth the trouble.
//
vector<unsigned> getAllTrampsAtPoint(instInstance *instance)
{
    vector<unsigned> pointsToCheck;
    instInstance *start;
    instInstance *next;
    point *thePoint;

    if (instance) {
      if (activePoints.defines(instance->location)) {
        thePoint = activePoints[instance->location];
        start = thePoint->inst;
        // Base tramp
        pointsToCheck += start->baseAddr; 
        pointsToCheck += start->trampBase;
        // All mini-tramps at this point
        for (next = start->next; next; next = next->next) {
	  if ((next->location == instance->location) && 
	      (next->proc == instance->proc) &&
	      (next->when == instance->when)) {
  	      if (next != instance) {
                pointsToCheck += start->trampBase;
	      }
	  }
        }
      }
    }
    return(pointsToCheck);
}

/*
 * The tramps are chained together left to right, so we need to find the
 *    tramps to the left anf right of the one to delete, and then patch the
 *    call from the left one to the old to call the right one.
 *    Also we need to patch the return from the right one to go to the left
 *    one.
 *
 */
void deleteInst(instInstance *old, vector<unsigned> pointsToCheck)
{
    point *thePoint;
    instInstance *lag;
    instInstance *left;
    instInstance *right;
    instInstance *othersAtPoint;

    if (!old) {
      logLine("Internal error in inst.C: instInstance pointer \"old\" is NULL\n");
      return;
    }

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

  if (old->proc->status() != exited) {
    if (!othersAtPoint) {
	clearBaseBranch(old->proc, old);
	//activeSlots->value -= 1.0;
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

    vector<unsigVecType> tmp;
    tmp += (unsigVecType) pointsToCheck;

#ifdef FREEDEBUG1
    sprintf(errorLine,"***** (pid=%d) In inst.C, calling inferiorFree, pointer=0x%x\n",old->proc->pid,old->trampBase);
    logLine(errorLine);
#endif

    inferiorFree(old->proc, old->trampBase, textHeap, tmp);
  }

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
    delete old;
    //free(old);
}

//
// Routine that checks whether a particular address is valid before deleting
// the corresponding instrumentation associated to it.
//
bool isValidAddress(process * , Address )
{
  bool result=true;
  //
  // Note: It seems that it is not necessary to do this step. In any case,
  // calling "proc->symbols->isValidAddress(where)" usually returns false
  // even for cases when the address looks ok. If it is required to have
  // such a routine, we will have to figure out a better one. An idea
  // could be to get the address for "start" and length of the code and
  // make sure that the address "where" is between those two values (that's
  // what gdb goes for sparcs) - naim
  // 

#ifdef FREEDEBUG
  if (!result) {
    sprintf(errorLine,"==> TEST <== isValidAddress is FALSE\n");
    logLine(errorLine);
  }
#endif

  return(result);
}

void installDefaultInst(process *proc, vector<instMapping*>& initialReqs)
{
    unsigned ir_size = initialReqs.size(); 
    for (unsigned u=0; u<ir_size; u++) {
      instMapping *item = initialReqs[u];
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

      AstNode ast;
      if (item->where & FUNC_ARG)
	ast = AstNode(item->inst, *(item->arg));
      else
	ast = AstNode(item->inst, AstNode(Constant, 0));

      if (item->where & FUNC_EXIT) {
	  for (unsigned i = 0; i < func->funcReturns.size(); i++) {
		(void) addInstFunc(proc, func->funcReturns[i], ast,
				   callPreInsn, orderLastAtPoint);
	  }
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
    }

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
unsigned findTags(const string ) {
  return 0;
#ifdef notdef
  if (tagDict.defines(funcName))
    return (tagDict[funcName]);
  else
    return 0;
#endif
}
