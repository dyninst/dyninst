/*
 * Copyright (c) 1996-2001 Barton P. Miller
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

// $Id: process.C,v 1.246 2001/04/03 17:50:03 shergali Exp $

extern "C" {
#ifdef PARADYND_PVM
int pvmputenv (const char *);
int pvmendtask();
#endif
}

#if defined(USES_LIBDYNINSTRT_SO) && defined(i386_unknown_solaris2_5)
#include <sys/procfs.h>
#endif
#include "common/h/headers.h"
#include "dyninstAPI/src/symtab.h"
#ifndef BPATCH_LIBRARY
#include "dyninstAPI/src/pdThread.h"
#endif
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

#ifdef BPATCH_LIBRARY
#include "dyninstAPI/h/BPatch.h"
#else
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "paradynd/src/perfStream.h"
#include "paradynd/src/costmetrics.h"
#include "paradynd/src/mdld.h"
#include "paradynd/src/main.h"
#include "paradynd/src/init.h"
#include "pdutil/h/pdDebugOstream.h"
#include "common/h/int64iostream.h"
#endif

#if defined(SHM_SAMPLING) && defined(MT_THREAD) //inst-sparc.C
extern void generateRPCpreamble(char *insn, Address &base, process *proc, unsigned offset, int tid, unsigned pos);
extern void generateMTpreamble(char *insn, Address &base, process *proc);
#endif

#include "common/h/debugOstream.h"

#ifdef ATTACH_DETACH_DEBUG
debug_ostream attach_cerr(cerr, true);
#else
debug_ostream attach_cerr(cerr, false);
#endif


#ifdef INFERIOR_RPC_DEBUG
debug_ostream inferiorrpc_cerr(cerr, true);
#else
debug_ostream inferiorrpc_cerr(cerr, false);
#endif

#ifdef SHM_SAMPLING_DEBUG
debug_ostream shmsample_cerr(cerr, true);
#else
debug_ostream shmsample_cerr(cerr, false);
#endif

#ifdef FORK_EXEC_DEBUG
debug_ostream forkexec_cerr(cerr, true);
#else
debug_ostream forkexec_cerr(cerr, false);
#endif

#ifdef SIGNAL_DEBUG
debug_ostream signal_cerr(cerr, true);
#else
debug_ostream signal_cerr(cerr, false);
#endif

#ifdef SHAREDOBJ_DEBUG
debug_ostream sharedobj_cerr(cerr, true);
#else
debug_ostream sharedobj_cerr(cerr, false);
#endif

#ifndef BPATCH_LIBRARY
#ifdef METRIC_DEBUG
pdDebug_ostream metric_cerr(cerr, true);
#else
pdDebug_ostream metric_cerr(cerr, false);
#endif

#ifdef SAMPLEVALUE_DEBUG
pdDebug_ostream sampleVal_cerr(cerr, true);
#else
pdDebug_ostream sampleVal_cerr(cerr, false);
#endif

#ifdef AGGREGATE_DEBUG
pdDebug_ostream agg_cerr(cerr, true);
#else
pdDebug_ostream agg_cerr(cerr, false);
#endif
#endif

#define FREE_WATERMARK (hp->totalFreeMemAvailable/2)
#define SIZE_WATERMARK 100
static const timeLength MaxWaitingTime(10, timeUnit::sec());
static const timeLength MaxDeletingTime(2, timeUnit::sec());
unsigned inferiorMemAvailable=0;

unsigned activeProcesses; // number of active processes
vector<process*> processVec;
string process::programName;
string process::dyninstName;
string process::pdFlavor;
vector<string> process::arg_list;

#ifndef BPATCH_LIBRARY
extern string osName;
#endif

// PARADYND_DEBUG_XXX
int pd_debug_infrpc=0;
int pd_debug_catchup=0;
//

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

// Frame(process *): return toplevel (active) stack frame
// (platform-independent wrapper)
Frame::Frame(process *p)
  : uppermost_(true), pc_(0), fp_(0)
#if defined(MT_THREAD)
  , lwp_id_(0), thread_(NULL)
#endif
{
  // platform-dependent implementation
  getActiveFrame(p);
}

// getCallerFrame(): return stack frame of caller, 
// relative to current (callee) stack frame
// (platform-independent wrapper)
Frame Frame::getCallerFrame(process *p) const
{
  // if no previous frame exists, return zero frame
  Frame ret; // zero frame
  if (fp_ == 0) return Frame(); // zero frame

  // platform-dependent implementation
#if defined(MT_THREAD)
  if (!thread_ && lwp_id_) {
    // kernel-level thread
    ret = getCallerFrameLWP(p);
  } else {
    // user-level thread
    ret = getCallerFrameThread(p);
  }
#else
  ret = getCallerFrameNormal(p);
#endif

  // if this is the outermost frame, stop by returning zero frame
  extern bool isValidAddress(process *, Address);
  if (ret.pc_ == 0 || !isValidAddress(p, ret.pc_)) {
    return Frame(); // zero frame
  }

  return ret;
}

#if !defined(rs6000_ibm_aix4_1)
Address process::getTOCoffsetInfo(Address /*dest */)
{
  Address tmp;
  assert(0);
  return tmp; // this is to make the nt compiler happy! - naim
}
#endif

//
// Internal metric stackwalk_time
//
#ifndef BPATCH_LIBRARY
timeStamp startStackwalk = timeStamp::ts1970();
timeLength elapsedStackwalkTime = timeLength::Zero();
bool      stackwalking=false;

pdSample computeStackwalkTimeMetric(const metricDefinitionNode *) {
    // we don't need to use the metricDefinitionNode
    if (isInitFirstRecordTime()) {
        timeLength elapsed = elapsedStackwalkTime;
        if (stackwalking) {
          timeStamp now = getWallTime();
          elapsed += now - startStackwalk;
        }
        assert(elapsed >= timeLength::Zero());
        return pdSample(elapsed);
    } else {
        return pdSample(timeLength::Zero());
    }
}
#endif

#if !defined(i386_unknown_nt4_0)
// Windows NT has its own version of the walkStack function in pdwinnt.C
// Note: it may not always be possible to do a correct stack walk.
// If it can't do a complete walk, the function should return an empty
// vector, which means that there was an error, and we can't walk the stack.
vector<Address> process::walkStack(bool noPause)
{
  vector<Address> pcs;
  bool needToCont = noPause ? false : (status() == running);

#ifndef BPATCH_LIBRARY
  BEGIN_STACKWALK;
#endif

  if (!noPause && !pause()) {
     // pause failed...give up
     cerr << "walkStack: pause failed" << endl;
#ifndef BPATCH_LIBRARY
     END_STACKWALK;
#endif
     return pcs;
  }

  Address sig_addr = 0;
  u_int sig_size = 0;
  if(signal_handler){
      const image *sig_image = (signal_handler->file())->exec();
      if(getBaseAddress(sig_image, sig_addr)){
          sig_addr += signal_handler->getAddress(this);
      } else {
          sig_addr = signal_handler->getAddress(this);
      }
      sig_size = signal_handler->size();
      // printf("signal_handler = %s size = %d addr = 0x%lx\n",
      //     (signal_handler->prettyName()).string_of(),sig_size,sig_addr);
  }

  if (pause()) {
    Frame currentFrame(this);
    Address fpOld = 0;
    while (!currentFrame.isLastFrame()) {
      Address fpNew = currentFrame.getFP();
      // successive frame pointers might be the same (e.g. leaf functions)
      if (fpOld > fpNew) {
        // not moving up stack
        if (!noPause && needToCont && !continueProc())
          cerr << "walkStack: continueProc failed" << endl;
        vector<Address> ev; // empty vector
#ifndef BPATCH_LIBRARY
        END_STACKWALK;
#endif
        return ev;
      }
      fpOld = fpNew;

      Address next_pc = currentFrame.getPC();
      // printf("currentFrame pc = %p\n",next_pc);
      pcs += next_pc;
      // is this pc in the signal_handler function?
      if(signal_handler && (next_pc >= sig_addr)
          && (next_pc < (sig_addr+sig_size))){
          // check to see if a leaf function was executing when the signal
          // handler was called.  If so, then an extra frame should be added
          // for the leaf function...the call to getCallerFrame
          // will get the function that called the leaf function
          Address leaf_pc = 0;
          if(this->needToAddALeafFrame(currentFrame,leaf_pc)){
              pcs += leaf_pc;
          }
      }
      currentFrame = currentFrame.getCallerFrame(this); 
    }
    pcs += currentFrame.getPC();
  }

  if (!noPause && needToCont) {
     if (!continueProc()){
        cerr << "walkStack: continueProc failed" << endl;
     }
  }  

#ifndef BPATCH_LIBRARY
  END_STACKWALK;
#endif
  return(pcs);
}

#if defined(MT_THREAD)
void process::walkAStack(int /*id*/, 
  Frame currentFrame, 
  Address sig_addr, 
  u_int sig_size, 
  vector<Address>&pcs,
  vector<Address>&fps) {

  pcs.resize(0);
  fps.resize(0);
  Address fpOld = 0;
  while (!currentFrame.isLastFrame()) {
      Address fpNew = currentFrame.getFP();
      fpOld = fpNew;

      Address next_pc = currentFrame.getPC();
      pcs += next_pc;
      fps += fpOld ;
      // is this pc in the signal_handler function?
      if(signal_handler && (next_pc >= sig_addr)
            && (next_pc < (sig_addr+sig_size))){
            // check to see if a leaf function was executing when the signal
            // handler was called.  If so, then an extra frame should be added
            // for the leaf function...the call to getCallerFrame
            // will get the function that called the leaf function
            Address leaf_pc = 0;
            if(this->needToAddALeafFrame(currentFrame,leaf_pc)){
                pcs += leaf_pc;
                fps += fpOld ;
            }
      }
      currentFrame = currentFrame.getCallerFrame(this); 
    }
    pcs += currentFrame.getPC();
    fps += fpOld ;
}

vector<vector<Address> > process::walkAllStack(bool noPause) {
  vector<vector<Address> > result ;
  vector<Address> pcs;
  vector<Address> fps ;
  bool needToCont = noPause ? false : (status() == running);
 
#ifndef BPATCH_LIBRARY
  BEGIN_STACKWALK;
#endif

  if (!noPause && !pause()) {
     // pause failed...give up
     cerr << "walkAllStack: pause failed" << endl;
#ifndef BPATCH_LIBRARY
     END_STACKWALK;
#endif
     return result;
  }

  Address sig_addr = 0;
  u_int sig_size = 0;
  if(signal_handler){
      const image *sig_image = (signal_handler->file())->exec();
      if(getBaseAddress(sig_image, sig_addr)){
          sig_addr += signal_handler->getAddress(this);
      } else {
          sig_addr = signal_handler->getAddress(this);
      }
      sig_size = signal_handler->size();
      //sprintf(errorLine, "signal_handler = %s size = %d addr = 0x%lx\n",
      //    (signal_handler->prettyName()).string_of(),sig_size,sig_addr);
      //logLine(errorLine);
  }

  if (pause()) {
    // take a look at mmaps

    // Walk the lwp stack first, walk a thread stack only if it is not active
    int *IDs, i=0;
    vector<Address> lwp_stack_lo;
    vector<Address> lwp_stack_hi;

    if (getLWPIDs(&IDs)) {
      while(IDs[i] != 0) {
        int lwp_id = IDs[i] ;
        Address fp, pc;
        if (getLWPFrame(lwp_id, &fp, &pc)) {
          Frame currentFrame(lwp_id, fp, pc, true);
          walkAStack(lwp_id, currentFrame, sig_addr, sig_size, pcs, fps);
          result += pcs ;
          lwp_stack_hi += fps[fps.size()-1];
          lwp_stack_lo += fps[0];
        }
        i++ ;
      }
      delete [] IDs;
    }

    //for (unsigned j=0; j< lwp_stack_lo.size(); j++) {
    //  sprintf(errorLine, "lwp[%d], stack_lo=0x%lx, stack_hi=0x%lx\n", 
    //    j, lwp_stack_lo[j], lwp_stack_hi[j]);
    //  logLine(errorLine);
    //}

    //Walk thread stacks
    for (unsigned i=0; i<threads.size(); i++) {
      Frame   currentFrame(threads[i]);
      Address stack_lo = currentFrame.getFP();
      //sprintf(errorLine, "stack_lo[%d]=0x%lx\n", i, stack_lo);
      //logLine(errorLine);

      bool active = false ;
      for (unsigned j=0; j< lwp_stack_lo.size(); j++) {
        if (stack_lo >= lwp_stack_lo[j] && stack_lo <= lwp_stack_hi[j]){
          active = true;
          break;
        }
      }
        
      if (!active) {
        walkAStack(i, currentFrame, sig_addr, sig_size, pcs, fps);
        result += pcs ;
      }
    }//threads
  }

  if (!noPause && needToCont) {
     if (!continueProc()){
        cerr << "walkAllStack: continueProc failed" << endl;
     }
  }  
     
#ifndef BPATCH_LIBRARY
  END_STACKWALK;
#endif
  return(result);
}
#endif //MT_THREAD
#endif

void process::correctStackFuncsForTramps(vector<Address> &pcs, 
                                         vector<pd_Function *> &funcs)
{
  unsigned i;
  instPoint *ip;
  function_base *fn;
  for(i=0;i<pcs.size();i++) {
    //if( funcs[ i ] == NULL ) {
    ip = findInstPointFromAddress(this, pcs[i]);
    if( ip ) {
      fn = const_cast<function_base*>( ip->iPgetFunction() );
      if( fn )
        // funcs[ i ] = dynamic_cast<pd_Function*>( fn );
        funcs[ i ] = (pd_Function *) fn;
    }
    //}
  }
}


