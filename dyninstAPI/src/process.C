/*
 * Copyright (c) 1996-2002 Barton P. Miller
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

// $Id: process.C,v 1.396 2003/03/12 01:50:05 schendel Exp $

extern "C" {
#ifdef PARADYND_PVM
int pvmputenv (const char *);
int pvmendtask();
#endif
}

#include <ctype.h>

#if defined(i386_unknown_solaris2_5)
#include <sys/procfs.h>
#endif
#include "common/h/headers.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/dyninstP.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/dynamiclinking.h"
// #include "paradynd/src/mdld.h"
#include "common/h/Timer.h"
#include "common/h/Time.h"
#include "common/h/timing.h"

#include "dyninstAPI/src/rpcMgr.h"

#ifdef BPATCH_LIBRARY
#include "dyninstAPI/h/BPatch.h"

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)
#include "dyninstAPI/src/writeBackElf.h"
#include "dyninstAPI/src/saveSharedLibrary.h" 
#elif defined(rs6000_ibm_aix4_1)
#include "dyninstAPI/src/writeBackXCOFF.h"
#endif

#else
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "paradynd/src/perfStream.h"
#include "paradynd/src/metricFocusNode.h"
#include "paradynd/src/costmetrics.h"
#include "paradynd/src/mdld.h"
#include "paradynd/src/main.h"
#include "paradynd/src/init.h"
#include "pdutil/h/pdDebugOstream.h"
#include "common/h/int64iostream.h"
#endif

#ifndef BPATCH_LIBRARY
#ifdef PAPI
#include "paradynd/src/papiMgr.h"
#endif
#endif

#ifndef BPATCH_LIBRARY
extern void generateRPCpreamble(char *insn, Address &base, process *proc, 
                                unsigned offset, int tid, unsigned pos);
#endif

#include "common/h/debugOstream.h"

#include "common/h/Timer.h"

unsigned enable_pd_attach_detach_debug = 0;

#if ENABLE_DEBUG_CERR == 1
#define attach_cerr if (enable_pd_attach_detach_debug) cerr
#else
#define attach_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

unsigned enable_pd_inferior_rpc_debug = 0;

#if ENABLE_DEBUG_CERR == 1
#define inferiorrpc_cerr if (enable_pd_inferior_rpc_debug) cerr
#else
#define inferiorrpc_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

unsigned enable_pd_shm_sampling_debug = 0;

#if ENABLE_DEBUG_CERR == 1
#define shmsample_cerr if (enable_pd_shm_sampling_debug) cerr
#else
#define shmsample_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

unsigned enable_pd_fork_exec_debug = 0;

#if ENABLE_DEBUG_CERR == 1
#define forkexec_cerr if (enable_pd_fork_exec_debug) cerr
#else
#define forkexec_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

unsigned enable_pd_signal_debug = 0;

#if ENABLE_DEBUG_CERR == 1
#define signal_cerr if (enable_pd_signal_debug) cerr
#else
#define signal_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

unsigned enable_pd_sharedobj_debug = 0;

#if ENABLE_DEBUG_CERR == 1
#define sharedobj_cerr if (enable_pd_sharedobj_debug) cerr
#else
#define sharedobj_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

#ifndef BPATCH_LIBRARY


unsigned enable_pd_metric_debug = 0;

#if ENABLE_DEBUG_CERR == 1
#define metric_cerr if (enable_pd_metric_debug) cerr
#else
#define metric_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

unsigned enable_pd_samplevalue_debug = 0;

#if ENABLE_DEBUG_CERR == 1
#define sampleVal_cerr if (enable_pd_samplevalue_debug) cerr
#else
#define sampleVal_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

unsigned enable_pd_aggregate_debug = 0;

#if ENABLE_DEBUG_CERR == 1
#define agg_cerr if (enable_pd_aggregate_debug) cerr
#else
#define agg_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

#endif

#define FREE_WATERMARK (hp->totalFreeMemAvailable/2)
#define SIZE_WATERMARK 100
static const timeLength MaxWaitingTime(10, timeUnit::sec());
static const timeLength MaxDeletingTime(2, timeUnit::sec());
unsigned inferiorMemAvailable=0;

unsigned activeProcesses; // number of active processes
pdvector<process*> processVec;
string process::programName;
string process::dyninstRT_name;
string process::pdFlavor;

#ifndef BPATCH_LIBRARY
extern string osName;
#endif

// PARADYND_DEBUG_XXX
int pd_debug_infrpc=0;
int pd_debug_catchup=0;
//

bool reachedLibState(libraryState_t lib, libraryState_t state) { return (lib >= state); }

void setLibState(libraryState_t &lib, libraryState_t state) {
    if (lib > state) cerr << "Error: attempting to revert library state" << endl;
    else lib = state;
}


process *findProcess(int pid) { // make a public static member fn of class process
  unsigned size=processVec.size();
  for (unsigned u=0; u<size; u++)
    if (processVec[u] && processVec[u]->getPid() == pid)
      return processVec[u];
  return NULL;
}

bool waitingPeriodIsOver()
{
  static timeStamp initPrevious = timeStamp::ts1970();
  static timeStamp previous = initPrevious;
  bool waiting=false;
  if (previous == initPrevious) {
    previous=getCurrentTime();
    waiting=true;
  }
  else {
    timeStamp current=getCurrentTime();
    if ( (current-previous) > MaxWaitingTime ) {
      previous=getCurrentTime();
      waiting=true;
    }
  }
  return(waiting);
}
ostream& operator<<(ostream&s, const Frame &f) {
  s << "PC: " << f.pc_ << " FP: " << f.fp_
    << " PID: " << f.pid_;
  if (f.thread_)
    s << " TID: " << f.thread_->get_tid();
  if (f.lwp_)
    s << " LWP: " << f.lwp_->get_lwp_id();
  
  return s;
}

/* AIX method defined in aix.C; hijacked for IA-64's GP. */
#if !defined(rs6000_ibm_aix4_1) && !defined( ia64_unknown_linux2_4 )
Address process::getTOCoffsetInfo(Address /*dest */)
{
  Address tmp = 0;
  assert(0 && "getTOCoffsetInfo not implemented");
  return tmp; // this is to make the nt compiler happy! - naim
}
#else
Address process::getTOCoffsetInfo(Address dest)
{
  // We have an address, and want to find the module the addr is
  // contained in. Given the probabilities, we (probably) want
  // the module dyninst_rt is contained in.
  // I think this is the right func to use

  if (symbols->findFuncByAddr(dest, this))
    return (symbols->getObject()).getTOCoffset();
  if (shared_objects)
    for(u_int j=0; j < shared_objects->size(); j++)
      if (((*shared_objects)[j])->getImage()->findFuncByAddr(dest, this))
#if ! defined(ia64_unknown_linux2_4)
        return (((*shared_objects)[j])->getImage()->getObject()).getTOCoffset();
#else
	{
	/* Entries in the .dynamic section are not relocated. */
	return (((*shared_objects)[j])->getImage()->getObject()).getTOCoffset() +
		((*shared_objects)[j])->getBaseAddress();
	}
#endif
  // Serious error! Assert?
  return 0;
}

#endif

// Windows NT has its own version of the walkStack function in pdwinnt.C

// Note: stack walks may terminate early. In this case, return what we can.
// Relies on the getCallerFrame method in the various <os>.C files

#if !defined(mips_unknown_ce2_11) && !defined(i386_unknown_nt4_0)
bool process::walkStackFromFrame(Frame startFrame,
				 pdvector<Frame> &stackWalk)
{
  Address next_pc = 0;
  Address fpOld   = 0;
  Address fpNew   = 0;

  Frame currentFrame = startFrame;

#ifndef BPATCH_LIBRARY
  startTimingStackwalk();
#endif

  if (status_ == running) {
    cerr << "Error: stackwalk attempted on running process" << endl;
#ifndef BPATCH_LIBRARY
    stopTimingStackwalk();
#endif
    return false;
  }

  // Step through the stack frames
  while (!currentFrame.isLastFrame()) {
    
    // grab the frame pointer
    fpNew = currentFrame.getFP();

    // Check that we are not moving up the stack
    // successive frame pointers might be the same (e.g. leaf functions)
    if (fpOld > fpNew) {
      
      // AIX:
      // There's a signal function in the MPI library that we're not
      // handling properly. Instead of returning an empty stack,
      // return what we have.
      // One thing that makes me feel better: gdb is getting royally
      // confused as well. This sucks for catchup.
      
      // We should check to see if this early exit is warranted.
      return false;
    }
    fpOld = fpNew;
    
    next_pc = currentFrame.getPC();
    stackWalk.push_back(currentFrame);
    if (pd_debug_catchup)
      cerr << "Stack debug: " << currentFrame << endl;
    
    currentFrame = currentFrame.getCallerFrame(this); 
  }
  // Clean up after while loop (push end frame)
  stackWalk.push_back(currentFrame);

#ifndef BPATCH_LIBRARY
  stopTimingStackwalk();
#endif
  return true;
}
#endif

// Return a vector (possibly with one object) of active frames
// in the process

bool process::getAllActiveFrames(pdvector<Frame> &activeFrames)
{
  Frame active;
  bool success = true;
  if (!threads.size()) { // Nothing defined in the thread data structures
    // So use the default LWP instead (Dyninst)
    active = getDefaultLWP()->getActiveFrame();
    if (active == Frame()) { // Hrm.. should getActive return a bool?
      return false;
    }
    activeFrames.push_back(active);
  }
  else { // Iterate through threads
    for (unsigned i = 0; i < threads.size(); i++) {
      active = threads[i]->getActiveFrame();
      if (active == Frame())
	success = true;
      else
	activeFrames.push_back(active);
    }
  }
  return success;
}

bool process::walkStacks(pdvector<pdvector<Frame> >&stackWalks)
{
  pdvector<Frame> stackWalk;
  if (!threads.size()) { // Nothing defined in thread data structures
    if (!getDefaultLWP()->walkStack(stackWalk))
      return false;
    // Use the walk from the default LWP
    stackWalks.push_back(stackWalk);
  }
  else { // Have threads defined
    for (unsigned i = 0; i < threads.size(); i++) {
      if (!threads[i]->walkStack(stackWalk))
         return false;
      stackWalks.push_back(stackWalk);
      stackWalk.resize(0);
    }
  }
  return true;
}

// triggeredInStackFrame is used to determine whether instrumentation
//   added at the specified instPoint/callWhen/callOrder would have been
//   executed based on the supplied pc.
//
// If the pc is within instrumentation for the instPoint, the callWhen
//   and callOrder must be examined.  triggeredInStackFrame will return
//   true if the pc is located after the identified point of
//   instrumentation.
//   
// If the pc is not in instrumentation for the supplied instPoint, and
//   the instPoint is located at the function entry or if the instPoint
//   is for preInsn at a function call and the pc is located at the
//   return address of the callsite (indicating that the call is
//   currently executing), triggeredInStackFrame will return true.

bool process::triggeredInStackFrame(instPoint* point,  Frame frame, 
				    callWhen when, callOrder order)
{
  //this->print(stderr, ">>> triggeredInStackFrame(): ");
  trampTemplate *tempTramp;
  bool retVal = false;
  pd_Function *instPoint_fn = dynamic_cast<pd_Function *>
                   (const_cast<function_base *>(point->iPgetFunction()));
  pd_Function *stack_fn;
  stack_fn = findAddressInFuncsAndTramps(frame.getPC());
  if (pd_debug_catchup) {
     char line[200];
     bool didA = false;
     if (stack_fn) {
	pdvector<string> name = stack_fn->prettyNameVector();
	if (name.size()) {
           sprintf(line, "stack_func: %-20.20s ", name[0].c_str());
           didA = true;
        }
     }
     if(!didA)  sprintf(line, "stack_func: %-20.20s ", "");

     if (instPoint_fn) {
        pdvector<string> name = instPoint_fn->prettyNameVector();
        strcat(line, "instP_func: ");
        if (name.size())
           strcat(line, name[0].c_str());
     }
     cerr << "triggeredInStackFrame- " << line << endl;
  }
  if (stack_fn != instPoint_fn) {
    return false;
  }
  Address pc = frame.getPC();
  if ( pd_debug_catchup )
     cerr << "  Stack function matches function containing instPoint" << endl;

  if (pc == point->iPgetAddress()) {
      if (pd_debug_catchup) {
          fprintf(stderr, "Found pc at start of instpoint, returning false\n");
      }
      return false;
  }
  
  
  //  Is the pc within the instPoint instrumentation?
  instPoint* currentIp = findInstPointFromAddress(pc);

  if ( currentIp && currentIp == point )
  {
    tempTramp = findBaseTramp(currentIp, this);

    if ( tempTramp )
    {
      //  Check if pc in basetramp
        if ( tempTramp->inBasetramp(pc) )
      {
        if ( pd_debug_catchup )
        {
          cerr << "  Found pc in BaseTramp" << endl;
          fprintf(stderr, "    baseTramp range is (%lx - %lx)\n",
                  tempTramp->baseAddr,
                  (tempTramp->baseAddr + tempTramp->size));
          fprintf(stderr, "    localPreReturnOffset is %lx\n",
                  tempTramp->baseAddr+tempTramp->localPreReturnOffset);
          fprintf(stderr, "    localPostReturnOffset is %lx\n",
                  tempTramp->baseAddr+tempTramp->localPostReturnOffset);
        }
        
        if ( (when == callPreInsn && 
            pc >= tempTramp->baseAddr+tempTramp->localPreReturnOffset) ||
          (when == callPostInsn &&
            pc >= tempTramp->baseAddr+tempTramp->localPostReturnOffset) )
        {
          if ( pd_debug_catchup )
            cerr << "  pc is after requested instrumentation point, returning true." << endl;
          retVal = true;
        }
        else
        {
          if ( pd_debug_catchup )
            cerr << "  pc is before requested instrumentation point, returning false." << endl;
        }
      }
      else //  pc is in a mini-tramp
      {
	installed_miniTramps_list *mtList;
	getMiniTrampList(currentIp, when, &mtList);
	List<instInstance*>::iterator curMT = mtList->get_begin_iter();
	List<instInstance*>::iterator endMT = mtList->get_end_iter();	 

        bool pcInTramp = false;

	for(; curMT != endMT && !pcInTramp; curMT++)
        {
	  instInstance *currInstance = *curMT;
          if ( pd_debug_catchup )
          {
            fprintf(stderr, "  Checking for pc in mini-tramp (%lx - %lx)\n",
                    currInstance->trampBase, currInstance->returnAddr);
          }
          
          if ( pc >= currInstance->trampBase &&
                pc <= currInstance->returnAddr )
          {
            // We have found the mini-tramp that is currently being executed
            pcInTramp = true;

            if ( pd_debug_catchup )
            {
              cerr << "  Found pc in mini-tramp" << endl;
              cerr << "    Requested instrumentation is for ";
              switch(when) {
                case callPreInsn:
                  cerr << "PreInsn ";
                  break;
                case callPostInsn:
                  cerr << "PostInsn ";
                  break;
              }
              switch(order) {
                case orderFirstAtPoint:
                  cerr << "prepend ";
                  break;
                case orderLastAtPoint:
                  cerr << "append ";
                  break;
              }
              cerr << endl;

              cerr << "    The pc is in ";
              switch(when) {
                case callPreInsn:
                  cerr << "PreInsn ";
                  break;
                case callPostInsn:
                  cerr << "PostInsn ";
                  break;
              }
              cerr << "instrumentation\n";
            }
            
            // The request should be triggered if it is for:
            //   1)  pre-instruction instrumentation to prepend
            //   2)  pre-instruction instrumentation to append
            //         and the pc is in PostInsn instrumentation
            //   3)  post-instruction instrumentation to prepend
            //         and the pc is in PostInsn instrumentation
            if ( (when == callPreInsn && (order == orderFirstAtPoint || 
                  (order == orderLastAtPoint &&
                    when == callPostInsn))) ||
                 (when == callPostInsn && order == orderFirstAtPoint &&
		    when == callPostInsn) )
            {
              if ( pd_debug_catchup )
                cerr << "  pc is after requested instrumentation point, returning true." << endl;
              retVal = true;
            }
            else
            {
              if ( pd_debug_catchup )
                cerr << "  pc is before requested instrumentation point, returning false." << endl;
            }
          }
        }
      }
    } 
  }
  else  // pc not in instrumentation
  {
    //  If the instrumentation point is located at the entry of the
    //    function, it would be triggered.
    //  If the instrumentation point is a call site, the instrumentation
    //    is preInsn and the pc points to the return address of the call,
    //    the instrumentation should be triggered as any postInsn instrumentation
    //    will be executed.
#if defined(mips_sgi_irix6_4) || defined(mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001
    if (point->ipType_ == IPT_ENTRY) {
      if ( pd_debug_catchup )
        cerr << "  pc not in instrumentation, requested instrumentation for function entry, returning true." << endl;
      retVal = true;
    } else if (point->ipType_ == IPT_CALL && when == callPreInsn) {
      // check if the $pc corresponds to the native call insn
      Address base;
      getBaseAddress(stack_fn->file()->exec(), base);
      Address native_ra = base + stack_fn->getAddress(0) + point->offset_ + point->size_;
      if (pc == native_ra)
      {
        if ( pd_debug_catchup )
          cerr << "  Requested instrumentation is preInsn for callsite being executed.  Returning true." << endl;
        retVal = true;
      }
      else
      {
        if ( pd_debug_catchup )
          cerr << "  Function at requested preInsn callsite is not being executed.  Returning false." << endl;
      }
    }
    else
    {
      if ( pd_debug_catchup )
        cerr << "  Requested instrumentation point is not appropriate for catchup, returning false." << endl;
    }      
#elif defined(sparc_sun_solaris2_4) || defined(alpha_dec_osf4_0)
    if (point->ipType == functionEntry) {
      if ( pd_debug_catchup )
        cerr << "  pc not in instrumentation, requested instrumentation for function entry, returning true." << endl;
      retVal = true;
    } else if (point->ipType == callSite && when == callPreInsn) {
      // looking at gdb, sparc-solaris seems to record PC of the 
      //  call site + 8, as opposed to the PC of the call site.
      Address base, target;
      getBaseAddress( stack_fn->file()->exec(), base );
      target = base + point->addr + 2 * sizeof(instruction);
      if (pc == target) {
        if ( pd_debug_catchup )
          cerr << "  Requested instrumentation is preInsn for callsite being executed.  Returning true." << endl;
        retVal = true;
      } else {
        trampTemplate *bt = findBaseTramp( point, this );
        Address target = bt->baseAddr + bt->emulateInsOffset + 2 * sizeof(instruction);
        if( pc == target )
        {
          if ( pd_debug_catchup )
            cerr << "  Requested instrumentation is preInsn for callsite being executed.  Returning true." << endl;
          retVal = true;
        }
        else
        {
          if ( pd_debug_catchup )
            cerr << "  Function at requested preInsn callsite is not being executed.  Returning false." << endl;
        }
      }
    }
    else
    {
      if ( pd_debug_catchup )
        cerr << "  Requested instrumentation point is not appropriate for catchup, returning false." << endl;
    }      
#elif defined(rs6000_ibm_aix4_1)
    if ( point->ipLoc == ipFuncEntry ) {
      if ( pd_debug_catchup )
        cerr << "  pc not in instrumentation, requested instrumentation for function entry, returning true." << endl;
      retVal = true;
    } else if ( point->ipLoc == ipFuncCallPoint && when == callPreInsn ) {
      // check if the stack_pc points to the instruction after the call site
      Address base, target;
      getBaseAddress( stack_fn->file()->exec(), base );
      target = base + point->addr + sizeof(instruction);
      //cerr << " stack_pc should be " << (void*)target;
      if ( pc == target ) {
        if ( pd_debug_catchup )
          cerr << "  Requested instrumentation is preInsn for callsite being executed.  Returning true." << endl;
        retVal = true;
      }
      else
      {
        if ( pd_debug_catchup )
          cerr << "  Function at requested preInsn callsite is not being executed.  Returning false." << endl;
      }
      //cerr << endl;
    }
    else
    {
      //if ( pd_debug_catchup )
        //cerr << "  Requested instrumentation point is not appropriate for catchup, returning false." << endl;
    }      
#elif defined(i386_unknown_nt4_0) || defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0)
    if ( point->address() == point->func()->getAddress( this ) ) {
      if ( pd_debug_catchup )
        cerr << "  pc not in instrumentation, requested instrumentation for function entry, returning true." << endl;
      retVal = true;
    } else if ( point->insnAtPoint().isCall() && when == callPreInsn ) {
      // check if the pc points to the instruction after the call site
      Address base, target;
      getBaseAddress( stack_fn->file()->exec(), base );
      target = base + point->address() + point->insnAtPoint().size();
      //cerr << " pc should be " << (void*)target;
      if ( pc == target ) {
        if ( pd_debug_catchup )
          cerr << "  Requested instrumentation is preInsn for callsite being executed." << endl;
        //cerr << " -- HIT";
        retVal = true;
      }
      else
      {
        if ( pd_debug_catchup )
          cerr << "  Function at requested preInsn callsite is not being executed.  Returning false." << endl;
      }
      //cerr << endl;
    }
    else
    {
      if ( pd_debug_catchup )
        cerr << "  Requested instrumentation point is not appropriate for catchup, returning false." << endl;
    }      
#endif
  }

  return retVal;
}


static Address alignAddress(Address addr, unsigned align) {
  Address skew = addr % align;
  return (skew) ? (((addr/align)+1)*align) : addr;
}

// disItem was previously declared const, but heap management
// occasionally deletes such items
bool isFreeOK(process *proc, disabledItem &dis, pdvector<Frame> stackWalk)
{
  Address disPointer = dis.block.addr;
  inferiorHeap *hp = &proc->heap;

#if defined(hppa1_1_hp_hpux)
  if (proc->freeNotOK) return false;
#endif

  heapItem *ptr = NULL;
  if (!hp->heapActive.find(disPointer, ptr)) {
    sprintf(errorLine,"Warning: attempt to free undefined heap entry "
            "0x%p (pid=%d, heapActive.size()=%d)\n", 
            (void*)disPointer, proc->getPid(), 
            hp->heapActive.size());
    logLine(errorLine);
    return false;
  }
  assert(ptr);

  pdvector<addrVecType> &points = dis.pointsToCheck; 
  for (unsigned pci = 0; pci < stackWalk.size(); pci++) {
    Address pc = stackWalk[pci].getPC();
    // Condition 1: PC is inside current block
    if ((pc >= ptr->addr) && (pc <= ptr->addr + ptr->length)) {
      return false;
    }
    
    for (unsigned j = 0; j < points.size(); j++) {
      for (unsigned k = 0; k < points[j].size(); k++) {
        Address predStart = points[j][k]; // start of predecessor code block
        heapItem *pred = NULL;
        if (!hp->heapActive.find(predStart, pred)) {
          // predecessor code already freed: remove from list
          int size = points[j].size();
          points[j][k] = points[j][size-1];
          points[j].resize(size-1);
          k--; // move index back to account for resize()
          continue;
        }
        assert(pred);

        // Condition 2: current block is subset of predecessor block ???
        if ((ptr->addr >= pred->addr) && (ptr->addr <= pred->addr + pred->length)) {
          return false;
        }
        // Condition 3: PC is inside predecessor block
        if ((pc >= pred->addr) && (pc <= pred->addr + pred->length)) {
          return false;     
        }
      }
    }
  }
  return true;
}

extern "C" int heapItemCmpByAddr(const heapItem **A, const heapItem **B)
{
  heapItem *a = *(heapItem **)const_cast<heapItem **>(A);
  heapItem *b = *(heapItem **)const_cast<heapItem **>(B);

  if (a->addr < b->addr) {
      return -1;
  } else if (a->addr > b->addr) {
      return 1;
  } else {
      return 0;
  }
}

void process::inferiorFreeCompact(inferiorHeap *hp)
{
  pdvector<heapItem *> &freeList = hp->heapFree;
  unsigned i, nbuf = freeList.size();

  /* sort buffers by address */
  VECTOR_SORT(freeList, heapItemCmpByAddr);

  /* combine adjacent buffers */
  bool needToCompact = false;
  for (i = 1; i < freeList.size(); i++) {
    heapItem *h1 = freeList[i-1];
    heapItem *h2 = freeList[i];
    assert(h1->length != 0);
    assert(h1->addr + h1->length <= h2->addr);
    if (h1->addr + h1->length == h2->addr
        && h1->type == h2->type) {
      h2->addr = h1->addr;
      h2->length = h1->length + h2->length;
      h1->length = 0;
      nbuf--;
      needToCompact = true;
    }
  }

  /* remove any absorbed (empty) buffers */ 
  if (needToCompact) {
    pdvector<heapItem *> cleanList;
    unsigned end = freeList.size();
    for (i = 0; i < end; i++) {
      heapItem *h1 = freeList[i];
      if (h1->length != 0) {
        cleanList.push_back(h1);
      } else {
        delete h1;
      }
    }
    assert(cleanList.size() == nbuf);
    for (i = 0; i < nbuf; i++) {
      freeList[i] = cleanList[i];
    }
    freeList.resize(nbuf);
    assert(freeList.size() == nbuf);
  }
}

// Get inferior heaps from every library currently loaded.
// This is done at startup by initInferiorHeap().
// There's also another one which takes a shared library and
// only looks in there for the heaps

bool process::getInfHeapList(pdvector<heapDescriptor> &infHeaps)
{
  bool foundHeap = false;
  // First check the program (without shared libs)
  foundHeap = getInfHeapList(symbols, infHeaps);

  // Now iterate through shared libs
  if (shared_objects)
    for(u_int j=0; j < shared_objects->size(); j++)
      {
	if(((*shared_objects)[j]->getImage())){
		if (getInfHeapList(((*shared_objects)[j])->getImage(), infHeaps))
		  foundHeap = true;
	      }
	}

  return foundHeap;
}

