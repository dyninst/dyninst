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
 */

#include <assert.h>
//#include <sys/signal.h>
//#include <sys/param.h>


#include "rtinst/h/rtinst.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "paradynd/src/internalMetrics.h"
#include <strstream.h>
#include "dyninstAPI/src/stats.h"
#include "paradynd/src/init.h"
#include "paradynd/src/showerror.h"

dictionary_hash <string, unsigned> primitiveCosts(string::hash);

// get the address of the branch from the base to the minitramp
int getBaseBranchAddr(process *, instInstance *inst)
{
    int fromAddr;

    fromAddr = (int) inst->baseInstance->baseAddr;
    if (inst->when == callPreInsn) {
	fromAddr += inst->baseInstance->localPreOffset;
    } else {
	fromAddr += inst->baseInstance->localPostOffset;
    }
    return(fromAddr);
}

// get the address in the base tramp where the minitramp should return to
int getBaseReturnAddr(process *, instInstance *inst) {
    int returnAddr = (int)inst->baseInstance->baseAddr;
    if (inst->when == callPreInsn) {
      returnAddr += inst->baseInstance->localPreReturnOffset;
    } else {
      returnAddr += inst->baseInstance->localPostReturnOffset;
    }
    return(returnAddr);
}

// clear the basetramp jump to a minitramp
void clearBaseBranch(process *proc, instInstance *inst)
{
    int addr;

    addr = (int) inst->baseInstance->baseAddr;
    if (inst->when == callPreInsn) {
	addr += inst->baseInstance->localPreOffset;
    } else {
	addr += inst->baseInstance->localPostOffset;
    }
    // stupid kludge because the instPoint class is defined in a .C file
    // so we can't access any of its member functions
    generateNoOp(proc, addr);
    // If there is no instrumentation at this point, skip.
    unsigned fromAddr, toAddr;

    int trampCost;
    if (inst->when == callPreInsn) {
      fromAddr = inst->baseInstance->baseAddr + 
                 (unsigned)inst->baseInstance->skipPreInsOffset;
      toAddr = inst->baseInstance->baseAddr + 
#if defined(rs6000_ibm_aix4_1)
               (unsigned)inst->baseInstance->emulateInsOffset;
#else
               (unsigned)inst->baseInstance->updateCostOffset;
#endif
      inst->baseInstance->prevInstru = false;
      trampCost = -(inst->baseInstance->prevBaseCost);
    }
    else {
      fromAddr = inst->baseInstance->baseAddr + 
                 (unsigned)inst->baseInstance->skipPostInsOffset; 
      toAddr = inst->baseInstance->baseAddr + 
               (unsigned)inst->baseInstance->returnInsOffset;
      inst->baseInstance->postInstru = false;
      trampCost = -(inst->baseInstance->postBaseCost);
    }
    inst->baseInstance->updateTrampCost(proc, trampCost);
    
    generateBranch(proc,fromAddr,toAddr);
#if defined(MT_DEBUG)
    sprintf(errorLine,"generating branch from address 0x%x to address 0x%x - CLEAR\n",fromAddr,toAddr);
    logLine(errorLine);
#endif
}

// implicit assumption that tramps generate to less than 64K bytes!!!
static char insn[65536];

static dictionary_hash<const instPoint*, point*> activePoints(ipHash);

vector<instWaitingList *> instWList;

// Shouldn't this be a member fn of class process?
instInstance *addInstFunc(process *proc, instPoint *&location,
			  AstNode *&ast, // ast may change (sysFlag stuff)
			  callWhen when, callOrder order,
			  bool noCost)
{
    returnInstance *retInstance = NULL;
    instInstance *inst = addInstFunc(proc, location, ast, when, order,
				     noCost, retInstance);
    if (retInstance) {
       retInstance-> installReturnInstance(proc);
       // writes to addr space
    }

    // delete retInstance; 
    // safe if NULL (may have been alloc'd by findAndInstallBaseTramp)

    return inst;
}

