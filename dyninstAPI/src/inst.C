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

// $Id: inst.C,v 1.119 2004/03/31 04:09:21 tikir Exp $
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

int miniTrampHandle::_id = 1;

dictionary_hash <pdstring, unsigned> primitiveCosts(pdstring::hash);

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
    
#ifndef CHECK_ALL_CALL_POINTS
    // JAW -- need to checkCallPoints() here to ensure that the
    // vector "calls" has been fully initialized/filtered/classified.
    //
    //
    checkCallPoints();
#endif

    // possible algorithm : iterate over calls (vector of instPoint *)
    //   for each elem : use getCallee() to get statically determined
    //   callee....
    for(u=0;u<calls.size();u++) {
        //this call to getCallee is platform specific
        f = dynamic_cast<function_base *>(calls[u]->getCallee());
        
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

Address miniTrampHandle::getBaseBranchAddr() const
{
    Address fromAddr;

    // Start of the base tramp
    fromAddr = baseTramp->baseAddr;

    // Now get the correct offset based on our callWhen type
    if(when == callPreInsn) {
        fromAddr += baseTramp->localPreOffset;
    } else {
        fromAddr += baseTramp->localPostOffset;
    }
    return(fromAddr);
}

// get the address in the base tramp where the minitramp should return to
Address miniTrampHandle::getBaseReturnAddr() const 
{

    Address returnAddr = baseTramp->baseAddr;
    if(when == callPreInsn) {
      returnAddr += baseTramp->localPreReturnOffset;
    } else {
      returnAddr += baseTramp->localPostReturnOffset;
    }
    return(returnAddr);
}

// clear the basetramp jump to a minitramp
void clearBaseBranch(const miniTrampHandle *mini)
{
    process *proc = mini->baseTramp->proc;
    
    Address branchAddr = mini->getBaseBranchAddr();

#if defined(rs6000_ibm_aix4_1)
    resetBRL(proc, branchAddr, 0);
#else
    generateNoOp(proc, branchAddr);
#endif
    
    // If there is no instrumentation at this point, skip.
    Address fromAddr, toAddr;

    int trampCost;
    trampTemplate *baseTramp = mini->baseTramp;
    
    if (mini->when == callPreInsn) {
        fromAddr = baseTramp->baseAddr + baseTramp->skipPreInsOffset;
        toAddr = baseTramp->baseAddr + 
        // Note: while this skips the register save/restore, it also skips the
        // updateCost section. Suggestion: break the updateCost section into
        // explicit pre- and post- sections and move it entirely into the
        // save/restore pair
#if defined(rs6000_ibm_aix4_1) || defined(alpha_dec_osf4_0) || defined(sparc_sun_solaris2_4)
               baseTramp->emulateInsOffset;
#else
               baseTramp->updateCostOffset;
#endif
      baseTramp->prevInstru = false;
      trampCost = -(baseTramp->prevBaseCost);
    }
    else {
      fromAddr = baseTramp->baseAddr + 
                 baseTramp->skipPostInsOffset; 
      toAddr = baseTramp->baseAddr + 
               baseTramp->returnInsOffset;
      baseTramp->postInstru = false;
      trampCost = -(baseTramp->postBaseCost);
    }
    baseTramp->updateTrampCost(trampCost);
    generateBranch(proc,fromAddr,toAddr);
#if defined(MT_DEBUG)
    sprintf(errorLine,"generating branch from address 0x%lx to address 0x%lx"
        " - CLEAR\n",fromAddr,toAddr);
    logLine(errorLine);
#endif
}

#if defined( ia64_unknown_linux2_4 )
/* __attribute__((aligned)) is a gcc-ism that makes it possible to directly
   disassemble generated code in the mutator, before installation. */
static ia64_bundle_t insn[65536/sizeof(ia64_bundle_t)] __attribute__((aligned));
#else
// implicit assumption that tramps generate to less than 64K bytes!!!
static int insn[65536/sizeof(int)]; // Made into array of int so it would be
				    // aligned correctly on platforms that
				    // need it to be (like SPARC) - BRB
#endif
pdvector<instWaitingList *> instWList;

// Shouldn't this be a member fn of class process?
// writes into miniTrampHandle
loadMiniTramp_result addInstFunc(process *proc, miniTrampHandle * &mtHandle,
                                 instPoint *&location,
                                 AstNode *&ast, // ast may change (sysFlag stuff)
                                 callWhen when, callOrder order,
                                 bool noCost,
                                 bool trampRecursiveDesired)
{
   returnInstance *retInstance = NULL;
   // describes how to jmp to the base tramp
   loadMiniTramp_result res = loadMiniTramp(mtHandle, proc, location, ast, 
                                            when, order, noCost, retInstance,
                                            trampRecursiveDesired);
   
   if(res == success_res) {
       hookupMiniTramp(proc, mtHandle, order);
   } else {
       fprintf(stderr, "Failed to install minitramp\n");
       assert(mtHandle == NULL);
   }
   if (retInstance) {
       // Only true if a new base tramp was installed
       retInstance->installReturnInstance(proc);
      // writes to addr space
   }
   
   return res;
}

// Shouldn't this be a member fn of class process?
// The trampRecursiveDesired flag decides which base tramp is used,
// _if_ a base tramp is installed. If there is already a base tramp
// at the instrumentation point, the flag is ignored.

// writes to *mtInfo
loadMiniTramp_result loadMiniTramp(miniTrampHandle *&mtHandle, // filled in
                                   process *proc, 
                                   instPoint *&location,
                                   AstNode *&ast, // the ast could be changed 
                                   callWhen when,
#if defined(rs6000_ibm_aix4_1)
                                   callOrder order,
#else
                                   callOrder,
#endif
                                   bool noCost, returnInstance *&retInstance,
                                   bool trampRecursiveDesired)
{
   // retInstance gets filled in with info on how to jmp to the base tramp
   // (the call to findAndInstallBaseTramp doesn't do that)
   assert(proc && location);
   initTramps(proc->multithread_capable());

   mtHandle = new miniTrampHandle();
   mtHandle->ID = miniTrampHandle::get_new_id();
   mtHandle->when = when;
   
   bool deferred = false;  // dummy variable
   // This fills in the baseTramp member of location
   mtHandle->baseTramp = findOrInstallBaseTramp(proc, location, 
                                                retInstance, trampRecursiveDesired,
                                                noCost, deferred);
   if (!mtHandle->baseTramp) 
   {
       delete mtHandle;
       fprintf(stderr, "No base tramp!\n");
       mtHandle = NULL;

       if(deferred) return deferred_res;
       else         return failure_res;
   }
#if defined(MT_DEBUG_ON)
   sprintf(errorLine,"==>BaseTramp is in 0x%lx\n",
           location->baseTramp->baseAddr);
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
   ast->sysFlag(location);  

   // If the return value is in the case of compiler optimization,
   // modify the ast node tree to insert an instruction to get the 
   // return the value
   extern bool processOptimaRet(instPoint *location, AstNode *&ast);
   bool isOpt = processOptimaRet(location, ast);
#endif

   int trampCost = 0;
   
   /* VG(11/06/01): Added location, needed by effective address AST node */
   // returnAddr is an absolute address -- currently an offset, fixed below.
   mtHandle->returnAddr = ast->generateTramp(proc, (const instPoint *)location,
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

   mtHandle->cost = trampCost; 
   if(trampCost > 0)
       mtHandle->baseTramp->updateTrampCost(trampCost);

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
   inferiorHeapType htype = (proc->splitHeaps) ? ((inferiorHeapType) (textHeap | uncopiedHeap)) : (anyHeap);
#endif
   // Let's be intelligent about near_. If we've got any other minitramp,
   // try and allocate near that one. This is because we use single-instruction
   // jumps between minis.
   Address near_;
#if defined(rs6000_ibm_aix4_1)
   // On AIX, the branch from base to mini is limitless --
   // we use a register branch. Mini to mini is limited.
   // So cluster the minis in their own space.
   miniTramps_list *other_minis;

   other_minis = mtHandle->baseTramp->getMiniTrampList(when);

   if (other_minis == NULL) {

       // Arbitrary address in the text heap
       near_ = 0x1e000000;
   }
   else // Other minis. so let's try and get near them
       if (order == orderLastAtPoint) {
           near_ = other_minis->getLastMT()->returnAddr;
       }
       else {
           // First at point
           near_ = other_minis->getFirstMT()->miniTrampBase;
       }
#else
   // Place the minitramp near the base tramp. 
   near_ = mtHandle->baseTramp->baseAddr;
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
#if defined(bug_aix_proc_broken_fork)
       // We need the fork minitramp to go in the data heap
   if (mtHandle->baseTramp->location->pointFunc()->prettyName() == pdstring("__fork")) {
       mtHandle->miniTrampBase = proc->inferiorMalloc(count, (inferiorHeapType) (dataHeap | textHeap));
   }
   else
#endif
       if(proc->requestTextMiniTramp || ( (near_ < 0x20000000) && (near_ > 0x0)) ){ 
           mtHandle->miniTrampBase = proc->inferiorMalloc(count,anyHeap/* htype*/, 0x10000000 /*near_*/, &err);
       }else{
           mtHandle->miniTrampBase = proc->inferiorMalloc(count,htype,near_, &err);
       }   
#else
   mtHandle->miniTrampBase = proc->inferiorMalloc(count,htype,near_, &err);
#endif
   
    //fprintf(stderr, "Got %d bytes at 0x%x, near 0x%x\n", count, mtHandle->trampBase, near_);
   
    if (err) {
        cerr << "Returning inst.C: failed allocate" << endl;
        delete mtHandle;
        mtHandle = NULL;
        return failure_res;
    }
    assert(mtHandle->miniTrampBase);

    trampBytes += count;
    // Change returnAddr from offset to absolute
    mtHandle->returnAddr += mtHandle->miniTrampBase;

   /*
    * Now make the call to actually put the code in place.
    */
   installTramp(mtHandle, proc, (char *)insn, count);
   
   // And add the mini tramp to the code range tree
   proc->addCodeRange(mtHandle->miniTrampBase,
                      mtHandle);
   
   return success_res;
}

// Install the physical jumps to the new minitramp
void hookupMiniTramp(process *proc, miniTrampHandle *&mtHandle,
                     callOrder order) {
    miniTrampHandle *firstAtPoint = NULL;
    miniTrampHandle *lastAtPoint = NULL;
    
    const callWhen &when = mtHandle->when;
    
    miniTramps_list *mtList = mtHandle->baseTramp->getMiniTrampList(when);
    
    if(mtList == NULL) {
        mtList = new miniTramps_list;
        if (when == callPreInsn)
            mtHandle->baseTramp->pre_minitramps = mtList;
        else {
            mtHandle->baseTramp->post_minitramps = mtList;
        }
    } else if(order == orderFirstAtPoint) {
        firstAtPoint = mtList->getFirstMT();
    } else if(order == orderLastAtPoint) {
        lastAtPoint = mtList->getLastMT();
    }
    mtList->addMiniTramp(order, mtHandle);

#if defined(sparc_sun_solaris2_4)
	extern void generateBranchOrCallNoSaveRestore(process *proc,Address fromAddr, Address toAddr);
#endif

    if (mtList->numMiniTramps() == 1) {
        // jump from the minitramp back to the basetramp
        Address toAddr = mtHandle->getBaseReturnAddr();
        //fprintf(stderr, "1-  Branch from 0x%x to 0x%x\n",
        //mtHandle->returnAddr, toAddr);
#if defined(rs6000_ibm_aix4_1)
        // Jump to link register
        resetBR(proc, mtHandle->returnAddr);
#elif defined(sparc_sun_solaris2_4)
		generateBranchOrCallNoSaveRestore(proc,mtHandle->returnAddr,toAddr);
#else 
        generateBranch(proc, mtHandle->returnAddr, toAddr);
#endif
        
        // jump from the base tramp to the minitramp
        Address fromAddr = mtHandle->getBaseBranchAddr();
        //fprintf(stderr, "1-  Branch from 0x%x to 0x%x\n",
        //fromAddr, mtHandle->miniTrampBase);
#if defined(rs6000_ibm_aix4_1)
        resetBRL(proc, fromAddr, mtHandle->miniTrampBase);
#elif defined(sparc_sun_solaris2_4)
		generateBranchOrCallNoSaveRestore(proc, fromAddr, mtHandle->miniTrampBase);
#else
        generateBranch(proc, fromAddr, mtHandle->miniTrampBase);
#endif
        // just activated this slot.
        //activeSlots->value += 1.0;
    } else if (order == orderLastAtPoint) {
        /* patch previous tramp to call us rather than return */
        //fprintf(stderr, "2-  Branch from 0x%x to 0x%x\n",
        //lastAtPoint->returnAddr, mtHandle->miniTrampBase);
        
#if defined(sparc_sun_solaris2_4)
        generateBranchOrCallNoSaveRestore(proc, lastAtPoint->returnAddr, mtHandle->miniTrampBase);
#else
        generateBranch(proc, lastAtPoint->returnAddr, mtHandle->miniTrampBase);
#endif
        
        // jump from the minitramp to the basetramp
        Address toAddr = mtHandle->getBaseReturnAddr();
        //fprintf(stderr, "2-  Branch from 0x%x to 0x%x\n",
        //mtHandle->returnAddr, toAddr);
#if defined(rs6000_ibm_aix4_1)
        resetBR(proc, mtHandle->returnAddr);
#elif defined(sparc_sun_solaris2_4)
        generateBranchOrCallNoSaveRestore(proc, mtHandle->returnAddr, toAddr);
#else
        generateBranch(proc, mtHandle->returnAddr, toAddr);
#endif
    } else if(order == orderFirstAtPoint) {
        /* branch to the old first one */
        //fprintf(stderr, "3- Branch from 0x%x to 0x%x\n",
        //      mtHandle->returnAddr, firstAtPoint->miniTrampBase);
        
#if defined(sparc_sun_solaris2_4)
        generateBranchOrCallNoSaveRestore(proc, mtHandle->returnAddr, firstAtPoint->miniTrampBase);
#else
        generateBranch(proc, mtHandle->returnAddr, firstAtPoint->miniTrampBase);
#endif
        
        /* base tramp branches to us */
        Address fromAddr = mtHandle->getBaseBranchAddr();
        //fprintf(stderr, "3-  Branch from 0x%x to 0x%x\n",
        //fromAddr, mtHandle->miniTrampBase);
#if defined(rs6000_ibm_aix4_1)
        resetBRL(proc, fromAddr, mtHandle->miniTrampBase);
#elif defined(sparc_sun_solaris2_4)
        generateBranchOrCallNoSaveRestore(proc, fromAddr, mtHandle->miniTrampBase);
#else
        generateBranch(proc, fromAddr, mtHandle->miniTrampBase);      
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

// Given a miniTrampHandle parentMT, find the equivalent in the child
// process (matching by the ID member). Fill in childMT.

bool getInheritedMiniTramp(const miniTrampHandle *parentMT, 
                           miniTrampHandle * &childMT, 
                           process *childProc) {

    const instPoint *point = parentMT->baseTramp->location;
    assert(point);

    trampTemplate *childBase = childProc->baseMap[point];
    assert(childBase);
    
    miniTramps_list *mtList = childBase->getMiniTrampList(parentMT->when);

    if( mtList == NULL )
    {
        // This could be an assert... unsure -- bernat, 4OCT03
        return false;
    }

    List<miniTrampHandle*>::iterator curMT = mtList->get_begin_iter();
    List<miniTrampHandle*>::iterator endMT = mtList->get_end_iter();	 

    for(; curMT != endMT; curMT++) {
        if ((*curMT)->ID == parentMT->ID) {
            // Found it. 
            childMT = *curMT;
            return true;
        }
    }
    return false;
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
bool deleteInst(process *proc, miniTrampHandle *&mtHandle)
{
    
    callWhen when = mtHandle->when;
    miniTramps_list *mtList = mtHandle->baseTramp->getMiniTrampList(when);
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
   
   if (proc->checkIfMiniTrampAlreadyDeleted(mtHandle))
       return false;

   List<miniTrampHandle*>::iterator curMT = mtList->get_begin_iter();
   List<miniTrampHandle*>::iterator endMT = mtList->get_end_iter();	 
   miniTrampHandle *thisMT = NULL;
   miniTrampHandle *prevMT = NULL;
   miniTrampHandle *nextMT = NULL;

   for(; curMT != endMT; curMT++) {
       miniTrampHandle *inst = *curMT;
       if (inst->ID == mtHandle->ID) {           
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

#if defined(sparc_sun_solaris2_4)
	extern void generateBranchOrCallNoSaveRestore(process *proc,Address fromAddr, Address toAddr);
#endif

   if(proc->status() != exited) {
      bool noOtherMTsAtPoint = (prevMT==NULL && nextMT==NULL);
      if(noOtherMTsAtPoint) {
          clearBaseBranch(thisMT);
          //activeSlots->value -= 1.0;
      } else {
          if(prevMT) {
              if (nextMT) {
                  /* set left's return insn to branch to tramp to the right */
#if defined(sparc_sun_solaris2_4)
                  generateBranchOrCallNoSaveRestore(proc, prevMT->returnAddr,nextMT->miniTrampBase);
#else
                  generateBranch(proc, prevMT->returnAddr,nextMT->miniTrampBase);
#endif
              } else {
                  /* branch back to the correct point in the base tramp */
#if defined(rs6000_ibm_aix4_1)
                  resetBR(proc, prevMT->returnAddr);
#else
                  Address toAddr = thisMT->getBaseReturnAddr();
#if defined(sparc_sun_solaris2_4)
                  generateBranchOrCallNoSaveRestore(proc, prevMT->returnAddr, toAddr);
#else
                  generateBranch(proc, prevMT->returnAddr, toAddr);
#endif
#endif
              }
          } else {
              /* thisMT is first one make code call right tramp */
              int fromAddr;
              fromAddr = nextMT->getBaseBranchAddr();
#if defined(rs6000_ibm_aix4_1)
              resetBRL(proc, fromAddr, nextMT->miniTrampBase);
#else
#if defined(sparc_sun_solaris2_4)
              generateBranchOrCallNoSaveRestore(proc, fromAddr, nextMT->miniTrampBase);
#else
              generateBranch(proc, fromAddr, nextMT->miniTrampBase);
#endif
#endif
          }
      }     
   }

   int trampCost = 0 - (thisMT->cost);
   if(thisMT->cost > 0)
       thisMT->baseTramp->updateTrampCost(trampCost);

   // DON'T delete the miniTrampHandle. When it is deleted, the callback
   // is made... which should only happen when the memory is freed.
   // Place it on the list to be deleted.
   proc->deleteMiniTramp(thisMT);
   /* remove miniTrampHandle from linked list */
   // Note: not dangling, because the process knows about it.
   mtList->deleteMiniTramp(thisMT);
   if (mtList->numMiniTramps() == 0) {
       // No minitramps!
       delete mtList;
       if (when == callPreInsn)
           mtHandle->baseTramp->pre_minitramps = NULL;
       else
           mtHandle->baseTramp->post_minitramps = NULL;
       mtList = NULL;
   }
   
#ifdef BPATCH_LIBRARY
   if(BPatch::bpatch->baseTrampDeletion() && (mtList == NULL))
   {
       // Only delete if _BOTH_ miniTrampLists are empty
       trampTemplate *base = mtHandle->baseTramp;

       if (!base->pre_minitramps &&
           !base->post_minitramps) {
           
           if (deleteBaseTramp(proc, base))
               // Remove from the /proc baseMap
               proc->baseMap.undef(base->location);
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
unsigned getPrimitiveCost(const pdstring &name)
{

    static bool init=false;

    if (!init) { init = 1; initPrimitiveCost(); }

    if (!primitiveCosts.defines(name)) {
      return 1;
    } else
      return (primitiveCosts[name]);
}


// find any tags to associate semantic meaning to function
unsigned findTags(const pdstring ) {
  return 0;
#ifdef notdef
  if (tagDict.defines(funcName))
    return (tagDict[funcName]);
  else
    return 0;
#endif
}


void
trampTemplate::updateTrampCost(int trampCost) {
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