bool process::getInfHeapList(const image *theImage, // okay, boring name
                             pdvector<heapDescriptor> &infHeaps)
{

  // First we get the list of symbols we're interested in, then
  // we go through and add them to the heap list. This is done
  // for two reasons: first, the symbol address might be off (depends
  // on the image), and this lets us do some post-processing.
  bool foundHeap = false;
  pdvector<Symbol> heapSymbols;
  Address baseAddr = 0;

  // For maximum flexibility the findInternalByPrefix function
  // returns a list of symbols

  foundHeap = theImage->findInternalByPrefix(string("DYNINSTstaticHeap"),
					     heapSymbols);
  if (!foundHeap)
    // Some platforms preface with an underscore
    foundHeap = theImage->findInternalByPrefix(string("_DYNINSTstaticHeap"),
					       heapSymbols);
  // The address in the symbol isn't necessarily absolute
  getBaseAddress(theImage, baseAddr);
  for (u_int j = 0; j < heapSymbols.size(); j++)
    {
        // The string layout is: DYNINSTstaticHeap_size_type_unique
        // Can't allocate a variable-size array on NT, so malloc
        // that sucker
        char *temp_str = (char *)malloc(strlen(heapSymbols[j].name().c_str())+1);
        strcpy(temp_str, heapSymbols[j].name().c_str());
        char *garbage_str = strtok(temp_str, "_"); // Don't care about beginning
        assert(!strcmp("DYNINSTstaticHeap", garbage_str));
        // Name is as is.
        // If address is zero, then skip (error condition)
        if (heapSymbols[j].addr() == 0)
        {
            cerr << "Skipping heap " << heapSymbols[j].name().c_str()
                 << "with address 0" << endl;
            continue;
        }
        // Size needs to be parsed out (second item)
        // Just to make life difficult, the heap can have an optional
        // trailing letter (k,K,m,M,g,G) which indicates that it's in
        // kilobytes, megabytes, or gigabytes. Why gigs? I was bored.
        char *heap_size_str = strtok(NULL, "_"); // Second element, null-terminated
        unsigned heap_size = (unsigned) atol(heap_size_str);
        if (heap_size == 0)
            /* Zero size or error, either way this makes no sense for a heap */
        {
            free(temp_str);
            continue;
        }
        switch (heap_size_str[strlen(heap_size_str)-1])
        {
      case 'g':
      case 'G':
          heap_size *= 1024;
      case 'm':
      case 'M':
          heap_size *= 1024;
      case 'k':
      case 'K':
          heap_size *= 1024;
      default:
          break;
        }
        
        // Type needs to be parsed out. Can someone clean this up?
        inferiorHeapType heap_type;
        char *heap_type_str = strtok(NULL, "_");
        
        if (!strcmp(heap_type_str, "anyHeap"))
            heap_type = anyHeap;
        else if (!strcmp(heap_type_str, "lowmemHeap"))
            heap_type = lowmemHeap;
        else if (!strcmp(heap_type_str, "dataHeap"))
            heap_type = dataHeap;
        else if (!strcmp(heap_type_str, "textHeap"))
            heap_type = textHeap;
        else
        {
            cerr << "Unknown heap string " << heap_type_str << " read from file!" << endl;
            free(temp_str);
            continue;
        }
        
        infHeaps.push_back(heapDescriptor(heapSymbols[j].name(),
#ifdef mips_unknown_ce2_11 //ccw 13 apr 2001
                                          heapSymbols[j].addr(), 
#else
                                          heapSymbols[j].addr()+baseAddr, 
#endif
                                          heap_size, heap_type));
        free(temp_str);
    }
  return foundHeap;
}

/*
 * Returns true if the address given is within the signal handler function,
 * otherwise returns false.
 */
bool process::isInSignalHandler(Address addr)
{
#if defined(i386_unknown_linux2_0)
  if (addr == signal_restore)
    return true;
  else
    return false;
#else
  if (!signal_handler)
      return false;

  const image *sig_image = (signal_handler->file())->exec();

  Address sig_addr;
  if(getBaseAddress(sig_image, sig_addr)){
    sig_addr += signal_handler->getAddress(this);
  } else {
    sig_addr = signal_handler->getAddress(this);
  }

  if (addr >= sig_addr && addr < sig_addr + signal_handler->size())
    return true;
  else
    return false;
#endif
}

/*
 * This function adds an item to the dataUpdates vector
 * which is used to maintain a list of variables that have
 * been written by the mutator //ccw 26 nov 2001
 */
void process::saveWorldData(Address address, int size, const void* src){
#ifdef BPATCH_LIBRARY
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined(rs6000_ibm_aix4_1)
	if(collectSaveWorldData){  //ccw 16 jul 2002 : NEW LINE
		dataUpdate *newData = new dataUpdate;
		newData->address= address;
		newData->size = size;
		newData->value = new char[size];
		memcpy(newData->value, src, size);
		dataUpdates.push_back(newData);
	}
#else /* Get rid of annoying warnings */
//#if !defined(rs6000_ibm_aix4_1)
	Address tempaddr = address;
	int tempsize = size;
	const void *bob = src;
//#endif
#endif
#endif
}

#ifdef BPATCH_LIBRARY
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)  || defined(rs6000_ibm_aix4_1)
/* || defined(rs6000_ibm_aix4_1)*/

char* process::saveWorldFindDirectory(){

	const char* directoryNameExt = "_dyninstsaved";
	int dirNo = 0;
/* ccw */
	char cwd[1024];
        char* directoryName;
	int lastChar;
	getcwd(cwd, 1024);
	lastChar = strlen(cwd);

	if( cwd[lastChar] != '/' && lastChar != 1023){
		cwd[lastChar] = '/';
		cwd[++lastChar] ='\0';
	}

	directoryName = new char[strlen(cwd) +
                        strlen(directoryNameExt) + 3+1+1];
/* ccw */
	sprintf(directoryName,"%s%s%x",cwd, directoryNameExt,dirNo);
        while(dirNo < 0x1000 && mkdir(directoryName, S_IRWXU) ){
                 if(errno == EEXIST){
                         dirNo ++;
                 }else{
                         BPatch_reportError(BPatchSerious, 122, "dumpPatchedImage: cannot open directory to store mutated binary. No files saved\n");
                         delete [] directoryName;
                         return NULL;
                 }
                 sprintf(directoryName, "%s%s%x",cwd,
                         directoryNameExt,dirNo);
        }
	if(dirNo == 0x1000){
	         BPatch_reportError(BPatchSerious, 122, "dumpPatchedImage: cannot open directory to store mutated binary. No files saved\n");
	         delete [] directoryName;
	         return NULL;
	}
	return directoryName;

}
#endif

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)  
unsigned int process::saveWorldSaveSharedLibs(int &mutatedSharedObjectsSize, 
                                 unsigned int &dyninst_SharedLibrariesSize, 
                                 char* directoryName, unsigned int &count) {
   shared_object *sh_obj;
   unsigned int dl_debug_statePltEntry=0, tmp_dlopen;
   bool dlopenUsed = false;
   
   //In the mutated binary we need to catch the dlopen events and adjust the
   //instrumentation of the shared libraries (and jumps into the shared
   //libraries) as the base address of the shared libraries different for the
   //base addresses during the original mutator/mutatee run.

   //the r_debug interface ensures that a change to the dynamic linking
   //information causes _dl_debug_state to be called.  This is because dlopen
   //is too small and odd to instrument/breakpoint.  So our code will rely on
   //this fact. (all these functions are contained in ld-linux.so)

   //Our method: The Procedure Linking Table (.plt) for ld-linux contains an
   //entry that jumps to a specified address in the .rel.plt table. To call a
   //function, the compiler generates a jump to the correct .plt entry which
   //reads its jump value out of the .rel.plt.

   //On the sly, secretly replace the entry in .rel.plt with folgers crystals
   //and poof, we jump to our own function in RTcommon.c
   //(dyninst_dl_debug_state) [actually we replace the entry in .rel.plt with
   //the address of dyninst_dl_debug_state].  To ensure correctness,
   //dyninst_dl_debug_state contains a call to the real _dl_debug_state
   //immediately before it returns, thus ensuring any code relying on that
   //fact that _dl_debug_state is actually run remains happy.

   //It is very important then, that we know the location of the entry in the
   //.rel.plt table.  We need to record the offset of this entry with respect
   //to the base address of ld-linux.so (for obvious reasons) This offset is
   //then sent to RTcommon.c, and here is the slick part, by assigning it to
   //the 'load address' of the section "dyninstAPI_SharedLibraries," which
   //contains the shared library/basei address pairs used to fixup the saved
   //binary. This way when checkElfFile() reads the section the offset will
   //be there in the section header.

   //neat, eh?  this is how it will work in the future, currently this is not
   //yet fully implemented and part of the cvs tree.

   count = 0;
   for(int i=0;shared_objects && i<(int)shared_objects->size() ; i++) {
      sh_obj = (*shared_objects)[i];
      if(sh_obj->isDirty()){
         count ++;
         if(!dlopenUsed && sh_obj->isopenedWithdlopen()){
            BPatch_reportError(BPatchWarning,123,"dumpPatchedImage: dlopen used by the mutatee, this may cause the mutated binary to fail\n");
            dlopenUsed = true;
         }			
         //printf(" %s is DIRTY!\n", sh_obj->getName().c_str());
         
         Address textAddr, textSize;
         char *newName = new char[strlen(sh_obj->getName().c_str()) + 
                                  strlen(directoryName) + 1];
         memcpy(newName, directoryName, strlen(directoryName)+1);
         const char *file = strrchr(sh_obj->getName().c_str(), '/');
         strcat(newName, file);
         
         saveSharedLibrary *sharedObj = new saveSharedLibrary(
                          sh_obj->getBaseAddress(), sh_obj->getName().c_str(),
                          newName);
         sharedObj->writeLibrary();
         
         sharedObj->getTextInfo(textAddr, textSize);
         
         char *textSection = new char[textSize];
         readDataSpace((void*) (textAddr+ sh_obj->getBaseAddress()),
                       textSize,(void*)textSection, true);
         
         sharedObj->saveMutations(textSection);
         sharedObj->closeLibrary();
         /*			
         //this is for the dlopen problem....
         if(strstr(sh_obj->getName().c_str(), "ld-linux.so") ){
         //find the offset of _dl_debug_state in the .plt
         dl_debug_statePltEntry = 
         sh_obj->getImage()->getObject().getPltSlot("_dl_debug_state");
         }
         */			
         mutatedSharedObjectsSize += strlen(sh_obj->getName().c_str()) +1 ;
         delete [] textSection;
         delete [] newName;
      }
      //this is for the dlopen problem....
      if(strstr(sh_obj->getName().c_str(), "ld-linux.so") ){
         //find the offset of _dl_debug_state in the .plt
         dl_debug_statePltEntry = 
            sh_obj->getImage()->getObject().getPltSlot("_dl_debug_state");
      }
#if defined(sparc_sun_solaris2_4)
      
      if( ((tmp_dlopen = sh_obj->getImage()->getObject().getPltSlot("dlopen")) && !sh_obj->isopenedWithdlopen())){
         dl_debug_statePltEntry = tmp_dlopen + sh_obj->getBaseAddress();
      }
#endif
      //this is for the dyninst_SharedLibraries section we need to find out
      //the length of the names of each of the shared libraries to create the
      //data buffer for the section
      
      dyninst_SharedLibrariesSize += strlen(sh_obj->getName().c_str())+1;
      //add the size of the address
      dyninst_SharedLibrariesSize += sizeof(unsigned int);
   }
#if defined(sparc_sun_solaris2_4)
   if( (tmp_dlopen = getImage()->getObject().getPltSlot("dlopen"))) {
      dl_debug_statePltEntry = tmp_dlopen;
   }
   
   //dl_debug_statePltEntry = getImage()->getObject().getPltSlot("dlopen");
#endif
   dyninst_SharedLibrariesSize += 1;//for the trailing '\0'
   
   return dl_debug_statePltEntry;
}
	
char* process::saveWorldCreateSharedLibrariesSection(int dyninst_SharedLibrariesSize){
	//dyninst_SharedLibraries
	//the SharedLibraries sections contains a list of all the shared libraries
	//that have been loaded and the base address for each one.
	//The format of the sections is:
	//
	//sharedlibraryName
	//baseAddr
	//...
	//sharedlibraryName
	//baseAddr
	//'\0'
	
	char *dyninst_SharedLibrariesData = new char[dyninst_SharedLibrariesSize];
	char *ptr= dyninst_SharedLibrariesData;
	int size = shared_objects->size();
	shared_object *sh_obj;

	for(int i=0;shared_objects && i<size ; i++) {
		sh_obj = (*shared_objects)[i];

		memcpy((void*) ptr, sh_obj->getName().c_str(), strlen(sh_obj->getName().c_str())+1);
		//printf(" %s : ", ptr);
		ptr += strlen(sh_obj->getName().c_str())+1;

		unsigned int baseAddr = sh_obj->getBaseAddress();
		memcpy( (void*)ptr, &baseAddr, sizeof(unsigned int));
		//printf(" 0x%x \n", *(unsigned int*) ptr);
		ptr += sizeof(unsigned int);
	}
       	memset( (void*)ptr, '\0' , 1);

	return dyninst_SharedLibrariesData;
}
#endif

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)  || defined(rs6000_ibm_aix4_1)
void process::saveWorldCreateHighMemSections(
                        pdvector<imageUpdate*> &compactedHighmemUpdates, 
                        pdvector<imageUpdate*> &highmem_updates,
                        void *ptr) {

   unsigned int trampGuardValue;
   Address guardFlagAddr= trampGuardAddr();

   unsigned int pageSize = getpagesize();
   unsigned int startPage, stopPage;
   unsigned int numberUpdates=1;
   int startIndex, stopIndex;
   void *data;
   char name[50];
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)
	writeBackElf *newFile = (writeBackElf*) ptr;
#elif defined(rs6000_ibm_aix4_1)
	writeBackXCOFF *newFile = (writeBackXCOFF*) ptr;
#endif

   readDataSpace((void*) guardFlagAddr, sizeof(unsigned int),
                 (void*) &trampGuardValue, true);
   
   writeDataSpace((void*)guardFlagAddr, sizeof(unsigned int),
                  (void*) &numberUpdates);

   for(unsigned int j=0; j<compactedHighmemUpdates.size(); j++) {
      //the layout of dyninstAPIhighmem_%08x is:
      //pageData
      //address of update
      //size of update
      // ...
      //address of update
      //size of update
      //number of updates

      startPage = compactedHighmemUpdates[j]->address - 
                  compactedHighmemUpdates[j]->address % pageSize;
      stopPage = compactedHighmemUpdates[j]->address + 
                 compactedHighmemUpdates[j]->size -
                 (compactedHighmemUpdates[j]->address + 
                  compactedHighmemUpdates[j]->size) % pageSize;

      numberUpdates = 0;
      startIndex = -1;
      stopIndex = -1;
      
      for(unsigned index = 0;index < highmem_updates.size(); index++){
         //here we ignore anything with an address of zero.
         //these can be safely deleted in writeBackElf
         if( highmem_updates[index]->address && 
             startPage <= highmem_updates[index]->address &&
             highmem_updates[index]->address  < (startPage + pageSize /*compactedHighmemUpdates[j]->sizei*/)){
            numberUpdates ++;
            stopIndex = index;
            if(startIndex == -1){
               startIndex = index;
            }
           //printf(" HighMemUpdates address 0x%x \n", highmem_updates[index]->address );
         }
	//printf(" high mem updates: 0x%x", highmem_updates[index]->address);
      }
      unsigned int dataSize = compactedHighmemUpdates[j]->size + 
         sizeof(unsigned int) + 
         (2*(stopIndex - startIndex + 1) /*numberUpdates*/ * sizeof(unsigned int));
      
      data = new char[dataSize];
      
      //fill in pageData
      readDataSpace((void*) compactedHighmemUpdates[j]->address, 
                    compactedHighmemUpdates[j]->size, data, true);
      
      unsigned int *dataPtr = 
         (unsigned int*) ( (char*) data + compactedHighmemUpdates[j]->size);

      //fill in address of update
      //fill in size of update
      for(int index = startIndex; index<=stopIndex;index++){ 
         memcpy(dataPtr, &highmem_updates[index]->address,
                sizeof(unsigned int));
         dataPtr ++;
         memcpy(dataPtr, &highmem_updates[index]->size, sizeof(unsigned int));
         dataPtr++;
         //printf("%d J %d ADDRESS: 0x%x SIZE 0x%x\n",index, j,
         //highmem_updates[index]->address, highmem_updates[index]->size);
      }
      //fill in number of updates
      memcpy(dataPtr, &numberUpdates, sizeof(unsigned int));
      //printf(" NUMBER OF UPDATES 0x%x\n\n",numberUpdates);
      sprintf(name,"dyninstAPIhighmem_%08x",j);
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)
      newFile->addSection(compactedHighmemUpdates[j]->address,data ,dataSize,name,false);
#elif defined(rs6000_ibm_aix4_1)
	  sprintf(name, "dyH_%03x",j);
      newFile->addSection(&(name[0]), compactedHighmemUpdates[j]->address,compactedHighmemUpdates[j]->address,
		dataSize, (char*) data );
#endif
      
      //lastCompactedUpdateAddress = compactedHighmemUpdates[j]->address+1;
      delete [] (char*) data;
   }
   writeDataSpace((void*)guardFlagAddr, sizeof(unsigned int), 
                  (void*)&trampGuardValue);
}

void process::saveWorldCreateDataSections(void* ptr){

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)
	writeBackElf *newFile = (writeBackElf*) ptr;
#elif defined(rs6000_ibm_aix4_1)
	writeBackXCOFF *newFile = (writeBackXCOFF*) ptr;
#endif

	char *dataUpdatesData;
	int sizeofDataUpdatesData=0;
	for(unsigned int m=0;m<dataUpdates.size();m++){
		sizeofDataUpdatesData += (sizeof(int) + sizeof(Address)); //sizeof(size) +sizeof(Address);
		sizeofDataUpdatesData += dataUpdates[m]->size;
	}


	if(dataUpdates.size() > 0) {
		dataUpdatesData = new char[sizeofDataUpdatesData+(sizeof(int) + sizeof(Address))];
		char* ptr = dataUpdatesData;
		for(unsigned int k=0;k<dataUpdates.size();k++){
			memcpy(ptr, &dataUpdates[k]->size, sizeof(int));
			ptr += sizeof(int);
			memcpy(ptr, &dataUpdates[k]->address, sizeof(Address));
			ptr+=sizeof(Address);
			memcpy(ptr, dataUpdates[k]->value, dataUpdates[k]->size);
			ptr+=dataUpdates[k]->size;
			//printf(" DATA UPDATE : from: %x to %x , value %x\n", dataUpdates[k]->address,
		//	dataUpdates[k]->address+ dataUpdates[k]->size, (unsigned int) dataUpdates[k]->value);

		}
		*(int*) ptr=0;
		ptr += sizeof(int);
		*(unsigned int*) ptr=0;
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)
		newFile->addSection(0/*lastCompactedUpdateAddress*/, dataUpdatesData, 
			sizeofDataUpdatesData + (sizeof(int) + sizeof(Address)), "dyninstAPI_data", false);
#elif defined(rs6000_ibm_aix4_1)
		newFile->addSection("dyn_dat", 0/*lastCompactedUpdateAddress*/,0,
			sizeofDataUpdatesData + (sizeof(int) + sizeof(Address)), (char*) dataUpdatesData);

#endif

		delete [] (char*) dataUpdatesData;
	}

}
#endif

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) 

void process::saveWorldAddSharedLibs(void *ptr){ // ccw 14 may 2002 

	int dataSize=0;
	char *data, *dataptr;
	writeBackElf *newElf = (writeBackElf*)ptr;

	int i;

	for(unsigned i=0;i<loadLibraryUpdates.size();i++){
		dataSize += loadLibraryUpdates[i].length() + 1;
	}
	dataSize++;
	data = new char[dataSize];
	dataptr = data;

	for(unsigned j=0;j<loadLibraryUpdates.size();j++){
		memcpy( dataptr, loadLibraryUpdates[j].c_str(), loadLibraryUpdates[j].length()); 
		dataptr += loadLibraryUpdates[j].length();
		*dataptr = '\0';
		dataptr++; 
	}
	dataptr = '\0'; //mark the end
	if(dataSize > 1){
		newElf->addSection(0, data, dataSize, "dyninstAPI_loadLib", false);
	}
	delete [] data;
}

#endif
#endif


/*
 * Given an image, add all static heaps inside it
 * (DYNINSTstaticHeap...) to the buffer pool.
 */

void process::addInferiorHeap(const image *theImage)
{
  pdvector<heapDescriptor> infHeaps;

  /* Get a list of inferior heaps in the new image */
  if (getInfHeapList(theImage, infHeaps))
    {
      /* Add the vector to the inferior heap structure */
        for (u_int j=0; j < infHeaps.size(); j++)
        {
            heapItem *h = new heapItem (infHeaps[j].addr(), infHeaps[j].size(),
                                        infHeaps[j].type(), false);
            heap.bufferPool.push_back(h);
            heapItem *h2 = new heapItem(h);
            heap.heapFree.push_back(h2);
        }
    }
}


/*
 * Called to (re)initialize the static inferior heap structure.
 * To incrementally add a static inferior heap (in a dlopen()d module,
 * for example), use addInferiorHeap(image *)
 */
void process::initInferiorHeap()
{
  assert(this->symbols);
  inferiorHeap *hp = &heap;
  pdvector<heapDescriptor> infHeaps;

  // first initialization: add static heaps to pool
  if (hp->bufferPool.size() == 0) {
    bool err;
    Address heapAddr=0;
    int staticHeapSize = alignAddress(SYN_INST_BUF_SIZE, 32);

    // Get the inferior heap list
    getInfHeapList(infHeaps);
    
    bool lowmemHeapAdded = false;
    bool heapAdded = false;
    
    for (u_int j=0; j < infHeaps.size(); j++)
      {
	hp->bufferPool.push_back(new heapItem (infHeaps[j].addr(), infHeaps[j].size(),
					infHeaps[j].type(), false));
	heapAdded = true;
	if (infHeaps[j].type() == lowmemHeap)
	  lowmemHeapAdded = true;
      }
    if (!heapAdded)
      {
	// No heap added. Check for the old DYNINSTdata heap
	unsigned LOWMEM_HEAP_SIZE=(32*1024);
	cerr << "No heap found of the form DYNINSTstaticHeap_<size>_<type>_<unique>." << endl;
	cerr << "Attempting to use old DYNINSTdata inferior heap..." << endl;
	heapAddr = findInternalAddress(string("DYNINSTdata"), true, err);
	assert(heapAddr);
	hp->bufferPool.push_back(new heapItem(heapAddr, staticHeapSize - LOWMEM_HEAP_SIZE,
				       anyHeap, false));
	hp->bufferPool.push_back(new heapItem(heapAddr + staticHeapSize - LOWMEM_HEAP_SIZE,
				       LOWMEM_HEAP_SIZE, lowmemHeap, false));
	heapAdded = true; 
	lowmemHeapAdded = true;
      }
    if (!lowmemHeapAdded)
      {
	// Didn't find the low memory heap. 
	// Handle better?
	// Yeah, gripe like hell
	cerr << "No lowmem heap found (DYNINSTstaticHeap_*_lowmem), inferior RPCs" << endl;
	cerr << "will probably fail" << endl;
      }
    
  }

  // (re)initialize everything 
  hp->heapActive.clear();
  hp->heapFree.resize(0);
  hp->disabledList.resize(0);
  hp->disabledListTotalMem = 0;
  hp->freed = 0;
  hp->totalFreeMemAvailable = 0;
  
  /* add dynamic heap segments to free list */
  for (unsigned i = 0; i < hp->bufferPool.size(); i++) {
    heapItem *hi = new heapItem(hp->bufferPool[i]);
    hi->status = HEAPfree;
    hp->heapFree.push_back(hi);
    hp->totalFreeMemAvailable += hi->length;     
  }
  inferiorMemAvailable = hp->totalFreeMemAvailable;
}

bool process::initTrampGuard()
{
  // This is slightly funky. Dyninst does not currently support
  // multiple threads -- so it uses a single tramp guard flag, 
  // which resides in the runtime library. However, this is not
  // enough for MT paradyn. So Paradyn overrides this setting as 
  // part of its initialization.
  // Repeat: OVERRIDDEN LATER FOR PARADYN

  const string vrbleName = "DYNINSTtrampGuard";
  internalSym sym;
  bool flag = findInternalSymbol(vrbleName, true, sym);
  assert(flag);
  trampGuardAddr_ = sym.getAddr();
  return true;
}

// create a new inferior heap that is a copy of src. This is used when a process
// we are tracing forks.
inferiorHeap::inferiorHeap(const inferiorHeap &src):
    heapActive(addrHash16)
{
    for (unsigned u1 = 0; u1 < src.heapFree.size(); u1++) {
      heapFree.push_back(new heapItem(src.heapFree[u1]));
    }

    pdvector<heapItem *> items = src.heapActive.values();
    for (unsigned u2 = 0; u2 < items.size(); u2++) {
      heapActive[items[u2]->addr] = new heapItem(items[u2]);
    }
    
    for (unsigned u3 = 0; u3 < src.disabledList.size(); u3++) {
      disabledList.push_back(src.disabledList[u3]);
    }

    for (unsigned u4 = 0; u4 < src.bufferPool.size(); u4++) {
      bufferPool.push_back(new heapItem(src.bufferPool[u4]));
    }

    disabledListTotalMem = src.disabledListTotalMem;
    totalFreeMemAvailable = src.totalFreeMemAvailable;
    inferiorMemAvailable = totalFreeMemAvailable;
    freed = 0;
}

//
// This function will return the index corresponding to the next position
// available in heapFree.
//
int process::findFreeIndex(unsigned size, int type, Address lo, Address hi)
{
  pdvector<heapItem *> &freeList = heap.heapFree;

  int best = -1;
  for (unsigned i = 0; i < freeList.size(); i++) {
    heapItem *h = freeList[i];
    // check if free block matches allocation constraints
    if (h->type & type &&
        h->addr >= lo &&
        h->addr + size - 1 <= hi &&
        h->length >= size) 
      {
        if (best == -1) best = i;
        // check for better match
        if (h->length < freeList[best]->length) best = i;
      } 
  }
  return best;
}  

//
// dynamic inferior heap stuff
//
#if defined(USES_DYNAMIC_INF_HEAP)
#define HEAP_DYN_BUF_SIZE (0x100000)
// "imd_rpc_ret" = Inferior Malloc Dynamic RPC RETurn structure
typedef struct {
  bool ready;
  void *result;
} imd_rpc_ret;

void process::inferiorMallocCallback(process *proc, void *data, void *result)
{
  imd_rpc_ret *ret = (imd_rpc_ret *)data;
  ret->result = result;
  ret->ready = true;
}