vector<pd_Function *> process::convertPCsToFuncs(vector<Address> pcs) {
    vector <pd_Function *> ret;
    unsigned i;
        pd_Function *fn;
    for(i=0;i<pcs.size();i++) {
                fn = (pd_Function *)findFunctionIn(pcs[i]);
        ret += fn;
    }
    return ret;
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

bool process::triggeredInStackFrame(instPoint* point, pd_Function* stack_fn,
                                    Address pc, callWhen when, callOrder order)
{
  //this->print(stderr, ">>> triggeredInStackFrame(): ");
  trampTemplate *tempTramp;
  bool retVal = false;

  if (stack_fn != point->iPgetFunction()) return false;

  if ( pd_debug_catchup )
      cerr << "In triggeredInStackFrame : stack function matches function containing instPoint" << endl;
  
  //  Is the pc within the instPoint instrumentation?
  instPoint* currentIp = findInstPointFromAddress(this, pc);

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
        instInstance* currInstance = findMiniTramps(currentIp);
        bool pcInTramp = false;

        while ( currInstance != NULL && !pcInTramp )
        {
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
              switch(currInstance->when) {
                case callPreInsn:
                  cerr << "PreInsn ";
                  break;
                case callPostInsn:
                  cerr << "PostInsn ";
                  break;
              }
              cerr << "instrumentation." << endl;
            }
            
            // The request should be triggered if it is for:
            //   1)  pre-instruction instrumentation to prepend
            //   2)  pre-instruction instrumentation to append
            //         and the pc is in PostInsn instrumentation
            //   3)  post-instruction instrumentation to prepend
            //         and the pc is in PostInsn instrumentation
            if ( (when == callPreInsn && (order == orderFirstAtPoint || 
                  (order == orderLastAtPoint &&
                    currInstance->when == callPostInsn))) ||
                 (when == callPostInsn && order == orderFirstAtPoint &&
                    currInstance->when == callPostInsn) )
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
  
          currInstance = currInstance->next;
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
#if defined(mips_sgi_irix6_4)
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
      if ( pd_debug_catchup )
        cerr << "  Requested instrumentation point is not appropriate for catchup, returning false." << endl;
    }      
#elif defined(i386_unknown_nt4_0) || defined(i386_unknown_solaris2_5) \
|| defined(i386_unknown_linux2_0)
    if ( point->address() == point->func()->addr() ) {
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
bool isFreeOK(process *proc, disabledItem &dis, vector<Address> &pcs)
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

  vector<addrVecType> &points = dis.pointsToCheck; 
  for (unsigned pci = 0; pci < pcs.size(); pci++) {
    Address pc = pcs[pci];
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

int heapItemCmpByAddr(const void *A, const void *B)
{
  heapItem *a = *(heapItem **)const_cast<void*>(A);
  heapItem *b = *(heapItem **)const_cast<void*>(B);

  if (a->addr < b->addr) {
      return -1;
  } else if (a->addr > b->addr) {
      return 1;
  } else {
      return 0;
  }
}

void inferiorFreeCompact(inferiorHeap *hp)
{
  vector<heapItem *> &freeList = hp->heapFree;
  unsigned i, nbuf = freeList.size();

  /* sort buffers by address */
  freeList.sort(&heapItemCmpByAddr);

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
    vector<heapItem *> cleanList;
    unsigned end = freeList.size();
    for (i = 0; i < end; i++) {
      heapItem *h1 = freeList[i];
      if (h1->length != 0) {
        cleanList += h1;
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

void inferiorFreeDeferred(process *proc, inferiorHeap *hp, bool runOutOfMem)
{
  vector<Address> pcs = proc->walkStack();

#if defined(i386_unknown_nt4_0)
  // if walkStack() fails, assume not safe to delete anything
  if (pcs.size() == 0) return;
#endif

  // set allowed deletion time
  timeLength maxDelTime = MaxDeletingTime;
  if (runOutOfMem) {
    maxDelTime += MaxDeletingTime; // double allowed deletion time
    sprintf(errorLine, "Emergency attempt to free memory (pid=%d)\n", proc->getPid());
    logLine(errorLine);
  }
  timeStamp initTime = getCurrentTime();

  vector<disabledItem> &disabled = hp->disabledList;
  for (unsigned i = 0; i < disabled.size(); i++) {
    // exit if over allowed deletion time
    if (getCurrentTime() - initTime >= maxDelTime) {
      sprintf(errorLine, "inferiorFreeDeferred(): out of time\n");
      logLine(errorLine);
      return;
    }

    disabledItem &item = disabled[i];
    if (isFreeOK(proc, item, pcs)) {
      heapItem *np = NULL;
      Address pointer = item.block.addr;
      if (!hp->heapActive.find(pointer, np)) {
        showErrorCallback(96,"Internal error: "
                "attempt to free non-defined heap entry.");
        return;
      }
      assert(np);

      if (np->status != HEAPallocated) {
        sprintf(errorLine,"Attempt to re-free heap entry 0x%lx\n", pointer);
        logLine(errorLine);
        showErrorCallback(67, (const char *)errorLine); 
        return;
      }

      // remove from active list
      hp->heapActive.undef(pointer);
      // remove from disabled list
      disabled[i] = disabled[disabled.size()-1];
      disabled.resize(disabled.size()-1);
      hp->disabledListTotalMem -= np->length;
      // add to free list
      np->status = HEAPfree;      
      hp->heapFree += np;
      hp->totalFreeMemAvailable += np->length;
      // bookkeeping
      hp->freed += np->length;
      inferiorMemAvailable = hp->totalFreeMemAvailable;
      i--; // move index back to account for resize()
    }
  }
}

// Get inferior heaps from every library currently loaded.
// This is done at startup by initInferiorHeap().
// There's also another one which takes a shared library and
// only looks in there for the heaps

bool process::getInfHeapList(vector<heapDescriptor> &infHeaps)
{
  bool foundHeap = false;
  // First check the program (without shared libs)
  foundHeap = getInfHeapList(symbols, infHeaps);

  // Now iterate through shared libs
  if (shared_objects)
    for(u_int j=0; j < shared_objects->size(); j++)
      {
	if (getInfHeapList(((*shared_objects)[j])->getImage(), infHeaps))
	  foundHeap = true;
      }
  return foundHeap;
}

bool process::getInfHeapList(const image *theImage, // okay, boring name
			     vector<heapDescriptor> &infHeaps)
{

  // First we get the list of symbols we're interested in, then
  // we go through and add them to the heap list. This is done
  // for two reasons: first, the symbol address might be off (depends
  // on the image), and this lets us do some post-processing.
  bool foundHeap = false;
  vector<Symbol> heapSymbols;
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
      char *temp_str = (char *)malloc(strlen(heapSymbols[j].name().string_of())+1);
      strcpy(temp_str, heapSymbols[j].name().string_of());
      char *garbage_str = strtok(temp_str, "_"); // Don't care about beginning
      assert(!strcmp("DYNINSTstaticHeap", garbage_str));
      // Name is as is.
      // If address is zero, then skip (error condition)
      if (heapSymbols[j].addr() == 0)
	{
	  cerr << "Skipping heap " << heapSymbols[j].name().string_of()
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

      infHeaps += heapDescriptor(heapSymbols[j].name(),
				 heapSymbols[j].addr()+baseAddr,
				 heap_size, heap_type);
      free(temp_str);
    }
  return foundHeap;
}

/*
 * Given an image, add all static heaps inside it
 * (DYNINSTstaticHeap...) to the buffer pool.
 */

void process::addInferiorHeap(const image *theImage)
{
  vector<heapDescriptor> infHeaps;

  /* Get a list of inferior heaps in the new image */
  if (getInfHeapList(theImage, infHeaps))
    {
      /* Add the vector to the inferior heap structure */
      for (u_int j=0; j < infHeaps.size(); j++)
	{
	  heapItem *h = new heapItem (infHeaps[j].addr(), infHeaps[j].size(),
				      infHeaps[j].type(), false);
	  heap.bufferPool += h;
	  heapItem *h2 = new heapItem(h);
	  heap.heapFree += h2;
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
  vector<heapDescriptor> infHeaps;

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
	hp->bufferPool += new heapItem (infHeaps[j].addr(), infHeaps[j].size(),
					infHeaps[j].type(), false);
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
	hp->bufferPool += new heapItem(heapAddr, staticHeapSize - LOWMEM_HEAP_SIZE,
				       anyHeap, false);
	hp->bufferPool += new heapItem(heapAddr + staticHeapSize - LOWMEM_HEAP_SIZE,
				       LOWMEM_HEAP_SIZE, lowmemHeap, false);
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
    hp->heapFree += hi;
    hp->totalFreeMemAvailable += hi->length;     
  }
  inferiorMemAvailable = hp->totalFreeMemAvailable;
}

// create a new inferior heap that is a copy of src. This is used when a process
// we are tracing forks.
inferiorHeap::inferiorHeap(const inferiorHeap &src):
    heapActive(addrHash16)
{
    for (unsigned u1 = 0; u1 < src.heapFree.size(); u1++) {
      heapFree += new heapItem(src.heapFree[u1]);
    }

    vector<heapItem *> items = src.heapActive.values();
    for (unsigned u2 = 0; u2 < items.size(); u2++) {
      heapActive[items[u2]->addr] = new heapItem(items[u2]);
    }
    
    for (unsigned u3 = 0; u3 < src.disabledList.size(); u3++) {
      disabledList += src.disabledList[u3];
    }

    for (unsigned u4 = 0; u4 < src.bufferPool.size(); u4++) {
      bufferPool += new heapItem(src.bufferPool[u4]);
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
int findFreeIndex(process *p, unsigned size, int type, Address lo, Address hi)
{
  vector<heapItem *> &freeList = p->heap.heapFree;

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
void inferiorMallocCallback(process * /*p*/, void *data, void *result)
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
void inferiorMallocDynamic(process *p, int size, Address lo, Address hi)
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
  vector<AstNode*> args(3);
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
#if defined(MT_THREAD)
  p->postRPCtoDo(code, true, &inferiorMallocCallback, &ret, -1, -1, false, true);
#else
  p->postRPCtoDo(code, true, &inferiorMallocCallback, &ret, -1, true);
#endif
  extern void checkProcStatus();
  do {
#ifdef DETACH_ON_THE_FLY
    p->launchRPCifAppropriate((p->status()==running || p->juststopped), false);
#else
    p->launchRPCifAppropriate((p->status()==running), false);
#endif
    checkProcStatus();
  } while (!ret.ready);
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
    p->heap.bufferPool += h;
    // add new segment to free list
    heapItem *h2 = new heapItem(h);
    p->heap.heapFree += h2;
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

#if defined(rs6000_ibm_aix4_1)
// TODO: resolve unsigned comparison issues
const Address ADDRESS_LO = ((Address)0x10000000);
const Address ADDRESS_HI = ((Address)0x7fffffff);
#else
const Address ADDRESS_LO = ((Address)0);
const Address ADDRESS_HI = ((Address)~((Address)0));
#endif


Address inferiorMalloc(process *p, unsigned size, inferiorHeapType type, 
                       Address near_, bool *err)
{
  inferiorHeap *hp = &p->heap;
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
    case 1: // deferred free, compact free blocks
      inferiorFreeDeferred(p, hp, false);
      inferiorFreeCompact(hp);
      break;
    case 2: // allocate new segment (1MB, constrained)
      inferiorMallocDynamic(p, HEAP_DYN_BUF_SIZE, lo, hi);
      break;
    case 3: // allocate new segment (sized, constrained)
      inferiorMallocDynamic(p, size, lo, hi);
      break;
    case 4: // remove range constraints
      lo = ADDRESS_LO;
      hi = ADDRESS_HI;
      if (err) *err = true;
      break;
    case 5: // allocate new segment (1MB, unconstrained)
      inferiorMallocDynamic(p, HEAP_DYN_BUF_SIZE, lo, hi);
      break;
    case 6: // allocate new segment (sized, unconstrained)
      inferiorMallocDynamic(p, size, lo, hi);
      break;
    case 7: // deferred free, compact free blocks
      inferiorFreeDeferred(p, hp, true);
      inferiorFreeCompact(hp);
      break;
#else /* !(USES_DYNAMIC_INF_HEAP) */
    case 1: // deferred free, compact free blocks
      inferiorFreeDeferred(p, hp, true);
      inferiorFreeCompact(hp);
      break;
#endif /* USES_DYNAMIC_INF_HEAP */
      
    default: // error - out of memory
      sprintf(errorLine, "***** Inferior heap overflow: %d bytes "
              "freed, %d bytes requested\n", hp->freed, size);
      logLine(errorLine);
      showErrorCallback(66, (const char *) errorLine);    
#if defined(BPATCH_LIBRARY)
      return(0);
#else
      P__exit(-1);
#endif
    }
    freeIndex = findFreeIndex(p, size, type, lo, hi);
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
  return(h->addr);
}

void inferiorFree(process *p, Address block, 
                  const vector<addrVecType> &pointsToCheck)
{
  inferiorHeap *hp = &p->heap;

  // find block on active list
  heapItem *h = NULL;  
  if (!hp->heapActive.find(block, h)) {
    showErrorCallback(96,"Internal error: "
        "attempt to free non-defined heap entry.");
    return;
  }
  assert(h);

  // add block to disabled list
  hp->disabledList += disabledItem(h, pointsToCheck);
  hp->disabledListTotalMem += h->length;
  
  // perform deferred freeing if above watermark
  if (((hp->disabledListTotalMem > FREE_WATERMARK) ||
       (hp->disabledList.size() > SIZE_WATERMARK)) && waitingPeriodIsOver()) 
    {
      inferiorFreeDeferred(p, hp, false);
    }
}


#ifdef DETACH_ON_THE_FLY
extern void initDetachOnTheFly();
#endif

// initializes all DYNINST lib stuff: init the inferior heap and check for 
// required symbols.
// This is only called after we get the first breakpoint (SIGTRAP), because
// the DYNINST lib can be dynamically linked (currently it is only dynamically
// linked on Windows NT)
bool process::initDyninstLib() {
#if defined(i386_unknown_nt4_0)
   /***
     Kludge for Windows NT: we need to call waitProcs here so that
     we can load libdyninstRT when we attach to an already running
     process. The solution to avoid this kludge is to divide 
     attachProcess in two parts: first we attach to a process,
     and later, after libdyninstRT has been loaded,
     we install the call to DYNINSTinit.
    ***/

   // libDyninstRT should already be loaded when we get here,
   // except if the process was created via attach
   if (createdViaAttach) {
     // need to set reachedFirstBreak to false here, so that
     // libdyninstRT gets loaded in waitProcs.
     reachedFirstBreak = false;
     while (!hasLoadedDyninstLib) {
       int status;
       waitProcs(&status);
     }
   }
   assert(hasLoadedDyninstLib);
#endif

  initInferiorHeap();
  extern vector<sym_data> syms_to_find;
  if (!heapIsOk(syms_to_find))
    return false;

  return true;
}

//
// cleanup when a process is deleted
//
#ifdef BPATCH_LIBRARY
process::~process()
{
    detach(false);

    // remove inst points for this process
    cleanInstFromActivePoints(this);

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
}
#else
process::~process() {
  cpuTimeMgr->destroyMechTimers(this);
}
#endif

unsigned hash_bp(function_base * const &bp ) { return(addrHash4((Address) bp)); }

//
// Process "normal" (non-attach, non-fork) ctor, for when a new process
// is fired up by paradynd itself.
//

process::process(int iPid, image *iImage, int iTraceLink, int iIoLink
#ifdef SHM_SAMPLING
                 , key_t theShmKey,
                 const vector<fastInferiorHeapMgr::oneHeapStats> &iShmHeapStats
#endif
) :
#ifndef BPATCH_LIBRARY
             cpuTimeMgr(NULL),
#ifdef HRTIME
             hr_cpu_link(NULL),
#endif
#endif
             baseMap(ipHash), 
#ifdef BPATCH_LIBRARY
	     PDFuncToBPFuncMap(hash_bp),
	     instPointMap(hash_address),
#endif
             trampGuardFlagAddr(0),
             savedRegs(NULL),
             pid(iPid) // needed in fastInferiorHeap ctors below
#if defined(SHM_SAMPLING)
             ,previous(0),
             inferiorHeapMgr(theShmKey, iShmHeapStats, iPid),
             theSuperTable(this,
                        iShmHeapStats[0].maxNumElems,
                        iShmHeapStats[1].maxNumElems,
#if defined(MT_THREAD)  
                        iShmHeapStats[2].maxNumElems,
                        MAX_NUMBER_OF_THREADS/4)
#else
                        iShmHeapStats[2].maxNumElems)
#endif
#endif
{
#ifdef DETACH_ON_THE_FLY
  haveDetached = 0;
  juststopped = 0;
  needsDetach = 0;
  pendingSig = 0;
#endif /* DETACH_ON_THE_FLY */

#if defined(MT_THREAD)
    preambleForDYNINSTinit=true;
    inThreadCreation=false;
    DYNINSTthreadRPC = 0 ;      //for safe inferiorRPC
    DYNINSTthreadRPC_mp = NULL ;
    DYNINSTthreadRPC_cvp = NULL;
    DYNINSTthreadRPC_pending_p =  NULL;
    DYNINST_allthreads_p = 0 ;  //for look into the thread package
    allthreads = 0 ;            // and the live threads
#endif
    hasBootstrapped = false;
    save_exitset_ptr = NULL;

    // the next two variables are used only if libdyninstRT is dynamically linked
    hasLoadedDyninstLib = false;
    isLoadingDyninstLib = false;

#if defined(USES_LIBDYNINSTRT_SO) && !defined(i386_unknown_nt4_0)
    dyninstlib_brk_addr = 0;
    main_brk_addr = 0;
#endif

    reachedFirstBreak = false; // haven't yet seen first trap
    wasRunningWhenAttached = false;
    reachedVeryFirstTrap = false;
    createdViaAttach = false;
        createdViaFork = false;
    needToContinueAfterDYNINSTinit = false;  //Wait for press of "RUN" button

    symbols = iImage;
    mainFunction = NULL; // set in platform dependent function heapIsOk

    status_ = neonatal;
    exitCode_ = -1;
    continueAfterNextStop_ = 0;
    deferredContinueProc = false;

#ifndef BPATCH_LIBRARY
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
    bufStart = 0;
    bufEnd = 0;
    inExec = false;

    cumObsCost = 0;
    lastObsCostLow = 0;

    proc_fd = -1;

#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0) \
 || defined(i386_unknown_nt4_0)
    trampTableItems = 0;
    memset(trampTable, 0, sizeof(trampTable));
#endif
    currentPC_ = 0;
    hasNewPC = false;
    
    dynamiclinking = false;
    dyn = new dynamic_linking;
    shared_objects = 0;
    all_functions = 0;
    all_modules = 0;
    some_modules = 0;
    some_functions = 0;
    waiting_for_resources = false;
    signal_handler = 0;
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
#if defined(alpha_dec_osf4_0)
   changedPCvalue = 0;
#endif

   traceLink = iTraceLink;
   ioLink = iIoLink;

   RPCs_waiting_for_syscall_to_complete = false;
   stoppedInSyscall = false;

   // attach to the child process (machine-specific implementation)
   if (!attach()) { // error check?
      string msg = string("Warning: unable to attach to specified process :")
                   + string(pid);
      showErrorCallback(26, msg.string_of());
   }
/*
// A test. Let's see if my symbol-print-out works
   fprintf(stderr, "Attempting to find DYNINST internal symbols\n");
   vector<heapDescriptor*> inferiorHeaps;
   symbols->getDYNINSTHeaps(inferiorHeaps);
   fprintf(stderr, "Done\n");
*/
}

//
// Process "attach" ctor, for when paradynd is attaching to an already-existing
// process. 
//
process::process(int iPid, image *iSymbols,
                 int afterAttach, // 1 --> pause, 2 --> run, 0 --> leave as is
                 bool &success
#ifdef SHM_SAMPLING
                 , key_t theShmKey,
                 const vector<fastInferiorHeapMgr::oneHeapStats> &iShmHeapStats
#endif
                 ) :
#ifndef BPATCH_LIBRARY
                cpuTimeMgr(NULL),
#ifdef HRTIME
                hr_cpu_link(NULL),
#endif
#endif
                baseMap(ipHash),
#ifdef BPATCH_LIBRARY
	        PDFuncToBPFuncMap(hash_bp),
		instPointMap(hash_address),
#endif
                trampGuardFlagAddr(0),
                savedRegs(NULL),
                pid(iPid)
#ifdef SHM_SAMPLING
             ,previous(0),
             inferiorHeapMgr(theShmKey, iShmHeapStats, iPid),
             theSuperTable(this,
                           iShmHeapStats[0].maxNumElems,
                           iShmHeapStats[1].maxNumElems,
#if defined(MT_THREAD)
                           iShmHeapStats[2].maxNumElems,
                           MAX_NUMBER_OF_THREADS/4)
#else
                           iShmHeapStats[2].maxNumElems)
#endif
#endif
{
#ifdef DETACH_ON_THE_FLY
  haveDetached = 0;
  juststopped = 0;
  needsDetach = 0;
  pendingSig = 0;
#endif /* DETACH_ON_THE_FLY */


#if defined(SHM_SAMPLING) && defined(MT_THREAD)
   inThreadCreation=false;
   preambleForDYNINSTinit=true;
   DYNINSTthreadRPC = 0 ;
   DYNINSTthreadRPC_mp = NULL ;
   DYNINSTthreadRPC_cvp = NULL;
   DYNINSTthreadRPC_pending_p = NULL;
   DYNINST_allthreads_p = 0 ;
   allthreads = 0 ;
#endif
   RPCs_waiting_for_syscall_to_complete = false;
   save_exitset_ptr = NULL;
   stoppedInSyscall = false;

#if defined(USES_LIBDYNINSTRT_SO) && !defined(i386_unknown_nt4_0)
    dyninstlib_brk_addr = 0;
    main_brk_addr = 0;
#endif

#ifndef BPATCH_LIBRARY
   //  When running an IRIX MPI program, the IRIX MPI job launcher
   //  "mpirun" creates all the processes.  When we create process
   //  objects for these processes we aren't actually "attaching" to
   //  the program, but we use most of this constructor since we
   //  don't actually create the processes.

   if ( process::pdFlavor == "mpi" && osName.prefixed_by("IRIX") )
   {
      needToContinueAfterDYNINSTinit = false;  //Wait for press of "RUN" button         
      reachedFirstBreak = false; // haven't yet seen first trap
      createdViaAttach = false;
   }
   else
#endif
   {
      reachedFirstBreak = true;
      createdViaAttach = true;
   }

   hasBootstrapped = false;
   reachedVeryFirstTrap = true;
   createdViaFork = false;

   // the next two variables are used only if libdyninstRT is dynamically linked
   hasLoadedDyninstLib = false;
   isLoadingDyninstLib = false;

   symbols = iSymbols;
   mainFunction = NULL; // set in platform dependent function heapIsOk

   status_ = neonatal;
   exitCode_ = -1;
   continueAfterNextStop_ = 0;
   deferredContinueProc = false;

#ifndef BPATCH_LIBRARY
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
    bufStart = 0;
    bufEnd = 0;
    inExec = false;

    cumObsCost = 0;
    lastObsCostLow = 0;

    proc_fd = -1;

#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0) \
 || defined(i386_unknown_nt4_0)
    trampTableItems = 0;
    memset(trampTable, 0, sizeof(trampTable));
#endif
    currentPC_ = 0;
    hasNewPC = false;
    
    dynamiclinking = false;
    dyn = new dynamic_linking;
    shared_objects = 0;
    all_functions = 0;
    all_modules = 0;
    some_modules = 0;
    some_functions = 0;
    waiting_for_resources = false;
    signal_handler = 0;
    execed_ = false;

    splitHeaps = false;

#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1) || defined(alpha_dec_osf4_0)
        // XXXX - move this to a machine dependant place.

        // create a seperate text heap.
        //initInferiorHeap(true);
        splitHeaps = true;
#endif

#if defined(alpha_dec_osf4_0)
   changedPCvalue = 0;
#endif

   traceLink = -1; // will be set later, when the appl runs DYNINSTinit

   ioLink = -1; // (ARGUABLY) NOT YET IMPLEMENTED...MAYBE WHEN WE ATTACH WE DON'T WANT
                // TO REDIRECT STDIO SO WE CAN LEAVE IT AT -1.

   // Now the actual attach...the moment we've all been waiting for

   attach_cerr << "process attach ctor: about to attach to pid " << getPid() << endl;

   // It is assumed that a call to attach() doesn't affect the running status
   // of the process.  But, unfortunately, some platforms may barf if the
   // running status is anything except paused. (How to deal with this?)
   // Note that solaris in particular seems able to attach even if the process
   // is running.
   if (!attach()) {
      string msg = string("Warning: unable to attach to specified process: ")
                   + string(pid);
      showErrorCallback(26, msg.string_of());
      success = false;
      return;
   }

#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
   // Now that we're attached, we can reparse the image with correct
   // settings. TODO: get a correct symbols, and blow away the old one.
   int status = pid;
   fileDescriptor *desc = getExecFileDescriptor(symbols->name(), status, false);
   if (!desc) {
      string msg = string("Warning: unable to parse to specified process: ")
                   + string(pid);
      showErrorCallback(26, msg.string_of());
      success = false;
      return;
   }
   image *theImage = image::parseImage(desc);
   if (theImage == NULL) {
      string msg = string("Warning: unable to parse to specified process: ")
                   + string(pid);
      showErrorCallback(26, msg.string_of());
      success = false;
      return;
   }
   // this doesn't leak, since the old theImage was deleted. 
   symbols = theImage;
#endif

#if defined(mips_sgi_irix6_4) && !defined(BPATCH_LIBRARY)
   if ( process::pdFlavor == "mpi" && osName.prefixed_by("IRIX") )
   {
      pause_();
      insertTrapAtEntryPointOfMain();
      continueProc();
   }
#endif

#if defined(BPATCH_LIBRARY) && defined(rs6000_ibm_aix4_1)
   wasRunningWhenAttached = false; /* XXX Or should the default be true? */
#else
   wasRunningWhenAttached = isRunning_();
#endif

#if defined(BPATCH_LIBRARY) && defined(rs6000_ibm_aix4_1)
   // We use ptrace of AIX, which stops the process on attach.
   status_ = stopped;
#else
   // Note: we used to pause the program here, but not anymore.
   status_ = running;
#endif

#ifdef i386_unknown_nt4_0 // Except we still pause on NT.
    if (!pause())
        assert(false);
#endif

   if (afterAttach == 0)
      needToContinueAfterDYNINSTinit = wasRunningWhenAttached;
   else if (afterAttach == 1)
      needToContinueAfterDYNINSTinit = false;
   else if (afterAttach == 2)
      needToContinueAfterDYNINSTinit = true;
   else
      assert(false);

   // Does attach() send a SIGTRAP, a la the initial SIGTRAP sent at the
   // end of exec?  It seems that on some platforms it does; on others
   // it doesn't.  Ick.  On solaris, it doesn't.

   // note: we don't call getSharedObjects() yet; that happens once DYNINSTinit
   //       finishes (handleStartProcess)

   // Everything worked
   success = true;
}

// #if !defined(BPATCH_LIBRARY)

//
// Process "fork" ctor, for when a process which is already being monitored by
// paradynd executes the fork syscall.
//

process::process(const process &parentProc, int iPid, int iTrace_fd
#ifdef SHM_SAMPLING
                 ,key_t theShmKey,
                 void *applShmSegPtr,
                 const vector<fastInferiorHeapMgr::oneHeapStats> &iShmHeapStats
#endif
                 ) :
#ifndef BPATCH_LIBRARY
  cpuTimeMgr(NULL),
#ifdef HRTIME
  hr_cpu_link(NULL),
#endif
#endif
  baseMap(ipHash), // could change to baseMap(parentProc.baseMap)
#ifdef BPATCH_LIBRARY
  PDFuncToBPFuncMap(hash_bp),
  instPointMap(hash_address),
#endif
  trampGuardFlagAddr(0),
  savedRegs(NULL)
#ifdef SHM_SAMPLING
  ,previous(0),
  inferiorHeapMgr(parentProc.inferiorHeapMgr, applShmSegPtr, 
                  theShmKey, iShmHeapStats, iPid),
  theSuperTable(parentProc.getTable(), this)
#endif
{
#ifdef DETACH_ON_THE_FLY
  haveDetached = 0;
  juststopped = 0;
  needsDetach = 0;
  pendingSig = 0;
#endif /* DETACH_ON_THE_FLY */

    // This is the "fork" ctor
#if defined(SHM_SAMPLING) && defined(MT_THREAD)
    inThreadCreation=false;
    preambleForDYNINSTinit=true;
    DYNINSTthreadRPC = 0 ;
    DYNINSTthreadRPC_mp = NULL ;
    DYNINSTthreadRPC_cvp = NULL;
    DYNINSTthreadRPC_pending_p = NULL;
    DYNINST_allthreads_p = 0 ;
    allthreads = 0 ;
#endif
    RPCs_waiting_for_syscall_to_complete = false;
    save_exitset_ptr = NULL;
    stoppedInSyscall = false;


    hasBootstrapped = false;
       // The child of fork ("this") has yet to run DYNINSTinit.

    // the next two variables are used only if libdyninstRT is dynamically linked
    hasLoadedDyninstLib = false; // TODO: is this the right value?
    isLoadingDyninstLib = false;

        createdViaFork = true;
    createdViaAttach = parentProc.createdViaAttach;
    wasRunningWhenAttached = true;
    needToContinueAfterDYNINSTinit = true;

    symbols = parentProc.symbols; // shouldn't a reference count also be bumped?
    mainFunction = parentProc.mainFunction;

    traceLink = iTrace_fd;

    ioLink = -1; // when does this get set?

    status_ = neonatal; // is neonatal right?
    exitCode_ = -1;
    continueAfterNextStop_ = 0;
    deferredContinueProc = false;

    pid = iPid; 

#ifndef BPATCH_LIBRARY
    initCpuTimeMgr();

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

    parent = &parentProc;
    
    bufStart = 0;
    bufEnd = 0;

#if defined(USES_LIBDYNINSTRT_SO) && !defined(i386_unknown_nt4_0)
    dyninstlib_brk_addr = 0;
    main_brk_addr = 0;
#endif
#if defined(alpha_dec_osf4_0)
   changedPCvalue = 0;
#endif

    reachedFirstBreak = true; // initial TRAP has (long since) been reached
    reachedVeryFirstTrap = true;

    splitHeaps = parentProc.splitHeaps;

    heap = inferiorHeap(parentProc.heap);

    inExec = false;

    cumObsCost = 0;
    lastObsCostLow = 0;

    proc_fd = -1;

#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0) \
 || defined(i386_unknown_nt4_0)
    trampTableItems = 0;
    memset(trampTable, 0, sizeof(trampTable));
#endif
    currentPC_ = 0;
    hasNewPC = false;

    dynamiclinking = parentProc.dynamiclinking;
    dyn = new dynamic_linking;
    *dyn = *parentProc.dyn;

    shared_objects = 0;

    // make copy of parent's shared_objects vector
    if (parentProc.shared_objects) {
      shared_objects = new vector<shared_object*>;
      for (unsigned u1 = 0; u1 < parentProc.shared_objects->size(); u1++){
        *shared_objects += 
                new shared_object(*(*parentProc.shared_objects)[u1]);
      }
    }

    all_functions = 0;
    if (parentProc.all_functions) {
      all_functions = new vector<function_base *>;
      for (unsigned u2 = 0; u2 < parentProc.all_functions->size(); u2++)
        *all_functions += (*parentProc.all_functions)[u2];
    }

    all_modules = 0;
    if (parentProc.all_modules) {
      all_modules = new vector<module *>;
      for (unsigned u3 = 0; u3 < parentProc.all_modules->size(); u3++)
        *all_modules += (*parentProc.all_modules)[u3];
    }

    some_modules = 0;
    if (parentProc.some_modules) {
      some_modules = new vector<module *>;
      for (unsigned u4 = 0; u4 < parentProc.some_modules->size(); u4++)
        *some_modules += (*parentProc.some_modules)[u4];
    }
    
    some_functions = 0;
    if (parentProc.some_functions) {
      some_functions = new vector<function_base *>;
      for (unsigned u5 = 0; u5 < parentProc.some_functions->size(); u5++)
        *some_functions += (*parentProc.some_functions)[u5];
    }

    waiting_for_resources = false;
    signal_handler = parentProc.signal_handler;
    execed_ = false;

#if !defined(BPATCH_LIBRARY)
   // threads... // 6/2/99 zhichen
   for (unsigned i=0; i<parentProc.threads.size(); i++) {
     threads += new pdThread(this,parentProc.threads[i]);
#if defined(MT_THREAD)
     pdThread *thr = threads[i] ;
     string buffer;
     string pretty_name=string(thr->get_start_func()->prettyName().string_of());
     buffer = string("thr_")+string(thr->get_tid())+string("{")+pretty_name+string("}");
     resource *rid;
     rid = resource::newResource(this->rid, (void *)thr, nullString, buffer, 
				 timeStamp::ts1970(), "", MDL_T_STRING, true);
     thr->update_rid(rid);
#endif
   }
#endif

#if defined(SHM_SAMPLING) && defined(sparc_sun_sunos4_1_3)
   childUareaPtr = NULL;
#endif

   if (!attach()) {     // moved from ::forkProcess
      showErrorCallback(69, "Error in fork: cannot attach to child process");
      status_ = exited;
      return;
   }

   if( isRunning_() )
           status_ = running;
   else
           status_ = stopped;
   // would neonatal be more appropriate?  Nah, we've reached the first trap
}

// #endif

#ifdef SHM_SAMPLING
void process::registerInferiorAttachedSegs(void *inferiorAttachedAtPtr) {
   shmsample_cerr << "process pid " << getPid() << ": welcome to register with inferiorAttachedAtPtr=" << inferiorAttachedAtPtr << endl;

   inferiorHeapMgr.registerInferiorAttachedAt(inferiorAttachedAtPtr);
   theSuperTable.setBaseAddrInApplic(0,(intCounter*) inferiorHeapMgr.getSubHeapInApplic(0));
   theSuperTable.setBaseAddrInApplic(1,(tTimer*) inferiorHeapMgr.getSubHeapInApplic(1));
   theSuperTable.setBaseAddrInApplic(2,(tTimer*) inferiorHeapMgr.getSubHeapInApplic(2));
#if defined(MT_THREAD)
   // we are now ready to update the thread table for thread 0 - naim
   assert(threads.size()==1 && threads[0]!=NULL);
   getTable().addThread(threads[0]);
#endif
}
#endif


extern bool forkNewProcess(string &file, string dir, vector<string> argv, 
		    vector<string>envp, string inputFile, string outputFile,
		    int &traceLink, int &ioLink, 
		    int &pid, int &tid, 
		    int &procHandle, int &thrHandle,
		    int stdin_fd, int stdout_fd, int stderr_fd);

/*
 * Create a new instance of the named process.  Read the symbols and start
 *   the program
 */
process *createProcess(const string File, vector<string> argv, 
                        vector<string> envp, const string dir = "", 
                        int stdin_fd=0, int stdout_fd=1, int stderr_fd=2)
{
    // prepend the directory (if any) to the file, unless the filename
    // starts with a /
    string file = File;
    if (!file.prefixed_by("/") && dir.length() > 0)
      file = dir + "/" + file;

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
    int ioLink = -1;
    int pid;
    int tid;
    int procHandle;
    int thrHandle;

    if (!forkNewProcess(file, dir, argv, envp, inputFile, outputFile,
		   traceLink, ioLink, pid, tid, procHandle, thrHandle,
		   stdin_fd, stdout_fd, stderr_fd)) {
      // forkNewProcess is responsible for displaying error messages
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
    if (!desc)
      return NULL;

#ifndef BPATCH_LIBRARY
// NEW: We bump up batch mode here; the matching bump-down occurs after shared objects
//      are processed (after receiving the SIGSTOP indicating the end of running
//      DYNINSTinit; more specifically, procStopFromDYNINSTinit().
//      Prevents a diabolical w/w deadlock on solaris --ari
tp->resourceBatchMode(true);
#endif /* BPATCH_LIBRARY */

        image *img = image::parseImage(desc);
        if (!img) {
            // For better error reporting, two failure return values would be
            // useful.  One for simple error like because-file-not-because.
            // Another for serious errors like found-but-parsing-failed 
            //    (internal error; please report to paradyn@cs.wisc.edu)

            string msg = string("Unable to parse image: ") + file;
            showErrorCallback(68, msg.string_of());
            // destroy child process
            OS::osKill(pid);
            return(NULL);
        }

        /* parent */
        statusLine("initializing process data structures");

#ifdef SHM_SAMPLING
        vector<fastInferiorHeapMgr::oneHeapStats> theShmHeapStats(3);
        theShmHeapStats[0].elemNumBytes = sizeof(intCounter);
        theShmHeapStats[0].maxNumElems  = numIntCounters;

        theShmHeapStats[1].elemNumBytes = sizeof(tTimer);
        theShmHeapStats[1].maxNumElems  = numWallTimers;

        theShmHeapStats[2].elemNumBytes = sizeof(tTimer);
        theShmHeapStats[2].maxNumElems  = numProcTimers;
#endif

        process *ret = new process(pid, img, traceLink, ioLink
#ifdef SHM_SAMPLING
                                   , 7000, // shm seg key to try first
                                   theShmHeapStats
#endif
                                   );
           // change this to a ctor that takes in more args

        assert(ret);

        processVec += ret;
        activeProcesses++;

#ifndef BPATCH_LIBRARY
        if (!costMetric::addProcessToAll(ret))
           assert(false);
#endif
        // find the signal handler function
        ret->findSignalHandler(); // should this be in the ctor?

        // initializing vector of threads - thread[0] is really the 
        // same process

#ifndef BPATCH_LIBRARY
#if defined(i386_unknown_nt4_0)
        ret->threads += new pdThread(ret, tid, (handleT)thrHandle);
#else
        ret->threads += new pdThread(ret);
#endif
#endif
        // initializing hash table for threads. This table maps threads to
        // positions in the superTable - naim 4/14/97

        // we use this flag to solve race condition between inferiorRPC and 
        // continueProc message from paradyn - naim
        ret->deferredContinueProc = false;

        ret->numOfActCounters_is=0;
        ret->numOfActProcTimers_is=0;
        ret->numOfActWallTimers_is=0;

#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
        // XXXX - this is a hack since getExecFileDescriptor needed to wait for
        //    the TRAP signal.
        // We really need to move most of the above code (esp parse image)
        //    to the TRAP signal handler.  The problem is that we don't
        //    know the base addresses until we get the load info via ptrace.
        //    In general it is even harder, since dynamic libs can be loaded
        //    at any time.
        extern int handleSigChild(int pid, int status);

        (void) handleSigChild(pid, status);
#endif
    return ret;

}


void process::DYNINSTinitCompletionCallback(process* theProc,
                                            void* userData, // user data
                                            void* /*ret*/ // return value from DYNINSTinit
                                            ) {
   attach_cerr << "Welcome to DYNINSTinitCompletionCallback" << endl;
   if (NULL != userData && 0==strcmp((char*)userData, "viaCreateProcess"))
     theProc->handleCompletionOfDYNINSTinit(false);
   else
     theProc->handleCompletionOfDYNINSTinit(true);
}


bool attachProcess(const string &progpath, int pid, int afterAttach
#ifdef BPATCH_LIBRARY
                   , process *&newProcess
#endif
                   ) {
   // implementation of dynRPC::attach() (the igen call)
   // This is meant to be "the other way" to start a process (competes w/ createProcess)

   // progpath gives the full path name of the executable, which we use ONLY to
   // read the symbol table.

   // We try to make progpath optional, since given pid, we should be able to
   // calculate it with a clever enough search of the process' PATH, examining
   // its argv[0], examining its current directory, etc.  /proc gives us this
   // information on solaris...not sure about other platforms...

   // possible values for afterAttach: 1 --> pause, 2 --> run, 0 --> leave as is

   attach_cerr << "welcome to attachProcess for pid " << pid << endl;

   // QUESTION: When we attach to a process, do we want to redirect its stdout/stderr
   //           (like we do when we fork off a new process the 'usual' way)?
   //           My first guess would be no.  -ari
   //           But although we may ignore the io, we still need the trace stream.

   // When we attach to a process, we don't fork...so this routine is much simpler
   // than its "competitor", createProcess() (above).

   string fullPathToExecutable = process::tryToFindExecutable(progpath, pid);
   if (!fullPathToExecutable.length())
      return false;

#ifndef BPATCH_LIBRARY
   tp->resourceBatchMode(true);
      // matching bump-down occurs in procStopFromDYNINSTinit().
#endif

   int status = pid;
   fileDescriptor *desc = getExecFileDescriptor(fullPathToExecutable,
						status, false);
   if (!desc)
     return false;
   image *theImage = image::parseImage(desc);
   if (theImage == NULL) {
      // two failure return values would be useful here, to differentiate
      // file-not-found vs. catastrophic-parse-error.
      string msg = string("Unable to parse image: ") + fullPathToExecutable;
      showErrorCallback(68, msg.string_of());
      return false; // failure
   }

#ifdef SHM_SAMPLING
   vector<fastInferiorHeapMgr::oneHeapStats> theShmHeapStats(3);
   theShmHeapStats[0].elemNumBytes = sizeof(intCounter);
   theShmHeapStats[0].maxNumElems  = numIntCounters;

   theShmHeapStats[1].elemNumBytes = sizeof(tTimer);
   theShmHeapStats[1].maxNumElems  = numWallTimers;

   theShmHeapStats[2].elemNumBytes = sizeof(tTimer);
   theShmHeapStats[2].maxNumElems  = numProcTimers;
#endif

   // NOTE: the actual attach happens in the process "attach" constructor:
   bool success=false;
   process *theProc = new process(pid, theImage, afterAttach, success
#ifdef SHM_SAMPLING
                                  ,7000, // shm seg key to try first
                                  theShmHeapStats
#endif                            
                                  );
   assert(theProc);
   if (!success) {
       // XXX Do we need to do something to get rid of theImage, too?
       delete theProc;
       return false;
   }

   // Note: it used to be that the attach ctor called pause()...not anymore...so
   // the process is probably running even as we speak.

   processVec += theProc;
   activeProcesses++;

#ifndef BPATCH_LIBRARY
   theProc->threads += new pdThread(theProc);
#endif

#if defined(USES_LIBDYNINSTRT_SO) && !defined(i386_unknown_nt4_0)
   // we now need to dynamically load libdyninstRT.so.1 - naim
   if (!theProc->pause()) {
     logLine("WARNING: pause failed\n");
     assert(0);
   }
   theProc->handleStartProcess();
   if (!theProc->dyninstLibAlreadyLoaded()) {
     /* Ordinarily, dyninstlib has not been loaded yet.  But sometimes
        a zany user links it into their application, leaving no need
        to load it again (in fact, we will probably hang if we try to
        load it again).  This is checked in the call to
        handleStartProcess */
     theProc->dlopenDYNINSTlib();
     // this will set isLoadingDyninstLib to true - naim
     if (!theProc->continueProc()) {
       logLine("WARNING: continueProc failed\n");
       assert(0);
     }
     int status;
     while (!theProc->dyninstLibAlreadyLoaded()) {
       theProc->waitProcs(&status);
     }
   }
#endif

   theProc->initDyninstLib();

#ifndef BPATCH_LIBRARY
   if (!costMetric::addProcessToAll(theProc))
      assert(false);
#endif

   // find the signal handler function
   theProc->findSignalHandler(); // shouldn't this be in the ctor?

   // Now force DYNINSTinit() to be invoked, via inferiorRPC.
   string buffer = string("PID=") + string(pid) + ", running DYNINSTinit()...";
   statusLine(buffer.string_of());

#ifdef BPATCH_LIBRARY
   newProcess = theProc;

   vector<AstNode*> the_args(2);

   the_args[0] = new AstNode(AstNode::Constant, (void*)3);
   the_args[1] = new AstNode(AstNode::Constant, (void*)getpid());
#else /* BPATCH_LIBRARY */
   attach_cerr << "calling DYNINSTinit with args:" << endl;

   vector<AstNode*> the_args(3);

#ifdef SHM_SAMPLING
   the_args[0] = new AstNode(AstNode::Constant,
                             (void*)(theProc->getShmKeyUsed()));
   attach_cerr << theProc->getShmKeyUsed() << endl;

   const unsigned shmHeapTotalNumBytes = theProc->getShmHeapTotalNumBytes();
   the_args[1] = new AstNode(AstNode::Constant,
                             (void*)shmHeapTotalNumBytes);
   attach_cerr << shmHeapTotalNumBytes << endl;;
#else
   // 2 dummy args when not shm sampling -- just make sure they're not both -1, which
   // would indicate that we're called from fork
   the_args[0] = new AstNode(AstNode::Constant, (void*)0);
   the_args[1] = new AstNode(AstNode::Constant, (void*)0);
#endif

   /*
      The third argument to DYNINSTinit is our (paradynd's) pid. It is used
      by DYNINSTinit to build the socket path to which it connects to in order
      to get the trace-stream connection.  We make it negative to indicate
      to DYNINSTinit that it's being called from attach (sorry for that little
      kludge...if we didn't have it, we'd probably need to boost DYNINSTinit
      from 3 to 4 parameters).
      
      This socket is set up in controllerMainLoop (perfStream.C).
   */
   the_args[2] = new AstNode(AstNode::Constant, (void*)(-1 * traceConnectInfo));
   attach_cerr << (-1* getpid()) << endl;
#endif /* BPATCH_LIBRARY */

   AstNode *the_ast = new AstNode("DYNINSTinit", the_args);

   //  Do not call removeAst if Irix MPI, as the initialRequests vector 
   //  is being used more than once.
#ifndef BPATCH_LIBRARY
   if ( !(process::pdFlavor == "mpi" && osName.prefixed_by("IRIX")) )
#endif
      for (unsigned j=0;j<the_args.size();j++) removeAst(the_args[j]);

#if defined(MT_THREAD)
   theProc->postRPCtoDo(the_ast,
                        true, // true --> don't try to update cost yet
                        process::DYNINSTinitCompletionCallback, // callback
                        NULL, // user data
                        -1,   // we use -1 if this is not metric definition
                        -1,
                        false);  // NOT thread specific
#else
   theProc->postRPCtoDo(the_ast,
                        true, // true --> don't try to update cost yet
                        process::DYNINSTinitCompletionCallback, // callback
                        NULL, // user data
                        -1);  // we use -1 if this is not metric definition
#endif
      // the rpc will be launched with a call to launchRPCifAppropriate()
      // in the main loop (perfStream.C).
      // DYNINSTinit() ends with a DYNINSTbreakPoint(), so we pick up
      // where we left off in the processing of the forwarded SIGSTOP signal.
      // In other words, there's lots more work to do, but since we can't do it until
      // DYNINSTinit has run, we wait until the SIGSTOP is forwarded.

   // Note: we used to pause() the process while attaching.  Not anymore.
   // The attached process is running even as we speak.  (Though we'll interrupt
   // it pretty soon when the inferior RPC of DYNINSTinit gets launched).


#if defined(alpha_dec_osf4_0)
   // need to perform this after dyninst Heap is present and happy
   theProc->getDyn()->setMappingHooks(theProc);
#endif

   return true; // successful
}


#ifndef BPATCH_LIBRARY
bool attachToIrixMPIprocess(const string &progpath, int pid, int afterAttach) {

   //  This function has been cannibalized from attachProcess.
   //  IRIX MPI applications appear to present a unique attaching
   //  scenario: We technically "attach" to processes forked by
   //  the master MPI app process/daemon (which is started by
   //  mpirun), but since we haven't passed main yet, we want to
   //  add a breakpoint at main and call DYNINSTinit then.
        
   string fullPathToExecutable = process::tryToFindExecutable(progpath, pid);
   if (!fullPathToExecutable.length())
      return false;

   tp->resourceBatchMode(true);
      // matching bump-down occurs in procStopFromDYNINSTinit().

   int status = pid;
   fileDescriptor *desc = getExecFileDescriptor(fullPathToExecutable,
						status,
						false);

   image *theImage = image::parseImage(desc);
   if (theImage == NULL) {
      // two failure return values would be useful here, to differentiate
      // file-not-found vs. catastrophic-parse-error.
      string msg = string("Unable to parse image: ") + fullPathToExecutable;
      showErrorCallback(68, msg.string_of());
      return false; // failure
   }

#ifdef SHM_SAMPLING
   vector<fastInferiorHeapMgr::oneHeapStats> theShmHeapStats(3);
   theShmHeapStats[0].elemNumBytes = sizeof(intCounter);
   theShmHeapStats[0].maxNumElems  = numIntCounters;

   theShmHeapStats[1].elemNumBytes = sizeof(tTimer);
   theShmHeapStats[1].maxNumElems  = numWallTimers;

   theShmHeapStats[2].elemNumBytes = sizeof(tTimer);
   theShmHeapStats[2].maxNumElems  = numProcTimers;
#endif

   // NOTE: the actual attach happens in the process "attach" constructor:
   bool success=false;
   process *theProc = new process(pid, theImage, afterAttach, success
#ifdef SHM_SAMPLING
                                  ,7000, // shm seg key to try first
                                  theShmHeapStats
#endif                            
                                  );
   assert(theProc);
   if (!success) {
       delete theProc;
       return false;
   }

   processVec += theProc;
   activeProcesses++;

   theProc->threads += new pdThread(theProc);

   if (!costMetric::addProcessToAll(theProc))
      assert(false);

   // find the signal handler function
   theProc->findSignalHandler(); // shouldn't this be in the ctor?

   return true; // successful
}
#endif  // #ifndef BPATCH_LIBRARY


#ifdef SHM_SAMPLING
bool process::doMajorShmSample(timeStamp theWallTime) {
   bool result = true; // will be set to false if any processAll() doesn't complete
                       // successfully.
   if (!theSuperTable.doMajorSample())
      result = false;
      // inferiorProcessTimers used to take in a non-dummy process time as the
      // 2d arg, but it looks like that we need to re-read the process time for
      // each proc timer, at the time of sampling the timer's value, to avoid
      // ugly jagged spikes in histogram (i.e. to avoid incorrect sampled 
      // values).  Come to think of it: the same may have to be done for the 
      // wall time too!!!

   const timeStamp theProcTime = getCpuTime();

   // Now sample the observed cost.
   unsigned *costAddr = (unsigned *)this->getObsCostLowAddrInParadyndSpace();
   const unsigned theCost = *costAddr; // WARNING: shouldn't we be using a mutex?!

   this->processCost(theCost, theWallTime, theProcTime);

   return result;
}

bool process::doMinorShmSample() {
   // Returns true if the minor sample has successfully completed all outstanding
   // samplings.
   bool result = true; // so far...

   if (!theSuperTable.doMinorSample())
      result = false;

   return result;
}
#endif

extern void removeFromMetricInstances(process *);
extern void disableAllInternalMetrics();

void handleProcessExit(process *proc, int exitStatus) {

  proc->exitCode_ = exitStatus;

  if (proc->status() == exited)
    return;

  proc->Exited(); // updates status line

  --activeProcesses;

#ifndef BPATCH_LIBRARY
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
                      dictionary_hash<instInstance*,instInstance*> &map, // gets filled in
                      int iTrace_fd
#ifdef SHM_SAMPLING
                              ,key_t theKey,
                              void *applAttachedPtr
#endif
                              ) {
#ifdef SHM_SAMPLING
    vector<fastInferiorHeapMgr::oneHeapStats> theShmHeapStats(3);
    theShmHeapStats[0].elemNumBytes = sizeof(intCounter);
    theShmHeapStats[0].maxNumElems  = numIntCounters;
    
    theShmHeapStats[1].elemNumBytes = sizeof(tTimer);
    theShmHeapStats[1].maxNumElems  = numWallTimers;

    theShmHeapStats[2].elemNumBytes = sizeof(tTimer);
    theShmHeapStats[2].maxNumElems  = numProcTimers;
#endif

    forkexec_cerr << "paradynd welcome to process::forkProcess; parent pid=" << theParent->getPid() << "; calling fork ctor now" << endl;

    // Call the "fork" ctor:
    process *ret = new process(*theParent, (int)childPid, iTrace_fd
#ifdef SHM_SAMPLING
                               , theKey,
                               applAttachedPtr,
                               theShmHeapStats
#endif
                               );
    assert(ret);

    forkexec_cerr << "paradynd fork ctor has completed ok...child pid is " << ret->getPid() << endl;

    processVec += ret;
    activeProcesses++;

    if (!costMetric::addProcessToAll(ret))
       assert(false);

    // We used to do a ret->attach() here...it was moved to the fork ctor, so it's
    // been done already.

    /* all instrumentation on the parent is active on the child */
    /* TODO: what about instrumentation inserted near the fork time??? */
    ret->baseMap = theParent->baseMap; // WHY IS THIS HERE?

#ifdef BPATCH_LIBRARY
    /* XXX Not sure if this is the right thing to do. */
    ret->instPointMap = theParent->instPointMap;
#endif

    // the following writes to "map", s.t. for each instInstance in the parent
    // process, we have a map to the corresponding one in the child process.
    // that's all this routine does -- it doesn't actually touch
    // any instrumentation (because it doesn't need to -- fork() syscall copied
    // all of the actual instrumentation [but what about AIX and its weird load
    // behavior?])
    copyInstInstances(theParent, ret, map);
         // doesn't copy anything; just writes to "map"

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
  sampleVal_cerr << "processCost- cumObsCost: " << cumObsCost << "\n"; 
  timeLength observedCost(cumObsCost, getCyclesPerSecond());
  timeUnit tu = getCyclesPerSecond(); // just used to print out
  sampleVal_cerr << "processCost: cyclesPerSecond=" << tu
		 << "; cum obs cost=" << observedCost << "\n";
  
  // Notice how most of the rest of this is copied from processCost() of
  // metric.C.  Be sure to keep the two "in sync"!

  extern costMetric *totalPredictedCost; // init.C
  extern costMetric *observed_cost;      // init.C
  
  const timeStamp lastProcessTime = 
    totalPredictedCost->getLastSampleProcessTime(this);
  sampleVal_cerr << "processCost- lastProcessTime: " <<lastProcessTime << "\n";
  // find the portion of uninstrumented time for this interval
  timeLength userPredCost = timeLength::sec() + getCurrentPredictedCost();
  sampleVal_cerr << "processCost- userPredCost: " << userPredCost << "\n";
  const double unInstTime = (processTime - lastProcessTime) / userPredCost; 
  sampleVal_cerr << "processCost- unInstTime: " << unInstTime << "\n";
  // update predicted cost
  // note: currentPredictedCost is the same for all processes
  //       this should be changed to be computed on a per process basis
  pdSample newPredCost = totalPredictedCost->getCumulativeValue(this);
  sampleVal_cerr << "processCost- newPredCost: " << newPredCost << "\n";
  timeLength tempPredCost = getCurrentPredictedCost() * unInstTime;
  sampleVal_cerr << "processCost- tempPredCost: " << tempPredCost << "\n";
  newPredCost += pdSample(tempPredCost.getI(timeUnit::ns()));
  sampleVal_cerr << "processCost- tempPredCost: " << newPredCost << "\n";
  totalPredictedCost->updateValue(this, newPredCost, wallTime, processTime);
  // update observed cost
  observed_cost->updateValue(this,pdSample(observedCost),wallTime,processTime);
}
#endif

/*
 * Copy data from controller process to the named process.
 */
bool process::writeDataSpace(void *inTracedProcess, unsigned size,
                             const void *inSelf) {
  bool needToCont = false;
#ifdef DETACH_ON_THE_FLY
  bool needToDetach = false;
#endif

  if (status_ == exited)
    return false;

  if (status_ == running) {
    needToCont = true;
#ifdef DETACH_ON_THE_FLY
    if (this->haveDetached) {
	 needToDetach = true;
	 this->reattach();
    }
#endif /* DETACH_ON_THE_FLY */
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
#ifdef DETACH_ON_THE_FLY
    if (needToDetach)
	 return this->detachAndContinue();
    else
	 return this->continueProc();
#else
    return this->continueProc();
#endif /* DETACH_ON_THE_FLY */
  }
  return true;
}

bool process::readDataSpace(const void *inTracedProcess, unsigned size,
                            void *inSelf, bool displayErrMsg) {
  bool needToCont = false;
#ifdef DETACH_ON_THE_FLY
  bool needToDetach = false;
#endif

  if (status_ == exited)
    return false;

  if (status_ == running) {
    needToCont = true;
#ifdef DETACH_ON_THE_FLY
    if (this->haveDetached) {
	 needToDetach = true;
	 this->reattach();
    }
#endif /* DETACH_ON_THE_FLY */
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
          "unable to read %d@%s from process data space: %s (pid=%d)",
	  size, Address_str((Address)inTracedProcess), 
          sys_errlist[errno], getPid());
      string msg(errorLine);
      showErrorCallback(38, msg);
    }
    return false;
 }

  if (needToCont) {
#ifdef DETACH_ON_THE_FLY
    if (needToDetach)
	 needToCont = this->detachAndContinue();
    else
	 needToCont = this->continueProc();
#else
    needToCont = this->continueProc();
#endif /* DETACH_ON_THE_FLY */
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
#ifdef DETACH_ON_THE_FLY
  bool needToDetach = false;
#endif /* DETACH_ON_THE_FLY */

  if (status_ == exited)
    return false;

  if (status_ == running) {
    needToCont = true;
#ifdef DETACH_ON_THE_FLY
    if (this->haveDetached) {
	 needToDetach = true;
	 this->reattach();
    }
#endif /* DETACH_ON_THE_FLY */
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
#ifdef DETACH_ON_THE_FLY
    if (needToDetach)
	 return this->detachAndContinue();
    else
	 return this->continueProc();
#else
    return this->continueProc();
#endif /* DETACH_ON_THE_FLY */
  }
  return true;

}

bool process::writeTextSpace(void *inTracedProcess, u_int amount, 
                             const void *inSelf) {
  bool needToCont = false;
#ifdef DETACH_ON_THE_FLY
  bool needToDetach = false;
#endif /* DETACH_ON_THE_FLY */

  if (status_ == exited)
    return false;

  if (status_ == running) {
    needToCont = true;
#ifdef DETACH_ON_THE_FLY
    if (this->haveDetached) {
	 needToDetach = true;
	 this->reattach();
    }
#endif /* DETACH_ON_THE_FLY */
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
#ifdef DETACH_ON_THE_FLY
    if (needToDetach) 
	 return this->detachAndContinue();
    else
	 return this->continueProc();
#else
    return this->continueProc();
#endif /* DETACH_ON_THE_FLY */
  }
  return true;
}

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
bool process::readTextSpace(const void *inTracedProcess, u_int amount,
                            const void *inSelf)
{
  bool needToCont = false;
#ifdef DETACH_ON_THE_FLY
  bool needToDetach = false;
#endif /* DETACH_ON_THE_FLY */

  if (status_ == exited)
    return false;

  if (status_ == running) {
    needToCont = true;
#ifdef DETACH_ON_THE_FLY
    if (this->haveDetached) {
	 needToDetach = true;
	 this->reattach();
    }
#endif
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
          "unable to read %d@%s from process text space: %s (pid=%d)",
	  amount, Address_str((Address)inTracedProcess),
          sys_errlist[errno], getPid());
    string msg(errorLine);
    showErrorCallback(38, msg);
    return false;
  }

  if (needToCont) {
#ifdef DETACH_ON_THE_FLY
    if (needToDetach)
	 return this->detachAndContinue();
    else
	 return this->continueProc();
#else
    return this->continueProc();
#endif /* DETACH_ON_THE_FLY */
  }
  return true;

}
#endif /* BPATCH_SET_MUTATIONS_ACTIVE */

bool process::pause() {
  if (status_ == stopped || status_ == neonatal)
    return true;

  if (status_ == exited) {
    sprintf(errorLine, "warn : in process::pause, trying to pause exited process, returning FALSE\n");
    logLine(errorLine);
    return false;
  }

  if (status_ == running && reachedFirstBreak) {
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

  vector<shared_object *> *changed_objects = 0;
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
#if !defined(BPATCH_LIBRARY) && !defined(rs6000_ibm_aix4_1)
	    if (((*changed_objects)[i])->getName() == dyninstName)
	    {
#endif
               if(addASharedObject(*((*changed_objects)[i]))){
                 *shared_objects += (*changed_objects)[i];
		 if (((*changed_objects)[i])->getName() == dyninstName) {
		   hasLoadedDyninstLib = true;
		   isLoadingDyninstLib = false;
		 }
               } else {
                 //logLine("Error after call to addASharedObject\n");
                 delete (*changed_objects)[i];
               }
/* // not used
#ifdef MT_THREAD // ALERT: the following is thread package specific
               bool err ;
               if (!DYNINST_allthreads_p) {
                 DYNINST_allthreads_p = findInternalAddress("DYNINST_allthreads_p",true,err) ;
               }
               if (!allthreads) {
                 allthreads = findInternalAddress("_allthreads",true,err);
               }

               if (allthreads && DYNINST_allthreads_p) {
                  if (!writeDataSpace((caddr_t) DYNINST_allthreads_p, sizeof(void*), (caddr_t) &allthreads) ) {
                    cerr << "write _allthreads failed! " << endl ;
                  }
                  cerr << "DYNINST_allthreads_p=" << DYNINST_allthreads_p
                       << ", allthreads=" << allthreads << endl ;
               }
#endif//MT_THREAD
*/
#if !defined(BPATCH_LIBRARY) && !defined(rs6000_ibm_aix4_1)
	       } else {
               // for now, just delete shared_objects to avoid memory leeks
               delete (*changed_objects)[i];
	       }
#endif
          }
          delete changed_objects;
      } 
      else if((change_type == SHAREDOBJECT_REMOVED) && (changed_objects)) { 
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





//


//
//  If this process is a dynamic executable, then get all its 
//  shared objects, parse them, and define new instpoints and resources 
//
bool process::handleStartProcess(){

    // get shared objects, parse them, and define new resources 
    // For WindowsNT we don't call getSharedObjects here, instead
    // addASharedObject will be called directly by pdwinnt.C
#if !defined(i386_unknown_nt4_0)
    this->getSharedObjects();
#endif

#ifndef BPATCH_LIBRARY
    statusLine("building process call graph");
    this->FillInCallGraphStatic();
    if(resource::num_outstanding_creates)
       this->setWaitingForResources();
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

     name = (so->getName()).string_of();

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
bool process::addASharedObject(shared_object &new_obj){
    int ret;
    string msg;
    // FIXME

    image *img = image::parseImage(new_obj.getFileDesc());

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

#if defined(USES_LIBDYNINSTRT_SO) && !defined(i386_unknown_nt4_0)
    /* If we're not currently trying to load the runtime library,
       check whether this shared object is the runtime lib. */
    if (!isLoadingDyninstLib
	&& (ret = check_rtinst(this, &new_obj))) {
	 if (ret == 1) {
	      /* The runtime library has been loaded, but not initialized.
		 Proceed anyway. */
	      msg = string("Application was linked with Dyninst/Paradyn runtime library -- this is not necessary");
	      statusLine(msg.string_of());
	      this->hasLoadedDyninstLib = 1;
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
        *all_modules += *((const vector<module *> *)(new_obj.getModules())); 
    }
    if(all_functions){
      vector<function_base *> *normal_funcs = (vector<function_base *> *)
                const_cast< vector<pd_Function *> *>(new_obj.getAllFunctions());
        *all_functions += *normal_funcs; 
        normal_funcs = 0;
    }

    // if the signal handler function has not yet been found search for it
    if(!signal_handler){
        signal_handler = img->findOneFunction(SIGNAL_HANDLER);
    }

    // clear the include_funcs flag if this shared object should not be
    // included in the some_functions and some_modules lists
#ifndef BPATCH_LIBRARY
    vector<string> lib_constraints;
    if(mdl_get_lib_constraints(lib_constraints)){
        for(u_int j=0; j < lib_constraints.size(); j++){
           char *where = 0; 
           // if the lib constraint is not of the form "module/function" and
           // if it is contained in the name of this object, then exclude
           // this shared object
           char *obj_name = P_strdup(new_obj.getName().string_of());
           char *lib_name = P_strdup(lib_constraints[j].string_of());
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
            *some_modules += *((const vector<module *> *)(new_obj.getModules()));
        }
        if(some_functions) {
            // gets only functions not excluded by mdl "exclude_node" option
            *some_functions += 
                *((vector<function_base *> *)(new_obj.getSomeFunctions()));
        }
    }
#endif /* BPATCH_LIBRARY */


#ifdef BPATCH_LIBRARY

    assert(BPatch::bpatch);
    const vector<pdmodule *> *modlist = new_obj.getModules();
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
	    //string temp2 = string(j);
	    //temp2 += string(" ");
	    //temp2 += string("the shared obj, addr: ");
	    //temp2 += string(((*shared_objects)[j])->getBaseAddress());
	    //temp2 += string(" name: ");
	    //temp2 += string(((*shared_objects)[j])->getName());
	    //temp2 += string("\n");
	    //logLine(P_strdup(temp2.string_of()));
	    if(!addASharedObject(*((*shared_objects)[j]))){
	      //logLine("Error after call to addASharedObject\n");
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
function_base *process::findOneFunction(resource *func,resource *mod){

    if((!func) || (!mod)) { return 0; }
    if(func->type() != MDL_T_PROCEDURE) { return 0; }
    if(mod->type() != MDL_T_MODULE) { return 0; }

    const vector<string> &f_names = func->names();
    const vector<string> &m_names = mod->names();
    string func_name = f_names[f_names.size() -1]; 
    string mod_name = m_names[m_names.size() -1]; 
    
    //cerr << "process::findOneFunction called.  function name = " 
    //   << func_name.string_of() << endl;
    
    // KLUDGE: first search any shared libraries for the module name 
    //  (there is only one module in each shared library, and that 
    //  is the library name)
    if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
            module *next = 0;
            next = ((*shared_objects)[j])->findModule(mod_name,true);
            if(next){
                if(((*shared_objects)[j])->includeFunctions()){ 
                  //cerr << "function found in module " << mod_name.string_of() << endl;
                    return(((*shared_objects)[j])->findOneFunction(func_name,
                                                                   true));
                } 
                else { 
                  //cerr << "function found in module " << mod_name.string_of()
                  //    << " that module excluded" << endl;
                  return 0;
                } 
            }
        }
    }

    return(symbols->findOneFunction(func_name));
}
#endif /* BPATCH_LIBRARY */

#ifndef BPATCH_LIBRARY
// returns all the functions in the module "mod" that are not excluded by
// exclude_lib or exclude_func
// return 0 on error.
vector<function_base *> *process::getIncludedFunctions(module *mod) {

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
                    return((vector<function_base *> *)
                           ((*shared_objects)[j])->getSomeFunctions());
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


// findOneFunction: returns the function associated with func  
// this routine checks both the a.out image and any shared object
// images for this resource
function_base *process::findOneFunction(const string &func_name){
    // first check a.out for function symbol
    function_base *pdf = symbols->findOneFunction(func_name);
    if(pdf) {
      return pdf;
    }

    // search any shared libraries for the file name 
    if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
            pdf = ((*shared_objects)[j])->findOneFunction(func_name,false);
            if(pdf){
                return(pdf);
            }
    } }
    return(0);
}

// findOneFunctionFromAll: returns the function associated with func  
// this routine checks both the a.out image and any shared object
// images for this resource
function_base *process::findOneFunctionFromAll(const string &func_name){
    
    // first check a.out for function symbol
    function_base *pdf = symbols->findOneFunctionFromAll(func_name);
    if(pdf) return pdf;

    // search any shared libraries for the file name 
    if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
            pdf=((*shared_objects)[j])->findOneFunctionFromAll(func_name,false);
            if(pdf){
                return(pdf);
            }
    } }
    return(0);
}

// Returns the named symbol from the image or a shared object
bool process::getSymbolInfo( const string &name, Symbol &ret ) 
{
        if(!symbols)
                abort();

        bool sflag;
        sflag = symbols->symbol_info( name, ret );
        if( sflag )
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

// findpdFunctionIn: returns the function which contains this address
// This routine checks both the a.out image and any shared object images
// for this function
pd_Function *process::findpdFunctionIn(Address adr) {

    // first check a.out for function symbol
    // findFunctionInInstAndUnInst will search the instrumentable and
    // uninstrumentable functions. We are assuming that "adr" is the
    // entry point of the function, so we will use as the key to search
    // in the dictionary - naim
    pd_Function *pdf = symbols->findFunctionInInstAndUnInst(adr,this);
    if(pdf) return pdf;
    // search any shared libraries for the function 
    if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
          pdf = ((*shared_objects)[j])->findFunctionInInstAndUnInst(adr,this);
          if(pdf){
            return(pdf);          
          }
        }
    }

    if(!all_functions) getAllFunctions();

    // if the function was not found, then see if this addr corresponds
    // to  a function that was relocated in the heap
    if(all_functions){
        for(u_int j=0; j < all_functions->size(); j++){
            Address func_adr = ((*all_functions)[j])->getAddress(this);
            if((adr>=func_adr) && 
                (adr<=(((*all_functions)[j])->size()+func_adr))){
                // yes, this is very bad, but too bad
                return((pd_Function*)((*all_functions)[j]));
            }
        }
    }
    return(0);
}
    

// findFunctionIn: returns the function containing the address "adr"
// this routine checks both the a.out image and any shared object
// images for this resource
function_base *process::findFunctionIn(Address adr){
    // first check a.out for function symbol
    pd_Function *pdf = symbols->findFunctionIn(adr,this);
    if(pdf) return pdf;
    // search any shared libraries for the function 
    if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
            pdf = ((*shared_objects)[j])->findFunctionIn(adr,this);
            if(pdf){
                return(pdf);
            }
    } }

    if(!all_functions) getAllFunctions();

    // if the function was not found, then see if this addr corresponds
    // to  a function that was relocated in the heap
    if(all_functions){
        for(u_int j=0; j < all_functions->size(); j++){
            Address func_adr = ((*all_functions)[j])->getAddress(this);
            if((adr>=func_adr) && 
                (adr<=(((*all_functions)[j])->size()+func_adr))){
                return((*all_functions)[j]);
            }
        }
    }
    return(0);
}
        

        
// findModule: returns the module associated with mod_name 
// this routine checks both the a.out image and any shared object
// images for this resource
// if check_excluded is true it checks to see if the module is excluded
// and if it is it returns 0.  If check_excluded is false it doesn't check
module *process::findModule(const string &mod_name,bool check_excluded){

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
    return(symbols->findModule(mod_name));
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
                return getBaseAddress(((*shared_objects)[j])->getImage(), baseAddr); 
            }
      }
    }

    return false;
}