// Shouldn't this be a member fn of class process?
instInstance *addInstFunc(process *proc, instPoint *&location,
			  AstNode *&ast, // the ast could be changed 
			  callWhen when, callOrder order,
			  bool noCost,
			  returnInstance *&retInstance)
{
    assert(proc && location);
    initTramps();

    instInstance *ret = new instInstance;
    assert(ret);

    ret->proc = proc;
    ret->baseInstance = findAndInstallBaseTramp(proc, location, retInstance, 
						noCost);
    if (!ret->baseInstance)
       return(NULL);

#if defined(MT_DEBUG)
    sprintf(errorLine,"==>BaseTramp is in 0x%x\n",ret->baseInstance->baseAddr);
    logLine(errorLine);
#endif

    /* check if there are other inst points at this location. for this process
       at the same pre/post mode */
    instInstance *firstAtPoint = NULL;
    instInstance *lastAtPoint = NULL;

    point *thePoint;
    if (!activePoints.defines((const instPoint *)location)) {
      thePoint = new point;
      activePoints[(const instPoint*)location] = thePoint;
    } else
      thePoint = activePoints[(const instPoint*)location];
    assert(thePoint);

    instInstance *next;
    for (next = thePoint->inst; next; next = next->next) {
	if ((next->proc == proc) && (next->when == when)) {
	    if (!next->nextAtPoint) lastAtPoint = next;
	    if (!next->prevAtPoint) firstAtPoint = next;
	}
    }

    // must do this before findAndInstallBaseTramp, puts the tramp in to
    // get the correct cost.
    // int trampCost = getPointCost(proc, (const instPoint*)location);

    /* make sure the base tramp has been installed for this point */
    //ret->baseAddr = findAndInstallBaseTramp(proc, location, retInstance);
    //if (!ret->baseAddr) return(NULL);

    // 
    // Generate the code for this tramp.
    //
    // return value is offset of return stmnt.
    //

#if defined(MEMSET)
    // clear out old stuff - for debugging.
    memset(insn, 0x00, 65536);
#endif

    u_int count = 0;

#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)     
    ast->sysFlag((instPoint*)location);  
#endif

    int trampCost = 0;
    ret->returnAddr = ast->generateTramp(proc, insn, count, trampCost, noCost);

    if (!noCost) {
	ret->cost = trampCost; 
	ret->baseInstance->updateTrampCost(proc, trampCost);
    }

    ret->trampBase = inferiorMalloc(proc, count, textHeap);
    assert(ret->trampBase);

    if (!ret->trampBase) return(NULL);
    trampBytes += count;
    ret->returnAddr += ret->trampBase;

    ret->when = when;
    ret->location = (instPoint*)location;

    ret->next = thePoint->inst;
    ret->prev = NULL;
    if (thePoint->inst) thePoint->inst->prev = ret;
    thePoint->inst = ret;

    /*
     * Now make the call to actually put the code in place.
     *
     */
    installTramp(ret, insn, count); // install mini-tramp into inferior addr space

    if (!lastAtPoint) {
      
        // jump from the base tramp to the minitramp
	unsigned fromAddr = getBaseBranchAddr(proc, ret);
	generateBranch(proc, fromAddr, ret->trampBase);

	// jump from the minitramp back to the basetramp
	unsigned toAddr = getBaseReturnAddr(proc, ret);
	generateBranch(proc, ret->returnAddr, toAddr);

	// just activated this slot.
	//activeSlots->value += 1.0;
    } else if (order == orderLastAtPoint) {
	/* patch previous tramp to call us rather than return */
	generateBranch(proc,lastAtPoint->returnAddr,ret->trampBase);
	lastAtPoint->nextAtPoint = ret;
	ret->prevAtPoint = lastAtPoint;
	
	// jump from the minitramp to the basetramp
	unsigned toAddr = getBaseReturnAddr(proc, ret);
	generateBranch(proc, ret->returnAddr, toAddr);
    } else {
	/* first at point */
	firstAtPoint->prevAtPoint = ret;
	ret->nextAtPoint = firstAtPoint;

	/* branch to the old first one */
	generateBranch(proc, ret->returnAddr, firstAtPoint->trampBase);

	/* base tramp branches to us */
	unsigned fromAddr = getBaseBranchAddr(proc, ret);
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

void getAllInstInstancesForProcess(const process *proc,
				   vector<instInstance*> &result) {
    vector<point*> allPoints = activePoints.values();
 
    // find all instInstances of the parent process
    for (unsigned u = 0; u < allPoints.size(); u++) {
      for (instInstance *inst = allPoints[u]->inst; inst; inst = inst->next) {
	if (inst->proc == proc)
	  result += inst;
      }
    }
}

void copyInstInstances(const process *parent, const process *child, 
	    dictionary_hash<instInstance *, instInstance *> &instInstanceMapping)
{
//    vector<point*> allPoints = activePoints.values();
    vector<instInstance*> instsToCopy;
    getAllInstInstancesForProcess(parent, instsToCopy);
 
//    // find all instInstances of the parent process
//    for (unsigned u = 0; u < allPoints.size(); u++) {
//      for (instInstance *inst = allPoints[u]->inst; inst; inst = inst->next) {
//	if (inst->proc == parent)
//	  instsToCopy += inst;
//      }
//    }

    // duplicate the parent instance for the child, and define instMapping
    vector<instInstance *>newInsts;
    for (unsigned u1 = 0; u1 < instsToCopy.size(); u1++) {
      instInstance *old = instsToCopy[u1];
      instInstance *newInst = new instInstance;
      newInst->proc = child;
      newInst->when = old->when;
      newInst->location = old->location;
      newInst->trampBase = old->trampBase;
      newInst->returnAddr = old->returnAddr;
      newInst->baseInstance = old->baseInstance;
      newInst->cost = old->cost;
      instInstanceMapping[old] = newInst;
      newInsts += newInst;
    }

    // update nextAtPoint and prevAtPoint
    for (unsigned u2 = 0; u2 < newInsts.size(); u2++) {
      instInstance *newInst = newInsts[u2];
      instInstance *old = instsToCopy[u2];
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
        pointsToCheck += start->baseInstance->baseAddr; 
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
void deleteInst(instInstance *old, const vector<unsigned> &pointsToCheck)
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
		unsigned toAddr = getBaseReturnAddr(old->proc, old);
		generateBranch(old->proc, left->returnAddr, toAddr);
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
    int trampCost = 0-old->cost;
    old->baseInstance->updateTrampCost(old->proc, trampCost);

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


/*
 * return the time required to execute the passed primitive.
 *
 */
unsigned getPrimitiveCost(const string &name)
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


void
trampTemplate::updateTrampCost(process *proc, int trampCost) {
    
    int caddr;
    cost = cost + trampCost;
    if (cost < 0) cost = 0;

    char costInsn[40];
    unsigned csize = 0;

    // quick dirty hack; Will be changed soon so that we 
    // don't call getObservedCostAddr() every time  --ling  
    proc->getObservedCostAddr();   
    caddr = proc->costAddr(); 

    emit(updateCostOp, cost, 0, caddr, costInsn, csize, false);
    proc->writeDataSpace((caddr_t)costAddr, csize, costInsn);
}