void alignUp(int &val, int align)
{
  assert(val >= 0);
  assert(align >= 0);

  if (val % align != 0) {
    val = ((val / align) + 1) * align;
  }
}
// dynamically allocate a new inferior heap segment using inferiorRPC
void process::inferiorMallocDynamic(int size, Address lo, Address hi)
{
/* 03/07/2001 - Jeffrey Shergalis
 * TODO: This code was placed to prevent the infinite recursion on the
 * call to inferiorMallocDynamic, unfortunately it makes paradyn break
 * on Irix, temporarily fixed by the #if !defined(mips..., but should be properly fixed
 * in the future, just no time now
 */
#if !defined(mips_sgi_irix6_4)
  // Fun (not) case: there's no space for the RPC to execute.
  // It'll call inferiorMalloc, which will call inferiorMallocDynamic...
  // Avoid this with a static bool.
  static bool inInferiorMallocDynamic = false;

  if (inInferiorMallocDynamic) return;
  inInferiorMallocDynamic = true;
#endif

  // word-align buffer size 
  // (see "DYNINSTheap_align" in rtinst/src/RTheap-<os>.c)
  alignUp(size, 4);

  // build AstNode for "DYNINSTos_malloc" call
  string callee = "DYNINSTos_malloc";
  pdvector<AstNode*> args(3);
  args[0] = new AstNode(AstNode::Constant, (void *)size);
  args[1] = new AstNode(AstNode::Constant, (void *)lo);
  args[2] = new AstNode(AstNode::Constant, (void *)hi);
  AstNode *code = new AstNode(callee, args);
  removeAst(args[0]);
  removeAst(args[1]);
  removeAst(args[2]);

  // issue RPC and wait for result
  imd_rpc_ret ret = { false, NULL };

  /* set lowmem to ensure there is space for inferior malloc */
  postRPCtoDo(code, true, // noCost
              &inferiorMallocCallback, &ret, -1, // No particular thread or LWP
              true); // But use reserved memory
  bool wasRunning = (status() == running);
  do {
      bool result = launchRPCs(wasRunning);
      decodeAndHandleProcessEvent(false);
  } while (!ret.ready); // Loop until callback has fired.
  switch ((int)(Address)ret.result) {
  case 0:
#ifdef DEBUG
    sprintf(errorLine, "DYNINSTos_malloc() failed\n");
    logLine(errorLine);
#endif
    break;
  case -1:
    // TODO: assert?
    sprintf(errorLine, "DYNINSTos_malloc(): unaligned buffer size\n");
    logLine(errorLine);
    break;
  default:
    // add new segment to buffer pool
    heapItem *h = new heapItem((Address)ret.result, size, anyHeap, true, HEAPfree);
    heap.bufferPool.push_back(h);
    // add new segment to free list
    heapItem *h2 = new heapItem(h);
    heap.heapFree.push_back(h2);
    break;
  }

/* 03/07/2001 - Jeffrey Shergalis
 * Part of the above #if !defined(mips... patch for the recursion problem
 * TODO: Need a better solution
 */
#if !defined(mips_sgi_irix6_4)
  inInferiorMallocDynamic = false;
#endif
}
#endif /* USES_DYNAMIC_INF_HEAP */

const Address ADDRESS_LO = ((Address)0);
const Address ADDRESS_HI = ((Address)~((Address)0));
//unsigned int totalSizeAlloc = 0;

Address process::inferiorMalloc(unsigned size, inferiorHeapType type, 
				Address near_, bool *err)
{
   inferiorHeap *hp = &heap;
   if (err) *err = false;
   assert(size > 0);
   
   // allocation range
   Address lo = ADDRESS_LO; // Should get reset to a more reasonable value
   Address hi = ADDRESS_HI; // Should get reset to a more reasonable value
   
#if defined(USES_DYNAMIC_INF_HEAP)
   inferiorMallocAlign(size); // align size
   // Set the lo/hi constraints (if necessary)
   inferiorMallocConstraints(near_, lo, hi, type);
   
#else
   /* align to cache line size (32 bytes on SPARC) */
   size = (size + 0x1f) & ~0x1f; 
#endif /* USES_DYNAMIC_INF_HEAP */

   // find free memory block (7 attempts)
   // attempt 0: as is
   // attempt 1: deferred free, compact free blocks
   // attempt 2: allocate new segment (1 MB, constrained)
   // attempt 3: allocate new segment (sized, constrained)
   // attempt 4: remove range constraints
   // attempt 5: allocate new segment (1 MB, unconstrained)
   // attempt 6: allocate new segment (sized, unconstrained)
   // attempt 7: deferred free, compact free blocks (why again?)
   int freeIndex = -1;
   int ntry = 0;
   for (ntry = 0; freeIndex == -1; ntry++) {
      switch(ntry) {
	case 0: // as is
	   break;
#if defined(USES_DYNAMIC_INF_HEAP)
	case 1: // compact free blocks
	  gcInstrumentation();
	  inferiorFreeCompact(hp);
	  break;
	case 2: // allocate new segment (1MB, constrained)
	   inferiorMallocDynamic(HEAP_DYN_BUF_SIZE, lo, hi);
	   break;
	case 3: // allocate new segment (sized, constrained)
	   inferiorMallocDynamic(size, lo, hi);
	   break;
	case 4: // remove range constraints
	   lo = ADDRESS_LO;
	   hi = ADDRESS_HI;
	   if (err) {
	      *err = true;
	   }
	   break;
	case 5: // allocate new segment (1MB, unconstrained)
	   inferiorMallocDynamic(HEAP_DYN_BUF_SIZE, lo, hi);
	   break;
	case 6: // allocate new segment (sized, unconstrained)
	   inferiorMallocDynamic(size, lo, hi);
	   break;
	case 7: // deferred free, compact free blocks
	   inferiorFreeCompact(hp);
	   break;
#else /* !(USES_DYNAMIC_INF_HEAP) */
	case 1: // deferred free, compact free blocks
	   inferiorFreeCompact(hp);
	   break;
#endif /* USES_DYNAMIC_INF_HEAP */
	   
	default: // error - out of memory
	   sprintf(errorLine, "***** Inferior heap overflow: %d bytes "
		   "freed, %d bytes requested \n", hp->freed, size);
	   logLine(errorLine);
	   showErrorCallback(66, (const char *) errorLine);    
#if defined(BPATCH_LIBRARY)
	   return(0);
#else
	   P__exit(-1);
#endif
      }
      freeIndex = findFreeIndex(size, type, lo, hi);
//	printf("  type %x",type);
   }
   
   // adjust active and free lists
   heapItem *h = hp->heapFree[freeIndex];
   assert(h);
   // remove allocated buffer from free list
   if (h->length != size) {
      // size mismatch: put remainder of block on free list
      heapItem *rem = new heapItem(h);
      rem->addr += size;
      rem->length -= size;
      hp->heapFree[freeIndex] = rem;
   } else {
      // size match: remove entire block from free list
      unsigned last = hp->heapFree.size();
      hp->heapFree[freeIndex] = hp->heapFree[last-1];
      hp->heapFree.resize(last-1);
   }
   // add allocated block to active list
   h->length = size;
   h->status = HEAPallocated;
   hp->heapActive[h->addr] = h;
   // bookkeeping
   hp->totalFreeMemAvailable -= size;
   inferiorMemAvailable = hp->totalFreeMemAvailable;
   assert(h->addr);
   
   // ccw: 28 oct 2001
   // create imageUpdate here:
   // imageUpdate(h->addr,size)
   
#ifdef BPATCH_LIBRARY
//	if( h->addr > 0xd0000000){
//		printf(" \n ALLOCATION: %lx %lx ntry: %d\n", h->addr, size,ntry);
//		fflush(stdout);
//	}
#if defined(sparc_sun_solaris2_4 ) || defined(i386_unknown_linux2_0) || defined(rs6000_ibm_aix4_1)
   if(collectSaveWorldData){
      
#if defined(sparc_sun_solaris2_4)
      if(h->addr < 0xF0000000)
#elif defined(i386_unknown_linux2_0)
      if(h->addr < 0x40000000)
#elif defined(rs6000_ibm_aix4_1)
	if(h->addr < 0x20000000)
#endif	
      {
	 imageUpdate *imagePatch=new imageUpdate; 
	 imagePatch->address = h->addr;
	 imagePatch->size = size;
	 imageUpdates.push_back(imagePatch);
	 //totalSizeAlloc += size;
	 //printf(" PUSHBACK %x %x --- \n", imagePatch->address, imagePatch->size); 		
      } else {
	 //	totalSizeAlloc += size;
	 //printf(" HIGHMEM UPDATE %x %x \n", h->addr, size);
	 imageUpdate *imagePatch=new imageUpdate;
	 imagePatch->address = h->addr;
	 imagePatch->size = size;
	 highmemUpdates.push_back(imagePatch);
	 //printf(" PUSHBACK %x %x\n", imagePatch->address, imagePatch->size);
      }
      //fflush(stdout);
   }
#endif
#endif
   return(h->addr);
}

/* returns true if memory was allocated for a variable starting at address
   "block", otherwise returns false
*/
bool isInferiorAllocated(process *p, Address block) {
  heapItem *h = NULL;  
  inferiorHeap *hp = &p->heap;
  return hp->heapActive.find(block, h);
}

void process::inferiorFree(Address block)
{
  inferiorHeap *hp = &heap;

  // find block on active list
  heapItem *h = NULL;  
  if (!hp->heapActive.find(block, h)) {
    showErrorCallback(96,"Internal error: "
        "attempt to free non-defined heap entry.");
    return;
  }
  assert(h);

  // Remove from the active list
  hp->heapActive.undef(block);

  // Add to the free list
  h->status = HEAPfree;
  hp->heapFree.push_back(h);
  hp->totalFreeMemAvailable += h->length;
  hp->freed += h->length;
  inferiorMemAvailable = hp->totalFreeMemAvailable;
}


//
// cleanup when a process is deleted
//
process::~process()
{
   /*  The instPoints in the installedMiniTramps_... will be shared
       between forked processes.  Since there is currently no reference
       counting mechanism, don't delete instPoints until something like
       this exists.  Otherwise, an error occurs when the instPoint is
       attempted to be deleted after it was already deleted.
    // remove inst points for this process
    dictionary_hash_iter<const instPoint *, installed_miniTramps_list*>
      befList = installedMiniTramps_beforePt;
    for(; befList; befList++) {
      const instPoint *pt = befList.currkey();
      delete pt;
    }

    // remove inst points for this process
    dictionary_hash_iter<const instPoint *, installed_miniTramps_list*>
      aftList = installedMiniTramps_afterPt;
    for(; aftList; aftList++) {
      const instPoint *pt = aftList.currkey();
      delete pt;
    }
   */

#ifndef BPATCH_LIBRARY
    // the varMgr needs to be deleted before the shmMgr because the varMgr
    // needs to free the memory it's allocated from the sharedMemoryMgr
    delete shMetaOffsetData;
    delete shmMetaData;    // needs to occur before shmMgr is deleted
    delete theSharedMemMgr;
#endif
#ifdef BPATCH_LIBRARY
    detach(false);

    // remove it from the processVec
    unsigned int size = processVec.size();
    bool found = false;

    for (unsigned lcv=0; lcv < size; lcv++) {
	if (processVec[lcv] == this) {
	    assert(!found);
	    found = true;
	    processVec[lcv] = processVec[processVec.size()-1];
	    processVec.resize(size-1);
	}
    }
#else
  cpuTimeMgr->destroyMechTimers(this);
#endif
   dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(lwps);
   dyn_lwp *lwp;
   unsigned index;
   
   while (lwp_iter.next(index, lwp)) {
      deleteLWP(lwp);
   }
  
}

unsigned hash_bp(function_base * const &bp ) { return(addrHash4((Address) bp)); }

//
// Process "normal" (non-attach, non-fork) ctor, for when a new process
// is fired up by paradynd itself.
// This ctor. is also used in the case that the application has been fired up
// by another process and stopped just after executing the execv() system call.
// In the "normal" case, the parameter iTraceLink will be a non-negative value which
// corresponds to the pipe descriptor of the trace pipe used between the paradynd
// and the application. In the later case, iTraceLink will be -1. This will be 
// posteriorly used to properly initialize the createdViaAttachToCreated flag. - Ana
//

//removed all ioLink related code for output redirection
process::process(int iPid, image *iImage, int iTraceLink 
#ifdef SHM_SAMPLING
                 , key_t theShmKey
#endif
) :
  collectSaveWorldData(true),
#if defined(BPATCH_LIBRARY) && defined(rs6000_ibm_aix4_1)
 requestTextMiniTramp(0), //ccw 30 jul 2002
#endif
#ifndef BPATCH_LIBRARY
  cpuTimeMgr(NULL),
#ifdef HRTIME
  hr_cpu_link(NULL),
#endif
#ifdef PAPI
  papi(NULL),
#endif
#endif
  baseMap(ipHash), 
#ifdef BPATCH_LIBRARY
  PDFuncToBPFuncMap(hash_bp),
  instPointMap(hash_address),
#endif
  loadLibraryCallbacks_(string::hash),
#ifndef BPATCH_LIBRARY
  shmMetaData(NULL), shMetaOffsetData(NULL),
#endif
  savedRegs(NULL),
  installedMiniTramps_beforePt(ipHash),
  installedMiniTramps_afterPt(ipHash),
  pid(iPid), // needed in fastInferiorHeap ctors below
#if !defined(BPATCH_LIBRARY)
  previous(0),
#endif
  lwps(CThash),
  procHandle_(0)
    
{
#if !defined(BPATCH_LIBRARY) //ccw 22 apr 2002 : SPLIT
	PARADYNhasBootstrapped = false;
#endif


  dyninstlib_brk_addr = 0;

  main_brk_addr = 0;

    wasRunningWhenAttached_ = false;
    bootstrapState = unstarted;

    createdViaAttach = false;
    createdViaFork = false;
    createdViaAttachToCreated = false; 
    wasRunningWhenAttached_ = false; // Technically true...

#ifndef BPATCH_LIBRARY
      if (iTraceLink == -1 ) createdViaAttachToCreated = true;
                         // indicates the unique case where paradynd is attached to
                         // a stopped application just after executing the execv() --Ana
#endif
    needToContinueAfterDYNINSTinit = false;  //Wait for press of "RUN" button

    symbols = iImage;
    mainFunction = NULL; // set in platform dependent function heapIsOk

    theRpcMgr = new rpcMgr(this);

    status_ = neonatal;
    exitCode_ = -1;
    continueAfterNextStop_ = 0;
#ifndef BPATCH_LIBRARY
    theSharedMemMgr = new shmMgr(this, theShmKey, 2097152);
    shmMetaData = new sharedMetaData(*theSharedMemMgr, maxNumberOfThreads());

    initCpuTimeMgr();

    string buff = string(pid); // + string("_") + getHostName();
    rid = resource::newResource(machineResource, // parent
				(void*)this, // handle
				nullString, // abstraction
				iImage->name(), // process name
				timeStamp::ts1970(), // creation time
				buff, // unique name (?)
				MDL_T_STRING, // mdl type (?)
				true
				);
#endif

    parent = NULL;
    inExec = false;

    cumObsCost = 0;
    lastObsCostLow = 0;

#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0) \
 || defined(i386_unknown_nt4_0) || defined(ia64_unknown_linux2_4)
    trampTableItems = 0;
    memset(trampTable, 0, sizeof(trampTable));
#endif
    
    dynamiclinking = false;
    dyn = new dynamic_linking;
    shared_objects = 0;
    runtime_lib = 0;
    all_functions = 0;
    all_modules = 0;
    some_modules = 0;
    some_functions = 0;
    waiting_for_resources = false;
#if defined(i386_unknown_linux2_0)
    signal_restore = 0;
#else
    signal_handler = 0;
#endif
    execed_ = false;

#ifdef SHM_SAMPLING
#ifdef sparc_sun_sunos4_1_3
   kvmHandle = kvm_open(0, 0, 0, O_RDONLY, 0);
   if (kvmHandle == NULL) {
      perror("could not map child's uarea; kvm_open");
      exit(5);
   }

//   childUareaPtr = tryToMapChildUarea(iPid);
   childUareaPtr = NULL;
#endif
#endif

   splitHeaps = false;

#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1) || defined(alpha_dec_osf4_0)
        // XXXX - move this to a machine dependant place.

        // create a seperate text heap.
        //initInferiorHeap(true);
        splitHeaps = true;
#endif
          traceLink = iTraceLink; // notice that tracelink will be -1 in the unique
                                  // case called "AttachToCreated" - Ana
          
   //removed for output redirection
   //ioLink = iIoLink;

#if defined(rs6000_ibm_aix4_1)
   resetForkTrapData();
#endif

#if !defined(mips_unknown_ce2_11) && !defined(i386_unknown_nt4_0)
   createLWP(0);
#endif   

   // attach to the child process (machine-specific implementation)
   if (!attach()) { // error check?
      string msg = string("Warning: unable to attach to specified process :")
                   + string(pid);
      showErrorCallback(26, msg.c_str());
      cerr << "Process: failed attach!" << endl;
   }
  
#ifndef BPATCH_LIBRARY
#ifdef PAPI
   if (isPapiInitialized()) {
     papi = new papiMgr(this);
   }
#endif
#endif
} // end of normal constructor


//
// Process "attach" ctor, for when paradynd is attaching to an already-existing
// process. 
//
//
process::process(int iPid, image *iSymbols,
                 bool &success
#if !defined(BPATCH_LIBRARY)
                 , key_t theShmKey
#endif
                 ) :
  collectSaveWorldData(true),
#if defined(BPATCH_LIBRARY) && defined(rs6000_ibm_aix4_1)
  requestTextMiniTramp(0), //ccw 30 jul 2002
#endif
#if !defined(BPATCH_LIBRARY)
  cpuTimeMgr(NULL),
#ifdef HRTIME
  hr_cpu_link(NULL),
#endif // HRTIME
#ifdef PAPI
  papi(NULL),
#endif
#endif // BPATCH
  baseMap(ipHash),
#if defined(BPATCH_LIBRARY)
  PDFuncToBPFuncMap(hash_bp),
  instPointMap(hash_address),
#endif
  loadLibraryCallbacks_(string::hash),
#ifndef BPATCH_LIBRARY
  shmMetaData(NULL), shMetaOffsetData(NULL),
  PARADYNhasBootstrapped(false),
#endif
  savedRegs(NULL),
  installedMiniTramps_beforePt(ipHash),
  installedMiniTramps_afterPt(ipHash),
  pid(iPid),
#if !defined(BPATCH_LIBRARY)  && defined(i386_unknown_nt4_0)
  previous(0), //ccw 8 jun 2002
#endif
  lwps(CThash),
  procHandle_(0)
{
    dyninstlib_brk_addr = 0;
    main_brk_addr = 0;

	createdViaAttach = true;

#if !defined(i386_unknown_nt4_0)
    bootstrapState = initialized;
#else
    // We need to wait for the CREATE_PROCESS debug event.
    // Set to "begun" here, and fix up in the signal loop
    bootstrapState = begun;
#endif

    createdViaFork = false;
    createdViaAttachToCreated = false; 
    
    symbols = iSymbols;
    mainFunction = NULL; // set in platform dependent function heapIsOk
    
    status_ = neonatal;
    exitCode_ = -1;
    continueAfterNextStop_ = 0;
    
    theRpcMgr = new rpcMgr(this);
    
#ifndef BPATCH_LIBRARY
    theSharedMemMgr = new shmMgr(this, theShmKey, 2097152);
    shmMetaData = new sharedMetaData(*theSharedMemMgr, maxNumberOfThreads());

    initCpuTimeMgr();
    
    string buff = string(pid); // + string("_") + getHostName();
    rid = resource::newResource(machineResource, // parent
				(void*)this, // handle
				nullString, // abstraction
				symbols->name(),
				timeStamp::ts1970(), // creation time
				buff, // unique name (?)
				MDL_T_STRING, // mdl type (?)
				true
				);
#endif

    parent = NULL;
    inExec = false;

    cumObsCost = 0;
    lastObsCostLow = 0;


#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0) \
 || defined(i386_unknown_nt4_0) || defined(ia64_unknown_linux2_4)
    trampTableItems = 0;
    memset(trampTable, 0, sizeof(trampTable));
#endif
    
    dynamiclinking = false;
    dyn = new dynamic_linking;
    shared_objects = 0;
    runtime_lib = 0;
    all_functions = 0;
    all_modules = 0;
    some_modules = 0;
    some_functions = 0;
    waiting_for_resources = false;
#if defined(i386_unknown_linux2_0)
    signal_restore = 0;
#else
    signal_handler = 0;
#endif
    execed_ = false;

    splitHeaps = false;

#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1) || defined(alpha_dec_osf4_0)
        // XXXX - move this to a machine dependant place.

        // create a seperate text heap.
        //initInferiorHeap(true);
        splitHeaps = true;
#endif


   traceLink = -1; // will be set later, when the appl runs DYNINSTinit

   //removed for output redirection
   //ioLink = -1; // (ARGUABLY) NOT YET IMPLEMENTED...MAYBE WHEN WE ATTACH WE DON'T WANT
                // TO REDIRECT STDIO SO WE CAN LEAVE IT AT -1.

   // Now the actual attach...the moment we've all been waiting for

   attach_cerr << "process attach ctor: about to attach to pid " << getPid() << endl;

#if !defined(mips_unknown_ce2_11) && !defined(i386_unknown_nt4_0)
   createLWP(0);
#endif

   // It is assumed that a call to attach() doesn't affect the running status
   // of the process.  But, unfortunately, some platforms may barf if the
   // running status is anything except paused. (How to deal with this?)
   // Note that solaris in particular seems able to attach even if the process
   // is running.
   if (!attach()) {
      string msg = string("Warning: unable to attach to specified process: ")
                   + string(pid);
      showErrorCallback(26, msg.c_str());
      success = false;
      return;
   }

#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
   // Now that we're attached, we can reparse the image with correct
   // settings.
   // Process is paused 
   int status = pid;
   fileDescriptor *desc = getExecFileDescriptor(symbols->pathname(), status, false);
   if (!desc) {
      string msg = string("Warning: unable to parse to specified process: ")
                   + string(pid);
      showErrorCallback(26, msg.c_str());
      success = false;
      return;
   }
   image *theImage = image::parseImage(desc);
   if (theImage == NULL) {
      string msg = string("Warning: unable to parse to specified process: ")
                   + string(pid);
      showErrorCallback(26, msg.c_str());
      success = false;
      return;
   }
   // this doesn't leak, since the old theImage was deleted. 
   symbols = theImage;
#endif

   // Record what the process was doing when we attached, for possible
   // use later.
   wasRunningWhenAttached_ = isRunning_();
   if (wasRunningWhenAttached_) 
       status_ = running;
   else
       status_ = stopped;

   // Now pause the process before we go to work on it.
   if (!pause())
       assert(false);
   
#if defined(rs6000_ibm_aix4_1)
   resetForkTrapData();
#endif

   // Does attach() send a SIGTRAP, a la the initial SIGTRAP sent at the
   // end of exec?  It seems that on some platforms it does; on others
   // it doesn't.  Ick.  On solaris, it doesn't.

   // note: we don't call getSharedObjects() yet; that happens once DYNINSTinit
   //       finishes (initSharedObjects)


#ifndef BPATCH_LIBRARY
#ifdef PAPI
   if (isPapiInitialized()) {
     papi = new papiMgr(this);
   }
#endif
#endif

   // Everything worked
   success = true;
}



void copyOverInstInstanceObjects(
        dictionary_hash<const instPoint *, installed_miniTramps_list*> *mtdata)
{
   dictionary_hash_iter<const instPoint *, installed_miniTramps_list*> it =
      *mtdata;
   for(; it; it++) {
      installed_miniTramps_list *oldMTlist = it.currval();
      installed_miniTramps_list *newMTlist =
	it.currval() = new installed_miniTramps_list();
      newMTlist->clear();
      List<instInstance*>::iterator lstIter = oldMTlist->get_begin_iter();
      List<instInstance*>::iterator endIter = oldMTlist->get_end_iter();
      for(; lstIter != endIter; lstIter++) {
	instInstance *oldII = *lstIter;
	instInstance *newII = new instInstance(*oldII);
	newMTlist->addMiniTramp(orderLastAtPoint, newII);
      }
   }
}

// #if !defined(BPATCH_LIBRARY)

//
// Process "fork" ctor, for when a process which is already being monitored by
// paradynd executes the fork syscall.
//

process::process(const process &parentProc, int iPid, int iTrace_fd
#ifndef BPATCH_LIBRARY
                 ,key_t theShmKey,
                 void *applShmSegPtr
#endif
                 ) :
  collectSaveWorldData(true),
#if defined(BPATCH_LIBRARY) && defined(rs6000_ibm_aix4_1)
  requestTextMiniTramp(0), //ccw 30 jul 2002
#endif
#ifndef BPATCH_LIBRARY
  cpuTimeMgr(NULL),
#ifdef HRTIME
  hr_cpu_link(NULL),
#endif
#ifdef PAPI
  papi(NULL),
#endif
#endif
  baseMap(ipHash), // could change to baseMap(parentProc.baseMap)
#ifdef BPATCH_LIBRARY
  PDFuncToBPFuncMap(hash_bp),
  instPointMap(hash_address),
#endif
  loadLibraryCallbacks_(string::hash),
#ifndef BPATCH_LIBRARY
  shmMetaData(NULL), shMetaOffsetData(NULL),
  PARADYNhasBootstrapped(false),
#endif
  savedRegs(NULL),
  installedMiniTramps_beforePt(parentProc.installedMiniTramps_beforePt),
  installedMiniTramps_afterPt(parentProc.installedMiniTramps_afterPt),
#ifdef SHM_SAMPLING
  previous(0),