// getAllFunctions: returns a vector of all functions defined in the
// a.out and in the shared objects
// TODO: what to do about duplicate function names?
vector<function_base *> *process::getAllFunctions(){

    // if this list has already been created, return it
    if(all_functions) 
        return all_functions;

    // else create the list of all functions
    all_functions = new vector<function_base *>;
    const vector<function_base *> &blah = 
                    (vector<function_base *> &)(symbols->getAllFunctions());
    *all_functions += blah;

    if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
           vector<function_base *> *funcs = (vector<function_base *> *) 
                        const_cast< vector<pd_Function *> *>
                        (((*shared_objects)[j])->getAllFunctions());
           if(funcs){
               *all_functions += *funcs; 
           }
        }
    }
    return all_functions;
}
      
// getAllModules: returns a vector of all modules defined in the
// a.out and in the shared objects
// Includes "excluded" modules....
vector<module *> *process::getAllModules(){

    // if the list of all modules has already been created, the return it
    if(all_modules) return all_modules;

    // else create the list of all modules
    all_modules = new vector<module *>;
    *all_modules += *((const vector<module *> *)(&(symbols->getAllModules())));

    if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
           const vector<module *> *mods = (const vector<module *> *)
                        (((*shared_objects)[j])->getModules());
           if(mods) {
               *all_modules += *mods; 
           }
    } } 
    return all_modules;
}

