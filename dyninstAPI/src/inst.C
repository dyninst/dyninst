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

// $Id: inst.C,v 1.83 2001/09/07 21:15:09 tikir Exp $
// Code to install and remove instrumentation from a running process.

#include <assert.h>
//#include <sys/signal.h>
//#include <sys/param.h>

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/stats.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/instPoint.h"
#ifndef BPATCH_LIBRARY
#include "paradynd/src/init.h"
#endif

dictionary_hash <string, unsigned> primitiveCosts(string::hash);

#if defined(rs6000_ibm_aix4_1)
  extern void resetBRL(process *p, Address loc, unsigned val); //inst-power.C
  extern void resetBR( process *p, Address loc);               //inst-power.C
#endif

#ifndef BPATCH_LIBRARY
static unsigned function_base_ptr_hash(function_base *const &f) {
  function_base *ptr = f;
  unsigned l = (unsigned)(Address)ptr;
  return addrHash4(l); 
}

// Fill in <callees> with list of statically determined callees of
//  function.  
// Uses process specific info to try to fill in the unbound call
//  destinations through PLT entries.  Note that when it determines
//  the destination of a call through the PLT, it puts that
//  call destination into <callees>, but DOES NOT fill in the
//  call destination in the function's instPoint.  This is because
//  the (through PLT) binding is process specific.  It is possible, 
//  for example, that one could have 2 processes, both sharing the
//  same a.out image, but which are linked with different versions of
//  the same shared library (or with the same shared libraries in 
//  a different order), in which case the pd_Function data would be 
//  shared between the processes, but the (through-PLT) call 
//  destinations might NOT be the same.
// Should filter out any duplicates in this callees list....

bool pd_Function::getStaticCallees(process *proc,
				   vector <pd_Function *>&callees) {
    unsigned u;
    function_base *f;
    bool found;
    
    dictionary_hash<function_base *, function_base *> 
      filter(function_base_ptr_hash);
    
    callees.resize(0);

    // possible algorithm : iterate over calls (vector of instPoint *)
    //   for each elem : use iPgetCallee() to get statically determined
    //   callee....
    for(u=0;u<calls.size();u++) {
      //this call to iPgetCallee is platform specific
      f = const_cast<function_base *>(calls[u]->iPgetCallee());
      
      if (f == NULL) {
	//cerr << " unkown call destination";
	found = proc->findCallee((*calls[u]), f);
	if (f != NULL) {
	  //cerr << " found w/ process specific PLT info" << endl;
	} else {
	  //cerr << " not found in PLT info" << endl;
	}
      } else if (filter.defines(f)) {
	//cerr << " call destination " << f->prettyName().string_of() << 
	//" already seen by filer" << endl;
      }
      
      if (f != NULL && !filter.defines(f)) {
      callees += (pd_Function *)f;
	filter[f] = f;
	}
      }
    return true;
}
#endif

// get the address of the branch from the base to the minitramp
Address getBaseBranchAddr(process *, instInstance *inst)
{
    Address fromAddr;

    fromAddr = inst->baseInstance->baseAddr;
    if (inst->when == callPreInsn) {
	fromAddr += inst->baseInstance->localPreOffset;
    } else {
	fromAddr += inst->baseInstance->localPostOffset;
    }
    return(fromAddr);
}

// get the address in the base tramp where the minitramp should return to
Address getBaseReturnAddr(process *, instInstance *inst) {
    Address returnAddr = inst->baseInstance->baseAddr;
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
    Address addr;

    addr = inst->baseInstance->baseAddr;
    if (inst->when == callPreInsn) {
	addr += inst->baseInstance->localPreOffset;
    } else {
	addr += inst->baseInstance->localPostOffset;
    }
    // stupid kludge because the instPoint class is defined in a .C file
    // so we can't access any of its member functions
#if defined(rs6000_ibm_aix4_1)
    resetBRL(proc, addr, 0);
#else
    generateNoOp(proc, addr);
#endif
    // If there is no instrumentation at this point, skip.
    Address fromAddr, toAddr;

    int trampCost;
    if (inst->when == callPreInsn) {
      fromAddr = inst->baseInstance->baseAddr + 
                 inst->baseInstance->skipPreInsOffset;
      toAddr = inst->baseInstance->baseAddr + 
#if defined(rs6000_ibm_aix4_1) || defined(alpha_dec_osf4_0)
               inst->baseInstance->emulateInsOffset;
#else
               inst->baseInstance->updateCostOffset;
#endif
      inst->baseInstance->prevInstru = false;
      trampCost = -(inst->baseInstance->prevBaseCost);
    }
    else {
      fromAddr = inst->baseInstance->baseAddr + 
                 inst->baseInstance->skipPostInsOffset; 
      toAddr = inst->baseInstance->baseAddr + 
               inst->baseInstance->returnInsOffset;
      inst->baseInstance->postInstru = false;
      trampCost = -(inst->baseInstance->postBaseCost);
    }
    inst->baseInstance->updateTrampCost(proc, trampCost);
    generateBranch(proc,fromAddr,toAddr);
#if defined(MT_DEBUG)
    sprintf(errorLine,"generating branch from address 0x%lx to address 0x%lx"
        " - CLEAR\n",fromAddr,toAddr);
    logLine(errorLine);
#endif
}