#endif
  lwps(CThash),
  procHandle_(0)
{
   // This is the "fork" ctor
   bootstrapState = initialized;
#if !defined(BPATCH_LIBRARY) //ccw 22 apr 2002 : SPLIT
   PARADYNhasBootstrapped = false;
#endif

   createdViaAttachToCreated = false;
   createdViaFork = true;
   createdViaAttach = parentProc.createdViaAttach;
   wasRunningWhenAttached_ = true;
   needToContinueAfterDYNINSTinit = true;

   symbols = parentProc.symbols; //shouldn't a reference count also be bumped?
   symbols->updateForFork(this, &parentProc);
   mainFunction = parentProc.mainFunction;

   traceLink = iTrace_fd;

   //removed for output redireciton
   //ioLink = -1; // when does this get set?

   status_ = neonatal; // is neonatal right?
   exitCode_ = -1;
   continueAfterNextStop_ = 0;

   pid = iPid; 
   copyOverInstInstanceObjects(&installedMiniTramps_beforePt);
   copyOverInstInstanceObjects(&installedMiniTramps_afterPt);

   theRpcMgr = new rpcMgr(this);

#ifndef BPATCH_LIBRARY
   // since the child process inherits the parents instrumentation we'll
   // need to inherit the parent process's data also
   theSharedMemMgr = new shmMgr(*parentProc.theSharedMemMgr, theShmKey,
                                applShmSegPtr, pid);
   shmMetaData = new sharedMetaData(*(parentProc.shmMetaData), 
                                    *theSharedMemMgr);
   shMetaOffsetData = new sharedMetaOffsetData(*theSharedMemMgr, 
					       *(parentProc.shMetaOffsetData));
   initCpuTimeMgr();

   shmMetaData->adjustToNewBaseAddr(reinterpret_cast<Address>(
                                                              theSharedMemMgr->getBaseAddrInDaemon()));
   shmMetaData->initializeForkedProc(theSharedMemMgr->cookie, getPid());

   string buff = string(pid); // + string("_") + getHostName();
   rid = resource::newResource(machineResource, // parent
                               (void*)this, // handle
                               nullString, // abstraction
                               parentProc.symbols->name(),
                               timeStamp::ts1970(), // creation time
                               buff, // unique name (?)
                               MDL_T_STRING, // mdl type (?)
                               true
                               );
#endif

   parent = const_cast<process*>(&parentProc);
    
   dyninstlib_brk_addr = 0;

   main_brk_addr = 0;

   bootstrapState = initialized;

   splitHeaps = parentProc.splitHeaps;

   heap = inferiorHeap(parentProc.heap);

   inExec = false;

   cumObsCost = 0;
   lastObsCostLow = 0;

#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0) \
 || defined(i386_unknown_nt4_0) || defined(ia64_unknown_linux2_4)
   trampTableItems = 0;
   memset(trampTable, 0, sizeof(trampTable));
#endif

   dynamiclinking = parentProc.dynamiclinking;
   dyn = new dynamic_linking;
   *dyn = *parentProc.dyn;
   runtime_lib = parentProc.runtime_lib;

   shared_objects = 0;

   // make copy of parent's shared_objects vector
   if (parentProc.shared_objects) {
      shared_objects = new pdvector<shared_object*>;
      for (unsigned u1 = 0; u1 < parentProc.shared_objects->size(); u1++){
         (*shared_objects).push_back(
                                     new shared_object(*(*parentProc.shared_objects)[u1]));
      }
   }

   all_functions = 0;
   if (parentProc.all_functions) {
      all_functions = new pdvector<function_base *>;
      for (unsigned u2 = 0; u2 < parentProc.all_functions->size(); u2++)
         (*all_functions).push_back((*parentProc.all_functions)[u2]);
   }

   all_modules = 0;
   if (parentProc.all_modules) {
      all_modules = new pdvector<module *>;
      for (unsigned u3 = 0; u3 < parentProc.all_modules->size(); u3++)
         (*all_modules).push_back((*parentProc.all_modules)[u3]);
   }

   some_modules = 0;
   if (parentProc.some_modules) {
      some_modules = new pdvector<module *>;
      for (unsigned u4 = 0; u4 < parentProc.some_modules->size(); u4++)
         (*some_modules).push_back((*parentProc.some_modules)[u4]);
   }
    
   some_functions = 0;
   if (parentProc.some_functions) {
      some_functions = new pdvector<function_base *>;
      for (unsigned u5 = 0; u5 < parentProc.some_functions->size(); u5++)
         (*some_functions).push_back((*parentProc.some_functions)[u5]);
   }

   waiting_for_resources = false;
#if defined(i386_unknown_linux2_0)
   signal_restore = parentProc.signal_restore;
#else
   signal_handler = parentProc.signal_handler;
#endif
   execed_ = false;

#if defined(rs6000_ibm_aix4_1)
   resetForkTrapData();
#endif

#if defined(SHM_SAMPLING) && defined(sparc_sun_sunos4_1_3)
   childUareaPtr = NULL;
#endif

#if !defined(mips_unknown_ce2_11) && !defined(i386_unknown_nt4_0)
   createLWP(0);
#endif   

   if (!attach()) {     // moved from ::forkProcess
      showErrorCallback(69, "Error in fork: cannot attach to child process");
      status_ = exited;
      return;
   }
   // threads... // 6/2/99 zhichen
   for (unsigned i=0; i<parentProc.threads.size(); i++) {
      dyn_thread *from_thr = parentProc.threads[i];
      dyn_thread *new_thr = new dyn_thread(this, from_thr);
      threads.push_back(new_thr);
#if defined(MT_THREAD)
      dyn_thread *thr = threads[i] ;
      string buffer;
      string pretty_name=string(thr->get_start_func()->prettyName().c_str());
      buffer = string("thr_")+string(thr->get_tid())+string("{")+pretty_name+string("}");
      resource *rid;
      rid = resource::newResource(this->rid, (void *)thr, nullString, buffer, 
                                  timeStamp::ts1970(), "", MDL_T_STRING, true);
      thr->update_rid(rid);
#endif
   }

   if( isRunning_() )
      status_ = running;
   else
      status_ = stopped;
   // would neonatal be more appropriate?  Nah, we've reached the first trap

#ifndef BPATCH_LIBRARY
#ifdef PAPI
   if (isPapiInitialized()) {
      papi = new papiMgr(this);
   }
#endif
#endif

   initTrampGuard();
}

// #endif

#ifdef SHM_SAMPLING
void process::registerInferiorAttachedSegs(void *inferiorAttachedAtPtr) {
   shmsample_cerr << "process pid " << getPid() << ": welcome to register with inferiorAttachedAtPtr=" << inferiorAttachedAtPtr << endl;

   theSharedMemMgr->registerInferiorAttachedAt(inferiorAttachedAtPtr);
}

// returns the offset address (ie. the offset in the shared memory manager
// segment) of the offset meta offset data (used to communicate the location
// of the shared meta data to the RTinst library)
Address process::initSharedMetaData() {
   shmMetaData->mallocInShm();
   shmMetaData->initialize(theSharedMemMgr->cookie, getpid(), getPid());
   assert(shMetaOffsetData == NULL);
   shMetaOffsetData = new sharedMetaOffsetData(*theSharedMemMgr, 
					       maxNumberOfThreads());
   shmMetaData->saveOffsetsIntoRTstructure(shMetaOffsetData);

   Address offsetOfShMetaOffsetData = shMetaOffsetData->getAddrInDaemon()
                          - (Address) theSharedMemMgr->getBaseAddrInDaemon();
   return offsetOfShMetaOffsetData;
}
#endif

extern bool forkNewProcess(string &file, string dir, pdvector<string> argv, 
		    pdvector<string> envp, string inputFile, string outputFile,
		    int &traceLink, 
		    int &pid, int &tid, 
		    int &procHandle, int &thrHandle,
		    int stdin_fd, int stdout_fd, int stderr_fd);

/*
 * Create a new instance of the named process.  Read the symbols and start
 *   the program
 */
process *createProcess(const string File, pdvector<string> argv, 
                        pdvector<string> envp, const string dir = "", 
                        int stdin_fd=0, int stdout_fd=1, int stderr_fd=2)
{
	// prepend the directory (if any) to the filename,
	// unless the filename is an absolute pathname
	// 
	// The filename is an absolute pathname if it starts with a '/' on UNIX,
	// or a letter and colon pair on Windows.
    string file = File;
	if( dir.length() > 0 )
	{
#if !defined(i386_unknown_nt4_0)
		if( !file.prefixed_by("/") )
		{
			// file does not start  with a '/', so it is a relative pathname
			// we modify it to prepend the given directory
			if( dir.suffixed_by("/") )
			{
				// the dir already has a trailing '/', so we can
				// just concatenate them to get an absolute path
				file = dir + file;
			}
			else
			{
				// the dir does not have a trailing '/', so we must
				// add a '/' to get the absolute path
				file = dir + "/" + file;
			}
		}
		else
		{
			// file starts with a '/', so it is an absolute pathname
			// DO NOT prepend the directory, regardless of what the
			// directory variable holds.
			// nothing to do in this case
		}

#else // !defined(i386_unknown_nt4_0)
		if( (file.length() < 2) ||	// file is too short to be a drive specifier
			!isalpha( file[0] ) ||	// first character in file is not a letter
			(file[1] != ':') )		// second character in file is not a colon
		{
			file = dir + "\\" + file;
		}
#endif // !defined(i386_unknown_nt4_0)
	}

#if defined(BPATCH_LIBRARY) && !defined(BPATCH_REDIRECT_IO)
    string inputFile;
    string outputFile;
#else
    // check for I/O redirection in arg list.
    string inputFile;
    for (unsigned i1=0; i1<argv.size(); i1++) {
      if (argv[i1] == "<") {
        inputFile = argv[i1+1];
        for (unsigned j=i1+2, k=i1; j<argv.size(); j++, k++)
          argv[k] = argv[j];
        argv.resize(argv.size()-2);
      }
    }
    // TODO -- this assumes no more than 1 of each "<", ">"
    string outputFile;
    for (unsigned i2=0; i2<argv.size(); i2++) {
      if (argv[i2] == ">") {
        outputFile = argv[i2+1];
        for (unsigned j=i2+2, k=i2; j<argv.size(); j++, k++)
          argv[k] = argv[j];
        argv.resize(argv.size()-2);
      }
    }


#endif /* BPATCH_LIBRARY */

    int traceLink = -1;
    //remove all ioLink related code for output redirection
    //int ioLink = stdout_fd;

    int pid;
    int tid;
    // Ignored except on NT (where we modify in forkNewProcess, and ignore the result???)
    int procHandle_temp;
    int thrHandle_temp;

    if (!forkNewProcess(file, dir, argv, envp, inputFile, outputFile,
		   traceLink, pid, tid, procHandle_temp, thrHandle_temp,
		   stdin_fd, stdout_fd, stderr_fd)) {
        // forkNewProcess is responsible for displaying error messages
        // Note: if the fork succeeds, but exec fails, forkNew...
        // will return true. 
        return NULL;
    }
#ifdef BPATCH_LIBRARY
    // Register the pid with the BPatch library (not yet associated with a
    // BPatch_thread object).
    assert(BPatch::bpatch != NULL);
    BPatch::bpatch->registerProvisionalThread(pid);
#endif

    int status = pid;
    
    // Get the file descriptor for the executable file
    // "true" value is for AIX -- waiting for an initial trap
    // it's ignored on other platforms
    fileDescriptor *desc = getExecFileDescriptor(file, status, true);
    if (!desc) {
        cerr << "Failed to find exec descriptor" << endl;
        return NULL;
    }
    // What a hack... getExecFileDescriptor waits for a trap
    // signal on AIX and returns the code in status. So we basically
    // have a pending TRAP that we need to handle, but not right
    // now.
#if defined(rs6000_ibm_aix4_1)
    int fileDescSignal = WSTOPSIG(status);
#endif

    image *img = image::parseImage(desc);
    if (!img) {
      // For better error reporting, two failure return values would be
      // useful.  One for simple error like because-file-not-because.
      // Another for serious errors like found-but-parsing-failed 
      //    (internal error; please report to paradyn@cs.wisc.edu)
        cerr << "Unable to parse target image" << endl;
      string msg = string("Unable to parse image: ") + file;
      showErrorCallback(68, msg.c_str());
      // destroy child process
      OS::osKill(pid);
      return(NULL);
    }
    
    /* parent */
    statusLine("initializing process data structures");
    
    process *theProc = new process(pid, img, traceLink
			       
#ifdef SHM_SAMPLING
			       , 7000 // shm seg key to try first
#endif
			       );
    // change this to a ctor that takes in more args

    assert(theProc);
#ifdef mips_unknown_ce2_11 //ccw 27 july 2000 : 29 mar 2001
    //the MIPS instruction generator needs the Gp register value to
    //correctly calculate the jumps.  In order to get it there it needs
    //to be visible in Object-nt, and must be taken from process.
    void *cont;
    //DebugBreak();
    cont = theProc->GetRegisters(thrHandle_temp); //ccw 10 aug 2000 : ADD thrHandle HERE!
    img->getObjectNC().set_gp_value(((w32CONTEXT*) cont)->IntGp);
#endif
    
    processVec.push_back(theProc);
    activeProcesses++;
    
    // find the signal handler function
    theProc->findSignalHandler(); // should this be in the ctor?
    
    // In the ST case, threads[0] (the only one) is effectively
    // a pass-through for process operations. In MT, it's the
    // main thread of the process and is handled correctly
    
#if defined(i386_unknown_nt4_0) || (defined mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001
    dyn_lwp *lwp = theProc->createLWP(tid, 0, (void*)thrHandle_temp);
	 theProc->threads += new dyn_thread(theProc, tid, 0, lwp);
#else
    theProc->threads += new dyn_thread(theProc);
    dyn_thread *new_thr = theProc->threads[theProc->threads.size()-1];
#endif

#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
    // XXXX - this is a hack since getExecFileDescriptor needed to wait for
    //    the TRAP signal.
    // We really need to move most of the above code (esp parse image)
    //    to the TRAP signal handler.  The problem is that we don't
    //    know the base addresses until we get the load info via ptrace.
    //    In general it is even harder, since dynamic libs can be loaded
    //    at any time.
    handleProcessEvent(theProc, procSignalled, fileDescSignal, 0);
    
#endif
    theProc->loadDyninstLib();
    while (!theProc->reachedBootstrapState(bootstrapped)) {
        // We're waiting for something... so wait
        // true: block until a signal is received (efficiency)
        decodeAndHandleProcessEvent(true);
    }
    
    return theProc;    
}


process *attachProcess(const string &progpath, int pid) 
{
  // implementation of dynRPC::attach() (the igen call)
  // This is meant to be "the other way" to start a process (competes w/ createProcess)
  
  // progpath gives the full path name of the executable, which we use ONLY to
  // read the symbol table.
  
  // We try to make progpath optional, since given pid, we should be able to
  // calculate it with a clever enough search of the process' PATH, examining
  // its argv[0], examining its current directory, etc.  /proc gives us this
  // information on solaris...not sure about other platforms...

    // No longer take the afterAttach argument. Instead, the process object records
    // the state of the process at attach, and the user can read and act on this as
    // they please -- bernat, JAN03

  attach_cerr << "welcome to attachProcess for pid " << pid << endl;

  // QUESTION: When we attach to a process, do we want to redirect its stdout/stderr
  //           (like we do when we fork off a new process the 'usual' way)?
  //           My first guess would be no.  -ari
  //           But although we may ignore the io, we still need the trace stream.
  
  // When we attach to a process, we don't fork...so this routine is much simpler
  // than its "competitor", createProcess() (above).
  
  string fullPathToExecutable = process::tryToFindExecutable(progpath, pid);
  if (!fullPathToExecutable.length())
      return NULL;
      
  int status = pid;
  fileDescriptor *desc = getExecFileDescriptor(fullPathToExecutable,
					       status, false);
  if (!desc)
      return NULL;
  image *theImage = image::parseImage(desc);
  if (theImage == NULL) {
    // two failure return values would be useful here, to differentiate
    // file-not-found vs. catastrophic-parse-error.
    string msg = string("Unable to parse image: ") + fullPathToExecutable;
    showErrorCallback(68, msg.c_str());
    return NULL;
  }
  
  // NOTE: the actual attach happens in the process "attach" constructor:
  bool success=false;
  process *theProc = new process(pid, theImage, success
#ifdef SHM_SAMPLING
				 ,7000 // shm seg key to try first
#endif                            
				 );

  assert(theProc);

  if (!success) {
    // XXX Do we need to do something to get rid of theImage, too?
    delete theProc;
    return NULL;
  }
  
  // Note: it used to be that the attach ctor called pause()...not anymore...so
  // the process is probably running even as we speak.
  
  processVec.push_back(theProc);
  activeProcesses++;

  theProc->threads += new dyn_thread(theProc);

  // we now need to dynamically load libdyninstRT.so.1 - naim
  if (!theProc->pause()) {
    logLine("WARNING: pause failed\n");
    assert(0);
  }
  
  // find the signal handler function
  theProc->findSignalHandler(); // shouldn't this be in the ctor?

  if (!theProc->loadDyninstLib()) {
      delete theProc;
      return NULL;
  }
    // The process is paused at this point. Run if appropriate.

#if defined(alpha_dec_osf4_0)
  // need to perform this after dyninst Heap is present and happy

    // Actually, we need to perform this after DYNINSTinit() has
    // been invoked in the mutatee.  So, the following line was
    // moved to BPatch_thread.C.   RSC 08-26-2002
  //theProc->getDyn()->setMappingHooks(theProc);
#endif
  
  return theProc; // successful
}




/***************************************************************************
 **** Runtime library initialization code (Dyninst)                     ****
 ***************************************************************************/

/*
 * Gratuitously large comment. This diagrams the startup flow of 
 * messages between the mutator and mutatee. Entry points 
 * for create and attach process are both given.
 *     Mutator           Signal              Mutatee
 * Create:
 *     Fork/Exec
 *                     <-- Trap              Halted in exec
 *     Install trap in main             
 *                     <-- Trap              Halted in main
 *  Attach: (also paused, not in main)
 *     Install call to dlopen/
 *     LoadLibrary       
 *                     <-- Trap              In library load
 *     Set parameters in library
 *                     <-- Trap              Finished loading
 *     Restore code and leave paused
 *     Finalize library
 *       If finalizing fails, 
 *       init via iRPC
 */

/*
 * In all cases, the process is left paused at the entry of main
 * (create) or where it was (attach). No permanent instrumentation
 * is inserted.
 */

// Load and initialize the runtime library: returns when the RT lib
// is both loaded and initialized.
// Return val: false=error condition

bool process::loadDyninstLib() {
    // Wait for the process to get to an initialized (dlopen exists)
    // state
    while (!reachedBootstrapState(initialized)) {
        decodeAndHandleProcessEvent(true);
    }
    assert(status_ == stopped);
    

    // We've hit the initialization trap, so load dyninst lib and
    // force initialization
    string buffer = string("PID=") + string(pid);
    buffer += string(", initializing shared objects");       
    statusLine(buffer.c_str());
    if (!initSharedObjects())
        assert(0 && "Failed to init shared objects!");
    if (dyninstLibAlreadyLoaded()) {
        logLine("ERROR: dyninst library already loaded, we missed initialization!");
        assert(0);
    }

#if defined(alpha_dec_osf4_0)
    // This installs a trap at dlopen/dlclose. Should be wrapped
    // into initSharedObjects
    // need to perform this after dyninst Heap is present and happy
    dyn->setMappingHooks(this);
#endif

    // Set the name of the dyninst RT lib
    if (dyninstRT_name.length() == 0) {
        // Get env variable
        if (getenv("DYNINSTAPI_RT_LIB") != NULL) {
            dyninstRT_name = getenv("DYNINSTAPI_RT_LIB");
        }
        else {
            string msg = string("Environment variable " + string("DYNINSTAPI_RT_LIB")
                                + " has not been defined for process ") + string(pid);
            showErrorCallback(101, msg);
            assert(0 && "Dyninst RT lib not defined!");
        }
    }
#if !defined(i386_unknown_nt4_0)
    // Check to see if the library given exists.
    if (access(dyninstRT_name.c_str(), R_OK))
#else
    if (_access(dyninstRT_name.c_str(), 04))
#endif
    {
        string msg = string("Runtime library ") + dyninstRT_name
        + string(" does not exist or cannot be accessed!");
        showErrorCallback(101, msg);
        assert(0 && "Dyninst RT lib cannot be accessed!");
    }
    // Set up a callback to be run when dyninst lib is loaded
    // NT has some odd naming problems, so we only use the root
#if defined(i386_unknown_nt4_0)
    char dllFilename[_MAX_FNAME];
    _splitpath (dyninstRT_name.c_str(),
                NULL, NULL, dllFilename, NULL);
    
    registerLoadLibraryCallback(string(dllFilename), dyninstLibLoadCallback, NULL);
#else
    registerLoadLibraryCallback(dyninstRT_name, dyninstLibLoadCallback, NULL);
#endif

    // Force a call to dlopen(dyninst_lib)
    buffer = string("PID=") + string(pid);
    buffer += string(", loading dyninst library");       
    statusLine(buffer.c_str());
    loadDYNINSTlib();
    setBootstrapState(loadingRT);
    
    if (!continueProc()) {
        assert(0);
    }

    // Loop until the dyninst lib is loaded
    while (!reachedBootstrapState(loadedRT)) {
        decodeAndHandleProcessEvent(true);
    }

    // Get rid of the callback
    unregisterLoadLibraryCallback(dyninstRT_name);

    buffer = string("PID=") + string(pid);
    buffer += string(", initializing mutator-side structures");
    statusLine(buffer.c_str());    
    // The following calls depend on the RT library being parsed,
    // but not on dyninstInit being run
    initInferiorHeap();
    // This must be done after the inferior heap is initialized
    initTrampGuard();
    extern pdvector<sym_data> syms_to_find;
    if (!heapIsOk(syms_to_find))
        return false;
    
    // The library is loaded, so do mutator-side initialization
    buffer = string("PID=") + string(pid);
    buffer += string(", finalizing RT library");
    statusLine(buffer.c_str());    
    int result = finalizeDyninstLib();

    if (!result) {
        // For some reason we haven't run dyninstInit successfully.
        // Probably because we didn't set parameters before 
        // dyninstInit was automatically run. Catchup with
        // an inferiorRPC is the best bet.
        buffer = string("PID=") + string(pid);
        buffer += string(", finalizing library via inferior RPC");
        statusLine(buffer.c_str());    

        iRPCDyninstInit();
    }

    buffer = string("PID=") + string(pid);
    buffer += string(", dyninst RT lib ready");
    statusLine(buffer.c_str());    

    return true;
}

// Set up the parameters for DYNINSTinit in the RT lib

bool process::setDyninstLibInitParams() {

   attach_cerr << "process::setDYNINSTinitArguments()" << endl;

   int pid = getpid();
   
   // Cause: 
   // 1 = created
   // 2 = forked
   // 3 = attached
   
   int cause;
   if (createdViaAttach)
       cause = 3;
   else if (createdViaFork)
       cause = 2;
   else 
       cause = 1;
   
   // Now we write these variables into the following global vrbles
   // in the dyninst library:
   // libdyninstAPI_RT_init_localCause
   // libdyninstAPI_RT_init_localPid

   Symbol causeSym;
   if (!getSymbolInfo("libdyninstAPI_RT_init_localCause", causeSym))
       if (!getSymbolInfo("_libdyninstAPI_RT_init_localCause", causeSym))
           assert(0 && "Could not find symbol libdyninstAPI_RT_init_localCause");
   assert(causeSym.type() != Symbol::PDST_FUNCTION);
   writeDataSpace((void*)causeSym.addr(), sizeof(int), (void *)&cause);
   
   Symbol pidSym;
   if (!getSymbolInfo("libdyninstAPI_RT_init_localPid", pidSym))
       if (!getSymbolInfo("_libdyninstAPI_RT_init_localPid", pidSym))
           assert(0 && "Could not find symbol libdyninstAPI_RT_init_localPid");
   assert(pidSym.type() != Symbol::PDST_FUNCTION);
   writeDataSpace((void*)pidSym.addr(), sizeof(int), (void *)&pid);
   
   attach_cerr << "process::installBootstrapInst() complete" << endl;
   
   return true;
}

// Callback for the above
void process::dyninstLibLoadCallback(process *p, string /*ignored*/, void * /*ignored*/) {
    p->setDyninstLibInitParams();
}

// Call DYNINSTinit via an inferiorRPC
bool process::iRPCDyninstInit() {
    // Duplicates the parameter code in setDyninstLibInitParams()
    int cause;
    int pid = getpid();

    if (createdViaAttach)
        cause = 3;
    else if (createdViaFork)
        cause = 2;
    else 
        cause = 1;
    pdvector<AstNode*> the_args(2);
    the_args[0] = new AstNode(AstNode::Constant, (void*)cause);
    the_args[1] = new AstNode(AstNode::Constant, (void*)pid);
    AstNode *dynInit = new AstNode("DYNINSTinit", the_args);
    removeAst(the_args[0]); removeAst(the_args[1]);
    
    postRPCtoDo(dynInit,
                true, // Don't update cost
                process::DYNINSTinitCompletionCallback,
                NULL, // No user data
                -1); // Not a metric definition
                // No particular thread or LWP

    // We loop until dyninst init has run (check via the callback)
    while (!reachedBootstrapState(bootstrapped)) {
        launchRPCs(false); // false: not running
        decodeAndHandleProcessEvent(true);
    }
    return true;
}

bool process::attach() {
   dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(lwps);
   dyn_lwp *lwp;
   unsigned index;
   
   while (lwp_iter.next(index, lwp)) {
      if (!lwp->openFD()) {
         deleteLWP(lwp);
         return false;
      }
   }
   return attach_();
}

void process::DYNINSTinitCompletionCallback(process* theProc,
                                            void* /*userData*/, // user data
                                            void* /*ret*/) // return value from DYNINSTinit
{
    theProc->finalizeDyninstLib();
}

// Callback: finish mutator-side processing for dyninst lib

bool process::finalizeDyninstLib() {

    assert(status_ == stopped);

   if (reachedBootstrapState(bootstrapped)) {
       return true;
   }

   DYNINST_bootstrapStruct bs_record;
   if (!extractBootstrapStruct(&bs_record))
      assert(false);

   // Read the structure; if event 0 then it's undefined! (not yet written)
   if (bs_record.event == 0){
       return false;
   }

   assert(bs_record.event == 1 || bs_record.event == 2 || bs_record.event==3);

   bool calledFromFork = (bs_record.event == 2);
   bool calledFromAttach = (bs_record.event == 3);
   
   if (!calledFromFork) {
       getObservedCostAddr();

       // Install initial instrumentation requests
       string str=string("PID=") + string(bs_record.pid) + ", installing default (DYNINST) inst...";
       statusLine(str.c_str());
       
       extern pdvector<instMapping*> initialRequests; // init.C
       installInstrRequests(initialRequests);
   }
   
#if defined(BPATCH_LIBRARY)
   // It is now safe to call the exec callback
   if (wasExeced()) {
       BPatch::bpatch->registerExec(thread);
   }
   
#endif
   
   if (calledFromFork) {
       process *parentProcess = findProcess(bs_record.ppid);
       if (parentProcess) {
           if (parentProcess->status() == stopped) {
               if (!parentProcess->continueProc())
                   assert(false);
           }
           else
               parentProcess->continueAfterNextStop();
       }
       
   }
   
   if (!calledFromAttach) {
       string str=string("PID=") + string(bs_record.pid) + ", dyninst ready.";
       statusLine(str.c_str());
   }

   // Ready to rock
   setBootstrapState(bootstrapped);
    
   return true;
}

dyn_thread *process::STdyn_thread() { 
   assert(! multithread_capable());
   assert(threads.size()>0);
   return threads[0];
}

/*
 * This function is needed in the unique case where we want to 
 * attach to an application which has been previously stopped
 * in the exec() system call as in the normal case (createProcess).
 * In this particular case, the SIGTRAP due to the exec() has been 
 * previously caught by another process, therefore we should taking into
 * account this issue in the necessary platforms. 
 * Basically, is an hybrid case between addprocess() and attachProcess().
 * 
 */