#ifndef BPATCH_LIBRARY
// getIncludedFunctions: returns a vector of all functions defined in the
// a.out and in the shared objects
// TODO: what to do about duplicate function names?
vector<function_base *> *process::getIncludedFunctions(){
    //cerr << "process " << programName << " :: getIncludedFunctions() called"
    //   << endl;
    // if this list has already been created, return it
    if(some_functions) 
        return some_functions;

    // else create the list of all functions
    some_functions = new vector<function_base *>;
    const vector<function_base *> &incl_funcs = 
        (vector<function_base *> &)(symbols->getIncludedFunctions());
        *some_functions += incl_funcs;

    //cerr << " (process::getIncludedFunctions), about to add incl_funcs to some_functions, incl_funcs = " << endl;
    //print_func_vector_by_pretty_name(string(">>>"), &incl_funcs);

    if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
            if(((*shared_objects)[j])->includeFunctions()){
                // kludge: can't assign a vector<derived_class *> to 
                // a vector<base_class *> so recast
                vector<function_base *> *funcs = (vector<function_base *> *)
                        (((*shared_objects)[j])->getSomeFunctions());
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
vector<module *> *process::getIncludedModules(){

    //cerr << "process::getIncludedModules called" << endl;

    // if the list of all modules has already been created, the return it
    if(some_modules) {
        //cerr << "some_modules already created, returning it:" << endl;
        //print_module_vector_by_short_name(string("  "), (vector<pdmodule*>*)some_modules);
        return some_modules;
    }

    // else create the list of all modules
    some_modules = new vector<module *>;
    *some_modules += *((const vector<module *> *)(&(symbols->getIncludedModules())));

    if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
            if(((*shared_objects)[j])->includeFunctions()){
               const vector<module *> *mods = (const vector<module *> *) 
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

// findSignalHandler: if signal_handler is 0, then it checks all images
// associated with this process for the signal handler function.
// Otherwise, the signal handler function has already been found
void process::findSignalHandler(){

    if(SIGNAL_HANDLER == 0) return;
    if(!signal_handler) { 
        // first check a.out for signal handler function
        signal_handler = symbols->findOneFunction(SIGNAL_HANDLER);

        // search any shared libraries for signal handler function
        if(!signal_handler && dynamiclinking && shared_objects) { 
            for(u_int j=0;(j < shared_objects->size()) && !signal_handler; j++){
                signal_handler = 
                      ((*shared_objects)[j])->findOneFunction(SIGNAL_HANDLER,false);
        } }
        signal_cerr << "process::findSignalHandler <" << SIGNAL_HANDLER << ">";
        if (!signal_handler) signal_cerr << " NOT";
        signal_cerr << " found." << endl;
    }
}


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
        ret_sym = internalSym(baseAddr+sym.addr(), name);
        return true;
     }

     if (warn) {
        string msg;
        msg = string("Unable to find symbol: ") + name;
        statusLine(msg.string_of());
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
#if defined(USES_LIBDYNINSTRT_SO) \
&& !defined(i386_unknown_linux2_0) \
&& !defined(alpha_dec_osf4_0) \
&& !defined(i386_unknown_nt4_0)
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
        return sym.addr()+baseAddr;
     }
     if (warn) {
        string msg;
        msg = string("Unable to find symbol: ") + name;
        statusLine(msg.string_of());
        showErrorCallback(28, msg);
     }
     err = true;
     return 0;
}



bool process::continueProc() {
  if (status_ == exited) return false;

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

   // since the exec syscall has run, we're not ready to enable any m/f pairs or
   // sample anything.
   // So we set hasBootstrapped to false until we run DYNINSTinit again.
   hasBootstrapped = false;

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
   signal_handler = 0;
#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0) \
 || defined(i386_unknown_nt4_0)
   trampTableItems = 0;
   memset(trampTable, 0, sizeof(trampTable));
#endif
   baseMap.clear();
#ifdef BPATCH_LIBRARY
   instPointMap.clear(); /* XXX Should delete instPoints first? */
   PDFuncToBPFuncMap.clear(),
#endif
     cleanInstFromActivePoints(this);
   
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
       showErrorCallback(68, msg.string_of());
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
#if !defined(USES_LIBDYNINSTRT_SO) || defined(i386_unknown_nt4_0)
    initInferiorHeap();
#endif

    /* update process status */
    reachedFirstBreak = false;
       // we haven't yet seen initial SIGTRAP for this proc (is this right?)
    reachedVeryFirstTrap = false;

    status_ = stopped; // was 'exited'

   // TODO: We should remove (code) items from the where axis, if the exec'd process
   // was the only one who had them.

   // the exec'd process has the same fd's as the pre-exec, so we don't need
   // to re-initialize traceLink or ioLink (is this right???)

   // we don't need to re-attach after an exec (is this right???)
 
#ifdef SHM_SAMPLING
   inferiorHeapMgr.handleExec();
      // reuses the shm seg (paradynd's already attached to it); resets applic-attached-
      // at to NULL.  Quite similar to the (non-fork) ctor, really.

   theSuperTable.handleExec();
#endif

   // this is a new process, so we have to invalidate
   // the address of the guard flag
   trampGuardFlagAddr = 0;

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
    Frame frame(this);
    Address pc = frame.getPC();

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

void process::cleanRPCreadyToLaunch(int mid) 
{
  vectorSet<inferiorRPCtoDo> tmpRPCsWaitingToStart;
  while (RPCsWaitingToStart.size() > 0) {
    inferiorRPCtoDo currElem = RPCsWaitingToStart.removeOne();
    if (currElem.mid == mid)
      break;
    else
      tmpRPCsWaitingToStart += currElem;
  }
  // reconstruct RPCsWaitingToStart
  while (tmpRPCsWaitingToStart.size() > 0) {
    RPCsWaitingToStart += tmpRPCsWaitingToStart.removeOne();
  }
}

#if defined(MT_THREAD)
void process::postRPCtoDo(AstNode *action, bool noCost,
                          inferiorRPCcallbackFunc callbackFunc,
                          void *userData,
                          int mid, 
                          int thrId,
                          bool isSAFE,
                          bool lowmem)
#else
void process::postRPCtoDo(AstNode *action, bool noCost,
                          inferiorRPCcallbackFunc callbackFunc,
                          void *userData,
                          int mid,
                          bool lowmem)
#endif
{
   // posts an RPC, but does NOT make any effort to launch it.
   inferiorRPCtoDo theStruct;
   theStruct.action = action;
   theStruct.noCost = noCost;
   theStruct.callbackFunc = callbackFunc;
   theStruct.userData = userData;
   theStruct.mid = mid;
   theStruct.lowmem = lowmem;

#if defined(MT_THREAD)
   theStruct.thrId = thrId;
   theStruct.isSafeRPC = isSAFE;
   if (pd_debug_infrpc)
     cerr <<"posting "<<(isSAFE? "SAFE":"")<<"inferiorRPC for tid"<<thrId<<endl;
#endif
   RPCsWaitingToStart += theStruct;
   
   // PARADYND_DEBUG_INFRPC
   //
   if ( pd_debug_infrpc )
     cerr << "postRPCtoDo is called" << endl;
}

bool process::existsRPCreadyToLaunch() const {
   if (currRunningRPCs.empty() && !RPCsWaitingToStart.empty())
      return true;
   return false;
}

bool process::existsRPCinProgress() const {
#if defined(MT_THREAD)
   if (!currRunningRPCs.empty()) {
     for (unsigned k=0; k< currRunningRPCs.size(); k++)
       if ( !currRunningRPCs[k].isSafeRPC ) 
         return true ;
   }
   return false ;
#else
   return (!currRunningRPCs.empty());
#endif
}

#if defined(MT_THREAD)
bool process::need_to_wait(void) {
  //
  // Check if there is any slot left for safe RPC
  //
  if (!currRunningRPCs.empty()){
    for (unsigned k=0; k< currRunningRPCs.size(); k++) 
      if (!currRunningRPCs[k].isSafeRPC) {
        return true ;
      } 
       else {
        if (DYNINSTthreadRPC) {
          unsigned freeSlot = 0 ;
          for (unsigned j=0; j<MAX_PENDING_RPC; j++) {
            if (DYNINSTthreadRPC[j].flag ==0)
              freeSlot ++ ;
          }
          if (freeSlot == 0)
            return true ;
        } 
      }//else
  }
  return false ;
}

#if defined(USES_RPC_TO_TRIGGER_RPC)
typedef struct {
  bool ready;
  void *result;
} sig_rpc_ret;

void signalRPCthreadCallback(process * /*p*/, void *data, void *result)
{
  sig_rpc_ret *ret = (sig_rpc_ret *)data;
  ret->result = result;
  ret->ready = true;
}
#endif

void signalRPCthread(process *p) {
  mutex_lock(p->DYNINSTthreadRPC_mp);
  *(p->DYNINSTthreadRPC_pending_p) = 1;
  cond_signal(p->DYNINSTthreadRPC_cvp);
  mutex_unlock(p->DYNINSTthreadRPC_mp);

#if defined(USES_RPC_TO_TRIGGER_RPC) // Temporary here
  string callee = "DYNINSTsignalRPCthread";
  vector<AstNode*> args(0);
  AstNode *code = new AstNode(callee, args);

  sig_rpc_ret ret = {false, NULL};
  p->postRPCtoDo(code, true, &signalRPCthreadCallback, &ret, -1, -1, false);
  extern  void checkProcStatus();
  do {
   p->launchRPCifAppropriate(true, false);
   checkProcStatus();
  } while (!ret.ready);
#endif

  if (pd_debug_infrpc)
    cerr <<"PD: Signaled RPC thread ..." << endl ;
}
#endif 

//
// this should work for both threaded and non-threaded case
//
bool process::launchRPCifAppropriate(bool wasRunning, bool finishingSysCall) {
   // asynchronously launches an inferiorRPC iff RPCsWaitingToStart.size() > 0
   // AND if currRunningRPCs.size()==0 (the latter for safety)

#if defined(MT_THREAD)
   handleDoneSAFEinferiorRPC();
   if (need_to_wait())
#else
   if (!currRunningRPCs.empty())
#endif
     {
       // an RPC is currently executing, so it's not safe to launch a new one.
       inferiorrpc_cerr << "RPC currently executing!" << endl;
       return false;
     }

   if (RPCsWaitingToStart.empty())
     {
       // duh, no RPC is waiting to run, so there's nothing to do.
       inferiorrpc_cerr << "No RPC to execute!" << endl;
       return false;
     }

   if (status_ == exited)
     {
       inferiorrpc_cerr << "Inferior process exited!" << endl;
       return false;
     }

   if (status_ == neonatal)
      // not sure if this should be some kind of error...is the inferior ready
      // to execute inferior RPCs??? For now, we'll allow it.
      ; 


   // Do not remove it yet
   inferiorRPCtoDo todo = RPCsWaitingToStart[0] ;
   /* ****************************************************** */

   // Steps to take (on sparc, at least)
   // 1) pause the process and wait for it to stop
   // 2) Get a copy of the registers...store them away
   // 3) create temp trampoline: save, action, restore, trap, illegal
   //    (the illegal is just ot be sure that the trap never returns)
   // 4) set the PC and nPC regs to addr of temp tramp   || NB: not for SAFErpc!
   // 5) Continue the process...go back to the main loop (SIGTRAP will 
   //    eventually be delivered)

   // When SIGTRAP is received,
   // 5) verify that PC is the location of the TRAP instr in the temp tramp
   // 6) free temp trampoline
   // 7) SETREGS to restore all regs, including PC and nPC.
   // 8) continue inferior, if the inferior had been running when we had
   //    paused it in step 1, above.

#if defined(MT_THREAD)
   if (!todo.isSafeRPC) //only wait for syscall to finish if the rpc is SAFErpc
#endif
     if (!finishingSysCall && RPCs_waiting_for_syscall_to_complete) {
#ifndef i386_unknown_linux2_0
#ifndef MT_THREAD
            assert(executingSystemCall());
#endif
#endif
        return false;
     }

   if (!pause()) {
      cerr << "launchRPCifAppropriate failed because pause failed" << endl;
      return false;
   }

   void *theSavedRegs ;
   // If we're in the middle of a system call, then it's not safe to launch
   // an inferiorRPC on most platforms.  On some platforms, it's safe, but the
   // actual launching won't take place until the system call finishes.  In such
   // cases it's a good idea to set a breakpoint for when the sys call exits
   // (using /proc PIOCSEXIT), and launch the inferiorRPC then.
#if defined(MT_THREAD)
   if (!todo.isSafeRPC) {
#endif
     if (!finishingSysCall && executingSystemCall()) {
        if (RPCs_waiting_for_syscall_to_complete) {
           inferiorrpc_cerr << "launchRPCifAppropriate: "
                            << "still waiting for syscall to complete" << endl;
           if (wasRunning) {
              inferiorrpc_cerr << "launchRPC: "
                               << "continuing so syscall may complete" << endl;
              (void)continueProc();
           }
           else
              inferiorrpc_cerr << "launchRPC: sorry not continuing (problem?)"
                               << endl;
         return false;
        }

        // don't do the inferior rpc until the system call finishes.  Determine
        // which system call is in the way, and set a breakpoint at its exit
        // point so we know when it's safe to launch the RPC.  Platform-specific
        // details:

        inferiorrpc_cerr << "launchRPCifAppropriate: within a system call"
                        << endl;

      if (!set_breakpoint_for_syscall_completion()) {
         // sorry, this platform can't set a breakpoint at the system call
         // completion point.  In such cases, keep polling executingSystemCall()
         // inefficiently.
	   if (wasRunning)
            (void)continueProc();

          inferiorrpc_cerr << "launchRPCifAppropriate: couldn't set bkpt for "
                          << "syscall completion; will just poll." << endl;
          return false;
       }

       inferiorrpc_cerr << "launchRPCifAppropriate: "
                        << "set bkpt for syscall completion" << endl;

       // a SIGTRAP will get delivered when the RPC is ready to go.  Until then,
       // mark the rpc as deferred.  Setting this flag prevents executing
       // this code too many times.

      RPCs_waiting_for_syscall_to_complete = true;
          was_running_before_RPC_syscall_complete = wasRunning;

      //if (wasRunning)
          (void)continueProc();

      return false;
   }

   if (finishingSysCall)
           clear_breakpoint_for_syscall_completion();

   // We're not in the middle of a system call, so we can fire off the rpc now!
   if (RPCs_waiting_for_syscall_to_complete)
      // not any more
      RPCs_waiting_for_syscall_to_complete = false;

      theSavedRegs = getRegisters(); // machine-specific implementation
      // result is allocated via new[]; we'll delete[] it later.
      // return value of NULL indicates total failure.
      // return value of (void *)-1 indicates that the state of the machine
      //    isn't quite ready for an inferiorRPC, and that we should try again
      //    'later'.  In particular, we must handle the (void *)-1 case very
      //     gracefully (i.e., leave the vrble 'RPCsWaitingToStart' untouched).

     if (theSavedRegs == (void *)-1) {
       inferiorrpc_cerr << "launchRPCifAppropriate: deferring" << endl;
       if (wasRunning)
         (void)continueProc();
       return false;
     }

     if (theSavedRegs == NULL) {
       cerr << "launchRPCifAppropriate failed because getRegisters() failed"
            << endl;
       if (wasRunning)
         (void)continueProc();
       return false;
     }
#if defined(MT_THREAD)
   }// if (!isSafeRPC)
#endif

   if (!wasRunning)
     inferiorrpc_cerr << "NOTE: launchIfAppropriate: wasRunning==false!!"
                      << endl;

   // Now it is safe to remove the first from the vector
   RPCsWaitingToStart.removeOne();
   // note: this line should always be below the test for (void*)-1, thus
   // leaving 'RPCsWaitingToStart' alone in that case.

   inferiorRPCinProgress inProgStruct;
   inProgStruct.callbackFunc = todo.callbackFunc;
   inProgStruct.userData = todo.userData;
   inProgStruct.savedRegs = theSavedRegs;
#if defined(MT_THREAD)
   inProgStruct.isSafeRPC = todo.isSafeRPC;
   if (todo.isSafeRPC) {
      inProgStruct.wasRunning = wasRunning ;
   } else 
#endif
   if( finishingSysCall )
           inProgStruct.wasRunning = was_running_before_RPC_syscall_complete;
   else
           inProgStruct.wasRunning = wasRunning;



      // If finishing up a system call, current state is paused, but we want to
      // set wasRunning to true so that it'll continue when the inferiorRPC
      // completes.  Sorry for the kludge.
#if defined(MT_THREAD)
   Address tempTrampBase = createRPCtempTramp(todo.action,
                               todo.noCost,
                               inProgStruct.callbackFunc != NULL,
                               inProgStruct.breakAddr,
                               inProgStruct.stopForResultAddr,
                               inProgStruct.justAfter_stopForResultAddr,
                               inProgStruct.resultRegister,
                               todo.lowmem,
                               todo.thrId,
                               todo.isSafeRPC);
#else
   Address tempTrampBase = createRPCtempTramp(todo.action,
                               todo.noCost,
                               inProgStruct.callbackFunc != NULL,
                               inProgStruct.breakAddr,
                               inProgStruct.stopForResultAddr,
                               inProgStruct.justAfter_stopForResultAddr,
                               inProgStruct.resultRegister,
                               todo.lowmem);
#endif
      // the last 4 args are written to

   if (tempTrampBase == 0) {
      cerr << "launchRPCifAppropriate failed because createRPCtempTramp failed"
           << endl;
      if (wasRunning)
         (void)continueProc();
      return false;
   }
   assert(tempTrampBase);

   inProgStruct.firstInstrAddr = tempTrampBase;

   currRunningRPCs += inProgStruct;

#ifndef i386_unknown_nt4_0
   Address curPC = currentPC();
#endif

   if (pd_debug_infrpc)
     inferiorrpc_cerr << "Changing pc (" << (void*)tempTrampBase << ") and exec.." << endl;

   // change the PC and nPC registers to the addr of the temp tramp
#if defined(MT_THREAD)
   if (!todo.isSafeRPC)
#endif
     if (!changePC(tempTrampBase, theSavedRegs)) {
        cerr << "launchRPCifAppropriate failed because changePC() failed" << endl;
        if (wasRunning)
          (void)continueProc();
        return false;
     }
   
   /* Flag in app is set when running irpc.  Used by app to ignore traps
      which are delivered before or during the irpc.  Also set beginning
      and ending addresses of irpc.

      Don't need to worry about hasNewPC option within currentPC when
      setting pcAtLastIRPC because this (hasNewPC) occurs only when
      relocating instructions to the basetrampoline.  We only use the
      pcAtLastIRPC address only for validating the address of
      regenerated traps are from the correct trap address.  However,
      instrumentation traps aren't relocated, since they're what
      replaces the relocated instructions. */

#ifndef i386_unknown_nt4_0
   if (pd_debug_infrpc)
     inferiorrpc_cerr << "setting the running RPC flag.\n";
   SendAppIRPCInfo(1, inProgStruct.firstInstrAddr, inProgStruct.breakAddr);
   SendAppIRPCInfo(curPC);
#endif

   if (!continueProc()) {
     cerr << "launchRPCifAppropriate: continueProc() failed" << endl;
     return false;
   }

#if defined(MT_THREAD)
   if (todo.isSafeRPC) {
     cerr << "SAFE inferiorRPC should be running now" << endl;
   } else {
     cerr << "inferiorRPC should be running now" << endl;
   }
#endif

   if (pd_debug_infrpc)
     inferiorrpc_cerr << "inferiorRPC should be running now" << endl;

#if defined(MT_THREAD)
   if (todo.isSafeRPC) { signalRPCthread(this); }
#endif

   return true; // success
}

#if defined(MT_THREAD)
Address process::createRPCtempTramp(AstNode *action,
                                     bool noCost,
                                     bool shouldStopForResult,
                                     Address &breakAddr,
                                     Address &stopForResultAddr,
                                     Address &justAfter_stopForResultAddr,
                                     Register &resultReg,
                                     bool lowmem,
                                     int thrId,
                                     bool isSAFE)
#else
Address process::createRPCtempTramp(AstNode *action,
                                     bool noCost,
                                     bool shouldStopForResult,
                                     Address &breakAddr,
                                     Address &stopForResultAddr,
                                     Address &justAfter_stopForResultAddr,
                                     Register &resultReg,
                                     bool lowmem)
#endif
{
   // Returns addr of temp tramp, which was allocated in the inferior heap.
   // You must free it yourself when done.

   // Note how this is, in many ways, a greatly simplified version of
   // addInstFunc().

   // Temp tramp structure: save; code; restore; trap; illegal
   // the illegal is just to make sure that the trap never returns
   // note that we may not need to save and restore anything, since we've
   // already done a GETREGS and we'll restore with a SETREGS, right?

   unsigned char insnBuffer[4096];

   initTramps(); // initializes "regSpace", but only the 1st time called
   extern registerSpace *regSpace;
   regSpace->resetSpace();

   Address count = 0; // number of bytes required for RPCtempTramp

   // The following is implemented in an arch-specific source file...
   if (!emitInferiorRPCheader(insnBuffer, count)) {
      // a fancy dialog box is probably called for here...
      cerr << "createRPCtempTramp failed because emitInferiorRPCheader failed."
           << endl;
      return 0;
   }

#if defined(MT_THREAD)
   if (thrId != -1) {
     //
     unsigned pos = 0;
     for (unsigned u=0; u<threads.size(); u++) {
       if (thrId == threads[u]->get_tid()) {
         pos = threads[u]->get_pos();
         break;
       }
     }

     //
     vector<AstNode* > param ;
     param +=  new  AstNode(AstNode::Constant,(void *) thrId) ;
     param +=  new  AstNode(AstNode::Constant,(void *) pos);

     function_base* DYNINSTstartThreadTimer = 
       findOneFunction(string("DYNINSTstartThreadTimer"));
     function_base* DYNINSTstartThreadTimer_inferiorRPC = 
       findOneFunction(string("DYNINSTstartThreadTimer_inferiorRPC"));
     action->replaceFuncInAst(DYNINSTstartThreadTimer, 
                              DYNINSTstartThreadTimer_inferiorRPC, param, 1);

     // replace DYNINSTthreadPos with DYNINSTthreadPosTID
     function_base* DYNINST_not_deleted =
       findOneFunction(string("DYNINST_not_deleted"));
     function_base* DYNINST_not_deletedTID =
       findOneFunction(string("DYNINST_not_deletedTID"));
     action->replaceFuncInAst(DYNINST_not_deleted, 
                              DYNINST_not_deletedTID, param, 0);
  
     // replace DYNINSTloop with DYNINSTloopTID
     function_base* DYNINSTloop =
       findOneFunction(string("DYNINSTloop"));
     function_base* DYNINSTloopTID =
       findOneFunction(string("DYNINSTloopTID"));
     action->replaceFuncInAst(DYNINSTloop, 
                            DYNINSTloopTID, param, 0);

     for (unsigned i=0; i<param.size(); i++) removeAst(param[i]) ;

     // Count the number of instructions actions needs  to set offset properly
     char tmp[1024] ;
     Address cnt = 0 ;
     action->generateCode(this, regSpace, (char*) tmp, cnt, noCost, true) ;
     regSpace->resetSpace();

     generateRPCpreamble((char*)insnBuffer,count,this,
                         (cnt)+7*sizeof(instruction), thrId, pos);
   }
#endif 

   resultReg = (Register)action->generateCode(this, regSpace,
                                    (char*)insnBuffer,
                                    count, noCost, true);

   if (!shouldStopForResult) {
      regSpace->freeRegister(resultReg);
   }
   else
      ; // in this case, we'll call freeRegister() the inferior rpc completes

   // Now, the trailer (restore, TRAP, illegal)
   // (the following is implemented in an arch-specific source file...)

   unsigned breakOffset, stopForResultOffset, justAfter_stopForResultOffset;
   if (!emitInferiorRPCtrailer(insnBuffer, count,
                       breakOffset, shouldStopForResult, stopForResultOffset,
#if defined(MT_THREAD)
                       justAfter_stopForResultOffset, isSAFE)) {
#else
                       justAfter_stopForResultOffset)) {
#endif
      // last 4 args except shouldStopForResult are modified by the call
      cerr << "createRPCtempTramp failed because emitInferiorRPCtrailer failed." << endl;
      return 0;
   }

   Address tempTrampBase;
   if (lowmem)
     {
       /* lowmemHeap should always have free space, so this will not
          require a recursive inferior RPC. */
       tempTrampBase = inferiorMalloc(this, count, lowmemHeap);
     }
   else
     {
       /* May cause another inferior RPC to dynamically allocate a new heap
          in the inferior. */
       tempTrampBase = inferiorMalloc(this, count, anyHeap);
     }
   assert(tempTrampBase);

   breakAddr                      = tempTrampBase + breakOffset;
   if (shouldStopForResult) {
      stopForResultAddr           = tempTrampBase + stopForResultOffset;
      justAfter_stopForResultAddr = tempTrampBase + justAfter_stopForResultOffset;
   }
   else {
      stopForResultAddr = justAfter_stopForResultAddr = 0;
   }

   if (pd_debug_infrpc)
      cerr << "createRPCtempTramp: temp tramp base=" << (void*)tempTrampBase
           << ", stopForResultAddr=" << (void*)stopForResultAddr
           << ", justAfter_stopForResultAddr=" << (void*)justAfter_stopForResultAddr
           << ", breakAddr=" << (void*)breakAddr
           << ", count=" << count << " so end addr="
           << (void*)(tempTrampBase + count - 1) << endl;


   /* Now, write to the tempTramp, in the inferior addr's data space
      (all tramps are allocated in data space) */
   if (!writeDataSpace((void*)tempTrampBase, count, insnBuffer)) {
      // should put up a nice error dialog window
      cerr << "createRPCtempTramp failed because writeDataSpace failed" << endl;
      return 0;
   }

#if defined(MT_THREAD)
   if (!DYNINSTthreadRPC) {
      RTINSTsharedData* RTsharedData = (RTINSTsharedData* ) getRTsharedDataInParadyndSpace() ;
      DYNINSTthreadRPC = RTsharedData->rpcToDoList ;
      assert(DYNINSTthreadRPC) ;
      memset((char*)DYNINSTthreadRPC, '\0', sizeof(rpcToDo)*MAX_PENDING_RPC) ;

      // mutex, conditional variable, and global to communicate with the RPC thread
      
      DYNINSTthreadRPC_mp        = &(RTsharedData->rpc_mutex);
      DYNINSTthreadRPC_cvp       = &(RTsharedData->rpc_cv);
      DYNINSTthreadRPC_pending_p = &(RTsharedData->rpc_pending);

      //mutex_init(DYNINSTthreadRPC_mp, USYNC_PROCESS, NULL);
      //cond_init(DYNINSTthreadRPC_cvp, USYNC_PROCESS, NULL);
   }

   if (isSAFE) { /* post it in the shared-memory segment */
     for (unsigned k=0; k< MAX_PENDING_RPC; k++) {
       if (DYNINSTthreadRPC[k].flag ==0) {
         DYNINSTthreadRPC[k].rpc = (void (*) (void)) tempTrampBase ;
         DYNINSTthreadRPC[k].flag = 1 ; 
         break ;
       } 
     }
   }
#endif /*MT_THREAD*/

   extern int trampBytes; // stats.h
   trampBytes += count;

   return tempTrampBase;
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

#if defined(MT_THREAD)
bool process::handleDoneSAFEinferiorRPC(void) {
  if (DYNINSTthreadRPC) { 
    for (unsigned d=0; d< MAX_PENDING_RPC; d++) {
      if (DYNINSTthreadRPC[d].flag ==2) { //done execute
        //
        unsigned cookie = (unsigned) (DYNINSTthreadRPC[d].rpc);
        if (cookie == 0) {
          cerr << " a weird condition occurred in handleDoneSAFEinferiorRPC ..." << endl;
          continue ;
        }

        for (unsigned k=0; k < currRunningRPCs.size(); k++) {
          if (currRunningRPCs[k].firstInstrAddr == cookie) {
            inferiorRPCinProgress &theStruct = currRunningRPCs[k];

            if (pd_debug_infrpc) {
              sprintf(errorLine, "find completed SAFE rpc, cookie=0x%x", cookie);
              cerr << errorLine << endl;
            }

            // step 1) restore registers: not needed for SAFE RPC

            if (currRunningRPCs.empty() && deferredContinueProc) {
              deferredContinueProc=false;
              if (continueProc()) statusLine("application running");
            }

            // step 2) delete temp tramp
            vector< addrVecType > pointsToCheck;
            inferiorFree(this, theStruct.firstInstrAddr, pointsToCheck);

            // step 3) pause process, if appropriate
            //if (!theStruct.wasRunning && status() == running) {
            //  cerr << "SAFE RPC completion: pause " << endl;
            //  if (!pause()) {
            //    cerr << "RPC completion: pause failed" << endl;
            //  }
            //}

            // step 4) invoke user callback, if any
            if (theStruct.callbackFunc) {
               theStruct.callbackFunc(this, theStruct.userData, theStruct.resultValue);
            }
            currRunningRPCs.removeByIndex(k);
            break; //break the for()
          }
        }
        //
        DYNINSTthreadRPC[d].rpc = 0 ; //make it free again
        DYNINSTthreadRPC[d].flag = 0 ; //make it free again
      }
    }
  }
  return true;
}
#endif

bool process::handleTrapIfDueToRPC() {
   // get curr PC register (can assume process is stopped), search for it in
   // 'currRunningRPCs'.  If found, restore regs, do callback, delete tramp, and
   // return true.  Returns false if not processed.

   assert(status_ == stopped); // a TRAP should always stop a process (duh)
   
   if (currRunningRPCs.empty()) {
     if (pd_debug_infrpc)
        cerr << "handleTrapIfDueToRPC, currRunningRPCs is empty !" << endl;
      return false; // no chance of a match
   }
#if defined(MT_THREAD)
   unsigned k;
   unsigned rSize = 0 ;
   for (k=0; k<currRunningRPCs.size(); k++) {
     if (!currRunningRPCs[k].isSafeRPC)
       rSize ++ ;
   }
   assert(rSize<=1);
#else
   assert(currRunningRPCs.size() == 1);
      // it's unsafe to have > 1 RPCs going on at a time within a single process
#endif
   // Okay, time to do a stack trace.
   // If we determine that the PC of any level of the back trace
   // equals the current running RPC's stopForResultAddr or breakAddr,
   // then we have a match.  Note: since all platforms currently
   // inline their trap/ill instruction instead of make a fn call e.g. to
   // DYNINSTbreakPoint(), we can probably get rid of the stack walk.

   int match_type = 0; // 1 --> stop for result, 2 --> really done
   Frame theFrame(this);
 
   unsigned the_index = 0;

   while (true) {
      // do we have a match?
      const Address framePC = theFrame.getPC();

      bool find_match = false;
      for (the_index=0; the_index<currRunningRPCs.size(); the_index++) {
         if ((Address)framePC == currRunningRPCs[the_index].breakAddr) {
           // we've got a match!
           match_type = 2;
           find_match = true;
           break;
         }
         else if (currRunningRPCs[the_index].callbackFunc != NULL &&
                 ((Address)framePC == currRunningRPCs[the_index].stopForResultAddr)) {
           match_type = 1;
           find_match = true;
           break;
         }
      }

      if (find_match) {
        sprintf(errorLine, "handleTrapIfDueToRPC found matching RPC, "
                "pc=%s, match_type=%d, the_index=%u", 
                Address_str(framePC), match_type, the_index);
        if (pd_debug_infrpc)
          cerr << errorLine << endl;
        break;
      }

      if (theFrame.isLastFrame()) {
         // well, we've gone as far as we can, with no match.
          if (pd_debug_infrpc) {
            sprintf(errorLine, "handleTrapIfDuetoRPC, "
                "cannot find match for PC=%s", Address_str(framePC));
            cerr << errorLine << endl;
          }
         return false;
      }

      // else, backtrace 1 more level
      theFrame = theFrame.getCallerFrame(this);
   }

   assert(match_type == 1 || match_type == 2);

   inferiorRPCinProgress &theStruct = currRunningRPCs[the_index];

   if (match_type == 1) {
      // We have stopped in order to grab the result.  Grab it and write
      // to theStruct.resultValue, for use when we get match_type==2.

      inferiorrpc_cerr << "Welcome to handleTrapIfDueToRPC match type 1" << endl;
      if (pd_debug_infrpc) 
        cerr << "Welcome to handleTrapIfDueToRPC match type 1 (grab the result)" << endl;

      assert(theStruct.callbackFunc != NULL);
        // must be a callback to ever see this match_type

      //  In the case where the resultRegister is the Null_Register, we can assume that
          //  the return value will be ignored and can be set to 0.  This happens in some cases when
          //  catch-up instrumentation is being executed: there isn't a result value that we are
          //  interested in, but we call a callback function anyway.
      Address returnValue = 0;

          if ( theStruct.resultRegister != Null_Register )
          {
        returnValue = read_inferiorRPC_result_register(theStruct.resultRegister);
        extern registerSpace *regSpace;
        regSpace->freeRegister(theStruct.resultRegister);
          }

      theStruct.resultValue = (void *)returnValue;

      // we don't remove the RPC from 'currRunningRPCs', since it's not yet done.
      // we continue the process...but not quite at the PC where we left off, since
      // that will just re-do the trap!  Instead, we need to continue at the location
      // of the next instruction.
      if (!changePC(theStruct.justAfter_stopForResultAddr))
            assert(false);

      if (!continueProc())
          cerr << "RPC getting result: continueProc failed" << endl;
      return true;
   }

   inferiorrpc_cerr << "Welcome to handleTrapIfDueToRPC match type 2" << endl;
   if (pd_debug_infrpc) 
     cerr << "Welcome to handleTrapIfDueToRPC match type 2 (rpc finished)" << endl;

   assert(match_type == 2);

#ifndef i386_unknown_nt4_0
   /* If the application was paused when it was at a trap, reset the rt library
      flag which indicated this. */
   SendAppIRPCInfo(0, 0, 0);
#endif

   // step 1) restore registers:

   if (!restoreRegisters(theStruct.savedRegs)) {
           cerr << "handleTrapIfDueToRPC failed because restoreRegisters failed" << endl;
           assert(false);
   }


   if (currRunningRPCs.empty() && deferredContinueProc) {
     // We have a pending continueProc that we had to delay because
     // there was an inferior RPC in progress at that time, but now
     // we are ready to execute it - naim
     deferredContinueProc=false;
     if (continueProc()) statusLine("application running");
   }

   // step 2) delete temp tramp
   vector<addrVecType> pointsToCheck; // empty (safe to delete immediately)
   inferiorFree(this, theStruct.firstInstrAddr, pointsToCheck);

   // step 3) continue process, if appropriate
   if (theStruct.wasRunning) {
      inferiorrpc_cerr << "end of rpc -- continuing process, since it had been running" << endl;

      if (!continueProc()) {
                  cerr << "RPC completion: continueProc failed" << endl;
      }
   }
   else
      inferiorrpc_cerr << "end of rpc -- leaving process paused, since it wasn't running before" << endl;

   // step 4) invoke user callback, if any
   // note: I feel it's important to do the user callback last, since it
   // may perform arbitrary actions (such as making igen calls) which can lead
   // to re-actions (such as receiving igen calls) that can alter the process
   // (e.g. continuing it).  So clearly we need to restore registers, change the
   // PC, etc. BEFORE any such thing might happen.  Hence we do the callback last.
   // I think the only potential controversy is whether we should do the callback
   // before step 3, above.

   inferiorrpc_cerr << "handleTrapIfDueToRPC match type 2 -- about to do callbackFunc, if any" << endl;


   // save enough information to call the callback function, if needed
   inferiorRPCcallbackFunc cb = theStruct.callbackFunc;
   void* userData = theStruct.userData;
   void* resultValue = theStruct.resultValue;

   // release the RPC struct
#if defined(i386_unknown_nt4_0)
   delete    theStruct.savedRegs;       // not an array on WindowsNT
#else
   delete [] static_cast<char *>(theStruct.savedRegs);
#endif
   currRunningRPCs.removeByIndex(the_index);

   // call the callback function if needed
   if( cb != NULL )
   {
      (*cb)(this, userData, resultValue);
   }

   inferiorrpc_cerr << "handleTrapIfDueToRPC match type 2 -- done with callbackFunc, if any" << endl;

   return true;
}


void process::installBootstrapInst() {
   // instrument main to call DYNINSTinit().  Don't use the shm seg for any
   // temp tramp space, since we can't assume that it's been initialized yet.
   // We build an ast saying: "call DYNINSTinit() with args
   // key_base, nbytes, paradynd_pid"

   attach_cerr << "process::installBootstrapInst()" << endl;
#ifdef BPATCH_LIBRARY
   vector<AstNode *> the_args(2);

   the_args[0] = new AstNode(AstNode::Constant, (void*)1);
   the_args[1] = new AstNode(AstNode::Constant, (void*)getpid());

   AstNode *ast = new AstNode("DYNINSTinit", the_args);
   removeAst(the_args[0]) ;
   removeAst(the_args[1]) ;
#else
   vector<AstNode *> the_args(3);

   // 2 dummy args when not shm sampling (just don't use -1, which is reserved
   // for fork)
   unsigned numBytes = 0;
   
#ifdef SHM_SAMPLING
   key_t theKey   = getShmKeyUsed();
   numBytes = getShmHeapTotalNumBytes();
#else
   int theKey = 0;
#endif

#ifdef SHM_SAMPLING_DEBUG
   cerr << "paradynd inst.C: about to call DYNINSTinit() with key=" << theKey
        << " and #bytes=" << numBytes << endl;
#endif

   the_args[0] = new AstNode(AstNode::Constant, (void*)theKey);
   the_args[1] = new AstNode(AstNode::Constant, (void*)numBytes);
#ifdef BPATCH_LIBRARY
   // Unused by the dyninstAPI library
   the_args[2] = new AstNode(AstNode::Constant, (void *)0);
#else
   //  for IRIX MPI, we want to appear to be attaching 
   if ( process::pdFlavor == "mpi" && osName.prefixed_by("IRIX") 
                                   && traceConnectInfo > 0 )
       traceConnectInfo *= -1;

   the_args[2] = new AstNode(AstNode::Constant, (void*)traceConnectInfo);
#endif

   AstNode *ast = new AstNode("DYNINSTinit", the_args);
   for (unsigned j=0; j<the_args.size(); j++) {
       removeAst(the_args[j]);
   }
#endif /* BPATCH_LIBRARY */

   function_base *func = getMainFunction();
   if (func) {
       instPoint *func_entry = const_cast<instPoint*>(func->funcEntry(this));
#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0)
       if (func_entry->usesTrap(this)) {
            // we can't instrument main's entry with a trap yet
            // since DYNINSTinit installs our instrumentation trapHandler
            showErrorCallback(108,"main() entry uninstrumentable (w/o trap)");
            extern void cleanUpAndExit(int);
            cleanUpAndExit(-1); 
            return;
       }
#endif
       addInstFunc(this, func_entry, ast, callPreInsn,
               orderFirstAtPoint,
               true, // true --> don't try to have tramp code update the cost
               false // Use recursive guard
               );   
       // returns an "instInstance", which we ignore (but should we?)
       removeAst(ast);
       attach_cerr << "wrote call to DYNINSTinit to entry of main" << endl;
    } else {
       printf("no main function, skipping DYNINSTinit\n");
       hasBootstrapped = true;
#ifdef BPATCH_LIBRARY
       if (wasExeced()) {
	     BPatch::bpatch->registerExec(thread);
       }
#endif
    }

#if defined(alpha_dec_osf4_0)
      // need to perform this after dyninst Heap is present and happy
      dyn->setMappingHooks(this);
#endif
    attach_cerr << "process::installBootstrapInst() complete" << endl;
}

void process::installInstrRequests(const vector<instMapping*> &requests) {
#ifndef BPATCH_LIBRARY    // metric_cerr is a pdDebug_ostream
   metric_cerr << "process::installInstrRequests*" << requests.size() << endl;
#endif
   for (unsigned lcv=0; lcv < requests.size(); lcv++) {

      instMapping *req = requests[lcv];

      function_base *func = findOneFunction(req->func);
      if (!func)
         continue;  // probably should have a flag telling us whether errors
                    // should be silently handled or not
#ifndef BPATCH_LIBRARY    // metric_cerr is a pdDebug_ostream
      metric_cerr << "Found " << req->func << endl;
#endif
      AstNode *ast;
      if ((req->where & FUNC_ARG) && req->args.size()>0) {
        ast = new AstNode(req->inst, req->args);
      } else {
        AstNode *tmp = new AstNode(AstNode::Constant, (void*)0);
        ast = new AstNode(req->inst, tmp);
        removeAst(tmp);
      }
      if (req->where & FUNC_EXIT) {
         const vector<instPoint*> func_rets = func->funcExits(this);
         for (unsigned j=0; j < func_rets.size(); j++)
            (void)addInstFunc(this, func_rets[j], ast,
                              req->when, req->order, false, false);

      }

      if (req->where & FUNC_ENTRY) {
         instPoint *func_entry = const_cast<instPoint *>(func->funcEntry(this));
         (void)addInstFunc(this, func_entry, ast,
                           req->when, req->order, false, false);

      }

      if (req->where & FUNC_CALL) {
         vector<instPoint*> func_calls = func->funcCalls(this);
         if (func_calls.size() == 0)
            continue;

         for (unsigned j=0; j < func_calls.size(); j++)
            (void)addInstFunc(this, func_calls[j], ast,
                              req->when, req->order, false, false);

      }

      removeAst(ast);
   }
}

#ifdef SHM_SAMPLING
bool process::extractBootstrapStruct(PARADYN_bootstrapStruct *bs_record)
{
  const string vrbleName = "PARADYN_bootstrap_info";
  internalSym sym;
  bool flag = findInternalSymbol(vrbleName, true, sym);
  assert(flag);
  Address symAddr = sym.getAddr();

  // bulk read of bootstrap structure
  if (!readDataSpace((const void*)symAddr, sizeof(*bs_record), bs_record, true)) {
    cerr << "extractBootstrapStruct failed because readDataSpace failed" << endl;
    return false;
  }

  // address-in-memory: re-read pointer field with proper alignment
  // (see rtinst/h/trace.h)
  assert(sizeof(int64_t) == 8); // sanity check
  assert(sizeof(int32_t) == 4); // sanity check

  // read pointer size
  int32_t ptr_size;
  internalSym sym2;
  bool ret2;
  ret2 = findInternalSymbol("PARADYN_attachPtrSize", true, sym2);
  if (!ret2) return false;
  ret2 = readDataSpace((void *)sym2.getAddr(), sizeof(int32_t), &ptr_size, true);
  if (!ret2) return false;
  // problem scenario: 64-bit application, 32-bit paradynd
  assert((size_t)ptr_size <= sizeof(bs_record->appl_attachedAtPtr.ptr));

  // re-align pointer if necessary
  if ((size_t)ptr_size < sizeof(bs_record->appl_attachedAtPtr.ptr)) {
    // assumption: 32-bit application, 64-bit paradynd
    assert(ptr_size == sizeof(int32_t));
    assert(sizeof(bs_record->appl_attachedAtPtr.ptr) == sizeof(int64_t));
    assert(sizeof(bs_record->appl_attachedAtPtr.words.hi) == sizeof(int32_t));
    // read 32-bit pointer from high word
    Address val_a = (unsigned)bs_record->appl_attachedAtPtr.words.hi;
    void *val_p = (void *)val_a;
    bs_record->appl_attachedAtPtr.ptr = val_p;
    fprintf(stderr, "    %p ptr *\n", bs_record->appl_attachedAtPtr.ptr);
  }
  
  return true;
}
#endif /* SHM_SAMPLING */

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

bool process::handleStopDueToExecEntry() {
   // returns true iff we are processing a stop due to the entry point of exec
   // The exec hasn't yet occurred.

   assert(status_ == stopped);

   DYNINST_bootstrapStruct bs_record;
   if (!extractBootstrapStruct(&bs_record))
      assert(false);

   if (bs_record.event != 4)
      return false;

   assert(getPid() == bs_record.pid);

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

   // shouldn't we be setting reachedFirstBreak to false???

   return true;
}

int process::procStopFromDYNINSTinit() {
   // possible return values:
   // 0 --> no, the stop wasn't the end of DYNINSTinit
   // 1 --> handled stop at end of DYNINSTinit, leaving program paused
   // 2 --> handled stop at end of DYNINSTinit...which had been invoked via
   //       inferiorRPC...so we've continued the process in order to let the
   //       inferiorRPC get its sigtrap.

   // Note that DYNINSTinit could have been run under several cases:
   // 1) the normal case     (detect by bs_record.event==1 && execed_ == false)
   // 2) called after a fork (detect by bs_record.event==2)
   // 3) called after an exec (detect by bs_record.event==1 and execed_ == true)
   // 4) called for an attach (detect by bs_record.event==3)
   // note that bs_record.event == 4 is reserved for "sending" a tr_exec "record".
   //
   // The exec case is tricky: we must loop thru all component mi's of this process
   // and decide now whether or not to carry them over to the new process.

   // if 0 is returned, there must be no side effects.

   assert(status_ == stopped);

   if (hasBootstrapped)
      return 0;

   DYNINST_bootstrapStruct bs_record;
   if (!extractBootstrapStruct(&bs_record))
      assert(false);

   // Read the structure; if event 0 then it's undefined! (not yet written)
   if (bs_record.event == 0)
      return 0;

   forkexec_cerr << "procStopFromDYNINSTinit pid " << getPid() << "; got rec" << endl;

   assert(bs_record.event == 1 || bs_record.event == 2 || bs_record.event==3);
   assert(bs_record.pid == getPid());

#ifndef BPATCH_LIBRARY
   if (bs_record.event != 3 || (process::pdFlavor == "mpi" && osName.prefixed_by("IRIX")) )
#else
   if (bs_record.event != 3 )
#endif
   {
      // we don't want to do this stuff (yet) when DYNINSTinit was run via attach...we
      // want to wait until the inferiorRPC (thru which DYNINSTinit is being run)
      // completes.
      handleCompletionOfDYNINSTinit(false);
      return 1;
   }
   else {
      if (!continueProc())
         assert(false);
      return 2;
   }
}

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

bool process::writeTimerFuncAddr_(const char *rtinstVar,
				   const char *rtinstFunc)
{
   bool err = false;
   Address rtfuncAddr = findInternalAddress(rtinstFunc, true, err);
   assert(err==false);
   err = false;
   Address timeFuncVarAddr = findInternalAddress(rtinstVar, true, err);
   assert(err==false);
   return writeTextSpace((void *)(timeFuncVarAddr),
   			 sizeof(rtfuncAddr), (void *)(&rtfuncAddr));
}

void process::writeTimerFuncAddr(const char *rtinstVar, const char *rtinstFunc)
{
   int appAddrWidth = getImage()->getObject().getAddressWidth();
   bool result;

   if(sizeof(Address)==8 && appAddrWidth==4)
     result = writeTimerFuncAddr_Force32(rtinstVar, rtinstFunc);     
   else
     result = writeTimerFuncAddr_(rtinstVar, rtinstFunc);          

   if(result == false) {
     cerr << "!!!  Couldn't write timer func address into rt library !!\n";
   }
}

void process::writeTimerLevels() {
   char rtTimerStr[61];
   rtTimerStr[60] = 0;
   string cStr = cpuTimeMgr->get_rtTimeQueryFuncName(cpuTimeMgr_t::LEVEL_BEST);
   strncpy(rtTimerStr, cStr.string_of(), 59);
   writeTimerFuncAddr("pDYNINSTgetCPUtime", rtTimerStr);
   //logStream << "Setting cpu time retrieval function in rtinst to " 
   //     << rtTimerStr << "\n" << flush;

   string wStr=wallTimeMgr->get_rtTimeQueryFuncName(wallTimeMgr_t::LEVEL_BEST);
   strncpy(rtTimerStr, wStr.string_of(), 59);
   writeTimerFuncAddr("pDYNINSTgetWalltime", rtTimerStr);
   //logStream << "Setting wall time retrieval function in rtinst to " 
   //     << rtTimerStr << "\n" << flush;
}
#endif

void process::handleCompletionOfDYNINSTinit(bool fromAttach) {
   // 'event' values: (1) DYNINSTinit was started normally via paradynd
   // or via exec, (2) called from fork, (3) called from attach.

   inferiorrpc_cerr << "handleCompletionOfDYNINSTinit..." << endl ;
   DYNINST_bootstrapStruct bs_record;
   if (!extractBootstrapStruct(&bs_record))
      assert(false);

   if (!fromAttach) // reset to 0 already if attaching, but other fields (attachedAtPtr) ok
      assert(bs_record.event == 1 || bs_record.event == 2 || bs_record.event==3);

   assert(bs_record.pid == getPid());

   // Note: the process isn't necessarily paused at this moment.  In particular,
   // if we had attached to the process, then it will be running even as we speak.
   // While we're parsing the shared libraries, we should pause.  So do that now.
   bool wasRunning;
   if (needToContinueAfterDYNINSTinit) 
     wasRunning = true;
   else 
     wasRunning = status_ == running;
   (void)pause();

#ifndef BPATCH_LIBRARY
   const bool calledFromExec   = (bs_record.event == 1 && execed_);
// jkh - why was the above line commented out??
#endif
   const bool calledFromFork   = (bs_record.event == 2);
   const bool calledFromAttach = fromAttach || bs_record.event == 3;
   if (calledFromAttach 
#ifndef BPATCH_LIBRARY
      && !(process::pdFlavor == "mpi" && osName.prefixed_by("IRIX")) 
#endif
      )
      assert(createdViaAttach);

#ifdef SHM_SAMPLING
   PARADYN_bootstrapStruct bs_struct;
   if (!extractBootstrapStruct(&bs_struct))
      assert(false);

   if (!calledFromFork)
      registerInferiorAttachedSegs(bs_struct.appl_attachedAtPtr.ptr);
#endif

   if (!calledFromFork)
      getObservedCostAddr();

   // handleStartProcess gets shared objects, so no need to do it again after a fork.
   // (question: do we need to do this after an exec???)
   if (!calledFromFork) {
      string str=string("PID=") + string(bs_record.pid) + ", calling handleStartProcess...";
      statusLine(str.string_of());

#if !defined(USES_LIBDYNINSTRT_SO) || defined(i386_unknown_nt4_0)
      if (!handleStartProcess()) {
        // reads in shared libraries...can take a while
        logLine("WARNING: handleStartProcess failed\n");
      }
#endif

#ifndef BPATCH_LIBRARY
      // we decrement the batch mode here; it matches the bump-up in createProcess()
      tp->resourceBatchMode(false);
#endif

      str=string("PID=") + string(bs_record.pid) + ", installing default inst...";
      statusLine(str.string_of());

      extern vector<instMapping*> initialRequests; // init.C
      installInstrRequests(initialRequests);

      str=string("PID=") + string(bs_record.pid) + ", propagating mi's...";
      statusLine(str.string_of());

      forkexec_cerr << "procStopFromDYNINSTinit pid " << getPid() << "; about to propagate mi's" << endl;

#ifndef BPATCH_LIBRARY
      if (!calledFromExec) {
         // propagate any metric that is already enabled to the new process.
         // For a forked process, this isn't needed because handleFork() has its own
         // special propagation algorithm (it propagates every aggregate mi having the
         // parent as a component, except for aggregate mi's whose focus is specifically
         // refined to the parent).
         vector<metricDefinitionNode *> MIs = allMIs.values();
         for (unsigned j = 0; j < MIs.size(); j++) {
            MIs[j]->propagateToNewProcess(this);
            // change to a process:: method which takes in the metricDefinitionNode
         }
      }
      else {
         // exec propagates in its own, special way that differs from a new process.
         // (propagate all mi's that make sense in the new process)
         metricDefinitionNode::handleExec(this);
      }
#endif

      forkexec_cerr << "procStopFromDYNINSTinit pid " << getPid() << "; done propagate mi's" << endl;
   }

   hasBootstrapped = true; // now, shm sampling may safely take place.
#ifdef BPATCH_LIBRARY
   if (wasExeced()) {
	 BPatch::bpatch->registerExec(thread);
   }
#endif

   string str=string("PID=") + string(bs_record.pid) + ", executing new-prog callback...";
   statusLine(str.string_of());

#ifndef BPATCH_LIBRARY
   timeStamp currWallTime = calledFromExec ? timeStamp::ts1970():getWallTime();
   if (!calledFromExec && !calledFromFork) {
      // The following must be done before any samples are sent to
      // paradyn; otherwise, prepare for an assert fail.

      if (!isInitFirstRecordTime())
	setFirstRecordTime(currWallTime);
   }
#endif

   assert(status_ == stopped);

#ifndef BPATCH_LIBRARY
   tp->newProgramCallbackFunc(bs_record.pid, this->arg_list, 
                              machineResource->part_name(),
                              calledFromExec,
                              wasRunning);
      // in paradyn, this will call paradynDaemon::addRunningProgram().
      // If the state of the application as a whole is 'running' paradyn will
      // soon issue an igen call to us that'll continue this process.

   if (!calledFromExec)
      tp->firstSampleCallback(getPid(), currWallTime.getD(timeUnit::sec(), 
	                                                  timeBase::bStd()));
   // verify that the wall and cpu timer levels chosen by the daemon
   // are available in the rt library
   verifyTimerLevels();

   // This is currently broken on aix because the daemon isn't correctly able
   // to grab the value to use when assigning an address to a function
   // pointer.  On aix function addresses for such things as setting function
   // pointers are really addresses of a structure in the data segment which
   // give information about the true function address.  However, our symbol
   // table parsing code only grabs the true function address in the text
   // segment.  We have thought of two solutions.  One solution is to add
   // support in the symbol table parsing code to also grab the data segment
   // function pointer struct, and set the function pointer in the rtinst
   // library to this value.  Another solution is to communicate to the
   // rtinst library the desired function to use by means of setting a
   // numeric value and having the rtinst library setting the function
   // pointer itself.
#ifndef rs6000_ibm_aix4_1
   writeTimerLevels();
#endif
#endif

   if (calledFromFork) {
      // the parent proc has been waiting patiently at the start of DYNINSTfork
      // (i.e. the fork syscall executed but that's it).  We can continue it now.
      process *parentProcess = findProcess(bs_record.ppid);
      if (parentProcess) {
#ifdef DETACH_ON_THE_FLY
         if (kill(parentProcess->getPid(), SIGCONT) < 0) {
            perror("kill error");
	    assert(false);
	 }
#else
         if (parentProcess->status() == stopped) {
            if (!parentProcess->continueProc())
               assert(false);
         }
         else
            parentProcess->continueAfterNextStop();
#endif
      }
   }

   if (!calledFromAttach) {
      str=string("PID=") + string(bs_record.pid) + ", ready.";
      statusLine(str.string_of());
   }

   if (calledFromAttach && !wasRunning) {
      statusLine("application paused");
   }

   assert(status_ == stopped);
      // though not for long, if 'wasRunning' is true (paradyn will soon continue us)

   inferiorrpc_cerr << "handleCompletionOfDYNINSTinit...done" << endl;
}

void process::getObservedCostAddr() {

#ifndef SHM_SAMPLING
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
  if (status_ == exited) {
    sprintf(errorLine, "attempt to ptrace exited process %d\n", pid);
    logLine(errorLine);
    return(false);
  } else
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
  if (status_ == exited)
     return;

  // snag the last shared-memory sample:
#ifdef SHM_SAMPLING
  (void)doMajorShmSample(getWallTime());
#endif

#ifndef BPATCH_LIBRARY
  // close down the trace stream:
  if (traceLink >= 0) {
     //processTraceStream(proc); // can't do this since it's a blocking read (deadlock)
     P_close(traceLink);
     traceLink = -1;
  }

// Should not close the ioLink if a child is alive. Hope that 
// the operating system will close this link when no processes are left around
#if 0 
  // close down the ioLink:
  if (ioLink >= 0) {
     //processAppIO(proc); // can't do this since it's a blocking read (deadlock)
     P_close(ioLink);
     ioLink = -1;
  }
#endif  
  // for each component mi of this process, remove it from all aggregate mi's it
  // belongs to.  (And if the agg mi now has no components, fry it too.)
  // Also removes this proc from all cost metrics (but what about internal metrics?)
  removeFromMetricInstances(this);
#endif

  status_ = exited;

  detach(false);
     // the process will continue to run (presumably, it will finish _very_ soon)

//  status_ = exited;

#ifndef BPATCH_LIBRARY
  tp->processStatus(pid, procExited);
#endif
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
  CallGraphSetEntryFuncCallback(symbols->file(), 
                                entry_pdf->ResourceFullName());
    
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
  vector<instPoint*> callPoints;
  callPoints = func->funcCalls(this);
  
  unsigned i;
  for(i = 0; i < callPoints.size(); i++){
    if(!findCallee(*(callPoints[i]),temp)){
      if(!MonitorCallSite(callPoints[i])){
        fprintf(stderr, 
             "ERROR in daemon, unable to monitorCallSite for function :%s\n",
             function_name.string_of());
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

rawTime64 process::getRawCpuTime(int lwp_id) {
  return cpuTimeMgr->getRawTime(this, lwp_id, cpuTimeMgr_t::LEVEL_BEST);
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
      if (PDFuncToBPFuncMap.defines((function_base*)ip->iPgetFunction())) {
  	bpfunc = PDFuncToBPFuncMap[(function_base*)ip->iPgetFunction()];
      } else {
	// XXX Should create with correct module, but we dont' know it --
	//     it doesn't seem to be used anywhere anyway?
	bpfunc = new BPatch_function(this,
				     (function_base*)ip->iPgetFunction(),
				     NULL);
      }
    }

    BPatch_point *pt = new BPatch_point(this, bpfunc, ip, pointType);
    instPointMap[addr] = pt;
    return pt;
  }
}
#endif