// implicit assumption that tramps generate to less than 64K bytes!!!
static int insn[65536/sizeof(int)]; // Made into array of int so it would be
				    // aligned correctly on platforms that
				    // need it to be (like SPARC) - BRB

static dictionary_hash<const instPoint*, point*> activePoints(ipHash);

vector<instWaitingList *> instWList;

// Shouldn't this be a member fn of class process?
instInstance *addInstFunc(process *proc, instPoint *&location,
			  AstNode *&ast, // ast may change (sysFlag stuff)
			  callWhen when, callOrder order,
			  bool noCost,
			  bool trampRecursiveDesired)
{
    returnInstance *retInstance = NULL;
       // describes how to jmp to the base tramp

    bool deferred = false;  // dummy variable
    instInstance *inst = addInstFunc(proc, location, ast, when, order,
				     noCost, retInstance, deferred,
                                     trampRecursiveDesired);
    if (retInstance) {
       // Looking at the code for the other addInstFunc below, it seems that
       // this will always be true...retInstance is never NULL.

       retInstance-> installReturnInstance(proc);
       // writes to addr space
    }

    // delete retInstance; 
    // safe if NULL (may have been alloc'd by findAndInstallBaseTramp)

    return inst;
}

// Shouldn't this be a member fn of class process?
// The trampRecursiveDesired flag decides which base tramp is used,
// _if_ a base tramp is installed. If there is already a base tramp
// at the instrumentation point, the flag is ignored.
instInstance *addInstFunc(process *proc, instPoint *&location,
			  AstNode *&ast, // the ast could be changed 
			  callWhen when, callOrder order,
			  bool noCost,
			  returnInstance *&retInstance, 
                          bool &deferred,
			  bool trampRecursiveDesired = false)
{

    // retInstance gets filled in with info on how to jmp to the base tramp
    // (the call to findAndInstallBaseTramp doesn't do that)

    assert(proc && location);
    initTramps();

    instInstance *ret = new instInstance;
    assert(ret);

    ret->proc = proc;
    ret->baseInstance = findAndInstallBaseTramp(proc, location, retInstance,
   					        trampRecursiveDesired, noCost, 
                                                deferred);

    if (!ret->baseInstance)
       return(NULL);

#if defined(MT_DEBUG_ON)
    sprintf(errorLine,"==>BaseTramp is in 0x%lx\n",ret->baseInstance->baseAddr);
    logLine(errorLine);
#endif

    /* check if there are other inst points at this location. for this process
       at the same pre/post mode */
    instInstance *firstAtPoint = NULL;
    instInstance *lastAtPoint = NULL;

    point *thePoint;
    if (!activePoints.find((const instPoint *)location, thePoint)) {
       thePoint = new point;
       activePoints[(const instPoint*)location] = thePoint;
    }
    
//    point *thePoint;
//    if (!activePoints.defines((const instPoint *)location)) {
//      thePoint = new point;
//      activePoints[(const instPoint*)location] = thePoint;
//    } else
//      thePoint = activePoints[(const instPoint*)location];
//    assert(thePoint);

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

    // 
    // Generate the code for this (mini-)tramp.
    //
    // return value is offset of return stmnt.
    //

#if defined(MEMSET)
    // clear out old stuff - for debugging.
    memset(insn, 0x00, 65536);
#endif

    Address count = 0;

#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)     
    ast->sysFlag((instPoint*)location);  

    // If the return value is in the case of compiler optimization,
    // modify the ast node tree to insert an instruction to get the 
    // return the value
    extern bool processOptimaRet(instPoint *location, AstNode *&ast);
    bool isOpt = processOptimaRet(location, ast);
#endif

    int trampCost = 0;
    ret->returnAddr = ast->generateTramp(proc, (char *)insn, count, trampCost, noCost);

#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)
    // The ast node might be shared, so remove the changes made to
    // ast node tree  
    if (isOpt)
	ast->optRetVal(NULL); 
#endif

    if (!noCost) {
	ret->cost = trampCost; 
	ret->baseInstance->updateTrampCost(proc, trampCost);
    }
#if defined(rs6000_ibm_aix4_1)
    // We use the data heap on AIX because it is unlimited, unlike
    // the textHeap. The text heap is allocated from spare parts of 
    // pages, and as such can run out. Since minitramps can be arbitrarily
    // far from the base tramp (link register branching), but only a 
    // single jump from each other, we cluster them in the dataHeap.
    // Note for future reference: shouldn't this near_ be the address
    // of the previous/next minitramp instead of the base tramp?
    inferiorHeapType htype = dataHeap;
#else
    inferiorHeapType htype = (proc->splitHeaps) ? (textHeap) : (anyHeap);
#endif
    Address near_ = ret->baseInstance->baseAddr;
    bool err = false;
    ret->trampBase = inferiorMalloc(proc, count, htype, near_, &err);

    if (err) return NULL;
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
    installTramp(ret, (char *)insn, count); // install mini-tramp into inferior addr space

    if (!lastAtPoint) {
        // jump from the base tramp to the minitramp
        Address fromAddr = getBaseBranchAddr(proc, ret);
#if defined(rs6000_ibm_aix4_1)
	resetBRL(proc, fromAddr, ret->trampBase);
#else
	generateBranch(proc, fromAddr, ret->trampBase);
#endif

	// jump from the minitramp back to the basetramp
#if defined(rs6000_ibm_aix4_1)
	resetBR(proc, ret->returnAddr);
#else
	Address toAddr = getBaseReturnAddr(proc, ret);
	generateBranch(proc, ret->returnAddr, toAddr);
#endif

	// just activated this slot.
	//activeSlots->value += 1.0;
    } else if (order == orderLastAtPoint) {
	/* patch previous tramp to call us rather than return */
	generateBranch(proc,lastAtPoint->returnAddr,ret->trampBase);
	lastAtPoint->nextAtPoint = ret;
	ret->prevAtPoint = lastAtPoint;
	
	// jump from the minitramp to the basetramp
#if defined(rs6000_ibm_aix4_1)
	resetBR(proc, ret->returnAddr);
#else
	Address toAddr = getBaseReturnAddr(proc, ret);
	generateBranch(proc, ret->returnAddr, toAddr);
#endif
    } else {
	/* first at point */
	firstAtPoint->prevAtPoint = ret;
	ret->nextAtPoint = firstAtPoint;

	/* branch to the old first one */
	generateBranch(proc, ret->returnAddr, firstAtPoint->trampBase);

	/* base tramp branches to us */
	Address fromAddr = getBaseBranchAddr(proc, ret);
#if defined(rs6000_ibm_aix4_1)
	resetBRL(proc, fromAddr, ret->trampBase);
#else
	generateBranch(proc, fromAddr, ret->trampBase);
#endif
    }

    return(ret);
}