bool AttachToCreatedProcess(int pid,const string &progpath)
{


    int traceLink = -1;

    string fullPathToExecutable = process::tryToFindExecutable(progpath, pid);
    
    if (!fullPathToExecutable.length()) {
      return false;
    }  

#ifdef BPATCH_LIBRARY
    // Register the pid with the BPatch library (not yet associated with a
    // BPatch_thread object).
    assert(BPatch::bpatch != NULL);
    BPatch::bpatch->registerProvisionalThread(pid);
#endif

    int status = pid;
    
    // Get the file descriptor for the executable file
    // "true" value is for AIX -- waiting for an initial trap
    // it's ignored on other platforms
    fileDescriptor *desc = 
        getExecFileDescriptor(fullPathToExecutable, status, true);
    // What a hack... getExecFileDescriptor waits for a trap
    // signal on AIX and returns the code in status. So we basically
    // have a pending TRAP that we need to handle, but not right
    // now.
#if defined(rs6000_ibm_aix4_1)
    int fileDescSignal = WSTOPSIG(status);
#endif 

    if (!desc) {
      return false;
    }

#ifndef BPATCH_LIBRARY

// We bump up batch mode here; the matching bump-down occurs after 
// shared objects are processed (after receiving the SIGSTOP indicating
// the end of running DYNINSTinit; more specifically, 
// finalizeDyninstInit(). Prevents a diabolical w/w deadlock on 
// solaris --ari
    tp->resourceBatchMode(true);

#endif

    image *img = image::parseImage(desc);

    if (img==NULL) {

        // For better error reporting, two failure return values would be
        // useful.  One for simple error like because-file-not-because.
        // Another for serious errors like found-but-parsing-failed 
        //    (internal error; please report to paradyn@cs.wisc.edu)

        string msg = string("Unable to parse image: ") + fullPathToExecutable;
        showErrorCallback(68, msg.c_str());
        // destroy child process
        OS::osKill(pid);
        return(false);
    }

    /* parent */
    statusLine("initializing process data structures");

    // The same process ctro. is used as in the "normal" case but
    // here, traceLink is -1 instead of a positive value.
    process *ret = new process(pid, img, traceLink

#ifdef SHM_SAMPLING
			       , 7000  // shm seg key to try first
#endif
                                   );

    assert(ret);

#ifdef mips_unknown_ce2_11 //ccw 27 july 2000 : 29 mar 2001

    //the MIPS instruction generator needs the Gp register value to
    //correctly calculate the jumps.  In order to get it there it needs
    //to be visible in Object-nt, and must be taken from process.
    void *cont;
    //DebugBreak();
    //ccw 10 aug 2000 : ADD thrHandle HERE
    
    cont = ret->GetRegisters(thrHandle_temp);

    img->getObjectNC().set_gp_value(((w32CONTEXT*) cont)->IntGp);
#endif

    processVec.push_back(ret);
    activeProcesses++;

#ifndef BPATCH_LIBRARY
    if (!costMetric::addProcessToAll(ret)) {
        assert(false);
    }
#endif

    // find the signal handler function
    ret->findSignalHandler(); // should this be in the ctor?

    // initializing vector of threads - thread[0] is really the 
    // same process

#if defined(i386_unknown_nt4_0) || (defined mips_unknown_ce2_11)
    //ccw 20 july 2000 : 29 mar 2001
    dyn_lwp *lwp = ret->createLWP(0);
    ret->threads += new dyn_thread(ret, 0, 0, lwp);
#else
    ret->threads += new dyn_thread(ret);     
#endif

    // initializing hash table for threads. This table maps threads to
    // positions in the superTable - naim 4/14/97

#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
    // XXXX - this is a hack since getExecFileDescriptor needed to wait for
    //        the TRAP signal.
    // We really need to move most of the above code (esp parse image)
    // to the TRAP signal handler.  The problem is that we don't
    // know the base addresses until we get the load info via ptrace.
    // In general it is even harder, since dynamic libs can be loaded
    // at any time.
    handleProcessEvent(ret, procSignalled, fileDescSignal, 0);    
#endif
    
    return(true);

} // end of AttachToCreatedProcess


#ifndef BPATCH_LIBRARY
extern void disableAllInternalMetrics();
void paradyn_handleProcessExit(process *proc, int exitStatus);
#endif

void handleProcessExit(process *proc, int exitStatus) {
  proc->exitCode_ = exitStatus;

  if (proc->status() == exited)
    return;

  --activeProcesses;

  proc->Exited(); // updates status line

#ifndef BPATCH_LIBRARY
  paradyn_handleProcessExit(proc, exitStatus);

  if (activeProcesses == 0)
    disableAllInternalMetrics();
#endif

#ifdef PARADYND_PVM
  if (pvm_running) {
    PDYN_reportSIGCHLD(proc->getPid(), exitStatus);
  }
#endif

  // Perhaps these lines can be un-commented out in the future, but since
  // cleanUpAndExit() does the same thing, and it always gets called
  // (when paradynd detects that paradyn died), it's not really necessary
  // here.  -ari
//  for (unsigned lcv=0; lcv < processVec.size(); lcv++)
//     if (processVec[lcv] == proc) {
//        delete proc; // destructor removes shm segments...
//      processVec[lcv] = NULL;
//     }
}


#ifndef BPATCH_LIBRARY
/*
   process::forkProcess: called when a process forks, to initialize a new
   process object for the child.

   the variable childHasInstrumentation is true if the child process has the 
   instrumentation of the parent. This is the common case.
   On some platforms (AIX) the child does not have any instrumentation because
   the text segment of the child is not a copy of the parent text segment at
   the time of the fork, but a copy of the original text segment of the parent,
   without any instrumentation.
   (actually, childHasInstr is obsoleted by aix's completeTheFork() routine)
*/
process *process::forkProcess(const process *theParent, pid_t childPid,
			      int iTrace_fd, key_t theKey,
			      void *applAttachedPtr) {
   forkexec_cerr << "paradynd welcome to process::forkProcess\n; parent pid=" 
		 << theParent->getPid() << "; calling fork ctor now\n";
   
   // Call the "fork" ctor:
   process *ret = new process(*theParent, (int)childPid, iTrace_fd, theKey,
			      applAttachedPtr);
   assert(ret);

   forkexec_cerr << "paradynd fork ctor has completed ok...child pid is " 
		 << ret->getPid() << "\n";

   processVec += ret;
   activeProcesses++;

   if (!costMetric::addProcessToAll(ret))
      assert(false);

   // We used to do a ret->attach() here...it was moved to the fork ctor, so
   // it's been done already.

   /* all instrumentation on the parent is active on the child */
   /* TODO: what about instrumentation inserted near the fork time??? */
   ret->baseMap = theParent->baseMap; // WHY IS THIS HERE?

#ifdef BPATCH_LIBRARY
   /* XXX Not sure if this is the right thing to do. */
   ret->instPointMap = theParent->instPointMap;
#endif

   return ret;
}
#endif

#ifdef SHM_SAMPLING
void process::processCost(unsigned obsCostLow,
                          timeStamp wallTime,
                          timeStamp processTime) {
  // wallTime and processTime should compare to DYNINSTgetWallTime() and
  // DYNINSTgetCPUtime().

  // check for overflow, add to running total, convert cycles to seconds, and
  // report.  Member vrbles of class process: lastObsCostLow and cumObsCost
  // (the latter a 64-bit value).

  // code to handle overflow used to be in rtinst; we borrow it pretty much
  // verbatim. (see rtinst/RTposix.c)
  if (obsCostLow < lastObsCostLow) {
    // we have a wraparound
    cumObsCost += ((unsigned)0xffffffff - lastObsCostLow) + obsCostLow + 1;
  }
  else
    cumObsCost += (obsCostLow - lastObsCostLow);
  
  lastObsCostLow = obsCostLow;
  //  sampleVal_cerr << "processCost- cumObsCost: " << cumObsCost << "\n"; 
  timeLength observedCost(cumObsCost, getCyclesPerSecond());
  timeUnit tu = getCyclesPerSecond(); // just used to print out
  //  sampleVal_cerr << "processCost: cyclesPerSecond=" << tu
  //		 << "; cum obs cost=" << observedCost << "\n";
  
  // Notice how most of the rest of this is copied from processCost() of
  // metric.C.  Be sure to keep the two "in sync"!

  extern costMetric *totalPredictedCost; // init.C
  extern costMetric *observed_cost;      // init.C
  
  const timeStamp lastProcessTime = 
    totalPredictedCost->getLastSampleProcessTime(this);
  //  sampleVal_cerr << "processCost- lastProcessTime: " <<lastProcessTime << "\n";
  // find the portion of uninstrumented time for this interval
  timeLength userPredCost = timeLength::sec() + getCurrentPredictedCost();
  //  sampleVal_cerr << "processCost- userPredCost: " << userPredCost << "\n";
  const double unInstTime = (processTime - lastProcessTime) / userPredCost; 
  //  sampleVal_cerr << "processCost- unInstTime: " << unInstTime << "\n";
  // update predicted cost
  // note: currentPredictedCost is the same for all processes
  //       this should be changed to be computed on a per process basis
  pdSample newPredCost = totalPredictedCost->getCumulativeValue(this);
  //  sampleVal_cerr << "processCost- newPredCost: " << newPredCost << "\n";
  timeLength tempPredCost = getCurrentPredictedCost() * unInstTime;
  //  sampleVal_cerr << "processCost- tempPredCost: " << tempPredCost << "\n";
  newPredCost += pdSample(tempPredCost.getI(timeUnit::ns()));
  //  sampleVal_cerr << "processCost- tempPredCost: " << newPredCost << "\n";
  totalPredictedCost->updateValue(this, wallTime, newPredCost, processTime);
  // update observed cost
  pdSample sObsCost(observedCost);
  observed_cost->updateValue(this, wallTime, sObsCost, processTime);
}
#endif

/*
 * Copy data from controller process to the named process.
 */
bool process::writeDataSpace(void *inTracedProcess, unsigned size,
                             const void *inSelf) {
  bool needToCont = false;

  if (status_ == exited)
    return false;

  if (status_ == running) {
    needToCont = true;
    if (! pause())
      return false;
  }

  if (status_ != stopped && status_ != neonatal) {
    sprintf(errorLine, "Internal error: "
                "Unexpected process state %d in process::writeDataSpace",
                (int)status_);
    logLine(errorLine);
    showErrorCallback(39, errorLine);
    return false;
  }

  bool res = writeDataSpace_(inTracedProcess, size, inSelf);
  if (!res) {
    string msg = string("System error: unable to write to process data space:")
                   + string(sys_errlist[errno]);
    showErrorCallback(38, msg);
    return false;
  }

  if (needToCont) {
    return this->continueProc();
  }
  return true;
}

bool process::readDataSpace(const void *inTracedProcess, unsigned size,
                            void *inSelf, bool displayErrMsg) {
  bool needToCont = false;

  if (status_ == exited)
    return false;

  if (status_ == running) {
    needToCont = true;
    if (! pause()) {
      sprintf(errorLine, "in readDataSpace, status_ = running, but unable to pause()\n");
      logLine(errorLine);
      return false;
    }
  }
  if (status_ != stopped && status_ != neonatal) {
    sprintf(errorLine, "Internal error: "
                "Unexpected process state %d in process::readDataSpace",
                (int)status_);
    logLine(errorLine);
    showErrorCallback(39, errorLine);
    return false;
  }

  bool res = readDataSpace_(inTracedProcess, size, inSelf);
  if (!res) {
    if (displayErrMsg) {
      sprintf(errorLine, "System error: "
          "<>unable to read %d@%s from process data space: %s (pid=%d)",
	  size, Address_str((Address)inTracedProcess), 
          sys_errlist[errno], getPid());
      string msg(errorLine);
      showErrorCallback(38, msg);
	printf(" EXIT ");
    }
    return false;
 }

  if (needToCont) {
    needToCont = this->continueProc();
    if (!needToCont) {
        sprintf(errorLine, "warning : readDataSpace, needToCont FALSE, returning FALSE\n");
        logLine(errorLine);
    }
    return needToCont;
  }
  return true;

}

bool process::writeTextWord(caddr_t inTracedProcess, int data) {
  bool needToCont = false;

  if (status_ == exited)
    return false;

  if (status_ == running) {
    needToCont = true;
    if (! pause())
      return false;
  }

  if (status_ != stopped && status_ != neonatal) {
    sprintf(errorLine, "Internal error: "
                "Unexpected process state %d in process::writeTextWord",
                (int)status_);
    showErrorCallback(39, errorLine);
    return false;
  }

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
  if (!isAddrInHeap((Address)inTracedProcess)) {
    if (!saveOriginalInstructions((Address)inTracedProcess, sizeof(int)))
        return false;
    afterMutationList.insertTail((Address)inTracedProcess, sizeof(int), &data);
  }
#endif

  bool res = writeTextWord_(inTracedProcess, data);
  if (!res) {
    string msg = string("System error: unable to write to process text word:")
                   + string(sys_errlist[errno]);
    showErrorCallback(38, msg);
    return false;
  }

  if (needToCont) {
    return this->continueProc();
  }
  return true;

}

bool process::writeTextSpace(void *inTracedProcess, u_int amount, 
                             const void *inSelf) {
  bool needToCont = false;

  if (status_ == exited)
    return false;

  if (status_ == running) {
    needToCont = true;
    if (! pause())
      return false;
  }

  if (status_ != stopped && status_ != neonatal) {
    sprintf(errorLine, "Internal error: "
                "Unexpected process state %d in process::writeTextSpace",
                (int)status_);
    showErrorCallback(39, errorLine);
    return false;
  }

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
  if (!isAddrInHeap((Address)inTracedProcess)) {
    if (!saveOriginalInstructions((Address)inTracedProcess, amount))
        return false;
    afterMutationList.insertTail((Address)inTracedProcess, amount, inSelf);
  }
#endif

  bool res = writeTextSpace_(inTracedProcess, amount, inSelf);
  if (!res) {
    string msg = string("System error: unable to write to process text space:")
                   + string(sys_errlist[errno]);
    showErrorCallback(38, msg);
    return false;
  }

  if (needToCont) {
    return this->continueProc();
  }
  return true;
}

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
bool process::readTextSpace(const void *inTracedProcess, u_int amount,
                            const void *inSelf)
{
  bool needToCont = false;

  if (status_ == exited){
	printf(" STATUS == EXITED");
    return false;
	}
  if (status_ == running) {
    needToCont = true;
    if (! pause())
      return false;
  }

  if (status_ != stopped && status_ != neonatal) {
    sprintf(errorLine, "Internal error: "
                "Unexpected process state %d in process::readTextSpace",
                (int)status_);
    showErrorCallback(39, errorLine);
    return false;
  }

  bool res = readTextSpace_(const_cast<void*>(inTracedProcess), amount, inSelf);
  if (!res) {
    sprintf(errorLine, "System error: "
          "[]unable to read %d@%s from process text space: %s (pid=%d)",
	  amount, Address_str((Address)inTracedProcess),
          sys_errlist[errno], getPid());
    string msg(errorLine);
    showErrorCallback(38, msg);
    return false;
  }

  if (needToCont) {
    return this->continueProc();
  }
  return true;

}
#endif /* BPATCH_SET_MUTATIONS_ACTIVE */

bool process::pause() {
  if (status_ == stopped || status_ == neonatal) {
      return true;
  }
  
  if (status_ == exited) {
    sprintf(errorLine, "warn : in process::pause, trying to pause exited process, returning FALSE\n");
    logLine(errorLine);
    return false;
  }

  if (status_ == running && reachedBootstrapState(initialized)) {
      bool res = pause_();
      if (!res) {
          sprintf(errorLine, "warn : in process::pause, pause_ unable to pause process\n");
          logLine(errorLine);
          return false;
      }
      
      status_ = stopped;
  }
  else {
    // The only remaining combination is: status==running but haven't yet
    // reached first break.  We never want to pause before reaching the
    // first break (trap, actually).  But should we be returning true or false in this
    // case?
  }

  return true;
}

// handleIfDueToSharedObjectMapping: if a trap instruction was caused by
// a dlopen or dlclose event then return true
bool process::handleIfDueToSharedObjectMapping(){

   if(!dyn) { 
      return false;
   }

   pdvector<shared_object *> *changed_objects = 0;
   u_int change_type = 0;
   bool error_occured = false;
   bool ok = dyn->handleIfDueToSharedObjectMapping(this,&changed_objects,
                                                   change_type,error_occured);
   // if this trap was due to dlopen or dlclose, and if something changed
   // then figure out how it changed and either add or remove shared objects
   if(ok && !error_occured && (change_type != SHAREDOBJECT_NOCHANGE)) {

      // if something was added then call process::addASharedObject with
      // each element in the vector of changed_objects
      if((change_type == SHAREDOBJECT_ADDED) && changed_objects) {

         for(u_int i=0; i < changed_objects->size(); i++) {
             // TODO: currently we aren't handling dlopen because  
             // we don't have the code in place to modify existing metrics
             // This is what we really want to do:
             // Paradyn -- don't add new symbols unless it's the runtime
             // library
             // UGLY.
             
             if(addASharedObject(*((*changed_objects)[i]))){
                 (*shared_objects).push_back((*changed_objects)[i]);

                 // Check to see if there is a callback registered for this
                 // library, and if so call it.
                 string libname =  (*changed_objects)[i]->getName();

                 runLibraryCallback(libname);

             } // addASharedObject, above
             else {
                 //logLine("Error after call to addASharedObject\n");
                 delete (*changed_objects)[i];
             }
         }
         delete changed_objects;
         
      } else if((change_type == SHAREDOBJECT_REMOVED) && (changed_objects)) { 

         // TODO: handle this case
         // if something was removed then call process::removeASharedObject
         // with each element in the vector of changed_objects
         // for now, just delete shared_objects to avoid memory leeks
         for(u_int i=0; i < changed_objects->size(); i++){
            delete (*changed_objects)[i];
         }

         delete changed_objects;
      }

      // TODO: add support for adding or removing new code resource once the 
      // process has started running...this means that currently collected
      // metrics may have to have aggregate components added or deleted
      // this should be added to process::addASharedObject and 
      // process::removeASharedObject  
   }

   return ok;
}

// Register a callback to be made when a library is detected
// in handleSharedObjectMapping (above). In many cases this allows
// a user to modify the library before any associated _init function
// is run. Note: ordering is NOT guaranteed. This is best-effort.

bool process::registerLoadLibraryCallback(string libname, 
                                          loadLibraryCallbackFunc callback,
                                          void *data) {
    if (loadLibraryCallbacks_.defines(libname)) {
        cerr << "Possible error: predefined callback for "
             << libname << " being overwritten!" << endl;
        // Return false here? For now, continue
    }
    libraryCallback *lib = new libraryCallback();
    lib->callback = callback;
    lib->data = data;
    loadLibraryCallbacks_[libname] = lib;
    return true;    
}

// Unregister the above callback. Arbitrary decision:
// callbacks are persistent until unregistered.
bool process::unregisterLoadLibraryCallback(string libname) {
    if (loadLibraryCallbacks_.defines(libname)) {
        libraryCallback *lib = loadLibraryCallbacks_[libname];
        loadLibraryCallbacks_.undef(libname);
        delete lib;
        return true;
    }
    return false;
}

//
bool process::runLibraryCallback(string libname) {
    if (loadLibraryCallbacks_.defines(libname)) {
        libraryCallback *lib = loadLibraryCallbacks_[libname];
        (lib->callback)(this, libname, lib->data);
    }
    return true;
}

//
//  If this process is a dynamic executable, then get all its 
//  shared objects, parse them, and define new instpoints and resources 
//
bool process::initSharedObjects(){

    // get shared objects, parse them, and define new resources 
    // For WindowsNT we don't call getSharedObjects here, instead
    // addASharedObject will be called directly by pdwinnt.C
#if !defined(i386_unknown_nt4_0)  && !(defined mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001
    this->getSharedObjects();
#endif

    return true;
}

/* Checks whether the shared object SO is the runtime instrumentation
   library.  The test looks for "libdyninst" in the library name (both
   the Dyninst and Paradyn rtinst libs have this substring on every
   platform).

   Returns:
   0  if SO is not the runtime library
   1  if SO is the runtime library, but it has not been initialized
   2  if SO is the runtime library, and it has been initialized by Dyninst
   3  if SO is the runtime library, and it has been initialized by Paradyn
 */
static int
check_rtinst(process *proc, shared_object *so)
{
     const char *name, *p;
     static const char *libdyn = "libdyninst";
     static int len = 10; /* length of libdyn */

     name = (so->getName()).c_str();

     p = strrchr(name, '/');
     if (!p)
	  /* name is relative to current directory */
	  p = name;
     else
	  ++p; /* skip '/' */

     if (0 != strncmp(p, libdyn, len)) {
	  return 0;
     }

     /* Now we check if the library has initialized */
     Symbol sym;
     if (! so->getSymbolInfo("DYNINSThasInitialized", sym)) {
	  return 0;
     }
     Address addr = sym.addr() + so->getBaseAddress();
     unsigned int val;
     if (! proc->readDataSpace((void*)addr, sizeof(val), (void*)&val, true)) {
	  return 0;
     }
     if (val == 0) {
	  /* The library has been loaded, but not initialized */
	  return 1;
     } else
	  return val;
}

// addASharedObject: This routine is called whenever a new shared object
// has been loaded by the run-time linker
// It processes the image, creates new resources
bool process::addASharedObject(shared_object &new_obj, Address newBaseAddr){
    int ret;
    string msg;
    // FIXME

    image *img = image::parseImage(new_obj.getFileDesc(),newBaseAddr); 

    if(!img){
        //logLine("error parsing image in addASharedObject\n");
        return false;
    }
    new_obj.addImage(img);
    // TODO: check for "is_elf64" consistency (Object)

    // If the static inferior heap has been initialized, then 
    // add whatever heaps might be in this guy to the known heap space.
    if (heap.bufferPool.size() != 0)
      // Need to search/add heaps here, instead of waiting for
      // initInferiorHeap.
      addInferiorHeap(img);

#if !defined(i386_unknown_nt4_0)  && !(defined mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001
    /* If we're not currently trying to load the runtime library,
       check whether this shared object is the runtime lib. */
    if (!(bootstrapState == loadingRT)
        && (ret = check_rtinst(this, &new_obj))) {
        if (ret == 1) {
            /* The runtime library has been loaded, but not initialized.
               Proceed anyway. */
            msg = string("Application was linked with Dyninst/Paradyn runtime library -- this is not necessary");
            statusLine(msg.c_str());
        } else {
            /* The runtime library has been loaded into the inferior
               and previously initialized, probably by a previous
               run or Dyninst or Paradyn.  Bail.  */
	      if (ret == 2)
		   msg = string("This process was previously modified by Dyninst -- cannot reattach");
	      else if (ret == 3)
		   msg = string("This process was previously modified by Paradyn -- cannot reattach");
	      else
		   assert(0);
	      showErrorCallback(26, msg);
	      return false;
	 }
    }
#endif

    // if the list of all functions and all modules have already been 
    // created for this process, then the functions and modules from this
    // shared object need to be added to those lists 
    if(all_modules){
      pdvector<module *> *vptr = const_cast< pdvector<module *> *>(reinterpret_cast< const pdvector<module *> *>(new_obj.getModules()));
      VECTOR_APPEND(*all_modules, *vptr);
    }
    if(all_functions){
      pdvector<function_base *> *normal_funcs = (pdvector<function_base *> *)
                const_cast< pdvector<pd_Function *> *>(new_obj.getAllFunctions());
	// need to concat two vectors ...
        VECTOR_APPEND(*all_functions, *normal_funcs); 
        normal_funcs = 0;
    }

    // if the signal handler function has not yet been found search for it
#if defined(i386_unknown_linux2_0)
    if(!signal_restore){
      Symbol s;
      if (img->symbol_info(SIGNAL_HANDLER, s)) {
	  // Add base address of shared library
	  signal_restore = s.addr() + new_obj.getBaseAddress();
      }
    }
#else
    if(!signal_handler){
        signal_handler = img->findFuncByName(SIGNAL_HANDLER);
    }
#endif

    // clear the include_funcs flag if this shared object should not be
    // included in the some_functions and some_modules lists
#ifndef BPATCH_LIBRARY
    pdvector<string> lib_constraints;
    if(mdl_get_lib_constraints(lib_constraints)){
        for(u_int j=0; j < lib_constraints.size(); j++){
           char *where = 0; 
           // if the lib constraint is not of the form "module/function" and
           // if it is contained in the name of this object, then exclude
           // this shared object
           char *obj_name = P_strdup(new_obj.getName().c_str());
           char *lib_name = P_strdup(lib_constraints[j].c_str());
           if(obj_name && lib_name && (where=P_strstr(obj_name, lib_name))){
              new_obj.changeIncludeFuncs(false); 
           }
           if(lib_name) free(lib_name);
           if(obj_name) free(obj_name);
        }
    }

    // This looks a bit wierd at first glance, but apparently is ok -
    //  A shared object has 1 module.  If that module is excluded,
    //  then new_obj.includeFunctions() should return FALSE.  As such,
    //  the some_modules += new_obj.getModules() is OK as long as
    //  shared objects have ONLY 1 module.
    if(new_obj.includeFunctions()){
        if(some_modules) {
            *some_modules += *((const pdvector<module *> *)(new_obj.getModules()));
        }
        if(some_functions) {
            // gets only functions not excluded by mdl "exclude_node" option
            *some_functions += 
                *((pdvector<function_base *> *)(new_obj.getIncludedFunctions()));
        }
    }
#endif /* BPATCH_LIBRARY */


#ifdef BPATCH_LIBRARY

    assert(BPatch::bpatch);
    const pdvector<pdmodule *> *modlist = new_obj.getModules();
    if (modlist != NULL) {
      for (unsigned i = 0; i < modlist->size(); i++) {
        pdmodule *curr = (*modlist)[i];
        string name = curr->fileName();

	BPatch_thread *thr = BPatch::bpatch->getThreadByPid(pid);
	if(!thr)
	  continue;  //There is no BPatch_thread yet, so nothing else to do
	// this occurs in the attach case - jdd 6/30/99
	
	BPatch_image *image = thr->getImage();
	assert(image);

	BPatch_module *bpmod = NULL;
	if ((name != "DYN_MODULE") && (name != "LIBRARY_MODULE")) {
	  if( image->ModuleListExist() )
	    bpmod = new BPatch_module(this, curr, image);
	}
	// add to module list
	if( bpmod){
	  //cout<<"Module: "<<name<<" in Process.C"<<endl;
	  image->addModuleIfExist(bpmod);
	}

        // XXX - jkh Add the BPatch_funcs here

        if (BPatch::bpatch->dynLibraryCallback) {
          BPatch::bpatch->dynLibraryCallback(thr, bpmod, true);
        }
      }
    }
    
#endif /* BPATCH_LIBRARY */

    return true;
}

