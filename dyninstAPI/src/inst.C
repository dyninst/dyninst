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

// $Id: inst.C,v 1.103 2003/01/31 18:55:42 chadd Exp $
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

int instInstance::_id = 1;

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
				   pdvector <pd_Function *>&callees) {
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
	//cerr << " call destination " << f->prettyName().c_str() << 
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
Address getBaseBranchAddr(process *, const instInstance *inst, callWhen when)
{
    Address fromAddr;

    fromAddr = inst->baseInstance->baseAddr;
    if(when == callPreInsn) {
	fromAddr += inst->baseInstance->localPreOffset;
    } else {
	fromAddr += inst->baseInstance->localPostOffset;
    }
    return(fromAddr);
}

// get the address in the base tramp where the minitramp should return to
Address getBaseReturnAddr(process *, const instInstance *inst, callWhen when) {
    Address returnAddr = inst->baseInstance->baseAddr;
    if(when == callPreInsn) {
      returnAddr += inst->baseInstance->localPreReturnOffset;
    } else {
      returnAddr += inst->baseInstance->localPostReturnOffset;
    }
    return(returnAddr);
}

// clear the basetramp jump to a minitramp
void clearBaseBranch(process *proc, const instInstance *inst, callWhen when)
{
    Address addr;

    addr = inst->baseInstance->baseAddr;
    if (when == callPreInsn) {
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
    if (when == callPreInsn) {
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

pdvector<instWaitingList *> instWList;

// Shouldn't this be a member fn of class process?
// writes into miniTrampHandle
loadMiniTramp_result addInstFunc(miniTrampHandle *mtHandle_save, process *proc,
			    instPoint *&location,
			    AstNode *&ast, // ast may change (sysFlag stuff)
			    callWhen when, callOrder order,
			    bool noCost,
			    bool trampRecursiveDesired)
{
   returnInstance *retInstance = NULL;
   // describes how to jmp to the base tramp

   instInstance *mtInfo = new instInstance;
   loadMiniTramp_result res = loadMiniTramp(mtInfo, proc, location, ast, 
					    when, order, noCost, retInstance,
					    trampRecursiveDesired);
   
   miniTrampHandle mtHandle;
   mtHandle.inst     = mtInfo;
   mtHandle.location = location;
   mtHandle.when     = when;
   
   if(res == success_res) {
      hookupMiniTramp(proc, mtHandle, order);
   } else {
      delete mtInfo;
      mtHandle.inst = NULL;
   }
   if (retInstance) {
      // Looking at the code for the other addInstFunc below, it seems that
      // this will always be true...retInstance is never NULL.
      
      retInstance->installReturnInstance(proc);
      // writes to addr space
   }
   
   // delete retInstance; 
   // safe if NULL (may have been alloc'd by findAndInstallBaseTramp)

   installed_miniTramps_list *mtList;
   proc->getMiniTrampList(mtHandle.location, when, &mtList);
   (*mtHandle_save) = mtHandle;
   return res;
}

// Shouldn't this be a member fn of class process?
// The trampRecursiveDesired flag decides which base tramp is used,
// _if_ a base tramp is installed. If there is already a base tramp
// at the instrumentation point, the flag is ignored.

// writes to *mtInfo
loadMiniTramp_result loadMiniTramp(instInstance *mtInfo, process *proc, 
			    instPoint *&location,
		            AstNode *&ast, // the ast could be changed 
		            callWhen when, callOrder order, bool noCost,
		            returnInstance *&retInstance,
		            bool trampRecursiveDesired)
{
   // retInstance gets filled in with info on how to jmp to the base tramp
   // (the call to findAndInstallBaseTramp doesn't do that)
   assert(proc && location);
   initTramps();

   if(mtInfo->ID == instInstance::uninitialized_id) 
     mtInfo->ID = instInstance::get_new_id();

   bool deferred = false;  // dummy variable
   mtInfo->baseInstance = findAndInstallBaseTramp(proc, location, 
					    retInstance, trampRecursiveDesired,
					    noCost, deferred);
   if (! mtInfo->baseInstance) {
      if(deferred) return deferred_res;
      else         return failure_res;
   }
#if defined(MT_DEBUG_ON)
   sprintf(errorLine,"==>BaseTramp is in 0x%lx\n",
	   mtInfo->baseInstance->baseAddr);
   logLine(errorLine);
#endif

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
   
   /* VG(11/06/01): Added location, needed by effective address AST node */
   mtInfo->returnAddr = ast->generateTramp(proc, (const instPoint *)location,
				      (char *)insn, count, &trampCost, noCost);

#if defined(DEBUG)
   cerr << endl << endl << endl << "mini tramp: " << endl;
   for (unsigned i = 0; i < count/4; i++)
      fprintf(stderr, "0x%x,\n", insn[i]);
#endif
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)
   // The ast node might be shared, so remove the changes made to
   // ast node tree  
   if (isOpt)
      ast->optRetVal(NULL); 
#endif

   mtInfo->cost = trampCost; 
   if(trampCost > 0)
      mtInfo->baseInstance->updateTrampCost(proc, trampCost);

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
   // Let's be intelligent about near_. If we've got any other minitramp,
   // try and allocate near that one. This is because we use single-instruction
   // jumps between minis.
   Address near_;
#if defined(rs6000_ibm_aix4_1)
   // On AIX, the branch from base to mini is limitless --
   // we use a register branch. Mini to mini is limited.
   // So cluster the minis in their own space.
   installed_miniTramps_list *mtList;
   proc->getMiniTrampList(location, when, &mtList);

   if (mtList->numMiniTramps() == 0) // We are the only mini in the chain
      near_ = 0;
   else // Other minis. so let's try and get near them
      if (order == orderLastAtPoint)
	 near_ = mtList->getLastMT()->returnAddr;
      else // First at point
	 near_ = mtList->getFirstMT()->trampBase;
#else
   // Non-AIX, old behavior
   near_ = mtInfo->baseInstance->baseAddr;
#endif
   bool err = false;

	//ccw 30 jul 2002 
	//the following allows the aix inferiorMalloc to
	//work nicely with save the world.  If we request
	//a minitramp to be in the text section (requestTextMiniTramp)
	//then the given params are ignored and we request as such.
	//
	//more importantly, if we are attaching to a series of minitramps
	//that have been allocated in the text heap then request this
	//one to be in the text heap as well.
	//
	//otherwise just use the arguments given
#if defined(BPATCH_LIBRARY) && defined(rs6000_ibm_aix4_1)
	if(proc->requestTextMiniTramp || ( (near_ < 0x20000000) && (near_ > 0x0)) ){ 
		mtInfo->trampBase = proc->inferiorMalloc(count,anyHeap/* htype*/, 0x10000000 /*near_*/, &err);
	}else{
		mtInfo->trampBase = proc->inferiorMalloc(count,htype,near_, &err);
	}   
#else
	mtInfo->trampBase = proc->inferiorMalloc(count,htype,near_, &err);
#endif

   //fprintf(stderr, "Got %d bytes at 0x%x\n", count, (*mtInfo)->trampBase);
   
   if (err) {
      cerr << "Returning inst.C:line 369" << endl;
      return failure_res;
   }
   assert(mtInfo->trampBase);

   if (! mtInfo->trampBase) {
      cerr << "Returning inst.C:line 375" << endl;
      return failure_res;
   }
   trampBytes += count;
   mtInfo->returnAddr += mtInfo->trampBase;

   /*
    * Now make the call to actually put the code in place.
    */
   installTramp(mtInfo, proc, (char *)insn, count, location, when);
   
   return success_res;
}

// returns the address of the new instInstance (the one to use)
// this new instInstance is a copied from the one passed in, into the data
// structures
void hookupMiniTramp(process *proc, const miniTrampHandle &mtHandle,
		     callOrder order) {
   instInstance *firstAtPoint = NULL;
   instInstance *lastAtPoint = NULL;
   instInstance *inst = mtHandle.inst;
   installed_miniTramps_list *mtList = NULL;
   const callWhen &when = mtHandle.when;

   proc->getMiniTrampList(mtHandle.location, when, &mtList);

   if(mtList == NULL) {
      proc->newMiniTrampList(mtHandle.location, when, &mtList);
   } else if(order == orderFirstAtPoint) {
      firstAtPoint = mtList->getFirstMT();
   } else if(order == orderLastAtPoint) {
      lastAtPoint = mtList->getLastMT();
   }
   mtList->addMiniTramp(order, inst);

   if (mtList->numMiniTramps() == 1) {
      // jump from the minitramp back to the basetramp
#if defined(rs6000_ibm_aix4_1)
      resetBR(proc, inst->returnAddr);
#else
      Address toAddr = getBaseReturnAddr(proc, inst, when);
      //fprintf(stderr, "1-  Branch from 0x%x to 0x%x\n",
      //      inst->returnAddr, toAddr);
      generateBranch(proc, inst->returnAddr, toAddr);
#endif

      // jump from the base tramp to the minitramp
      Address fromAddr = getBaseBranchAddr(proc, inst, when);
#if defined(rs6000_ibm_aix4_1)
      resetBRL(proc, fromAddr, inst->trampBase);
#else
      //fprintf(stderr, "1-  Branch from 0x%x to 0x%x\n",
      //      fromAddr, inst->trampBase);
      generateBranch(proc, fromAddr, inst->trampBase);
#endif

      // just activated this slot.
      //activeSlots->value += 1.0;
   } else if (order == orderLastAtPoint) {
      /* patch previous tramp to call us rather than return */
      //fprintf(stderr, "2-  Branch from 0x%x to 0x%x\n",
      //      lastAtPoint->returnAddr, inst->trampBase);

      generateBranch(proc, lastAtPoint->returnAddr, inst->trampBase);
      
      // jump from the minitramp to the basetramp
#if defined(rs6000_ibm_aix4_1)
      resetBR(proc, inst->returnAddr);
#else
      Address toAddr = getBaseReturnAddr(proc, inst, when);
      //fprintf(stderr, "2-  Branch from 0x%x to 0x%x\n",
      //      inst->returnAddr, toAddr);
      generateBranch(proc, inst->returnAddr, toAddr);
#endif
   } else if(order == orderFirstAtPoint) {
      /* branch to the old first one */
      //fprintf(stderr, "3-  Branch from 0x%x to 0x%x\n",
      //     inst->returnAddr, firstAtPoint->trampBase);
      generateBranch(proc, inst->returnAddr, firstAtPoint->trampBase);
      
      /* base tramp branches to us */
      Address fromAddr = getBaseBranchAddr(proc, inst, when);
#if defined(rs6000_ibm_aix4_1)
      resetBRL(proc, fromAddr, inst->trampBase);
#else
      //fprintf(stderr, "3-  Branch from 0x%x to 0x%x\n",
      //      fromAddr, inst->trampBase);
      generateBranch(proc, fromAddr, inst->trampBase);      
#endif
   } else {
      assert(false);  // shouldn't get here
   }
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

trampTemplate *findBaseTramp(const instPoint * ip, const process *proc) {
   const installed_miniTramps_list *mtListBef;
   proc->getMiniTrampList(ip, callPreInsn, &mtListBef);
   
   if(mtListBef != NULL) {
      const List<instInstance*>::iterator curMT = mtListBef->get_begin_iter();
      const List<instInstance*>::iterator endMT = mtListBef->get_end_iter();

      for(; curMT != endMT; curMT++) {
	 const instInstance *inst = *curMT;
	 return inst->baseInstance;
      }
   }

   const installed_miniTramps_list *mtListAft;
   proc->getMiniTrampList(ip, callPostInsn, &mtListAft);

   if(mtListAft != NULL) {
      const List<instInstance*>::iterator curMT = mtListAft->get_begin_iter();
      const List<instInstance*>::iterator endMT = mtListAft->get_end_iter();

      for(; curMT != endMT; curMT++) {
	 const instInstance *inst = *curMT;
	 return inst->baseInstance;
      }
   }

   return NULL;
}

void getMiniTrampsAtPoint(process *proc, instPoint *loc, callWhen when,
			  pdvector<miniTrampHandle> *mt_buf) {
  installed_miniTramps_list *mtList = NULL;
  proc->getMiniTrampList(loc, when, &mtList);
  if(mtList == NULL) {
     return;  // no minitramps found for this point
  }

  miniTrampHandle handle;
  handle.location = loc;
  handle.when     = when;

  List<instInstance*>::iterator curMT = mtList->get_begin_iter();
  List<instInstance*>::iterator endMT = mtList->get_end_iter();	 
  for(; curMT != endMT; curMT++) {
     instInstance *inst = *curMT;
     handle.inst = inst;
     (*mt_buf).push_back(handle);
  }
}

// writes into childMT the miniTrampHandle for the instrumentation in given
// child process which was inherited from the given parentMT miniTrampHandle
// in the given parentProc
bool getInheritedMiniTramp(const miniTrampHandle *parentMT, 
			   miniTrampHandle *childMT, process *childProc) {
  installed_miniTramps_list *mtList = NULL;
  childProc->getMiniTrampList(parentMT->location, parentMT->when,  &mtList);

  List<instInstance*>::iterator curMT = mtList->get_begin_iter();
  List<instInstance*>::iterator endMT = mtList->get_end_iter();	 
  instInstance *instInParentProc = parentMT->inst;
  instInstance *foundII = NULL;
  for(; curMT != endMT; curMT++) {
    instInstance *instInChildProc = *curMT;
    if(instInParentProc->ID == instInChildProc->ID) {
      foundII = instInChildProc;
      break;
    }
  }
  if(foundII == NULL) return false;
  
  (*childMT).inst = foundII;
  (*childMT).when = parentMT->when;
  (*childMT).location = parentMT->location;
  return true;
}

// This procedure assumes that any mini-tramp for an inst request could refer 
// to any data pointer for that request. A more complex analysis could check 
// what data pointers each mini-tramp really used, but I don't think it is 
// worth the trouble.
//
pdvector<Address> getTrampAddressesAtPoint(process *proc, const instPoint *loc,
					 callWhen when)
{
   pdvector<Address> pointsToCheck;
   
   installed_miniTramps_list *mtList = NULL;
   proc->getMiniTrampList(loc, when,  &mtList);

   if(mtList != NULL && mtList->numMiniTramps()>0) {
      List<instInstance*>::iterator curMT = mtList->get_begin_iter();
      // Base tramp
      pointsToCheck.push_back((*curMT)->baseInstance->baseAddr); 
      pointsToCheck.push_back((*curMT)->trampBase);
      curMT++;

      // abernat, 22MAY02 Isn't this a bit overzealous?
      // At this point we've nuked the jumps, so we should
      // be safe just checking whether any PC is within
      // the range occupied by the minitramp in question,
      // not the entire list.
      
      List<instInstance*>::iterator endMT = mtList->get_end_iter();  
      for(; curMT != endMT; curMT++) {
	 instInstance *inst = *curMT;
	 pointsToCheck.push_back(inst->trampBase);
      }
   }

   return pointsToCheck;
}

/*
 * The tramps are chained together left to right, so we need to find the
 *    tramps to the left anf right of the one to delete, and then patch the
 *    call from the left one to the old to call the right one.
 *    Also we need to patch the return from the right one to go to the left
 *    one.
 *
 * New logic: this routine gaps the minitramp out of the execution
 * sequence, but leaves deletion for later. There is a routine in 
 * the process object which maintains a list of elements to be deleted,
 * and the associated data to ensure that deletion is safe. I've
 * added a callback function to the instInstance class which is 
 * called when deletion actually takes place. This allows recordkeeping
 * for any data which may rely on the minitramp (i.e. Paradyn variables)
 *
 */

// returns true if deleted, false if not deleted (because non-existant
// or already deleted
bool deleteInst(process *proc, const miniTrampHandle &mtHandle)
{
   installed_miniTramps_list *mtList;
   callWhen when = mtHandle.when;
   proc->getMiniTrampList(mtHandle.location, when, &mtList);
   if(mtList == NULL) {
      //cerr << "in inst.C: location is NOT defined" << endl;
      return false;
   }

   // First check: have we started to delete this guy already?
   // This happens when we try to delete an instInstance and GC it
   // We then pause the process, but if the process is exited Paradyn
   // tries to disable all instrumentation... thus causing this one
   // to be deleted again. Sigh. 
   
   // Better fix: figure out why we're double-deleting instrCodeNodes.
   
   if (proc->checkIfInstAlreadyDeleted(mtHandle.inst))
     return false;

   List<instInstance*>::iterator curMT = mtList->get_begin_iter();
   List<instInstance*>::iterator endMT = mtList->get_end_iter();	 
   instInstance *thisMT = NULL;
   instInstance *prevMT = NULL;
   instInstance *nextMT = NULL;

   for(; curMT != endMT; curMT++) {
      instInstance *inst = *curMT;
      if(inst == mtHandle.inst && inst->ID == mtHandle.inst->ID) {
	 thisMT = inst;
	 curMT++;
	 if(curMT == endMT) nextMT = NULL;
	 else               nextMT = *curMT;
	 break;
      }
      prevMT = inst;
   }
   if(thisMT == NULL)
      return false;   // must have already been deleted

   if(proc->status() != exited) {
      bool noOtherMTsAtPoint = (prevMT==NULL && nextMT==NULL);
      if(noOtherMTsAtPoint) {
	 clearBaseBranch(proc, thisMT, when);
	 //activeSlots->value -= 1.0;
      } else {
	 if(prevMT) {
	    if (nextMT) {
	       /* set left's return insn to branch to tramp to the right */
	       generateBranch(proc, prevMT->returnAddr,nextMT->trampBase);
	    } else {
	       /* branch back to the correct point in the base tramp */
#if defined(rs6000_ibm_aix4_1)
	       resetBR(proc, prevMT->returnAddr);
#else
	       Address toAddr = getBaseReturnAddr(proc, thisMT, when);
	       generateBranch(proc, prevMT->returnAddr, toAddr);
#endif
	    }
	 } else {
	    /* thisMT is first one make code call right tramp */
	    int fromAddr;
	    fromAddr = getBaseBranchAddr(proc, nextMT, when);
#if defined(rs6000_ibm_aix4_1)
	    resetBRL(proc, fromAddr, nextMT->trampBase);
#else
	    generateBranch(proc, fromAddr, nextMT->trampBase);
#endif
	 }
      }     
   }

   int trampCost = 0 - (thisMT->cost);
   if(thisMT->cost > 0)
      thisMT->baseInstance->updateTrampCost(proc, trampCost);

   // DON'T delete the instInstance. When it is deleted, the callback
   // is made... which should only happen when the memory is freed.
   // Place it on the list to be deleted.
   proc->deleteInstInstance(thisMT);
   /* remove instInstance from linked list */
   mtList->deleteMiniTramp(thisMT);  // deletes instInstance

#ifdef BPATCH_LIBRARY
   trampTemplate *baseInstance = thisMT->baseInstance;

   if(BPatch::bpatch->baseTrampDeletion() && mtList->numMiniTramps()==0)
   {
      extern bool deleteBaseTramp(process*, instPoint*, trampTemplate*,
				  instInstance *lastMT);
      instPoint *loc = mtHandle.location;
      if(deleteBaseTramp(proc, loc, baseInstance, thisMT)) {
          proc->removeMiniTrampList(loc, when);
          proc->baseMap.undef((const instPoint*)(loc));
      }
   }   
#endif
   return true;
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