bool trampTemplate::inBasetramp( Address addr ) {
	return addr >= baseAddr && addr < ( baseAddr + size );
}


bool trampTemplate::inSavedRegion( Address addr ) {
	if( !inBasetramp( addr ) )
		return false;
	addr -= baseAddr;
	return ( addr > (Address)savePreInsOffset && addr <= (Address)restorePreInsOffset )
		|| ( addr > (Address)savePostInsOffset && addr <= (Address)restorePostInsOffset );
}


instPoint * findInstPointFromAddress(const process *proc, Address addr) {
    unsigned u;

    vector<const instPoint*> ips;
    vector<trampTemplate*> bts;
    ips = proc->baseMap.keys();
    bts = proc->baseMap.values();
    assert( ips.size() == bts.size() );
    for( u = 0; u < bts.size(); ++u ) {
	if( bts[u]->inBasetramp( addr ) )
	{
	    return const_cast<instPoint*>( ips[u] );
	}
    }

    vector<point*> allPoints = activePoints.values();

    for( u = 0; u < allPoints.size(); ++u ) {
	for( instInstance *inst = allPoints[u]->inst; inst; inst = inst->next ) {
	    if( inst->proc == proc ) {
		if( ( inst->trampBase <= addr && inst->returnAddr >= addr )
		    || inst->baseInstance->inBasetramp( addr ) )
		{
		    return inst->location;
		}
	    }
	}
    }
    return NULL;
}