// getSharedObjects: This routine is called before main() or on attach
// to an already running process.  It gets and process all shared objects
// that have been mapped into the process's address space
bool process::getSharedObjects() {
#ifndef alpha_dec_osf4_0
    assert(!shared_objects);
#else
    // called twice on alpha.
    if (shared_objects) return true;
#endif

    shared_objects = dyn->getSharedObjects(this); 
    if(shared_objects){
        statusLine("parsing shared object files");

#ifndef BPATCH_LIBRARY
        tp->resourceBatchMode(true);
#endif
	// for each element in shared_objects list process the 
	// image file to find new instrumentaiton points
 	for(u_int j=0; j < shared_objects->size(); j++){
// 	    string temp2 = string(j);
// 	    temp2 += string(" ");
// 	    temp2 += string("the shared obj, addr: ");
// 	    temp2 += string(((*shared_objects)[j])->getBaseAddress());
// 	    temp2 += string(" name: ");
// 	    temp2 += string(((*shared_objects)[j])->getName());
// 	    temp2 += string("\n");
// 	    logLine(P_strdup(temp2.c_str()));
 	    if(!addASharedObject(*((*shared_objects)[j]))){
 	      logLine("Error after call to addASharedObject\n");
 	    }
	}

#ifndef BPATCH_LIBRARY
        tp->resourceBatchMode(false);
#endif
        return true;
    }
    // else this a.out does not have a .dynamic section
    dynamiclinking = false;
    return false;
}

// findOneFunction: returns the function associated with func  
// this routine checks both the a.out image and any shared object
// images for this resource.
// Semantics of excluded functions - Once "exclude" works for both
// static and dynamically linked objects, this should return NULL
// if the function being sought is excluded....
#ifndef BPATCH_LIBRARY
function_base *process::findOneFunction(resource *func, resource *mod){
    if((!func) || (!mod)) { return 0; }
    if(func->type() != MDL_T_PROCEDURE) { return 0; }
    if(mod->type() != MDL_T_MODULE) { return 0; }

    const pdvector<string> &f_names = func->names();
    const pdvector<string> &m_names = mod->names();
    string func_name = f_names[f_names.size() -1]; 
    string mod_name = m_names[m_names.size() -1]; 

    //cerr << "process::findOneFunction called.  function name = " 
    //   << func_name.c_str() << endl;
    
    // KLUDGE: first search any shared libraries for the module name 
    //  (there is only one module in each shared library, and that 
    //  is the library name)
    if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
            module *next = 0;
            next = ((*shared_objects)[j])->findModule(mod_name,true);

            if(next){
                if(((*shared_objects)[j])->includeFunctions()){ 
                  //cerr << "function found in module " << mod_name.c_str() << endl;
                    return(((*shared_objects)[j])->findFuncByName(func_name));
                } 
                else { 
                  //cerr << "function found in module " << mod_name.c_str()
                  //    << " that module excluded" << endl;
                  return 0;
                } 
            }
        }
    }

    return(symbols->findFuncByName(func_name));
}
#endif /* BPATCH_LIBRARY */

#ifndef BPATCH_LIBRARY
// returns all the functions in the module "mod" that are not excluded by
// exclude_lib or exclude_func
// return 0 on error.
pdvector<function_base *> *process::getIncludedFunctions(module *mod) {

    if((!mod)) { return 0; }

    //cerr << "process::getIncludedFunctions(" << mod->fileName() << ") called" << endl;

    // KLUDGE: first search any shared libraries for the module name 
    //  (there is only one module in each shared library, and that 
    //  is the library name)
    if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
            module *next = 0;
            next = ((*shared_objects)[j])->findModule(mod->fileName(), true);
            if(next){
                if(((*shared_objects)[j])->includeFunctions()){ 
                    return((pdvector<function_base *> *)
                           ((*shared_objects)[j])->getIncludedFunctions());
                } 
                else { return 0;} 
            }
        }
    }

    // this must be an a.out module so just return the list associated
    // with the module.
    // Now that exclude should work for both static and dynamically
    // linked executables, probably need to either filter excluded
    // files here, or let the module do it and pass the information not
    // to include excluded functions along with this proc call....
    // mcheyney 970927
    return(mod->getIncludedFunctions());
}
#endif /* BPATCH_LIBRARY */


// Parse name into library name and function name. If no library is specified,
// lib gets the empty string "". 
//
// e.g. libc.so/read => lib_name = libc.so, func_name = read
//              read => lib_name = "", func_name = read
void getLibAndFunc(const string &name, string &lib_name, string &func_name) {

  unsigned index = 0;
  unsigned length = name.length();
  bool hasNoLib = true;
 
  // clear the strings 
  lib_name = "";
  func_name = "";

  for (unsigned i = length-1; i > 0 && hasNoLib; i--) {
    if (name[i] == '/') {
      index = i;
      hasNoLib = false; 
    } 
  }

  if (hasNoLib) {
    // No library specified  
    func_name = name;
  } else { 
      // Grab library name and function name 
      lib_name = name.substr(0, index);
      func_name = name.substr(index+1, length-index);
  }
}


// Return true if library specified by lib_name matched name.
// This check is done ignoring the version number of name.
// Thus, for lib_name = libc.so, and name = libc.so.6, this function
// will return true (a match was made).
bool matchLibName(string &lib_name, const string &name) {

  // position in string "lib_name" where version information begins
  unsigned ln_index = lib_name.length();

  // position in string "name" where version information begins
  unsigned n_index = name.length();

  // Walk backwards from end of name, passing over the version number.
  // e.g. isolate the libc.so in libc.so.6
  while (isdigit(name[n_index-1]) || name[n_index-1] == '.') {
    n_index--;
  }

  // Walk backwards from end of lib_name, passing over the version number.
  // e.g. isolate the libc.so in libc.so.6
  while (isdigit(lib_name[ln_index-1]) || lib_name[ln_index-1] == '.') {
    ln_index--;
  }
 
  // If lib_name is the same as name (minus the version information, 
  // return true
  if ((lib_name.substr(0, ln_index)).wildcardEquiv(name.substr(0, n_index))) {
    return true;
  }  
 
  return false;
}

// findOneFunction: returns the function associated with func  
// this routine checks both the a.out image and any shared object
// images for this resource
function_base *process::findOneFunction(const string &name) const {

    string lib_name;
    string func_name;

    // Split name into library and function
    getLibAndFunc(name, lib_name, func_name);

    // If no library was specified, grab the first function we find
    if (lib_name == "") {

      // first check a.out for function symbol
      function_base *pdf = symbols->findFuncByName(func_name);
      if(pdf) {
        return pdf;
      }

      // search any shared libraries for the file name 
      if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
          pdf = ((*shared_objects)[j])->findFuncByName(func_name);
          if(pdf){
            return(pdf);
          }
        }
      }
    } else {
  
        // Search specified shared library for function 
        if(dynamiclinking && shared_objects){ 
          for(u_int j=0; j < shared_objects->size(); j++){
            shared_object *so = (*shared_objects)[j];
            
	    // Add prefix wildcard to make name matching easy
	    if (!lib_name.prefixed_by("*"))
	      lib_name = "*" + lib_name;             

            if(matchLibName(lib_name, so->getName())) {
              function_base *fb = so->findFuncByName(func_name);
              if (fb) {
                //cerr << "Found " << func_name << " in " << lib_name << endl;
              } 
              return fb;
            }
	  }
	}
    }
    return(0);
}

// findFuncByName: returns the function associated with func  
// this routine checks both the a.out image and any shared object
// images for this resource
pd_Function *process::findFuncByName(const string &name){

    string lib_name;
    string func_name;

    // Split name into library and function
    getLibAndFunc(name, lib_name, func_name);

    // If no library was specified, grab the first function we find
    if (lib_name == "") {
    
      // first check a.out for function symbol
      pd_Function *pdf = symbols->findFuncByName(func_name);
      if(pdf)  return pdf;

      // search any shared libraries for the file name 
      if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
          pdf=((*shared_objects)[j])->findFuncByName(func_name);
            if(pdf){
              return(pdf);
            }
	}
      }

    } else {
      
        // Library was specified, search lib for function func_name 
        if(dynamiclinking && shared_objects){ 
          for(u_int j=0; j < shared_objects->size(); j++){
            shared_object *so = (*shared_objects)[j];

            // Add prefix wildcard to make name matching easy
            lib_name = "*" + lib_name;             

            if(matchLibName(lib_name, so->getName())) {
              pd_Function *fb = so->findFuncByName(func_name);
              if (fb) {
                //cerr << "Found " << func_name << " in " << lib_name << endl;
              }
              return fb;
            }
	  }
	}
    }

    return(0);
}

// findAllFuncsByName: return a vector of all functions in all images
// matching the given string
bool process::findAllFuncsByName(const string &name, pdvector<function_base *> &res)
{
  string lib_name;
  string func_name;
  
  // Split name into library and function
  getLibAndFunc(name, lib_name, func_name);
  
  // If no library was specified, grab the first function we find
  if (lib_name == "") {
    
    // first check a.out for function symbol
    function_base *pdf = static_cast<function_base *>(symbols->findFuncByName(func_name));
    if (pdf)
      res.push_back(pdf);

    // search any shared libraries for the file name 
    if(dynamiclinking && shared_objects){
      for(u_int j=0; j < shared_objects->size(); j++){
	pdf=static_cast<function_base *>(((*shared_objects)[j])->findFuncByName(func_name));
	if(pdf){
           res.push_back(pdf);
	}
      }
    }
    
  } else {
    
    // Library was specified, search lib for function func_name 
    if(dynamiclinking && shared_objects){ 
      for(u_int j=0; j < shared_objects->size(); j++){
	shared_object *so = (*shared_objects)[j];
	
	// Add prefix wildcard to make name matching easy
	lib_name = "*" + lib_name;             
	
	if(matchLibName(lib_name, so->getName())) {
	  function_base *fb = static_cast<function_base *>(so->findFuncByName(func_name));
	  if (fb) {
	    res.push_back(fb);
	  }
	}
      }
    }
  }
  return true; 
}

// Returns the named symbol from the image or a shared object
bool process::getSymbolInfo( const string &name, Symbol &ret ) 
{
   if(!symbols)
      abort();

   bool sflag;
   sflag = symbols->symbol_info( name, ret );

   if(sflag)
      return true;

   if( dynamiclinking && shared_objects ) {
      for( u_int j = 0; j < shared_objects->size(); ++j ) {
         sflag = ((*shared_objects)[j])->getSymbolInfo( name, ret );
         if( sflag ) {
            ret.setAddr( ret.addr() + (*shared_objects)[j]->getBaseAddress() );
            return true;
         }
      }
   }

   return false;
}

// findFunctionIn: returns the function which contains this address
// This routine checks both the a.out image and any shared object images
// for this function

// New addition: keep a vector of functions. Return only the first, but
// complain if additional are found. 
// Kept around until we're sure the new findFuncByAddr works
#if 0
pd_Function *process::findFunctionIn(Address adr)
{

  pdvector <pd_Function *> returned_functions;
  pd_Function *pdf;
  // first check a.out for function symbol
  // Search all functions 
  pdf = symbols->findFuncByAddr(adr, this);
  if (pdf) returned_functions.push_back(pdf);

  // search any shared libraries for the function 
  if(dynamiclinking && shared_objects){
    for(u_int j=0; j < shared_objects->size(); j++){
      pdf = ((*shared_objects)[j])->findFuncByAddr(adr,this);
      if(pdf){
	returned_functions.push_back(pdf);
      }
    }
  }

  if (returned_functions.size() > 1) { // Got more than one return
#ifdef DEBUG
    cerr << "Warning: multiple matches found for address " << adr << endl;
#endif
    for (int i = 0; i < returned_functions.size(); i++)
      cerr << returned_functions[i]->prettyName() << endl;
  }

  if (returned_functions.size())
    return returned_functions[0];

  if(!all_functions) getAllFunctions();
  
  // if the function was not found, then see if this addr corresponds
  // to  a function that was relocated in the heap
  if(all_functions){
    for(u_int j=0; j < all_functions->size(); j++){
      Address func_adr = ((*all_functions)[j])->getAddress(this);
      if((adr>=func_adr) && 
	 (adr<=(((*all_functions)[j])->size()+func_adr))){
	// yes, this is very bad, but too bad
	cerr << "Found function in relocated heap. Please fix in symtab.C" << endl;
	return((pd_Function*)((*all_functions)[j]));
      }
    }
  }
  return(0);
}
#endif    
// findFuncByAddr: returns the function which contains this address
// This routine checks both the a.out image and any shared object images
// for this function

// New addition: keep a vector of functions. Return only the first, but
// complain if additional are found. 

// We've been having problems with multiple matches. In several cases, a
// given address both matches the absolute addr of a function and the offset
// of a function within a module. So instead of calling image::findFuncByAddr,
// we do it the hard way.

// We've been having problems with multiple matches. In several cases, a
// given address both matches the absolute addr of a function and the offset
// of a function within a module. So instead of calling image::findFuncByAddr,
// we do it the hard way.

pd_Function *process::findFuncByAddr(Address adr)
{
  pd_Function *pdf;
  // first check a.out for function symbol
  // Search all functions 
  pdf = symbols->findFuncByEntryAddr(adr, this);
  if (pdf) return pdf;
  
  if (dynamiclinking && shared_objects) {
    for(u_int j=0; j < shared_objects->size(); j++){
      pdf = ((*shared_objects)[j])->findFuncByEntryAddr(adr,this);
      if (pdf) return pdf;
    }
  }

  pdf = symbols->findFuncByOrigAddr(adr, this);
  if (pdf) return pdf;

  // search any shared libraries for the function 
  if(dynamiclinking && shared_objects){
    for(u_int j=0; j < shared_objects->size(); j++){
      pdf = ((*shared_objects)[j])->findFuncByOrigAddr(adr, this);
      if (pdf) return pdf;
    }
  }

  // So we checked by entry points and by absolute addresses, and got no
  // matches. Check by relocated addresses and (possibly) offsets within
  // a file.

  // first check a.out for function symbol
  // Search all functions 
  pdf = symbols->findFuncByRelocAddr(adr, this);
  if (pdf) return pdf;

  // search any shared libraries for the function 
  if(dynamiclinking && shared_objects){
    for(u_int j=0; j < shared_objects->size(); j++){
      pdf = ((*shared_objects)[j])->findFuncByRelocAddr(adr,this);
      if (pdf) return pdf;
    }
  }
  return NULL;
}
    
// findModule: returns the module associated with mod_name 
// this routine checks both the a.out image and any shared object
// images for this resource
// if check_excluded is true it checks to see if the module is excluded
// and if it is it returns 0.  If check_excluded is false it doesn't check
module *process::findModule(const string &mod_name, bool check_excluded) {
   // KLUDGE: first search any shared libraries for the module name 
   //  (there is only one module in each shared library, and that 
   //  is the library name)
   if(dynamiclinking && shared_objects){
      for(u_int j=0; j < shared_objects->size(); j++){
         module *next = ((*shared_objects)[j])->findModule(mod_name,
                                                           check_excluded);
         if(next) {
            return(next);
         }
      }
   }

   // check a.out for function symbol
   //  Note that symbols is data member of type image* (comment says
   //  "information related to the process"....
   module *mret = symbols->findModule(mod_name, check_excluded);
   return mret;
}

// getSymbolInfo:  get symbol info of symbol associated with name n
// this routine starts looking a.out for symbol and then in shared objects
// baseAddr is set to the base address of the object containing the symbol.
// This function appears to return symbol info even if module/function
// is excluded.  In extending excludes to statically linked executable,
// we preserve these semantics....
bool process::getSymbolInfo(const string &name, Symbol &info, 
                            Address &baseAddr) const 
{
   // first check a.out for symbol
   if(symbols->symbol_info(name,info))
      return getBaseAddress(symbols, baseAddr);

   // next check shared objects
   if(dynamiclinking && shared_objects) {
      for(u_int j=0; j < shared_objects->size(); j++) {
         if(((*shared_objects)[j])->getSymbolInfo(name,info)) {
            return getBaseAddress(((*shared_objects)[j])->getImage(), 
                                  baseAddr); 
         }
      }
   }
    
   return false;
}


// getAllFunctions: returns a vector of all functions defined in the
// a.out and in the shared objects
// TODO: what to do about duplicate function names?
pdvector<function_base *> *process::getAllFunctions(){

    // if this list has already been created, return it
    if(all_functions) 
        return all_functions;

    // else create the list of all functions
    all_functions = new pdvector<function_base *>;
    const pdvector<function_base *> &blah = 
                    (pdvector<function_base *> &)(symbols->getAllFunctions());
    VECTOR_APPEND(*all_functions,blah);

    if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
           pdvector<function_base *> *funcs = (pdvector<function_base *> *) 
                        const_cast< pdvector<pd_Function *> *>
                        (((*shared_objects)[j])->getAllFunctions());
           if(funcs){
               VECTOR_APPEND(*all_functions,*funcs); 
           }
        }
    }
    return all_functions;
}
      
// getAllModules: returns a vector of all modules defined in the
// a.out and in the shared objects
// Includes "excluded" modules....
pdvector<module *> *process::getAllModules(){

    // if the list of all modules has already been created, the return it
    if(all_modules) return all_modules;

    // else create the list of all modules
    all_modules = new pdvector<module *>;
    VECTOR_APPEND(*all_modules,*((const pdvector<module *> *)(&(symbols->getAllModules()))));

    if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
           const pdvector<module *> *mods = (const pdvector<module *> *)
                        (((*shared_objects)[j])->getModules());
           if(mods) {
               VECTOR_APPEND(*all_modules,*mods); 
           }
    } } 
    return all_modules;
}

#ifndef BPATCH_LIBRARY
// getIncludedFunctions: returns a vector of all functions defined in the
// a.out and in the shared objects
// TODO: what to do about duplicate function names?
pdvector<function_base *> *process::getIncludedFunctions(){
    //cerr << "process " << programName << " :: getIncludedFunctions() called"
    //   << endl;
    // if this list has already been created, return it
    if(some_functions) 
        return some_functions;

    // else create the list of all functions
    some_functions = new pdvector<function_base *>;
    const pdvector<function_base *> &incl_funcs = 
        (pdvector<function_base *> &)(symbols->getIncludedFunctions());
        *some_functions += incl_funcs;

    //cerr << " (process::getIncludedFunctions), about to add incl_funcs to some_functions, incl_funcs = " << endl;
    //print_func_vector_by_pretty_name(string(">>>"), &incl_funcs);

    if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
            if(((*shared_objects)[j])->includeFunctions()){
                // kludge: can't assign a vector<derived_class *> to 
                // a vector<base_class *> so recast
                pdvector<function_base *> *funcs = (pdvector<function_base *> *)
                        (((*shared_objects)[j])->getIncludedFunctions());
                if(funcs) { 
                    *some_functions += (*funcs); 
                } 
            } 
    } } 

    //cerr << " (process::getIncludedFunctions()) about to return fucntion list : ";
    //print_func_vector_by_pretty_name(string("  "), some_functions);

    return some_functions;
}

// getIncludedModules: returns a vector of all modules defined in the
// a.out and in the shared objects that are included as specified in
// the mdl
pdvector<module *> *process::getIncludedModules(){

    //cerr << "process::getIncludedModules called" << endl;

    // if the list of all modules has already been created, the return it
    if(some_modules) {
        //cerr << "some_modules already created, returning it:" << endl;
        //print_module_vector_by_short_name(string("  "), (vector<pdmodule*>*)some_modules);
        return some_modules;
    }

    // else create the list of all modules
    some_modules = new pdvector<module *>;
    *some_modules += *((const pdvector<module *> *)(&(symbols->getIncludedModules())));

    if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
            if(((*shared_objects)[j])->includeFunctions()){
               const pdvector<module *> *mods = (const pdvector<module *> *) 
                        (((*shared_objects)[j])->getModules());
               if(mods) {
                   *some_modules += *mods; 
               }
           }
    } } 

    //cerr << "some_modules newly created, returning it:" << endl;
    //print_module_vector_by_short_name(string("  "),
    //    (vector<pdmodule*>*)some_modules);
    return some_modules;
}
#endif /* BPATCH_LIBRARY */

// writes newly created installed_miniTramps_list into mtList
void process::newMiniTrampList(const instPoint *loc, callWhen when,
			       installed_miniTramps_list **mtList) {
  // operator[] creates an empty installed_miniTramps_list
  installed_miniTramps_list *newList = new installed_miniTramps_list;

  switch(when) {
    case callPreInsn: {
      installedMiniTramps_beforePt[loc] = newList;
      assert(newList->numMiniTramps() == 0);
      (*mtList) = newList;
      break;
    }
    case callPostInsn: {
      installedMiniTramps_afterPt[loc] = newList;
      assert(newList->numMiniTramps() == 0);
      (*mtList) = newList;
      break;
    }
    default:
      assert(false);
  }
}

void process::getMiniTrampLists(pdvector<mtListInfo> *vecBuf) {
  dictionary_hash_iter<const instPoint *, installed_miniTramps_list*>
    befIter = installedMiniTramps_beforePt;

  for(; befIter; befIter++) {
    const instPoint *theLoc = befIter.currkey();
    installed_miniTramps_list *curList = befIter.currval();
    mtListInfo listInfo;
    listInfo.loc = theLoc;
    listInfo.when = callPreInsn;
    listInfo.mtList = curList;
    (*vecBuf).push_back(listInfo);
  }

  dictionary_hash_iter<const instPoint *, installed_miniTramps_list*>
    aftIter = installedMiniTramps_afterPt;

  for(; aftIter; aftIter++) {
    const instPoint *theLoc = aftIter.currkey();
    installed_miniTramps_list *curList = aftIter.currval();
    mtListInfo listInfo;
    listInfo.loc = theLoc;
    listInfo.when = callPostInsn;
    listInfo.mtList = curList;
    (*vecBuf).push_back(listInfo);
  }
}

void process::getMiniTrampList(const instPoint *loc, callWhen when,
			       installed_miniTramps_list **mtList) {
  // creates an empty installed_miniTramps_list if doesn't already exist
  switch(when) {
    case callPreInsn: {
      dictionary_hash<const instPoint *, installed_miniTramps_list*>::iterator
	lst = installedMiniTramps_beforePt.find(loc);

      if(lst == installedMiniTramps_beforePt.end())
	(*mtList) = NULL;
      else
	(*mtList) = lst.currval();

      break;
    }
    case callPostInsn: {
      dictionary_hash<const instPoint *, installed_miniTramps_list*>::iterator
	lst = installedMiniTramps_afterPt.find(loc);
      if(lst == installedMiniTramps_afterPt.end())
	(*mtList) = NULL;
      else
	(*mtList) = lst.currval();
      break;
    }
    default:
      assert(false);
  }
}

void process::getMiniTrampList(const instPoint *loc, callWhen when,
			       const installed_miniTramps_list **mtList) const 
{
  // doesn't create an empty installed_miniTramps_list if doesn't already exist
  switch(when) {
    case callPreInsn: {
      dictionary_hash<const instPoint *, installed_miniTramps_list*>::iterator
	lst = installedMiniTramps_beforePt.find(loc);
      if(lst == installedMiniTramps_beforePt.end())
	(*mtList) = NULL;
      else
	(*mtList) = lst.currval();
      break;
    }
    case callPostInsn: {
      dictionary_hash<const instPoint *, installed_miniTramps_list*>::iterator
	lst = installedMiniTramps_afterPt.find(loc);
      if(lst == installedMiniTramps_afterPt.end())
	(*mtList) = NULL;
      else
	(*mtList) = lst.currval();
      break;
    }
    default:
      assert(false);
  }
}

void process::removeMiniTrampList(const instPoint *loc, callWhen when) {
  switch(when) {
    case callPreInsn:
      installedMiniTramps_beforePt.undef(loc);
      break;
    case callPostInsn:
      installedMiniTramps_afterPt.undef(loc);
      break;
    default:
      assert(false);
  }
}

instPoint *process::findInstPointFromAddress(Address addr,
					     trampTemplate **bt,
					     instInstance **mt)
{
   unsigned u;
   pdvector<const instPoint*> ips;
   pdvector<trampTemplate*> bts;
   ips = baseMap.keys();
   bts = baseMap.values();
   assert( ips.size() == bts.size() );

   for( u = 0; u < bts.size(); ++u ) {
      if( bts[u]->inBasetramp( addr ) )
      {
	 if(bt!=NULL)  (*bt) = bts[u];
	 return const_cast<instPoint*>(ips[u]);
      }
   }

   dictionary_hash_iter<const instPoint *, installed_miniTramps_list*>
      befIter = installedMiniTramps_beforePt;
   
   for(; befIter; befIter++) {
      installed_miniTramps_list *curList = befIter.currval();

      List<instInstance*>::iterator curMT = curList->get_begin_iter();
      List<instInstance*>::iterator endMT = curList->get_end_iter();
      for(; curMT != endMT; curMT++) {
	 instInstance *inst = *curMT;
	 if( ( inst->trampBase <= addr && addr <= inst->returnAddr)
	     || inst->baseInstance->inBasetramp(addr) )
	 {
	    if(mt!=NULL)  (*mt) = inst;
	    return const_cast<instPoint *>(befIter.currkey());
	 } 
      }
   }

   dictionary_hash_iter<const instPoint *, installed_miniTramps_list*>
      aftIter = installedMiniTramps_afterPt;
   for(; aftIter; aftIter++) {
      installed_miniTramps_list *curList = aftIter.currval();

      List<instInstance*>::iterator curMT = curList->get_begin_iter();
      List<instInstance*>::iterator endMT = curList->get_end_iter();	 
      for(; curMT != endMT; curMT++) {
	 instInstance *inst = *curMT;
	 if( ( inst->trampBase <= addr && addr <= inst->returnAddr)
	     || inst->baseInstance->inBasetramp(addr) )
	 {
	    if(mt!=NULL)  (*mt) = inst;
	    return const_cast<instPoint *>(aftIter.currkey());
	 } 
      }
   }

   return NULL;
}

pd_Function *process::findAddressInFuncsAndTramps(Address addr,
						  instPoint **ip,
						  trampTemplate **bt,
						  instInstance **mt)
{
  if(ip!=NULL)  (*ip) = NULL;
  if(bt!=NULL)  (*bt) = NULL;
  if(mt!=NULL)  (*mt) = NULL;

  pd_Function *func = NULL;
  func = (pd_Function *)findFuncByAddr(addr);
  instPoint *foundInstPt = findInstPointFromAddress(addr, bt, mt);
  if (ip)
    *ip = foundInstPt;
  if(foundInstPt != NULL) {
    function_base *func_base = 
      const_cast<function_base*>(foundInstPt->iPgetFunction());
    if(func_base) {
      func = static_cast<pd_Function *>(func_base);
    }
  }
  return func;
}
      
// getBaseAddress: sets baseAddress to the base address of the 
// image corresponding to which.  It returns true  if image is mapped
// in processes address space, otherwise it returns 0
bool process::getBaseAddress(const image *which, Address &baseAddress) const {

  if((Address)(symbols) == (Address)(which)){
      baseAddress = 0; 
      return true;
  }
  else if (shared_objects) {  
      // find shared object corr. to this image and compute correct address
      for(unsigned j=0; j < shared_objects->size(); j++){ 
          if(((*shared_objects)[j])->isMapped()){
            if(((*shared_objects)[j])->getImage() == which) { 
              baseAddress = ((*shared_objects)[j])->getBaseAddress();
              return true;
          } }
      }
  }
  return false;
}

#if defined(i386_unknown_linux2_0)
// findSignalHandler: if signal_restore is 0, then it checks all images
// associated with this process for the signal restore function.
// Otherwise, the signal restore function has already been found
void process::findSignalHandler(){
  if (!signal_restore) {
    Symbol s;
    if (getSymbolInfo(SIGNAL_HANDLER, s))
	signal_restore = s.addr();

    signal_cerr << "process::findSignalHandler <" << SIGNAL_HANDLER << ">";
    if (!signal_restore) signal_cerr << " NOT";
    signal_cerr << " found." << endl;
  }
}
#else
// findSignalHandler: if signal_handler is 0, then it checks all images
// associated with this process for the signal handler function.
// Otherwise, the signal handler function has already been found
void process::findSignalHandler(){

    if(SIGNAL_HANDLER == 0) return;
    if(!signal_handler) { 
        // first check a.out for signal handler function
        signal_handler = symbols->findFuncByName(SIGNAL_HANDLER);

        // search any shared libraries for signal handler function
        if(!signal_handler && dynamiclinking && shared_objects) { 
            for(u_int j=0;(j < shared_objects->size()) && !signal_handler; j++){
                signal_handler = 
                      ((*shared_objects)[j])->findFuncByName(SIGNAL_HANDLER);
        } }
        signal_cerr << "process::findSignalHandler <" << SIGNAL_HANDLER << ">";
        if (!signal_handler) signal_cerr << " NOT";
        signal_cerr << " found." << endl;
    }
}
#endif


bool process::findInternalSymbol(const string &name, bool warn,
                                 internalSym &ret_sym) const
{
     // On some platforms, internal symbols may be dynamic linked
     // so we search both the a.out and the shared objects
     Symbol sym;
     Address baseAddr;
     static const string underscore = "_";

     if (getSymbolInfo(name, sym, baseAddr)
         || getSymbolInfo(underscore+name, sym, baseAddr)) {
#ifdef mips_unknown_ce2_11 //ccw 29 mar 2001
        ret_sym = internalSym(sym.addr(), name);
#else
        ret_sym = internalSym(baseAddr+sym.addr(), name);
#endif
        return true;
     }

     if (warn) {
        string msg;
        msg = string("Unable to find symbol: ") + name;
        statusLine(msg.c_str());
        showErrorCallback(28, msg);
     }
     return false;
}

Address process::findInternalAddress(const string &name, bool warn, bool &err) const {
     // On some platforms, internal symbols may be dynamic linked
     // so we search both the a.out and the shared objects
     Symbol sym;
     Address baseAddr;
     static const string underscore = "_";
#if !defined(i386_unknown_linux2_0) \
 && !defined(ia64_unknown_linux2_4) \
 && !defined(alpha_dec_osf4_0) \
 && !defined(i386_unknown_nt4_0) \
 && !defined(mips_unknown_ce2_11) //ccw 20 july 2000
     // we use "dlopen" because we took out the leading "_"'s from the name
     if (name==string("dlopen")) {
       // if the function is dlopen, we use the address in ld.so.1 directly
       baseAddr = get_dlopen_addr();
       if (baseAddr != (Address)NULL) {
         err = false;
         return baseAddr;
       } else {
         err = true;
         return 0;
       }
       // replace above with tighter code below
       //err = (baseAddr != 0);
       //return baseAddr;
     }
#endif

     if (getSymbolInfo(name, sym, baseAddr)
         || getSymbolInfo(underscore+name, sym, baseAddr)) {
        err = false;
#ifdef mips_unknown_ce2_11 //ccw 29 mar 2001
        return sym.addr();//+baseAddr; ///ccw 28 oct 2000 THIS ADDS TOO MUCH TO THE LIBDYNINSTAPI.DLL!
#else
        return sym.addr()+baseAddr;
#endif
     }

     if (warn) {
        string msg;
        msg = string("Unable to find symbol: ") + name;
        statusLine(msg.c_str());
        showErrorCallback(28, msg);
     }
     err = true;
     return 0;
}

bool process::continueProc() {
  if (status_ == exited) return false;

  // We're already running
  if (status_ == running) return true;

  if (status_ != stopped && status_ != neonatal) {
    sprintf(errorLine, "Internal error: "
                "Unexpected process state %d in process::contineProc",
                (int)status_);
    showErrorCallback(39, errorLine);
    return false;
  }
  bool res = continueProc_();

  if (!res) {
    showErrorCallback(38, "System error: can't continue process");
    return false;
  }
  status_ = running;
  
  return true;
}

bool process::detach(const bool paused) {
   if (paused) {
      logLine("detach: pause not implemented\n"); // why not? --ari
   }

   dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(lwps);
   dyn_lwp *lwp;
   unsigned index;
   while (lwp_iter.next(index, lwp)) {
      lwp->closeFD();
   }

   bool res = detach_();
   
   if (!res) {
      // process may have exited
      return false;
   }
   return true;
}

#ifdef BPATCH_LIBRARY
// XXX Eventually detach() above should go away and this should be
//     renamed detach()
/* process::API_detach: detach from the application, leaving all
   instrumentation place.  Returns true upon success and false upon failure.
   Fails if the application is not stopped when the call is made.  The
   parameter "cont" indicates whether or not the application should be made
   running or not as a consquence of detaching (true indicates that it should
   be run).
*/
bool process::API_detach(const bool cont)
{
  // if (status() != neonatal && status() != stopped)
  // We should cleanup even if the process is not stopped - jkh 5/21/99
  if (status() == neonatal)
      return false;

  return API_detach_(cont);
}
#endif

/* process::handleExec: called when a process successfully exec's.
   Parse the new image, disable metric instances on the old image, create a
   new (and blank) shm segment.  The process hasn't yet bootstrapped, so we
   mustn't try to enable anything...
*/
void process::handleExec() {
    // NOTE: for shm sampling, the shm segment has been removed, so we
    //       mustn't try to disable any dataReqNodes in the standard way...

#if !defined(BPATCH_LIBRARY) //ccw 22 apr 2002 : SPLIT
	PARADYNhasBootstrapped = false;
#endif
   // all instrumentation that was inserted in this process is gone.
   // set exited here so that the disables won't try to write to process
   status_ = exited; 
   
   // Clean up state from old exec: all dynamic linking stuff, all lists 
   // of functions and modules from old executable
   
   // can't delete dynamic linking stuff here, because parent process
   // could still have pointers
   dynamiclinking = false;
   dyn = 0; // AHEM.  LEAKED MEMORY!  not if the parent still has a pointer
   // to this dynamic_linking object.
   dyn = new dynamic_linking;
   if(shared_objects){
     for(u_int j=0; j< shared_objects->size(); j++){
       delete (*shared_objects)[j];
     }
     delete shared_objects;
     shared_objects = 0;
   }
   
   // TODO: when can pdFunction's be deleted???  definitely not here.
   delete some_modules;
   delete some_functions;
   delete all_functions;
   delete all_modules;
   some_modules = 0;
   some_functions = 0;
   all_functions = 0;
   all_modules = 0;
#if defined(i386_unknown_linux2_0)
   signal_restore = 0;
#else
   signal_handler = 0;
#endif
#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0) \
 || defined(i386_unknown_nt4_0) || defined(ia64_unknown_linux2_4)
   trampTableItems = 0;
   memset(trampTable, 0, sizeof(trampTable));
#endif
   baseMap.clear();
#ifdef BPATCH_LIBRARY
   instPointMap.clear(); /* XXX Should delete instPoints first? */
   PDFuncToBPFuncMap.clear();
#endif
   installedMiniTramps_beforePt.clear();
   installedMiniTramps_afterPt.clear();

#ifdef SHM_SAMPLING   
   // the shared memory segment is unusable after the exec
   delete shMetaOffsetData;
   shMetaOffsetData = NULL;
   delete shmMetaData;          // frees malloced vars in shmMgr
   shmMetaData = NULL;
   shmMetaData = new sharedMetaData(*theSharedMemMgr, maxNumberOfThreads());
#endif
   
   int status = pid;
   fileDescriptor *desc = getExecFileDescriptor(execFilePath,
						status,
						false);
   if (!desc) return;
   image *img = image::parseImage(desc);
   
   if (!img) {
       // For better error reporting, two failure return values would be useful
       // One for simple error like because-file-not-found
       // Another for serious errors like found-but-parsing-failed (internal error;
       //    please report to paradyn@cs.wisc.edu)

       string msg = string("Unable to parse image: ") + execFilePath;
       showErrorCallback(68, msg.c_str());
       OS::osKill(pid);
          // err..what if we had attached?  Wouldn't a detach be appropriate in this case?
       return;
    }

    // delete proc->symbols ???  No, the image can be shared by more
    // than one process...images and instPoints can not be deleted...TODO
    // add some sort of reference count to these classes so that they can
    // be deleted
    symbols = img; // AHEM!  LEAKED MEMORY!!! ...not if parent has ptr to 
                   // previous image

    // see if new image contains the signal handler function
    this->findSignalHandler();

	// release our info about the heaps that exist in the process
	// (we will eventually call initInferiorHeap to rebuild the list and
	// reset our notion of which are available and which are in use)
	unsigned int heapIdx;
	for( heapIdx = 0; heapIdx < heap.bufferPool.size(); heapIdx++ )
	{
		delete heap.bufferPool[heapIdx];
	}
	heap.bufferPool.resize(0);

    // initInferiorHeap can only be called after symbols is set!
#if defined(i386_unknown_nt4_0) || (defined mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001
    initInferiorHeap();
#endif

    bootstrapState = unstarted;
    /* update process status */
       // we haven't yet seen initial SIGTRAP for this proc (is this right?)
    
    status_ = stopped; // was 'exited'

   // TODO: We should remove (code) items from the where axis, if the exec'd process
   // was the only one who had them.

   // the exec'd process has the same fd's as the pre-exec, so we don't need
   // to re-initialize traceLink (is this right???)

   // we don't need to re-attach after an exec (is this right???)
 
#ifdef SHM_SAMPLING
   theSharedMemMgr->handleExec();
      // reuses the shm seg (paradynd's already attached to it); resets applic-attached-
      // at to NULL.  Quite similar to the (non-fork) ctor, really.

   //theVariableMgr->handleExec();
#endif

   inExec = false;
   execed_ = true;
}

/* 
   process::cleanUpInstrumentation called when paradynd catch
   a SIGTRAP to find out if there's any previous unfinished instrumentation
   requests 
*/
bool process::cleanUpInstrumentation(bool wasRunning) {
  // Try to process an item off of the waiting list 'instWlist'.
  // If something was found & processed, then true will be returned.
  // Additionally, if true is returned, the process will be continued
  // if 'wasRunning' is true.
  // But if false is returned, then there should be no side effects: noone
  // should be continued, nothing removed from 'instWList', no change
  // to this->status_, and so on (this is important to avoid bugs).
  
  assert(status_ == stopped); // since we're always called after a SIGTRAP
  
  Address pc = 0; //FIXME
  assert(0 && "Fix the above PC to be correct");
  // Go thru the instWList to find out the ones to be deleted 
  bool done = false;
  u_int i=0;
  bool found = false;
  while(!done){
    //process *p = (instWList[i])->which_proc;
    if(((instWList[i])->pc_ == pc) && ((instWList[i])->which_proc == this)){
      (instWList[i])->cleanUp(this,pc);
      u_int last = instWList.size()-1;
      delete (instWList[i]);
      instWList[i] = instWList[last];
      instWList.resize(last);
      found = true;
    }
    else {
      i++;
    }
    if(i >= instWList.size()) done = true;
  }
  if(found && wasRunning) continueProc();
  return found;
}

bool process::checkTrappedSyscallsInternal(Address syscall)
{
    for (unsigned i = 0; i < syscallTraps_.size(); i++) {
        if (syscall == syscallTraps_[i]->syscall_id)
            return true;
    }
    
    return false;
}


dyn_lwp *process::checkSyscallExit() {
    // For each thread:
    // Get the LWP associated with the thread
    // Check to see if the LWP is at a syscall exit trap
    // Check to see if the trap is desired
    // Return the first thread to match the above conditions.

    for (unsigned iter = 0; iter < threads.size(); iter++) {
        dyn_thread *thr = threads[iter];
        int match_type = thr->get_lwp()->hasReachedSyscallTrap();
        if (match_type == 0)
            continue; // No match
        else if (match_type == 1) {
            // Uhh... crud. 
            thr->get_lwp()->stepPastSyscallTrap();
            // Question: what to return? The trap was incorrect,
            // and as such should silently disappear. For now, return
            // the thread that hit the trap, and the caller should 
            // determine there is nothing to be done.
            return thr->get_lwp();
        }
        else if (match_type == 2) {
            // Good, we did want a system call. Clear the
            // trap and return the thread for further processing
            thr->get_lwp()->clearSyscallExitTrap();
            return thr->get_lwp();
        }
        else assert(0 && "Invalid value from hasReachedSyscallTrap");
    }
    return NULL;
}


/* If the PC is at a tramp instruction, then the trap may have been
   decoded but not yet delivered to the application, when the pause
   was done.  When the app is restarted the trap is delivered before
   the inferior rpc (irpc) can run, which causes an assert in the rt
   library trap handler.  So we set a flag in the application
   indicating we're in an irpc.  The rt library trap handler when this
   flag is set (ie. the irpc is running) will not act upon the trap.
   The trap will get regenerated after the irpc is completed since the
   PC will be reset to it's previous value, which in this case is the
   trap instruction.  I suppose it could be also the case that the
   pause is done when the PC is at the trap instruction but before the
   trap is decoded and therefore pending.  The default behavior of the
   PC getting reset to it's previous value, obviously works in this
   case also.

   We also set variables in the applicatoin that indicate the starting
   and ending addresses of the irpc.  This is used for the trap handler
   to check the current PC against these addresses.  If the PC is in the
   irpc, then it's a previous trap that's interrupting the irpc.  If the
   PC isn't in the irpc then it's a trap caused by instrumentation called
   by an inferior rpc, in which case we process the trap as usual.
*/

void process::SendAppIRPCInfo(int runningRPC, unsigned begRPC, unsigned endRPC) {
   if (pd_debug_infrpc)
     inferiorrpc_cerr << "SendAppIRPCInfo(), flag: " << runningRPC<< ", begRPC: "
                      << begRPC << ", endRPC: " << endRPC << "\n";
   bool err = false;
   Address addr = 0;
   addr = findInternalAddress("curRPC",true, err);
   assert(err==false);
   rpcInfo newRPCInfo;
   newRPCInfo.runningInferiorRPC = runningRPC;
   newRPCInfo.begRPCAddr = begRPC;
   newRPCInfo.endRPCAddr = endRPC;

   bool retv = writeTextSpace((caddr_t)addr,
                              sizeof(rpcInfo), (caddr_t)&newRPCInfo);
   if(retv == false) {
     cerr << "!!!  Couldn't write rpcInfo structure into rt library !!\n";
   }
}

void process::SendAppIRPCInfo(Address curPC) {
   bool err = false;
   if (pd_debug_infrpc)
     inferiorrpc_cerr << "SendAppIRPCInfo(), lastPC: " << curPC << "\n";
   err = false;
   Address lastpcAddr = findInternalAddress("pcAtLastIRPC",true, err);
   assert(err==false);
   int lastPC = curPC;

   bool retv = writeTextSpace((caddr_t)lastpcAddr, sizeof(unsigned), 
                              (caddr_t)&lastPC);
   if(retv == false) {
     cerr << "!!!  Couldn't write pcAtLastIRPC variable into rt library !!\n";
   }
}

/*
If you want to check that ignored traps associated with irpcs are getting
regenerated, you can use this code in the metric::enableDataCollection
functions, after the process has been continued.

    if(procsToContinue.size()>0) {
      sleep(1);
      procsToContinue[0]->CheckAppTrapIRPCInfo();
    }
*/

#ifdef INFERIOR_RPC_DEBUG
void process::CheckAppTrapIRPCInfo() {
   cout << "CheckAppTrapIRPCInfo() - Entering\n";
   int trapNotHandled;
   bool err = false;
   Address addr = 0;
   addr = findInternalAddress("trapNotHandled",true, err);
   assert(err==false);
   if (!readDataSpace((caddr_t)addr, sizeof(int), &trapNotHandled, true))
     return;  // readDataSpace has it's own error reporting

   if(trapNotHandled)
     cerr << "!!! Error, previously ignored trap not regenerated (ABE)!!!\n"; 
   else
     cerr << "CheckAppTrapIRPCInfo() - trap got handled correctly (ABE).\n";
   cout << "CheckAppTrapIRPCInfo() - Leaving\n";
}
#endif    

#if defined(rs6000_ibm_aix4_1) && defined(BPATCH_LIBRARY)
//When we save the world on AIX we must instrument the
//beginning of main() to call dlopen() to load the
//dyninst rt shared library.  The file format of an
//AIX exec does not easily allow dyninst to add the
//dyninst rt lib to the list of libraries to load upon
//startup.  This is why we use dlopen().  If the
//instrumentation that calls DYNINSTinit() is placed
//at the beginning of main() when this dlopen() call
//is inserted, bad things happen.  So we remove
//the call to DYNINSTinit() and replace it with the
//call to dlopen() when we save the world.  
//The handle declared below denotes the DYNINSTinit()
//instrumentation.  It is deleted once the mutatee
//has executed DYNINSTinit() and is ready to execute the
//start of main().

BPatchSnippetHandle *handle; //ccw 17 jul 2002
#endif

void process::installInstrRequests(const pdvector<instMapping*> &requests) {
   for (unsigned lcv=0; lcv < requests.size(); lcv++) {
      instMapping *req = requests[lcv];

      string func_name;
      string lib_name;
      pdvector<function_base *>matchingFuncs;

      getLibAndFunc(req->func, lib_name, func_name);
      
      if (lib_name != "*") {
	function_base *func2 = static_cast<function_base *>(findFuncByName(req->func));
        if(func2 != NULL)
           matchingFuncs.push_back(func2);
        //else
        //   cerr << "couldn't find initial function " << req->func << "\n";
      }
      else {
	// Wildcard: grab all functions matching this name
	findAllFuncsByName(func_name, matchingFuncs);
      }

      for (unsigned funcIter = 0; funcIter < matchingFuncs.size(); funcIter++) {
	function_base *func = matchingFuncs[funcIter];
	
	if (!func) {
	  continue;  // probably should have a flag telling us whether errors
        }

	// should be silently handled or not
	AstNode *ast;
	if ((req->where & FUNC_ARG) && req->args.size()>0) {
	  ast = new AstNode(req->inst, req->args);
	} else {
	  AstNode *tmp = new AstNode(AstNode::Constant, (void*)0);
	  ast = new AstNode(req->inst, tmp);
	  removeAst(tmp);
	}
	if (req->where & FUNC_EXIT) {
	  const pdvector<instPoint*> func_rets = func->funcExits(this);
	  for (unsigned j=0; j < func_rets.size(); j++) {
	    instPoint *func_ret = const_cast<instPoint *>(func_rets[j]);
	    miniTrampHandle mtHandle;
            assert(addInstFunc(&mtHandle, this, func_ret, ast,
			       req->when, req->order, false, 
			       (!req->useTrampGuard))
		   == success_res);
	  }
	  
	}
	
	if (req->where & FUNC_ENTRY) {
	  instPoint *func_entry = const_cast<instPoint *>(func->funcEntry(this));
	  miniTrampHandle mtHandle;
	  assert(addInstFunc(&mtHandle, this, func_entry, ast,
			     req->when, req->order, false,
			     (!req->useTrampGuard))
		 == success_res);
	  
	  
	}
	
	if (req->where & FUNC_CALL) {
	  pdvector<instPoint*> func_calls = func->funcCalls(this);
	  if (func_calls.size() == 0)
            continue;
	  
	  for (unsigned j=0; j < func_calls.size(); j++) {
	    miniTrampHandle mtHandle;
            assert(addInstFunc(&mtHandle, this, func_calls[j], ast,
			       req->when, req->order, false, 
			       (!req->useTrampGuard))
		   == success_res);
	    
	  }
	}
	
	removeAst(ast);
      }
   }
}


bool process::extractBootstrapStruct(DYNINST_bootstrapStruct *bs_record)
{
  const string vrbleName = "DYNINST_bootstrap_info";
  internalSym sym;
  bool flag = findInternalSymbol(vrbleName, true, sym);
  assert(flag);
  Address symAddr = sym.getAddr();

  // bulk read of bootstrap structure
  if (!readDataSpace((const void*)symAddr, sizeof(*bs_record), bs_record, true)) {
    cerr << "extractBootstrapStruct failed because readDataSpace failed" << endl;
    return false;
  }
  return true;
}

#if 0
bool process::handleStopDueToExecEntry() {
   // returns true iff we are processing a stop due to the entry point of exec
   // The exec hasn't yet occurred.

   assert(status_ == stopped);
   PARADYN_bootstrapStruct bs_record;

   if (!extractBootstrapStruct(&bs_record))
      assert(false);

   if (bs_record.event != 4) {
      return false;
   }

#ifndef mips_unknown_ce2_11 //ccw 28 oct 2000 : 29 mar 2001
   assert(getPid() == bs_record.pid);
#endif
   // for now, we just set aside the following information, 
   // to be used after the exec actually happens 
   // (we'll get a SIGTRAP for that).
   // assert(!inExec); // If exec fails we should be able to call this again
   inExec = true;
   execFilePath = string(bs_record.path);

   // the process was stopped...let's continue it so we can process the exec...
   assert(status_ == stopped);
   if (!continueProc())
      assert(false);

   // should we set status_ to neonatal now?  Nah, probably having 
   // the inExec flag set is good enough...

   // shouldn't we be setting reachedInitializationTrap to false???

   return true;
}
#endif

#ifndef BPATCH_LIBRARY
void process::verifyTimerLevels() {
  int hintBestCpuTimerLevel, hintBestWallTimerLevel;
  bool err = false;
  int appAddrWidth = getImage()->getObject().getAddressWidth();
  Address addr = findInternalAddress("hintBestCpuTimerLevel",true,err);
  assert(err==false);
  if (!readDataSpace((caddr_t)addr, appAddrWidth, &hintBestCpuTimerLevel,true))
    return;  // readDataSpace has it's own error reporting
  int curCpuTimerLevel = int(cpuTimeMgr->getBestLevel())+1;
  if(curCpuTimerLevel < hintBestCpuTimerLevel) {
    char errLine[150];
    sprintf(errLine, "Chosen cpu timer level (%d) is not available in the rt library (%d is best).\n", curCpuTimerLevel, hintBestCpuTimerLevel);
    fprintf(stderr, errLine);
    assert(0);
  }

  addr = findInternalAddress("hintBestWallTimerLevel",true,err);
  assert(err==false);
  if (!readDataSpace((caddr_t)addr, appAddrWidth,&hintBestWallTimerLevel,true))
    return;  // readDataSpace has it's own error reporting
  int curWallTimerLevel = int(getWallTimeMgr().getBestLevel())+1;
  if(curWallTimerLevel < hintBestWallTimerLevel) {
    char errLine[150];
    sprintf(errLine, "Chosen wall timer level (%d) is not available in the rt library (%d is best).\n", curWallTimerLevel, hintBestWallTimerLevel);
    fprintf(stderr, errLine);
    assert(0);
  }  
}

/*
// being disabled since written for IRIX platform, now that don't support
// this platform, don't have way to test changes needed in this feature
// feel free to bring back to life if the need arises again
bool process::writeTimerFuncAddr_Force32(const char *rtinstVar,
			   const char *rtinstFunc)
{
   bool err = false;
   int rtfuncAddr = findInternalAddress(rtinstFunc, true, err);
   assert(err==false);

   err = false;
   int timeFuncVarAddr = findInternalAddress(rtinstVar, true, err);
   assert(err==false);

   return writeTextSpace((void *)(timeFuncVarAddr),
			 sizeof(rtfuncAddr), (void *)(&rtfuncAddr));
}
*/

/* That is, get the address of the thing to set the function pointer to.  In
   most cases, this will be the address of the desired function, however, on
   AIX it is the address of a structure which in turn points to the desired
   function. 
*/
Address process::getTimerQueryFuncTransferAddress(const char *helperFPtr) {
  bool err = false;
  Address transferAddrVar = findInternalAddress(helperFPtr, true, err);

  //logStream << "address of var " << helperFPtr << " = " << hex 
  //    << transferAddrVar <<"\n";

  int appAddrWidth = getImage()->getObject().getAddressWidth();

  Address transferAddr = 0;
  assert(err==false);
  if (!readDataSpace((caddr_t)transferAddrVar, appAddrWidth, 
		     &transferAddr, true)) {
    cerr << "getTransferAddress: can't read var " << helperFPtr << "\n";
    return 0;
  }
  return transferAddr;
}

bool process::writeTimerFuncAddr_(const char *rtinstVar,
				   const char *rtinstHelperFPtr)
{
   Address rtfuncAddr = getTimerQueryFuncTransferAddress(rtinstHelperFPtr);
   //logStream << "transfer address at var " << rtinstHelperFPtr << " = " 
   //     << hex << rtfuncAddr <<"\n";
   bool err = false;
   Address timeFuncVarAddr = findInternalAddress(rtinstVar, true, err);
   //logStream << "timeFuncVarAddr (" << rtinstVar << "): " << hex
   //   << timeFuncVarAddr << "\n";
   assert(err==false);
   return writeTextSpace((void *)(timeFuncVarAddr),
   			 sizeof(rtfuncAddr), (void *)(&rtfuncAddr));
}