trampTemplate * findBaseTramp( const instPoint * ip, const process *proc ) {
    if( activePoints.defines( ip ) ) {
	point *p = activePoints[ ip ];
	assert( p );
	for( instInstance *ii = p->inst; ii; ii = ii->next )
	    if( ii->proc == proc )
		return ii->baseInstance;
    }
    return NULL;
}

instInstance * findMiniTramps( const instPoint * ip ) {
    if( activePoints.defines( ip ) ) {
	point *p = activePoints[ ip ];
	assert( p );
	return p->inst;
    }
    return NULL;
}


// TODO: this functionality overlaps with "findInstPointFromAddress()"
pd_Function *findAddressInFuncsAndTramps(process *p, Address addr,
					 instPoint *&ip,
					 trampTemplate *&bt,
					 instInstance *&mt)
{
  unsigned n;

  // default return values
  ip = NULL;
  bt = NULL;
  mt = NULL;

  // look for address in user code
  function_base *fn = p->findFuncByAddr(addr);
  if (fn != NULL) return (pd_Function *)fn;

  // look for address in basetramps ("baseMap")
  vector<const instPoint *> ips = p->baseMap.keys();
  vector<trampTemplate *> bts = p->baseMap.values();
  n = ips.size();
  assert(n == bts.size());
  for (unsigned i = 0; i < n; i++) {
    if (bts[i]->inBasetramp(addr)) {
      ip = const_cast<instPoint *>(ips[i]);
      bt = bts[i];
      return (pd_Function*)const_cast<function_base*>(ip->iPgetFunction());
    }
  }
    
  // look for address in minitramps ("activePoints")
  vector<point *> pts = activePoints.values();
  instInstance *inst;
  n = pts.size();
  for (unsigned i2 = 0; i2 < n; i2++) {
    inst = pts[i2]->inst;
    for ( ; inst != NULL; inst = inst->next) {
      if (inst->proc == p) {
	if (addr >= inst->trampBase && addr <= inst->returnAddr) {
	  ip = inst->location;
	  mt = inst;
	  return (pd_Function*)const_cast<function_base*>(ip->iPgetFunction());
	} else if (inst->baseInstance->inBasetramp(addr)) {
	  // the basetramp search should have turned this up
	  assert(0);
	}
      }
    }
  }

  return NULL;
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
	  result.push_back(inst);
      }
    }
}