void process::writeTimerFuncAddr(const char *rtinstVar, 
				 const char *rtinstHelperFPtr)
{   
  bool result;
   // being disabled since written for IRIX platform, now that don't support
   // this platform, don't have way to test changes needed in this feature
   // feel free to bring back to life if the need arises again
   //int appAddrWidth = getImage()->getObject().getAddressWidth();
   //if(sizeof(Address)==8 && appAddrWidth==4)
   //result = writeTimerFuncAddr_Force32(rtinstVar, rtinstFunc);     
   //else
   result = writeTimerFuncAddr_(rtinstVar, rtinstHelperFPtr);          

   if(result == false) {
     cerr << "!!!  Couldn't write timer func address into rt library !!\n";
   }
}

void process::writeTimerLevels() {
   char rtTimerStr[61];
   rtTimerStr[60] = 0;
   string cStr = cpuTimeMgr->get_rtTimeQueryFuncName(cpuTimeMgr_t::LEVEL_BEST);
   strncpy(rtTimerStr, cStr.c_str(), 59);
   writeTimerFuncAddr("PARADYNgetCPUtime", rtTimerStr);
   //logStream << "Setting cpu time retrieval function in rtinst to " 
   //     << rtTimerStr << "\n" << flush;
   
   string wStr=wallTimeMgr->get_rtTimeQueryFuncName(wallTimeMgr_t::LEVEL_BEST);
   strncpy(rtTimerStr, wStr.c_str(), 59);
   writeTimerFuncAddr("PARADYNgetWalltime", rtTimerStr);
   //logStream << "Setting wall time retrieval function in rtinst to " 
   //     << rtTimerStr << "\n" << flush;
}
#endif

void process::getObservedCostAddr() {

#if !defined(SHM_SAMPLING) || 1 //ccw 19 apr 2002 : SPLIT
    bool err;
    costAddr_ = findInternalAddress("DYNINSTobsCostLow", true, err);
    if (err) {
        sprintf(errorLine,"Internal error: unable to find addr of DYNINSTobsCostLow\n");
        logLine(errorLine);
        showErrorCallback(79,errorLine);
        P_abort();
    }
#else
    costAddr_ = (Address)getObsCostLowAddrInApplicSpace();
#endif
}

bool process::checkStatus() {
  if (status_ == exited)
    return(false);
  else
    return true;
}

bool process::dumpCore(const string fileOut) {
  bool res = dumpCore_(fileOut);
  if (!res) {
    return false;
  }
  return true;
}


/*
 * The process was stopped by a signal. Update its status and notify Paradyn.
 */
void process::Stopped() {
  if (status_ == exited) return;
  if (status_ != stopped) {
    status_ = stopped;
#ifndef BPATCH_LIBRARY
    tp->processStatus(pid, procPaused);
#endif

    if ( checkContinueAfterStop() ) {
       if (!continueProc())
          assert(false);
    }
  }
}

/*
 *  The process has exited. Close it down.  Notify Paradyn.
 */
void process::Exited() {
  if (status_ == exited) {
    // Already done the exit (theoretically)
    return;
  }

  // snag the last shared-memory sample:
  status_ = exited;

  detach(false);
  // the process will continue to run (presumably, it will finish _very_
  // soon)

//  status_ = exited;

}

string process::getStatusAsString() const {
   // useful for debugging
   if (status_ == neonatal)
      return "neonatal";
   if (status_ == stopped)
      return "stopped";
   if (status_ == running)
      return "running";
   if (status_ == exited)
      return "exited";

   assert(false);
   return "???";
}

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
bool process::saveOriginalInstructions(Address addr, int size) {
    char *data = new char[size];
    assert(data);

    if (!readTextSpace((const void *)addr, size, data))
        return false;

    beforeMutationList.insertHead(addr, size, data);

    delete[] data;
    
    return true;
}

bool process::writeMutationList(mutationList &list) {
    bool needToCont = false;

    if (status_ == exited)
        return false;

    if (status_ == running) {
        needToCont = true;
        if (! pause())
            return false;
    }

    if (status_ != stopped && status_ != neonatal) {
        sprintf(errorLine, "Internal error: "
                "Unexpected process state %d in process::writeMutationList",
                (int)status_);
        showErrorCallback(39, errorLine);
        return false;
    }

    mutationRecord *mr = list.getHead();

    while (mr != NULL) {
        bool res = writeTextSpace_((void *)mr->addr, mr->size, mr->data);
        if (!res) {
            // XXX Should we do something special when an error occurs, since
            //     it could leave the process with only some mutations
            //     installed?
            string msg =
                string("System error: unable to write to process text space: ")
                + string(sys_errlist[errno]);
            showErrorCallback(38, msg); // XXX Own error number?
            return false;
        }
        mr = mr->next;
    }

    if (needToCont)
        return this->continueProc();
    return true;
}

bool process::uninstallMutations() {
    return writeMutationList(beforeMutationList);
}

bool process::reinstallMutations() {
    return writeMutationList(afterMutationList);
}

mutationRecord::mutationRecord(Address _addr, int _size, const void *_data) {
    prev = NULL;
    next = NULL;
    addr = _addr;
    size = _size;
    data = new char[size];
    assert(data);
    memcpy(data, _data, size);
}

mutationRecord::~mutationRecord()
{
    // allocate it as a char[], might as well delete it as a char[]...
    delete [] static_cast<char *>(data);
}

mutationList::~mutationList() {
    mutationRecord *p = head;

    while (p != NULL) {
        mutationRecord *n = p->next;
        delete p;
        p = n;
    }
}

void mutationList::insertHead(Address addr, int size, const void *data) {
    mutationRecord *n = new mutationRecord(addr, size, data);
    
    assert((head == NULL && tail == NULL) || (head != NULL && tail != NULL));

    n->next = head;
    if (head == NULL)
        tail = n;
    else
        head->prev = n;
    head = n;
}

void mutationList::insertTail(Address addr, int size, const void *data) {
    mutationRecord *n = new mutationRecord(addr, size, data);
    
    assert((head == NULL && tail == NULL) || (head != NULL && tail != NULL));

    n->prev = tail;
    if (tail == NULL)
        head = n;
    else
        tail->next = n;
    tail = n;
}
#endif /* BPATCH_SET_MUTATIONS_ACTIVE */

//
// Fill in the statically determinable components of the call
//  graph for process.  "statically determinable" refers to
//  the problem that some call destinations cannot be determined
//  statically, but rather instrumentation must be inserted to
//  determine the actual target (which may change depending on when 
//  the call is executed).  For example conmsider the assembly code 
//  fragment:
//   ....
//   call <random>  // puts random number (in some range) in g1
//   nop
//   call %g1
//   nop
//   ....
//  Code where the call target cannot be statically determined has
//   been observed w/ pointers to functions, switch statements, and 
//   some heavily optimized SPARC code.
//  Parameters:
//   Called just after an image is parsed and added to process
//   (image can represent either a.out or shared object).  
//   img - pointer to image just parsed and added.
//   shared_object - boolean inidcating whether img refers to an
//   a.out or shared object.
//
//  NOTE : Paradynd keeps 1 copy of each image, even when that image
//   appears in multiple processes (e.g. when that image represents
//   a shared object).  However, for keeping track of call graphs,
//   we want to keep a SEPERATE call graph for every process - this
//   includes the images which may be shared by multiple processes.
//   The reason for this is that when adding dynamically determined
//   call destinations, we want them to apply ONLY to the process
//   in which they are observed, NOT to other processes which may share
//   e.g. the same shared library. 
#ifndef BPATCH_LIBRARY
void process::FillInCallGraphStatic()
{
  // specify entry point (location in code hierarchy to begin call 
  //  graph searches) for call graph.  Currently, begin searches at
  //  "main" - note that main is usually NOT the actual entry point
  //  there is usually a function which does env specific initialization
  //  and sets up exit handling (at least w/ gcc on solaris).  However,
  //  this function is typically in an excluded module.  Anyway, setting
  //  main as the entry point should usually work fairly well, except
  //  that call graph PC searches will NOT catch time spent in the
  //  environment specific setup of _start.

  pd_Function *entry_pdf = (pd_Function *)findOneFunction("main");
  assert(entry_pdf);
  
  CallGraphAddProgramCallback(symbols->file());

  int thr = 0;
  // MT: forward the ID of the first thread.
  if (threads.size()) {
      thr = threads[0]->get_tid();
  }
#if defined(MT_THREAD)
  // Temporary hack -- ordering problem
  thr = 1;
  
#endif
  CallGraphSetEntryFuncCallback(symbols->file(), 
                                entry_pdf->ResourceFullName(), thr);
    
  // build call graph for executable
  symbols->FillInCallGraphStatic(this);
  // build call graph for module containing entry point
  // ("main" is not always defined in the executable)
  image *main_img = entry_pdf->file()->exec();
  if (main_img != symbols) 
    main_img->FillInCallGraphStatic(this);

  // TODO: build call graph for all shared objects?
  CallGraphFillDone(symbols->file());

}

void process::MonitorDynamicCallSites(string function_name){
  resource *r, *p;
  pdmodule *mod;
  r = resource::findResource(function_name);
  assert(r);
  p = r->parent();
  assert(p);
  mod = symbols->findModule(p->name());
  if(!mod){
    //Must be the weird case where main() isn't in the executable
    pd_Function *entry_pdf = (pd_Function *)findOneFunction("main");
    assert(entry_pdf);
    image *main_img = entry_pdf->file()->exec();
    assert(main_img);
    mod = main_img->findModule(p->name());
  }
  assert(mod);
  
  function_base *func, *temp;
  func = mod->findFunction(r->name());
  assert(func);

  //Should I just be using a resource::handle here instead of going through
  //all of this crap to find a pointer to the function???
  string exe_name = getImage()->file();
  pdvector<instPoint*> callPoints;
  callPoints = func->funcCalls(this);
  
  unsigned i;
  for(i = 0; i < callPoints.size(); i++){
    if(!findCallee(*(callPoints[i]),temp)){
      if(!MonitorCallSite(callPoints[i])){
        fprintf(stderr, 
             "ERROR in daemon, unable to monitorCallSite for function :%s\n",
             function_name.c_str());
      }
    }
  }
}
#endif

#ifndef BPATCH_LIBRARY
bool bForceSoftwareLevelCpuTimer() {
  char *pdkill;
  pdkill = getenv("PD_SOFTWARE_LEVEL_CPU_TIMER");
  if( pdkill )
    return true;
  else
    return false;
}

void process::initCpuTimeMgr() {
  if(cpuTimeMgr != NULL)  delete cpuTimeMgr;
  cpuTimeMgr = new cpuTimeMgr_t();
  initCpuTimeMgrPlt();

  if(bForceSoftwareLevelCpuTimer()) {
    cpuTimeMgr_t::mech_t *tm=cpuTimeMgr->getMechLevel(cpuTimeMgr_t::LEVEL_TWO);
    cpuTimeMgr->installMechLevel(cpuTimeMgr_t::LEVEL_BEST, tm);    
    if(bShowTimerInfo())
          cerr << "Forcing to software level cpu timer\n";
    sampleVal_cerr << "Forcing to software level cpu timer\n";
  } else {
    cpuTimeMgr->determineBestLevels(this);
  }
  cpuTimeMgr_t::timeMechLevel ml = cpuTimeMgr->getBestLevel();
  sampleVal_cerr << "Chosen cpu timer level: " << int(ml)+1 << "  "
		 << *cpuTimeMgr->getMechLevel(ml)
		 << "(timeBase is irrelevant for cpu time)\n\n";
  if(bShowTimerInfo()) {
    cerr << "Chosen cpu timer level: " << int(ml)+1 << "  "
	 << *cpuTimeMgr->getMechLevel(ml)
	 << "(timeBase is irrelevant for cpu time)\n\n";    
  }
}

timeStamp process::getCpuTime(int lwp_id) {
   if(status() == exited) {
      return timeStamp::tsLongAgoTime();
   }

   return cpuTimeMgr->getTime(this, lwp_id, cpuTimeMgr_t::LEVEL_BEST);
  /* can nicely handle case when we allow exceptions
     } catch(LevelNotInstalled &) {
     cerr << "getCpuTime: timer level not installed\n";
     assert(0);
     }
  */
}

bool process::yesAvail() {
  return true; 
}

rawTime64 process::getRawCpuTime_hw(int lwp)
{
  return lwps[lwp]->getRawCpuTime_hw();
}

rawTime64 process::getRawCpuTime_sw(int lwp)
{
  return lwps[lwp]->getRawCpuTime_sw();
}

rawTime64 process::getRawCpuTime(int lwp) {
  return cpuTimeMgr->getRawTime(this, lwp, cpuTimeMgr_t::LEVEL_BEST);
  /* can nicely handle case when we allow exceptions
     } catch(LevelNotInstalled &) {
     cerr << "getRawCpuTime: timer level not installed\n";
     assert(0);
     }
  */
}

timeStamp process::units2timeStamp(int64_t rawunits) {
  return cpuTimeMgr->units2timeStamp(rawunits, cpuTimeMgr_t::LEVEL_BEST);
  /* can nicely handle case when we allow exceptions
     } catch(LevelNotInstalled &) {
     cerr << "units2timeStamp: timer level not installed\n";
     assert(0);
     }
  */
}

timeLength process::units2timeLength(int64_t rawunits) {
    return cpuTimeMgr->units2timeLength(rawunits, cpuTimeMgr_t::LEVEL_BEST);

  /* can nicely handle case when we allow exceptions
     } catch(LevelNotInstalled &) {
     cerr << "units2timeStamp: timer level not installed\n";
     assert(0);
     }
  */
}

#endif

#ifdef BPATCH_LIBRARY
BPatch_point *process::findOrCreateBPPoint(BPatch_function *bpfunc,
					   instPoint *ip,
					   BPatch_procedureLocation pointType)
{
   Address addr = ip->iPgetAddress();

   if (ip->iPgetOwner() != NULL) {
      Address baseAddr;
      if (getBaseAddress(ip->iPgetOwner(), baseAddr)) {
         addr += baseAddr;
      }
   }

   if (instPointMap.defines(addr)) {
      return instPointMap[addr];
   } else {
      if (bpfunc == NULL) {
         bpfunc = findOrCreateBPFunc((pd_Function*)ip->iPgetFunction());
      }

      BPatch_point *pt = new BPatch_point(this, bpfunc, ip, pointType);
      instPointMap[addr] = pt;
      return pt;
   }
}

BPatch_function *process::findOrCreateBPFunc(pd_Function* pdfunc,
					     BPatch_module *bpmod)
{
    if (PDFuncToBPFuncMap.defines(pdfunc))
	return PDFuncToBPFuncMap[pdfunc];

    assert(thread);

    // Find the module that contains the function
    if (bpmod == NULL && pdfunc->file() != NULL) {
	BPatch_Vector<BPatch_module *> &mods=*thread->getImage()->getModules();
	for (unsigned int i = 0; i < mods.size(); i++) {
	    if (mods[i]->mod == pdfunc->file()) {
		bpmod = mods[i];
		break;
	    }
	}
	// The BPatch_function may have been created as a side effect
	// of the above
        if (PDFuncToBPFuncMap.defines(pdfunc))
	    return PDFuncToBPFuncMap[pdfunc];
    }

    BPatch_function *ret = new BPatch_function(this, pdfunc, bpmod);

    return ret;
}
#endif

// Add it at the bottom...
void process::deleteInstInstance(instInstance *delInst)
{
  // Add to the list and deal with it later.
  // The question is then, when to GC. I'd suggest
  // when we try to allocate memory, and leave
  // it a public member that can be called when
  // necessary
  struct instPendingDeletion *toBeDeleted = new instPendingDeletion();
  toBeDeleted->hot.push_back(delInst->trampBase);
  toBeDeleted->baseAddr = delInst->trampBase;
  toBeDeleted->oldMini = delInst;
  toBeDeleted->oldBase = NULL;
  pendingGCInstrumentation.push_back(toBeDeleted);
}

bool process::checkIfInstAlreadyDeleted(instInstance *delInst)
{
  for (unsigned i = 0; i < pendingGCInstrumentation.size(); i++)
    if (pendingGCInstrumentation[i]->oldMini == delInst)
      return true;
  return false;
}

void process::deleteBaseTramp(trampTemplate *baseTramp, 
			      instInstance *lastMiniTramp)
{
  struct instPendingDeletion *toBeDeleted = new instPendingDeletion();
  toBeDeleted->hot.push_back(baseTramp->baseAddr);
  toBeDeleted->hot.push_back(lastMiniTramp->trampBase);
  toBeDeleted->baseAddr = baseTramp->baseAddr;
  toBeDeleted->oldMini = NULL;
  toBeDeleted->oldBase = baseTramp;
  pendingGCInstrumentation.push_back(toBeDeleted);
}

// Post on this process
void process::postRPCtoDo(AstNode *action, bool noCost,
                          inferiorRPCcallbackFunc callbackFunc, void *userData,
                          int mid, bool lowmem)
{
   theRpcMgr->postRPCtoDo(action, noCost, callbackFunc, userData, mid, lowmem);
}

// Post on given thread
void process::postRPCtoDo(AstNode *action, bool noCost,
                          inferiorRPCcallbackFunc callbackFunc, void *userData,
                          int mid, dyn_thread *thr, bool lowmem) {
   theRpcMgr->postRPCtoDo(action, noCost, callbackFunc, userData, 
                          mid, thr, lowmem);
}

// Post on given lwp
void process::postRPCtoDo(AstNode *action, bool noCost,
                          inferiorRPCcallbackFunc callbackFunc, void *userData,
                          int mid, dyn_lwp *lwp, bool lowmem) {
   theRpcMgr->postRPCtoDo(action, noCost, callbackFunc, userData,
                          mid, lwp, lowmem);
}

bool process::launchRPCs(bool wasRunning) {
   return theRpcMgr->launchRPCs(wasRunning);
}

bool process::existsRPCPending() const {
   return theRpcMgr->existsRPCPending();
}

bool process::existsRPCinProgress() const {
   return theRpcMgr->existsRPCinProgress();
}

bool process::existsRPCWaitingForSyscall() const {
   return theRpcMgr->existsRPCWaitingForSyscall();
}

void process::cleanRPCreadyToLaunch(int mid) {
   theRpcMgr->cleanRPCreadyToLaunch(mid);
}

bool process::handleTrapIfDueToRPC() {
   return theRpcMgr->handleTrapIfDueToRPC();
}


// garbage collect instrumentation
void process::gcInstrumentation()
{
  // The without-a-passed-in-stackwalk version. Walk the stack
  // and pass it down.
  // First, idiot check...
  if (status() == exited)
    return; 

  if (pendingGCInstrumentation.size() == 0)
    return;

  // We need to pause the process. Otherwise we could have an incorrect
  // stack walk, etc. etc. etc. blahblahblah

  bool wasPaused = true;

  if (status() == running) wasPaused = false;
 
  if (!wasPaused && !pause()) {
    return;
  }

  pdvector< pdvector<Frame> > stackWalks;
  if (!walkStacks(stackWalks)) return;

  gcInstrumentation(stackWalks);
  if(!wasPaused) {
    continueProc();
  }
}

// garbage collect instrumentation
void process::gcInstrumentation(pdvector<pdvector<Frame> > &stackWalks)
{
  // Go through the list and try to clear out any
  // instInstances that are freeable.
  if (status() == exited) return;

  inferiorHeap *hp = &heap;

  if (pendingGCInstrumentation.size() == 0) return;

  for (unsigned i = 0; i < pendingGCInstrumentation.size(); i++) {
    instPendingDeletion *old = pendingGCInstrumentation[i];
    bool safeToDelete = true;
    pdvector<Address> hotAddrs = old->hot;
    // Get the heap item associated with the mini
    for (unsigned hotIter = 0; hotIter < hotAddrs.size(); hotIter++) {
      // Optimization...
      if (!safeToDelete) break;
      heapItem *ptr = NULL;
      if (!hp->heapActive.find(hotAddrs[hotIter], ptr)) {
	sprintf(errorLine,"Warning: attempt to free undefined heap entry 0x%p (pid=%d, heapActive.size()=%d)\n", 
		(void*)hotAddrs[hotIter], getPid(), 
		hp->heapActive.size());
	logLine(errorLine);
	continue;
      }
      // We only can delete an item if it is free for all
      // threads
      for (unsigned threadIter = 0;
	   threadIter < stackWalks.size();
	   threadIter++) {
	if (!safeToDelete) break;
	for (unsigned stackIter = 0;
	     stackIter < stackWalks[threadIter].size();
	     stackIter++) {
	  if (!safeToDelete) break;
	  Address pc = stackWalks[threadIter][stackIter].getPC();
	  // First check if PC is within the minitramp
	  if ((pc >= ptr->addr) && (pc <= ptr->addr + ptr->length))
	    safeToDelete = false;
	}
      }
    }
    if (safeToDelete) {
      inferiorFree(old->baseAddr);
      // Delete from list of GCs
      pendingGCInstrumentation[i] = 
	pendingGCInstrumentation[pendingGCInstrumentation.size()-1];
      pendingGCInstrumentation.resize(pendingGCInstrumentation.size()-1);
      // Back up iterator to cover the fresh one
      i--;
      if (old->oldMini)
	delete old->oldMini;
      if (old->oldBase)
	delete old->oldBase;
      delete old;
    }
  }
}


// Question: if we don't find, do we create?
// For now, yes.

dyn_lwp *process::getLWP(unsigned lwp_id)
{
  dyn_lwp *foundLWP;
  if (lwps.find(lwp_id, foundLWP)) {
    return foundLWP;
  }

  foundLWP = createLWP(lwp_id);
  if (!foundLWP->openFD()) {
     deleteLWP(foundLWP);
     return NULL;
  }
  return foundLWP;
}

dyn_lwp *process::createLWP(unsigned lwp_id) {
   dyn_lwp *lwp = new dyn_lwp(lwp_id, this);
   lwps[lwp_id] = lwp;
   theRpcMgr->newLwpFound(lwp);
   return lwp;
}

void process::deleteLWP(dyn_lwp *lwp_to_delete) {
   if(lwps.size() > 0 && lwp_to_delete!=NULL) {
      unsigned index = lwp_to_delete->get_lwp_id();
      theRpcMgr->deleteLwp(lwp_to_delete);
      lwps.undef(index);
   }
   delete lwp_to_delete;
}

dyn_lwp *process::getDefaultLWP() const
{
  dyn_lwp *lwp;
  if (lwps.find(0, lwp))
    return lwp;
  return NULL;
}

// MT section (move to processMT.C?)
#if defined(MT_THREAD)
// Called for new threads

dyn_thread *process::createThread(
  int tid, 
  unsigned pos, 
  unsigned stackbase, 
  unsigned startpc, 
  void* resumestate_p,  
  bool bySelf)
{
  dyn_thread *thr;
  fprintf(stderr, "Received notice of new thread.... tid %d, pos %d, stackbase 0x%x, startpc 0x%x\n", tid, pos, stackbase, startpc);
  // creating new thread
  thr = new dyn_thread(this, tid, pos, NULL);
  threads += thr;

  thr->update_resumestate_p(resumestate_p);
  function_base *pdf ;

  if (startpc) {
    thr->update_stack_addr(stackbase) ;
    thr->update_start_pc(startpc) ;
    pdf = findFuncByAddr(startpc) ;
    thr->update_start_func(pdf) ;
  } else {
    cerr << "createThread: zero startPC found!" << endl;
    pdf = findOneFunction("main");
    assert(pdf);
    //thr->update_start_pc(pdf->addr()) ;
    thr->update_start_pc(0);
    thr->update_start_func(pdf);
    thr->update_stack_addr(stackbase);
  }

  sprintf(errorLine,"+++++ creating new thread{%s/0x%x}, pos=%u, tid=%d, stack=0x%x, resumestate=0x%x, by[%s]\n",
	  pdf->prettyName().c_str(), startpc, pos,tid,stackbase,(unsigned)resumestate_p, bySelf?"Self":"Parent");
  logLine(errorLine);

  return(thr);
}

//
// CALLED for mainThread
//
void process::updateThread(dyn_thread *thr, int tid, 
			   unsigned pos, void* resumestate_p, 
			   resource *rid)
{
  assert(thr);
  thr->update_tid(tid);
  thr->update_pos(pos);
  thr->update_rid(rid);
  thr->update_resumestate_p(resumestate_p);
  function_base *f_main = findOneFunction("main");
  assert(f_main);

  //unsigned addr = f_main->addr();
  //thr->update_start_pc(addr) ;
  thr->update_start_pc(0) ;
  thr->update_start_func(f_main) ;

  sprintf(errorLine,"+++++ updateThread--> creating new thread{main}, pos=%u, tid=%d, resumestate=0x%x\n", pos,tid, (unsigned) resumestate_p);
  logLine(errorLine);
}

//
// CALLED from Attach
//
void process::updateThread(
  dyn_thread *thr, 
  int tid, 
  unsigned pos, 
  unsigned stackbase, 
  unsigned startpc, 
  void* resumestate_p) 
{
  assert(thr);
  //  
  sprintf(errorLine," updateThread(tid=%d, pos=%d, stackaddr=0x%x, startpc=0x%x)\n",
	 tid, pos, stackbase, startpc);
  logLine(errorLine);

  thr->update_tid(tid);
  thr->update_pos(pos);
  thr->update_resumestate_p(resumestate_p);

  function_base *pdf;

  if(startpc) {
    thr->update_start_pc(startpc) ;
    pdf = findFuncByAddr(startpc) ;
    thr->update_start_func(pdf) ;
    thr->update_stack_addr(stackbase) ;
  } else {
    pdf = findOneFunction("main");
    assert(pdf);
    thr->update_start_pc(startpc) ;
    //thr->update_start_pc(pdf->addr()) ;
    thr->update_start_func(pdf);
    thr->update_stack_addr(stackbase);
  } //else

  sprintf(errorLine,"+++++ creating new thread{%s/0x%x}, pos=%u, tid=%d, stack=0x%xs, resumestate=0x%x\n",
    pdf->prettyName().c_str(), startpc, pos, tid, stackbase, (unsigned) resumestate_p);
  logLine(errorLine);
}

void process::deleteThread(int tid)
{
   pdvector<dyn_thread *>::iterator iter = threads.end();
   while(iter != threads.begin()) {
      dyn_thread *thr = *(--iter);
      if(thr->get_tid() != (unsigned) tid)  continue;

      // ===  Found It  ==========================
      // Set the POS to "reusable"
      // Note: we don't acquire a lock. This is okay, because we're simply
      //       clearing the bit, which was not usable before now anyway.
      assert(shmMetaData->getPosToThread(thr->get_pos()) 
             == THREAD_AWAITING_DELETION);
      shmMetaData->setPosToThread(thr->get_pos(), 0);

      delete thr;    
      sprintf(errorLine,"----- deleting thread, tid=%d, threads.size()=%d\n",
              tid, threads.size());
      logLine(errorLine);

      threads.erase(iter);
   }
}

#endif /* MT*/

    