void copyInstInstances(const process *parent, const process *child, 
	    dictionary_hash<instInstance *, instInstance *> &instInstanceMapping)
{
    vector<instInstance*> instsToCopy;
    getAllInstInstancesForProcess(parent, instsToCopy);
 
    // duplicate the parent instance for the child, and define instMapping
    vector<instInstance *>newInsts;
    for (unsigned u1 = 0; u1 < instsToCopy.size(); u1++) {
      instInstance *old = instsToCopy[u1];
      instInstance *newInst = new instInstance;
      newInst->proc = const_cast<process *>(child);
      newInst->when = old->when;
      newInst->location = old->location;
      newInst->trampBase = old->trampBase;
      newInst->returnAddr = old->returnAddr;
      newInst->baseInstance = old->baseInstance;
      newInst->cost = old->cost;
      instInstanceMapping[old] = newInst;
      newInsts.push_back(newInst);
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
vector<Address> getAllTrampsAtPoint(instInstance *instance)
{
    vector<Address> pointsToCheck;
    instInstance *start;
    instInstance *next;
    point *thePoint;

    if (instance) {
      if (activePoints.defines(instance->location)) {
        thePoint = activePoints[instance->location];
        start = thePoint->inst;
        // Base tramp
        pointsToCheck.push_back(start->baseInstance->baseAddr); 
        pointsToCheck.push_back(start->trampBase);
        // All mini-tramps at this point
        for (next = start->next; next; next = next->next) {
	  if ((next->location == instance->location) && 
	      (next->proc == instance->proc) &&
	      (next->when == instance->when)) {
  	      if (next != instance) {
                pointsToCheck.push_back(next->trampBase);
                //pointsToCheck += start->trampBase;
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
void deleteInst(instInstance *old, const vector<Address> &pointsToCheck)
{
    point *thePoint;
    instInstance *lag;
    instInstance *left;
    instInstance *right;
    instInstance *othersAtPoint;

    if (!old) {
      // logLine("Internal error in inst.C: instInstance pointer \"old\" is NULL\n");
      return;
    }

    /* check if there are other inst points at this location. */
    othersAtPoint = NULL;
    left = right = NULL;

    if (!activePoints.defines(old->location)) {
#if !defined(MT_THREAD)
      abort();
#else
      cerr << "in inst.C: location is NOT defined in activePoints" << endl;
      return ;
#endif
    }
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
#if defined(rs6000_ibm_aix4_1)
		resetBR(old->proc, left->returnAddr);
#else
		Address toAddr = getBaseReturnAddr(old->proc, old);
		generateBranch(old->proc, left->returnAddr, toAddr);
#endif
	    }
	} else {
	    /* old is first one make code call right tramp */
	    int fromAddr;
	    fromAddr = getBaseBranchAddr(old->proc, right);
#if defined(rs6000_ibm_aix4_1)
	    resetBRL(old->proc, fromAddr, right->trampBase);
#else
	    generateBranch(old->proc, fromAddr, right->trampBase);
#endif
	}
    }

    vector< vector<Address> > tmp;
    tmp.push_back((vector<Address>) pointsToCheck);

#ifdef FREEDEBUG1
    sprintf(errorLine,"***** (pid=%d) In inst.C, calling inferiorFree, "
	    "pointer=0x%lx\n",old->proc->pid,old->trampBase);
    logLine(errorLine);
#endif
    inferiorFree(old->proc, old->trampBase, tmp);
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

    // If the thePoint->inst value is NULL then there is no more
    // instrumenttation instances left for this point.

    if(BPatch::bpatch->baseTrampDeletion() &&
       !thePoint->inst)
    {
	extern bool deleteBaseTramp(process*,instPoint*,instInstance*);
	if(deleteBaseTramp(old->proc,old->location,old)){
		activePoints.undef((const instPoint*)(old->location));
                old->proc->baseMap.undef((const instPoint*)(old->location));
                delete[] old->baseInstance->trampTemp;
                delete old->baseInstance;
        }
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

#ifndef alpha_dec_osf4_0 /* XXX We don't calculate cost yet on Alpha */
    cost = cost + trampCost;
    if (cost < 0) cost = 0;

    char costInsn[40];
    Address csize = 0;

    // quick dirty hack; Will be changed soon so that we 
    // don't call getObservedCostAddr() every time  --ling  
    proc->getObservedCostAddr();   
    Address caddr = proc->costAddr(); 

    emitVupdate(updateCostOp, cost, 0, caddr, costInsn, csize, false);
    proc->writeDataSpace((caddr_t)costAddr, csize, costInsn);
#endif
}

void cleanInstFromActivePoints(process *proc)
{
    assert(proc);
    vector<point*> allPoints = activePoints.values();
 
    // is it ok to have activePoints elements with empty points? - naim
    for (unsigned u = 0; u < allPoints.size(); u++) {
      instInstance *inst = allPoints[u]->inst;
      while (inst) {
	assert(inst->proc);
	if (inst->proc->getPid() == proc->getPid()) {
	  if (!inst->prev) {
	    // this is the first one on the list
	    if (!inst->next) {
	      // this is the only one on the list
	      delete inst;
	      allPoints[u]->inst = NULL;
	      inst = NULL;
	    } else {
	      inst->next->prev = NULL;
	      allPoints[u]->inst = inst->next;
	      delete inst;
	      inst = allPoints[u]->inst;
	    }
	  } else {
	    // this is not the first one
	    if (!inst->next) {
	      // this is the last one
	      inst->prev->next = NULL;
	      delete inst;
	      inst = NULL;
	    } else {
	      // we are somewhere in the middle of the list
	      inst->prev->next = inst->next;
	      inst->next->prev = inst->prev;
	      instInstance *next_inst = inst->next;
	      delete inst;
	      inst = next_inst;
	    }
	  }
	} else {
	  inst = inst->next;
	}
      }
    }
}

