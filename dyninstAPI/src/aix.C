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
 * excluded
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

// $Id: aix.C,v 1.186 2004/04/06 16:37:07 bernat Exp $

#include <dlfcn.h>
#include <sys/types.h>
#include <sys/ldr.h>

#include <pthread.h>
#include "common/h/headers.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/stats.h"
#include "common/h/Types.h"
#include "common/h/Dictionary.h"
#include "dyninstAPI/src/Object.h"
#include "dyninstAPI/src/instP.h" // class miniTrampHandle
#include "common/h/pathName.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/inst-power.h" // Tramp constants

#if defined(AIX_PROC)
#include <sys/procfs.h>
#include <poll.h>
#endif

#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <scnhdr.h>
#include <sys/time.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <procinfo.h> // struct procsinfo
#include <sys/types.h>
#include <signal.h>

// Things to use for Paradyn only
#ifndef BPATCH_LIBRARY
#include "common/h/Time.h"
#include "common/h/timing.h"
#include "paradynd/src/init.h"
#include "paradynd/src/instReqNode.h"
#endif

#include "writeBackXCOFF.h"

#if !defined(BPATCH_LIBRARY)
#ifdef USES_PMAPI
#include <pmapi.h>
#include "rtinst/h/rthwctr-aix.h"
#endif
#endif



#if !defined(BPATCH_LIBRARY)
/* Getprocs() should be defined in procinfo.h, but it's not */
extern "C" {
extern int getprocs(struct procsinfo *ProcessBuffer,
		    int ProcessSize,
		    struct fdsinfo *FileBuffer,
		    int FileSize,
		    pid_t *IndexPointer,
		    int Count);
extern int getthrds(pid_t, struct thrdsinfo *, int, tid_t *, int);
}
#endif

#include "dyninstAPI/src/showerror.h"
#include "common/h/debugOstream.h"

extern "C" {
extern int ioctl(int, int, ...);
};

extern void generateBreakPoint(instruction &);

// The following vrbles were defined in process.C:
extern unsigned enable_pd_attach_detach_debug;

#if ENABLE_DEBUG_CERR == 1
#define attach_cerr if (enable_pd_attach_detach_debug) cerr
#else
#define attach_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_inferior_rpc_debug;

#if ENABLE_DEBUG_CERR == 1
#define inferiorrpc_cerr if (enable_pd_inferior_rpc_debug) cerr
#else
#define inferiorrpc_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_shm_sampling_debug;

#if ENABLE_DEBUG_CERR == 1
#define shmsample_cerr if (enable_pd_shm_sampling_debug) cerr
#else
#define shmsample_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_fork_exec_debug;

#if ENABLE_DEBUG_CERR == 1
#define forkexec_cerr if (enable_pd_fork_exec_debug) cerr
#else
#define forkexec_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

// The frame threesome: normal (singlethreaded), thread (given a pthread ID),
// and LWP (given an LWP/kernel thread).
// The behavior is identical unless we're in a leaf node where
// the LR is in a register, then it's different.

Frame Frame::getCallerFrame(process *p) const
{
  typedef struct {
    unsigned oldFp;
    unsigned savedCR;
    unsigned savedLR;
    unsigned compilerInfo;
    unsigned binderInfo;
    unsigned savedTOC;
  } linkArea_t;

  linkArea_t thisStackFrame;
  linkArea_t lastStackFrame;
  linkArea_t stackFrame;

  Frame ret; // zero frame

  // Are we in a leaf function?
  bool isLeaf = false;
  bool noFrame = false;

  if (uppermost_) {
      codeRange *range = p->findCodeRangeByAddress(pc_);
      pd_Function *func = range->is_pd_Function();
      if (func) {
          isLeaf = func->makesNoCalls();
          noFrame = func->hasNoStackFrame();
      }
  }
  ret.pid_ = pid_;
  ret.thread_ = thread_;
  ret.lwp_ = lwp_;
 
  // Get current stack frame link area
  if (!p->readDataSpace((caddr_t)fp_, sizeof(linkArea_t),
                        (caddr_t)&thisStackFrame, false))
      return Frame();
  p->readDataSpace((caddr_t) thisStackFrame.oldFp, sizeof(linkArea_t),
                   (caddr_t) &lastStackFrame, false);
  
  if (noFrame)
      stackFrame = thisStackFrame;
  else
      stackFrame = lastStackFrame;

  // Figure out where to grab the LR value from. Switch off a "cookie"
  // we store if we're in a base tramp or have modified the LR
  if ((stackFrame.binderInfo & MODIFIED_LR_MASK) == MODIFIED_LR) {
      // The actual LR is stored in the "compilerInfo" word
      ret.pc_ = stackFrame.compilerInfo;
      ret.fp_ = thisStackFrame.oldFp;
  }
  else if ((stackFrame.binderInfo & IN_TRAMP_MASK) == IN_TRAMP) {
      ret.pc_ = stackFrame.savedLR;
      // Skip the next stack frame, as we "grew" the frame rather than making
      // a new one
      if (!p->readDataSpace((caddr_t) thisStackFrame.oldFp,
                            sizeof(unsigned),
                            (caddr_t) &ret.fp_, false))
          return Frame();
  }
  else if (isLeaf) {
      // isLeaf: get the LR from the register instead of saved location on the stack
      if (lwp_ && lwp_->get_lwp_id()) {
          dyn_saved_regs regs;
          bool status = lwp_->getRegisters(&regs);
          if (! status) {
              return Frame();
          }
          ret.pc_ = regs.theIntRegs.__lr;
      }
      else if (thread_ && thread_->get_tid()) {
          cerr << "NOT IMPLEMENTED YET" << endl;
      }
      else { // normal
          dyn_saved_regs regs;
          bool status = p->getRepresentativeLWP()->getRegisters(&regs);
          if (!status) {
              return Frame();
          }
          ret.pc_ = regs.theIntRegs.__lr;
      }
      if (noFrame)
          ret.fp_ = fp_;
      else
          ret.fp_ = thisStackFrame.oldFp;
  }
  else {
      // Common case.
      ret.pc_ = stackFrame.savedLR;
      if (noFrame)
          ret.fp_ = fp_;
      else
          ret.fp_ = thisStackFrame.oldFp;
      ret.fp_ = thisStackFrame.oldFp;
  }

#ifdef DEBUG_STACKWALK
  bperr( "PC %x, FP %x\n", ret.pc_, ret.fp_);
#endif

  return ret;
}


#ifdef DEBUG 
void decodeInstr(unsigned instr_raw) {
  // Decode an instruction. Fun, eh?
  union instructUnion instr;
  instr.raw = instr_raw;

  switch(instr.generic.op) {
  case Bop:
    bperr( "Branch (abs=%d, link=%d) to 0x%x\n",
	    instr.iform.aa, instr.iform.lk, instr.iform.li);
    break;
  case CMPIop:
    bperr( "CMPI reg(%d), 0x%x\n",
	    instr.dform.ra, instr.dform.d_or_si);
    break;
  case SIop:
    bperr( "SI src(%d), tgt(%d), 0x%x\n",
	    instr.dform.ra, instr.dform.rt, instr.dform.d_or_si);
    break;
  case CALop:
    bperr( "CAL src(%d), tgt(%d), 0x%x\n",
	    instr.dform.ra, instr.dform.rt, instr.dform.d_or_si);
    break;
  case CAUop:
    bperr( "CAU src(%d), tgt(%d), 0x%x\n",
	    instr.dform.ra, instr.dform.rt, instr.dform.d_or_si);
    break;
  case ORILop:
    bperr( "ORIL src(%d), tgt(%d), 0x%x\n",
	    instr.dform.rt, instr.dform.ra, instr.dform.d_or_si);
    break;
  case ANDILop:
    bperr( "CAU src(%d), tgt(%d), 0x%x\n",
	    instr.dform.rt, instr.dform.ra, instr.dform.d_or_si);
    break;
  case Lop:
    bperr( "L src(%d)+0x%x, tgt(%d)\n",
	    instr.dform.ra, instr.dform.d_or_si, instr.dform.rt);
    break;
  case STop:
    bperr( "L src(%d), tgt(%d)+0x%x\n",
	    instr.dform.rt, instr.dform.ra, instr.dform.d_or_si);
    break;
  case BCop:
    bperr( "BC op(0x%x), CR bit(0x%x), abs(%d), link(%d), tgt(0x%x)\n",
	    instr.bform.bo, instr.bform.bi, instr.bform.aa, instr.bform.lk, instr.bform.bd);
    break;
  case BCLRop:
    switch (instr.xform.xo) {
    case BCLRxop:
      bperr( "BCLR op(0x%x), bit(0x%x), link(%d)\n",
	      instr.xform.rt, instr.xform.ra, instr.xform.rc);
      break;
    default:
      bperr( "%x\n", instr.raw);
      break;
    }
    break;
  case 0:
    bperr( "NULL INSTRUCTION\n");
    break;
  default:
    bperr( "Unknown instr with opcode %d\n",
	    instr.generic.op);

    break;
  }
  return;
}      

#endif

// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) {
  int ttyfd = open ("/dev/tty", O_RDONLY);
  ioctl (ttyfd, TIOCNOTTY, NULL); 
  close (ttyfd);
}


int getNumberOfCPUs()
{
  return(1);
}

// #include "paradynd/src/costmetrics.h"

#if defined(duplicated_in_process_c_because_linux_ia64_needs_it)
Address process::getTOCoffsetInfo(Address dest)
{
  // We have an address, and want to find the module the addr is
  // contained in. Given the probabilities, we (probably) want
  // the module dyninst_rt is contained in. 
  // I think this is the right func to use

  if (symbols->findFuncByAddr(dest, this))
    return (Address) (symbols->getObject()).getTOCoffset();

  if (shared_objects)
    for(u_int j=0; j < shared_objects->size(); j++)
      if (((*shared_objects)[j])->getImage()->findFuncByAddr(dest, this))
	return (Address) (((*shared_objects)[j])->getImage()->getObject()).getTOCoffset();
  // Serious error! Assert?
  return 0;
}
#endif

#if !defined(BPATCH_LIBRARY)

rawTime64 dyn_lwp::getRawCpuTime_hw() 
{
#ifdef USES_PMAPI
   // Hardware method, using the PMAPI
   int ret;
   
   static bool need_init = true;
   if(need_init) {
      pm_info_t pinfo;
#ifdef PMAPI_GROUPS
      pm_groups_info_t pginfo;
      ret = pm_init(PM_VERIFIED | PM_CAVEAT | PM_GET_GROUPS, &pinfo, &pginfo);
#else
      ret = pm_init(PM_VERIFIED | PM_CAVEAT, &pinfo);
#endif
      // We ignore the return, but pm_init must be called to initialize the
      // library
      if (ret) pm_error("PARADYNos_init: pm_init", ret);
      need_init = false;
   }
   int lwp_to_use;
   tid_t indexPtr = 0;   
   struct thrdsinfo thrd_buf;

   if (lwp_id_ > 0) 
      lwp_to_use = lwp_id_;
   else {
      /* If we need to get the data for the entire process (ie. lwp_id_ == 0)
         then we need to the pm_get_data_group function requires any lwp in
         the group, so we'll grab the lwp of any active thread in the
         process */
      if(getthrds(proc_->getPid(), &thrd_buf, sizeof(struct thrdsinfo), 
                  &indexPtr, 1) == 0) {
         // perhaps the process ended
         return -1;
      }
      lwp_to_use = thrd_buf.ti_tid;
   }

   // PM counters are only valid when the process is paused. 
   bool needToCont = (proc_->status() == running);
   if(needToCont) { // process running
      if(! proc_->pause()) {
         return -1;  // pause failed, so returning failure
      }
   }

   pm_data_t data;
   if(lwp_id_ > 0) 
      ret = pm_get_data_thread(proc_->getPid(), lwp_to_use, &data);
   else {  // lwp == 0, means get data for the entire process (ie. all lwps)
      ret = pm_get_data_group(proc_->getPid(), lwp_to_use, &data);
      while(ret) {
         // if failed, could have been because the lwp (retrieved via
         // getthrds) was in process of being deleted.
         //cerr << "  prev lwp_to_use " << lwp_to_use << " failed\n";
         if(getthrds(proc_->getPid(), &thrd_buf, sizeof(struct thrdsinfo), 
                     &indexPtr, 1) == 0) {
            // couldn't get a valid lwp, go to standard error handling
            ret = 1;
            break;
         }
         lwp_to_use = thrd_buf.ti_tid;
         //cerr << "  next lwp_to_use is " << lwp_to_use << "\n";
         ret = pm_get_data_group(proc_->getPid(), lwp_to_use, &data);
      }
   }

   if (ret) {
      if(!proc_->hasExited()) {
         pm_error("dyn_lwp::getRawCpuTime_hw: pm_get_data_thread", ret);
         bperr( "Attempted pm_get_data(%d, %d, %d)\n",
		 proc_->getPid(), lwp_id_, lwp_to_use);
      }
      return -1;
   }
   rawTime64 result = data.accu[get_hwctr_binding(PM_CYC_EVENT)];

   // Continue the process
   if(needToCont) {
      proc_->continueProc();
   }

   //if(pos_junk != 101)
   //  ct_record(pos_junk, result, hw_previous_, lwp_id_, lwp_to_use);

   if(result < hw_previous_) {
      cerr << "rollback in dyn_lwp::getRawCpuTime_hw, lwp_to_use: " 
           << lwp_to_use << ", lwp: " << lwp_id_ << ", result: " << result 
           << ", previous result: " << hw_previous_ << "\n";
      result = hw_previous_;
   }
   else 
      hw_previous_ = result;
   
   return result;
#else
   return 0;
#endif
}

rawTime64 dyn_lwp::getRawCpuTime_sw()
{
  // returns user+sys time from the user area of the inferior process.
  // Since AIX 4.1 doesn't have a /proc file system, this is slightly
  // more complicated than solaris or the others. 

  // It must not stop the inferior process or assume it is stopped.
  // It must be "in sync" with rtinst's DYNINSTgetCPUtime()

  // Idea number one: use getprocs() (which needs to be included anyway
  // because of a use above) to grab the process table info.
  // We probably want pi_ru.ru_utime and pi_ru.ru_stime.

  // int lwp_id: thread ID of desired time. Ignored for now.
  // int pid: process ID that we want the time for. 
  
  // int getprocs (struct procsinfo *ProcessBuffer, // Array of procsinfos
  //               int ProcessSize,                 // sizeof(procsinfo)
  //               struct fdsinfo *FileBuffer,      // Array of fdsinfos
  //               int FileSize,                    // sizeof(...)
  //               pid_t *IndexPointer,             // Next PID after call
  //               int Count);                      // How many to retrieve
  
  // Constant for the number of processes wanted in info
  const unsigned int numProcsWanted = 1;
  struct procsinfo procInfoBuf[numProcsWanted];
  struct fdsinfo fdsInfoBuf[numProcsWanted];
  int numProcsReturned;
  // The pid sent to getProcs() is modified, so make a copy
  pid_t wantedPid = proc_->getPid();
  // We really don't need to recalculate the size of the structures
  // every call through here. The compiler should optimize these
  // to constants.
  const int sizeProcInfo = sizeof(struct procsinfo);
  const int sizeFdsInfo = sizeof(struct fdsinfo);
  
  if (lwp_id_ > 0) {
    // Whoops, we _really_ don't want to do this. 
    cerr << "Error: calling software timer routine with a valid kernel thread ID" << endl;
  }
  
  numProcsReturned = getprocs(procInfoBuf,
			      sizeProcInfo,
			      fdsInfoBuf,
			      sizeFdsInfo,
			      &wantedPid,
			      numProcsWanted);
  
  if (numProcsReturned == -1) // We have an error
    perror("Failure in getInferiorCPUtime");

  // Now we have the process table information. Since there is no description
  // other than the header file, I've included descriptions of used fields.
  /* 
     struct  procsinfo
     {
        // valid when the process is a zombie only 
        unsigned long   pi_utime;       / this process user time 
        unsigned long   pi_stime;       / this process system time 
        // accounting and profiling data 
        unsigned long   pi_start;       // time at which process began 
        struct rusage   pi_ru;          // this process' rusage info 
        struct rusage   pi_cru;         // children's rusage info 

     };
  */
  // Other things are included, but we don't need 'em here.
  // In addition, the fdsinfo returned is ignored, since we don't need
  // open file descriptor data.

  // This isn't great, since the returned time is in seconds run. It works
  // (horribly) for now, though. Multiply it by a million and we'll call 
  // it a day. Back to the drawing board.

  // Get the time (user+system?) in seconds
  rawTime64 result = 
    (rawTime64) procInfoBuf[0].pi_ru.ru_utime.tv_sec + // User time
    (rawTime64) procInfoBuf[0].pi_ru.ru_stime.tv_sec;  // System time
  
  result *= I64_C(1000000);
  // It looks like the tv_usec fields are actually nanoseconds in this
  // case. If so, it's undocumented -- but I'm getting numbers like
  // "980000000" which is either 980 _million_ microseconds (i.e. 980sec)
  // or .98 seconds if the units are nanoseconds.

  // IF STRANGE RESULTS HAPPEN IN THE TIMERS, make sure that usec is 
  // actually nanos, not micros.

  rawTime64 nanoseconds = 
    (rawTime64) procInfoBuf[0].pi_ru.ru_utime.tv_usec + // User time
    (rawTime64) procInfoBuf[0].pi_ru.ru_stime.tv_usec; //System time
  result += (nanoseconds / 1000);

  if (result < sw_previous_) // Time ran backwards?
    {
      // When the process exits we often get a final time call.
      // If the result is 0(.0), don't print an error.
      if (result) {
	char errLine[150];
	sprintf(errLine,"process::getRawCpuTime_sw - time going backwards in "
		"daemon - cur: %lld, prev: %lld\n", result, sw_previous_);
	cerr << errLine;
	logLine(errLine);
      }
      result = sw_previous_;
    }
  else sw_previous_=result;
  return result;

}


#endif

#if defined(USES_DYNAMIC_INF_HEAP)
static const Address branch_range = 0x01fffffc;
static const Address lowest_addr = 0x10000000;
static const Address highest_addr = 0xe0000000;
Address data_low_addr;
static const Address data_hi_addr = 0xcfffff00;
// Segment 0 is kernel space, and off-limits
// Segment 1 is text space, and OK
// Segment 2-12 (c) is data space
// Segment 13 (d) is shared library text, and scavenged
// Segment 14 (e) is kernel space, and off-limits
// Segment 15 (f) is shared library data, and we don't care about it.
// However, we can scavenge some space in with the shared libraries.

void process::inferiorMallocConstraints(Address near, Address &lo, 
					Address &hi, inferiorHeapType type)
{
  lo = lowest_addr;
  hi = highest_addr;
  if (near)
  {
      if (near < (lowest_addr + branch_range))
          lo = lowest_addr;
      else
          lo = near - branch_range;
      if (near > (highest_addr - branch_range))
          hi = highest_addr;
      else
          hi = near + branch_range;
  }
  switch (type)
  {
 case dataHeap:
     // mmap, preexisting dataheap constraints
     // so shift down lo and hi accordingly
     if (lo < data_low_addr) {
         lo = data_low_addr;
         // Keep within branch range so that we know we can
         // reach anywhere inside.
         if (hi < (lo + branch_range))
             hi = lo + branch_range;
     }
     if (hi > data_hi_addr) {
         hi = data_hi_addr;
/*
         if (lo > (hi - branch_range))
             lo = hi - branch_range;
*/
     }
     break;
 default:
     // no change
     break;
  }
}

void process::inferiorMallocAlign(unsigned &size)
{
     /* 32 byte alignment.  Should it be 64? */
  size = (size + 0x1f) & ~0x1f;
}
#endif

#ifndef BPATCH_LIBRARY
class instReqNode;

bool process::catchupSideEffect(Frame &frame, instReqNode *inst)
{
  // Okay, here's what we need to do: when a base tramp is
  // entered, we change the link register to point to the
  // exit tramp. We need to repeat that behavior. It
  // consists of:
  //   Get the address of the exit tramp for the current
  //     function.
  //   Write the old LR to the save slot.
  //   Write this address to where the LR is stored (or the 
  //     register if we have a leaf func)
  //   Layout: (starting at 0 and proceeding upwards)
  //        0: stack backchain
  //        4: saved TC
  //        8: saved LR
  //       12: our save slot for the LR

  Address exitTrampAddr;
  pd_Function *instFunc = (pd_Function *)(inst->Point()->pointFunc());
  if (!instFunc) return false;
  // Check: see if the PC is within the instFunc. We might be within
  // an entry or exit tramp, at which point we don't need to fix anything.
  // 27FEB03: if we're before the function _or at the first instruction_ there
  // is no need to fix anything, as we will jump into the base tramp normally
  // 10OCT03: fix up to include in-function call sites as well -- collapse
  // address before checking
  if ((frame.getPC() <= instFunc->get_address()) ||
      (frame.getPC() > instFunc->get_address() + instFunc->get_size()))
    return true;

  const pdvector <instPoint *>exitPoints = instFunc->funcExits(this);
  exitTrampAddr = baseMap[exitPoints[0]]->baseAddr;

  // If the function is a leaf function, we need to overwrite the LR directly.
  bool isLeaf = false;
  bool noFrame = false;
  if (frame.isUppermost()) {
    isLeaf = instFunc->makesNoCalls();
    noFrame = instFunc->hasNoStackFrame();
  }

  if (frame.isLastFrame(this))
      return false;
  Frame parentFrame = frame.getCallerFrame(this);
  Address oldReturnAddr;
  
  if (isLeaf) {
      // Stomp the LR
      dyn_lwp *lwp = frame.getLWP();
      if (lwp && lwp->get_lwp_id()) {
          // Get the current LR and reset it to our new version
#if defined(AIX_PROC)
          dyn_saved_regs regs;
          bool status = lwp->getRegisters(&regs);
          if (!status) {
              bperr( "Failure to get registers in catchupSideEffect\n");
              return false;
          }
          oldReturnAddr = regs.theIntRegs.__lr;
          regs.theIntRegs.__lr = exitTrampAddr;
          if (!lwp->restoreRegisters(regs)) {
              bperr( "Failure to restore registers in catchupSideEffect\n");
              return false;
          }
#else
          struct ptsprs spr_contents;
          Address oldLR;
          if (P_ptrace(PTT_READ_SPRS, lwp->get_lwp_id(),
                       (int *)&spr_contents, 0, 0) == -1) {
              perror("Failed to read SPRS in catchupSideEffect");
              return false;
          }
          oldReturnAddr = spr_contents.pt_lr;
          spr_contents.pt_lr = (unsigned) exitTrampAddr;
          if (P_ptrace(PTT_WRITE_SPRS, lwp->get_lwp_id(),
                       (int *)&spr_contents, 0, 0) == -1) {
              perror("Failed to write SPRS in catchupSideEffect");
              return false;
          }
#endif
      }
      else {
          // Old method
#if defined(AIX_PROC)
          dyn_saved_regs regs;
          bool status = getRepresentativeLWP()->getRegisters(&regs);
          if (!status) {
              bperr("Failure to get registers in catchupSideEffect\n");
              return false;
          }
          oldReturnAddr = regs.theIntRegs.__lr;
          regs.theIntRegs.__lr = exitTrampAddr;
          getRepresentativeLWP()->restoreRegisters(regs);
#else
          oldReturnAddr = P_ptrace(PT_READ_GPR, pid, (void *)LR, 0, 0);
          if (oldReturnAddr == -1)
          {
              perror("Failed to read LR in catchupSideEffect");
          }
          if (P_ptrace(PT_WRITE_GPR, pid, (void *)LR, exitTrampAddr, 0) == -1) {
              perror("Failed to write LR in catchupSideEffect");
              return false;
          }
#endif
      }
  }
  else {    
      // The LR is stored in the parent's frame to allow functions to
      // store the LR without creating a stack frame. 
      readDataSpace((void *)(parentFrame.getFP()+8), sizeof(Address), 
                    &oldReturnAddr, false);
      if (oldReturnAddr == exitTrampAddr) {
          // We must've already overwritten the link register, so we're fine 
          return true;
      }
      else { 
          // Write it into the save slot
          writeDataSpace((void*)(parentFrame.getFP()+8), sizeof(Address), 
                         &exitTrampAddr);
      }
  }
  // And write the actual LR into an unused word.

  // We can always use the "parent" stack frame -- if the current function
  // has not made a frame yet, then the two stack pointers will be
  // identical.
  
  writeDataSpace((void*)(parentFrame.getFP()+12), sizeof(Address), &oldReturnAddr);
  // And write the "we've modified the LR" trigger value
  Address trigger = MODIFIED_LR;
  writeDataSpace((void*)(parentFrame.getFP()+16), sizeof(Address), &trigger);

  return true;
  
}
#endif

#define DEBUG_MSG 0 
#define _DEBUG_MSG 0
void compactLoadableSections(pdvector <imageUpdate*> imagePatches, pdvector<imageUpdate*> &newPatches){
	int startPage, stopPage;
	imageUpdate *patch;
	//this function now returns only ONE section that is loadable.
	int pageSize = getpagesize();

	imageUpdate *curr, *next;
	bool foundDup=true;
	unsigned int j;

	VECTOR_SORT(imagePatches, imageUpdateSort);

	while(foundDup){
		foundDup = false;
		j =0;
	        while(imagePatches[j]->address==0 && j < imagePatches.size()){
       	        	j++;
        	}
		curr = imagePatches[j];

		for(j++;j<imagePatches.size();j++){
			next = imagePatches[j];		
			if(curr->address == next->address){
				//duplicate
				//find which is bigger and save that one.
				if(curr->size > next->size){
					next->address=0;
				}else{
					curr->address=0;
					curr=next;
				}
				foundDup =true;
			}else{
				curr=next;
			}

		}
		VECTOR_SORT(imagePatches, imageUpdateSort);
	}


	for(unsigned int i=0;i<imagePatches.size();i++){
		if(imagePatches[i]->address!=0){
			imagePatches[i]->startPage = imagePatches[i]->address- imagePatches[i]->address%pageSize;
			imagePatches[i]->stopPage = imagePatches[i]->address + imagePatches[i]->size- 
					(imagePatches[i]->address + imagePatches[i]->size )%pageSize;

		}
	}

	foundDup = true;

	while(foundDup){
		foundDup = false;
                j =0;
                while(imagePatches[j]->address==0 && j < imagePatches.size()){
                        j++;
                }
		imagePatches.erase(0,j-1);
		j=0;
		for(;j<imagePatches.size()-1;j++){
			if(imagePatches[j]->stopPage > imagePatches[j+1]->startPage){
				foundDup = true;
				if(imagePatches[j]->stopPage > imagePatches[j+1]->stopPage){
					imagePatches[j+1]->address = 0;	
				}else{

					imagePatches[j]->size = (imagePatches[j+1]->address + imagePatches[j+1]->size) -
						imagePatches[j]->address;
					imagePatches[j+1]->address = 0; 
					imagePatches[j]->stopPage = imagePatches[j]->address + imagePatches[j]->size-
                                        	(imagePatches[j]->address + imagePatches[j]->size )%pageSize;		
				}
			}  
		}
		VECTOR_SORT(imagePatches, imageUpdateSort);
	}

	unsigned int k=0;

	while(imagePatches[k]->address==0 && k < imagePatches.size()){
	        k++;
        }

	startPage = imagePatches[k]->startPage;
	stopPage = imagePatches[imagePatches.size()-1]->stopPage;
	int startIndex=k, stopIndex=imagePatches.size()-1;
	/*if(DEBUG_MSG){
		bperr("COMPACTING....\n");	
		bperr("COMPACTING %x %x %x\n", imagePatches[0]->startPage, stopPage, imagePatches[0]->address);
	}
	patch = new imageUpdate;
        patch->address = imagePatches[startIndex]->address;
        patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address +
                                   imagePatches[stopIndex]->size;
        newPatches.push_back(patch);
	if(DEBUG_MSG){
		bperr(" COMPACTED: %x --> %x \n", patch->address, patch->size);
	}*/
	bool finished = false;
	if(_DEBUG_MSG){
		bperr("COMPACTING....\n");	
		bperr("COMPACTING %x %x %x\n", imagePatches[0]->startPage, stopPage, imagePatches[0]->address);
	}
	for(;k<imagePatches.size();k++){
		if(imagePatches[k]->address!=0){
			if(_DEBUG_MSG){
				bperr("COMPACTING k[start] %x k[stop] %x stop %x addr %x size %x\n", imagePatches[k]->startPage, 
					imagePatches[k]->stopPage,stopPage, imagePatches[k]->address, imagePatches[k]->size);
			}
			if(imagePatches[k]->startPage <= (unsigned int)stopPage){
				stopIndex = k;
				stopPage = imagePatches[k]->stopPage;
			}else{

				patch = new imageUpdate;
				patch->address = imagePatches[startIndex]->address;
				patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address + 
						imagePatches[stopIndex]->size;
				newPatches.push_back(patch);
				if(_DEBUG_MSG){
					bperr(" COMPACTED: address %x --> %x    start %x  stop %x\n", 
						patch->address, patch->size, startPage,  stopPage);
				}
				finished = true;
				//was k+1	
				if(k < imagePatches.size()){
					while(imagePatches[k]->address==0 && k < imagePatches.size()){
						k++;
					}
					startIndex = k;
					stopIndex = k;
					startPage = imagePatches[k]->startPage;
					stopPage  = imagePatches[k]->stopPage;
					finished = false;
					if(k == imagePatches.size()){
						finished = true;
					}
				} 
			}
		}

	}

	if(!finished){
		patch = new imageUpdate;
                patch->address = imagePatches[startIndex]->address;
                patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address +
                                   imagePatches[stopIndex]->size;
                newPatches.push_back(patch);
		if(_DEBUG_MSG){
			bperr(" COMPACTED: %x --> %x \n", patch->address, patch->size);
			fflush(stdout);
		}
	}	

	
}

void compactSections(pdvector <imageUpdate*> imagePatches, pdvector<imageUpdate*> &newPatches){

	unsigned startPage, stopPage;
	imageUpdate *patch;

	int pageSize = getpagesize();

	imageUpdate *curr, *next;
	bool foundDup=true;
	unsigned int j;

	VECTOR_SORT(imagePatches, imageUpdateSort);

	while(foundDup){
		foundDup = false;
		j =0;
	        while(imagePatches[j]->address==0 && j < imagePatches.size()){
       	        	j++;
        	}
		curr = imagePatches[j];

		for(j++;j<imagePatches.size();j++){
			next = imagePatches[j];		
			if(curr->address == next->address){
				//duplicate
				//find which is bigger and save that one.
				if(curr->size > next->size){
					next->address=0;
				}else{
					curr->address=0;
					curr=next;
				}
				foundDup =true;
			}else{
				curr=next;
			}

		}
		VECTOR_SORT(imagePatches, imageUpdateSort);
	}
	if(DEBUG_MSG){
		bperr(" SORT 1 %d \n", imagePatches.size());
	
		for(unsigned int kk=0;kk<imagePatches.size();kk++){
			bperr("%d address 0x%x  size 0x%x \n",kk, imagePatches[kk]->address, imagePatches[kk]->size);
		}
		fflush(stdout);
	}

	unsigned int endAddr;
	for(unsigned int i=0;i<imagePatches.size();i++){
		if(imagePatches[i]->address!=0){
			imagePatches[i]->startPage = imagePatches[i]->address- (imagePatches[i]->address%pageSize);
				
			endAddr = imagePatches[i]->address + imagePatches[i]->size;
			imagePatches[i]->stopPage =  endAddr - (endAddr % pageSize);

			if(DEBUG_MSG){
				bperr("%d address %x end addr %x : start page %x stop page %x \n",
					i,imagePatches[i]->address ,imagePatches[i]->address + imagePatches[i]->size,
					imagePatches[i]->startPage, imagePatches[i]->stopPage);
			}
		}
	
	}
	foundDup = true;

	while(foundDup){
		foundDup = false;
                j =0;
                while(imagePatches[j]->address==0 && j < imagePatches.size()){
                        j++;
                }
		//imagePatches.erase(0,j-1); //is it correct to erase here? 
		//j = 0;
		for(;j<imagePatches.size()-1;j++){ 
			if(imagePatches[j]->address!=0 && imagePatches[j]->stopPage >= imagePatches[j+1]->startPage){
				foundDup = true;
				if(imagePatches[j]->stopPage > imagePatches[j+1]->stopPage){
					imagePatches[j+1]->address = 0;	
				}else{
					imagePatches[j]->size = (imagePatches[j+1]->address + imagePatches[j+1]->size) -
						imagePatches[j]->address;
					imagePatches[j+1]->address = 0; 
					endAddr = imagePatches[j]->address + imagePatches[j]->size;
					imagePatches[j]->stopPage =  endAddr - (endAddr % pageSize);
				}
			}  
		}
		VECTOR_SORT(imagePatches, imageUpdateSort);
	}

	unsigned int k=0;

	if(DEBUG_MSG){
		bperr(" SORT 3 %d \n", imagePatches.size());

		for(unsigned int kk=0;kk<imagePatches.size();kk++){
			bperr("%d address 0x%x  size 0x%x \n",kk, imagePatches[kk]->address, imagePatches[kk]->size);
		}
		fflush(stdout);
	}
	while(imagePatches[k]->address==0 && k < imagePatches.size()){
	        k++;
        }

	startPage = imagePatches[k]->startPage;
	stopPage = imagePatches[k]->stopPage;
	int startIndex=k, stopIndex=k;
	bool finished = false;
	if(DEBUG_MSG){
		bperr("COMPACTING....\n");	
		bperr("COMPACTING %x %x %x\n", imagePatches[0]->startPage, stopPage, imagePatches[0]->address);
	}
	for(;k<imagePatches.size();k++){
		if(imagePatches[k]->address!=0){
			if(DEBUG_MSG){
				bperr("COMPACTING k[start] %x k[stop] %x stop %x addr %x size %x\n", imagePatches[k]->startPage, 
					imagePatches[k]->stopPage,stopPage, imagePatches[k]->address, imagePatches[k]->size);
			}
			if(imagePatches[k]->startPage <= (unsigned int) stopPage){
				stopIndex = k;
				stopPage = imagePatches[k]->stopPage;
			}else{

				patch = new imageUpdate;
				patch->address = imagePatches[startIndex]->address;
				patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address + 
						imagePatches[stopIndex]->size;
				newPatches.push_back(patch);
				if(DEBUG_MSG){
					bperr(" COMPACTED: address %x --> %x    start %x  stop %x\n", 
						patch->address, patch->size, startPage,  stopPage);
				}
				finished = true;
				//was k+1	
				if(k < imagePatches.size()){
					while(imagePatches[k]->address==0 && k < imagePatches.size()){
						k++;
					}
					startIndex = k;
					stopIndex = k;
					startPage = imagePatches[k]->startPage;
					stopPage  = imagePatches[k]->stopPage;
					finished = false;
					if(k == imagePatches.size()){
						finished = true;
					}
				} 
			}
		}

	}

	if(!finished){
		patch = new imageUpdate;
                patch->address = imagePatches[startIndex]->address;
                patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address +
                                   imagePatches[stopIndex]->size;
                newPatches.push_back(patch);
		if(DEBUG_MSG){
			bperr(" COMPACTED: %x --> %x \n", patch->address, patch->size);
		}
	}	
	
}


void process::addLib(char* lname){

	BPatch_thread *appThread = bpatch_thread;
	BPatch_image *appImage = appThread->getImage();

   BPatch_Vector<BPatch_point *> *mainFunc;

	bool isTrampRecursive = BPatch::bpatch->isTrampRecursive();
    BPatch::bpatch->setTrampRecursive( true ); //ccw 31 jan 2003
    BPatch_Vector<BPatch_function *> bpfv;
    if (NULL == appImage->findFunction("main", bpfv) || !bpfv.size()) { 
      bperr("Unable to find function \"main\". Save the world will fail.\n");
      return;
   }

   BPatch_function *mainFuncPtr =bpfv[0];
   mainFunc = mainFuncPtr->findPoint(BPatch_entry);
    
   if (!mainFunc || ((*mainFunc).size() == 0)) {
      bperr( "    Unable to find entry point to \"main.\"\n");
      exit(1);
   }

   bpfv.clear();
   if (NULL == appImage->findFunction("dlopen", bpfv) || !bpfv.size()) {
      bperr("Unable to find function \"dlopen\". Save the world will fail.\n");
      return;
   }
   BPatch_function *dlopen_func = bpfv[0];
   
   BPatch_Vector<BPatch_snippet *> dlopen_args;
   BPatch_constExpr nameArg(lname);
   BPatch_constExpr rtldArg(4);
   
   dlopen_args.push_back(&nameArg);
   dlopen_args.push_back(&rtldArg);
   
   BPatch_funcCallExpr dlopenExpr(*dlopen_func, dlopen_args);
   
	//bperr(" inserting DLOPEN(%s)\n",lname);
	requestTextMiniTramp = 1;
   
   appThread->insertSnippet(dlopenExpr, *mainFunc, BPatch_callBefore,
                            BPatch_firstSnippet);
	requestTextMiniTramp = 0;
   
	BPatch::bpatch->setTrampRecursive( isTrampRecursive ); //ccw 31 jan 2003
}


//save world
char* process::dumpPatchedImage(pdstring imageFileName){ //ccw 28 oct 2001

	writeBackXCOFF *newXCOFF;
	//addLibrary *addLibraryXCOFF;
	//char name[50];	
	pdvector<imageUpdate*> compactedUpdates;
	pdvector<imageUpdate*> compactedHighmemUpdates;
	void *data;//, *paddedData;
	//Address guardFlagAddr;
	char *directoryName = 0;

	if(!collectSaveWorldData){
		BPatch_reportError(BPatchSerious,122,"dumpPatchedImage: BPatch_thread::enableDumpPatchedImage() not called.  No mutated binary saved\n");
		return NULL;
	}

	directoryName = saveWorldFindDirectory();
	if(!directoryName){
		return NULL;
	}
	strcat(directoryName, "/");


	//at this point build an ast to call dlopen("libdyninstAPI_RT.so.1",);
	//and insert it at the entry point of main.

	addLib("libdyninstAPI_RT.so.1");

	
	imageUpdates.sort(imageUpdateSort);// imageUpdate::mysort ); 

	compactLoadableSections(imageUpdates,compactedUpdates);

	highmemUpdates.sort( imageUpdateSort);
	if(highmemUpdates.size() > 0){
		compactSections(highmemUpdates, compactedHighmemUpdates);
	}

	imageFileName = "dyninst_mutatedBinary";
	char* fullName =
      new char[strlen(directoryName) + strlen(imageFileName.c_str())+1];
   strcpy(fullName, directoryName);
   strcat(fullName, imageFileName.c_str());

	bool openFileError;

	newXCOFF = new writeBackXCOFF( (char *)getImage()->file().c_str(), fullName /*"/tmp/dyninstMutatee"*/ , openFileError);

	if( openFileError ){
		delete [] fullName;
		return NULL;
	}
	newXCOFF->registerProcess(this);
	//int sectionsAdded = 0;
	//unsigned int newSize, nextPage, paddedDiff;
	//unsigned int pageSize = getpagesize();


	//This adds the LOADABLE HEAP TRAMP sections
	//AIX/XCOFF NOTES:
	//On AIX we allocate the heap tramps in two locations: on the heap
	//(0x20000000) and around the text section (0x10000000) The os loader will
	//ONLY load ONE text section, ONE data section and ONE bss section. We
	//cannot (from within the mutated binary) muck with addresses in the range
	//0x10000000 - 0x1fffffff so to reload these tramps we MUST expand the
	//text section and tack these on the end.  THIS WILL INCREASE THE FILE
	//SIZE BY A HUGE AMOUNT.  The file size will increase by (sizeof(text
	//section) + sizeof(tramps) + (gap between text section and tramps)) the
	//gap may be quite large

	//SO we do NOT do what we do on the other platforms, ie work around the
	//heap with the compactedUpdates. we just expand the text section and 
	//tack 'em on the end.

	assert(compactedUpdates.size() < 2);
	(char*) data = new char[compactedUpdates[0]->size];
	readDataSpace((void*) compactedUpdates[0]->address,
                 compactedUpdates[0]->size, data, true);	

	newXCOFF->attachToText(compactedUpdates[0]->address,
                          compactedUpdates[0]->size, (char*)data);

	if(compactedHighmemUpdates.size() > 0){
		saveWorldCreateHighMemSections(compactedHighmemUpdates, highmemUpdates,
                                     (void*) newXCOFF);
	}
	saveWorldAddSharedLibs((void*) newXCOFF);

   saveWorldCreateDataSections((void*)newXCOFF);

	newXCOFF->createXCOFF();
	newXCOFF->outputXCOFF();
/*
	char* fullName = new char[strlen(directoryName) + strlen ( (char*)imageFileName.c_str())+1];
   strcpy(fullName, directoryName);
   strcat(fullName, (char*)imageFileName.c_str());
   
   addLibraryXCOFF= new addLibrary(fullName, "/tmp/dyninstMutatee",
                                   "libdyninstAPI_RT.so.1");

	addLibraryXCOFF->outputXCOFF();
*/
   delete [] fullName;

	return directoryName;
}

// should use demangle.h here, but header is badly broken on AIX 5.1
typedef void *Name;
typedef enum { VirtualName, MemberVar, Function, MemberFunction, Class,
               Special, Long } NameKind;
typedef enum { RegularNames = 0x1, ClassNames = 0x2, SpecialNames = 0x4,
               ParameterText = 0x8, QualifierText = 0x10 } DemanglingOptions;

Name *(*P_native_demangle)(char *, char **, unsigned long) = NULL;
char *(*P_functionName)(Name *) = NULL;
char *(*P_varName)(Name *) = NULL;
char *(*P_text)(Name *) = NULL;
NameKind (*P_kind)(Name *) = NULL;

void loadNativeDemangler() 
{
   char *buffer[1024];

   P_native_demangle = NULL;
   
   void *hDemangler = dlopen("libdemangle.so.1", RTLD_LAZY|RTLD_MEMBER);
   if (hDemangler != NULL) {
      P_native_demangle = (Name*(*)(char*, char**, long unsigned int)) dlsym(hDemangler, "demangle");
      if (!P_native_demangle) 
         BPatch_reportError(BPatchSerious,122,
                   "unable to locate function demangle in libdemangle.so.1\n");

      P_functionName = (char*(*)(Name*)) dlsym(hDemangler, "functionName");
      if (!P_functionName) 
         BPatch_reportError(BPatchSerious,122,
               "unable to locate function functionName in libdemangle.so.1\n");
      
      P_varName = (char*(*)(Name*)) dlsym(hDemangler, "varName");
      if (!P_varName) 
         BPatch_reportError(BPatchSerious,122,
                    "unable to locate function varName in libdemangle.so.1\n");

      P_kind = (NameKind(*)(Name*)) dlsym(hDemangler, "kind");
      if (!P_kind) 
         BPatch_reportError(BPatchSerious,122,
                            "unable to locate function kind in libdemangle.so.1\n");
      
      P_text = (char*(*)(Name*)) dlsym(hDemangler, "text");
      if (!P_text) 
         BPatch_reportError(BPatchSerious,122,
                       "unable to locate function text in libdemangle.so.1\n");
   } else {
#if defined(BPATCH_LIBRARY)
      BPatch_reportError(BPatchSerious,122,"unable to load xlC external demangler libdemangle.so.1\n");
#else
      cerr << "unable to load external demangler libdemangle.so.1\n";
#endif
   }
}


extern "C" char *cplus_demangle(char *, int);
extern void dedemangle( const char * demangled, char * dedemangled );

#define DMGL_PARAMS      (1 << 0)       /* Include function args */
#define DMGL_ANSI        (1 << 1)       /* Include const, volatile, etc */

char * P_cplus_demangle( const char * symbol, bool nativeCompiler, bool includeTypes ) {
	/* If the symbol isn't from the native compiler, or the native demangler
	   isn't available, use the built-in. */
	bool nativeDemanglerAvailable =	P_cplus_demangle != NULL &&
									P_text != NULL &&
									P_varName != NULL &&
									P_functionName != NULL;
	if( !nativeCompiler || ! nativeDemanglerAvailable ) {
		char * demangled = cplus_demangle( const_cast<char *>(symbol),
					includeTypes ? DMGL_PARAMS | DMGL_ANSI : 0 );
		if( demangled == NULL ) { return NULL; }

		if( ! includeTypes ) {
			/* De-demangling never makes a string longer. */
			char * dedemangled = strdup( demangled );
			assert( dedemangled != NULL );

			dedemangle( demangled, dedemangled );
			assert( dedemangled != NULL );

			free( demangled );
			return dedemangled;
			}

		return demangled;
   } /* end if not using native demangler. */
   else if( nativeDemanglerAvailable ) {
		/* Use the native demangler, which apparently behaves funny. */
		Name * name;
		char * rest;
		
		/* P_native_demangle() won't actually demangled 'symbol'.
		   Find out what P_kind() of symbol it is and demangle from there. */
		name = (P_native_demangle)( const_cast<char*>(symbol), (char **) & rest,
			RegularNames | ClassNames | SpecialNames | ParameterText | QualifierText );
		if( name == NULL ) { return NULL; }

		char * demangled = NULL;
		switch( P_kind( name ) ) {
			case Function:
				demangled = (P_functionName)( name );			
				break;
			
			case MemberFunction:
				/* Doing it this way preserves the leading classnames. */
				demangled = (P_text)( name );
				break;

			case MemberVar:
				demangled = (P_varName)( name );
				break;

			case VirtualName:
			case Class:
			case Special:
			case Long:
				demangled = (P_text)( name );
				break;
			default: assert( 0 );
			} /* end P_kind() switch */

		/* Potential memory leak: no P_erase( name ) call.  Also, the
		   char *'s returned from a particular Name will be freed
		   when that name is erase()d or destroyed,	so strdup if we're
		   fond of them. */
   
		if( ! includeTypes ) {
			/* De-demangling never makes a string longer. */
			char * dedemangled = strdup( demangled );
			assert( dedemangled != NULL );

			dedemangle( demangled, dedemangled );
			assert( dedemangled != NULL );

			return dedemangled;
			}

		return demangled;
		} /* end if using native demangler. */
	else {
		/* We're trying to demangle a native binary but the native demangler isn't available.  Punt. */	
		return NULL;
		}
	} /* end P_cplus_demangle() */



#include <dlfcn.h> // dlopen constants

#define DLOPEN_MODE (RTLD_NOW | RTLD_GLOBAL)
extern void generateBreakPoint(instruction &);

/*************************************************************************/
/***  Code to handle dlopen()ing the runtime library                   ***/
/***                                                                   ***/
/***  get_dlopen_addr() -- return the address of the dlopen function   ***/
/***  Address dyninstlib_brk_addr -- address of the breakpoint at the  ***/
/***                                 end of the RT init function       ***/
/***  Address main_brk_addr -- address when we switch to dlopen()ing   ***/
/***                           the RT lib                              ***/
/***  loadDYNINSTlib() -- Write the (string) name of the RT lib,     ***/
/***                        set up and execute a call to dlopen()      ***/
/***  trapDueToDyninstLib() -- returns true if trap addr is at         ***/
/***                          dyninstlib_brk_addr                      ***/
/***  trapAtEntryPointOfMain() -- see above                            ***/
/***  handleIfDueToDyninstLib -- cleanup function                      ***/
/***  handleTrapAtEntryPointOfMain -- cleanup function                 ***/
/***  insertTrapAtEntryPointOfMain -- insert a breakpoint at the start ***/
/***                                  of main                          ***/
/*************************************************************************/


/* Auxiliary function */

bool checkAllThreadsForBreakpoint(process *proc, Address break_addr)
{
  pdvector<Frame> activeFrames;
  if (!proc->getAllActiveFrames(activeFrames)) return false;
  for (unsigned frame_iter = 0; frame_iter < activeFrames.size(); frame_iter++)
    if (activeFrames[frame_iter].getPC() == break_addr) {
      return true;
    }
  return false;
}

bool process::trapDueToDyninstLib()
{
  // Since this call requires a PTRACE, optimize it slightly
  if (dyninstlib_brk_addr == 0x0) return false;

  bool result = checkAllThreadsForBreakpoint(this, dyninstlib_brk_addr);
  if(result){ 
    dyninstlib_brk_addr = 0; //ccw 30 apr 2002 : SPLIT3
    //dyninstlib_brk_addr and paradynlib_brk_addr may be the same
    //if they are we dont want to get them mixed up. once we
    //see this trap is due to dyninst, reset the addr so
    //we can now catch the paradyn trap
  }
  return result;
  
}

bool process::trapAtEntryPointOfMain(Address)
{
  // Since this call requires a PTRACE, optimize it slightly
  // This won't trigger (ever) if we are attaching, btw.
  if (main_brk_addr == 0x0) return false;
  return checkAllThreadsForBreakpoint(this, main_brk_addr);
}

/*
 * Restore "the original instruction" written into main so that
 * we can proceed after the trap. Saved in "savedCodeBuffer",
 * which is a chunk of space we use for dlopening the RT library.
 */

bool process::handleTrapAtEntryPointOfMain()
{
    
    if (!main_brk_addr) return false;
    // Put back the original insn
    if (!writeDataSpace((void *)main_brk_addr, 
                        sizeof(instruction), (char *)savedCodeBuffer))
        return false;
    
    // And zero out the main_brk_addr so we don't accidentally
    // trigger on it.
    main_brk_addr = 0x0;
    return true;
}

/*
 * Stick a trap at the entry point of main. At this point,
 * libraries are mapped into the proc's address space, and
 * we can dlopen the RT library.
 */

bool process::insertTrapAtEntryPointOfMain()
{
  pd_Function *f_main = (pd_Function *) findOnlyOneFunction("main");
  if (!f_main) {
    // we can't instrument main - naim
    showErrorCallback(108,"main() uninstrumentable");
    return false;
  }
  assert(f_main);
  Address addr = f_main->getEffectiveAddress(this);
  // save original instruction first
  readDataSpace((void *)addr, sizeof(instruction), savedCodeBuffer, true);
  // and now, insert trap
  instruction insnTrap;
  generateBreakPoint(insnTrap);
  writeDataSpace((void *)addr, sizeof(instruction), (char *)&insnTrap);  
  main_brk_addr = addr;
  
  return true;
}

bool process::getDyninstRTLibName() {
    if (dyninstRT_name.length() == 0) {
        // Get env variable
        if (getenv("DYNINSTAPI_RT_LIB") != NULL) {
            dyninstRT_name = getenv("DYNINSTAPI_RT_LIB");
        }
        else {
           pdstring msg = pdstring("Environment variable ")
              + pdstring("DYNINSTAPI_RT_LIB")
              + pdstring(" has not been defined for process ")
              + pdstring(pid);
           showErrorCallback(101, msg);
           return false;
        }
    }
    // Check to see if the library given exists.
    if (access(dyninstRT_name.c_str(), R_OK)) {
        pdstring msg = pdstring("Runtime library ") + dyninstRT_name
        + pdstring(" does not exist or cannot be accessed!");
        showErrorCallback(101, msg);
        return false;
    }
    return true;
}


  

/*
 * loadDYNINSTlib()
 *
 * The evil black magic function. What we want: for the runtime
 * library to show up in the process' address space. Magically.
 * No such luck. What we do: patch in a call to dlopen(DYNINSTRT_NAME)
 * at the entry of main, then restore the original instruction
 * and continue.
 */

extern void pushStack(char *i, Address &base);
extern void popStack(char *i, Address &base);

bool process::loadDYNINSTlib()
{
  // This is actually much easier than on other platforms, since
  // by now we have the DYNINSTstaticHeap symbol to work with.
  // Basically, we can ptrace() anywhere in the text heap we want to,
  // so go somewhere untouched. Unfortunately, we haven't initialized
  // the tramp space yet (no point except on AIX) so we can't simply
  // call inferiorMalloc(). 

  // However, if we can get code_len_ + code_off_ from the object file,
  // then we can use the area above that point freely.

  // Steps: Get the library name (command line or ENV)
  //        Get the address for dlopen()
  //        Write in a call to dlopen()
  //        Write in a trap after the call
  //        Write the library name somewhere where dlopen can find it.
  // Actually, why not write the library name first?

  const Object binaryFile = symbols->getObject();
  Address codeBase = binaryFile.code_off() + binaryFile.code_len();
  // Round it up to the nearest instruction. 
  codeBase += sizeof(instruction) - (codeBase % sizeof(instruction));


  int count = 0; // how much we've written
  unsigned char scratchCodeBuffer[BYTES_TO_SAVE]; // space
  Address dyninstlib_addr;
  Address dlopencall_addr;
  Address dlopentrap_addr;

  // Do we want to save whatever is there? Can't see a reason why...

  // write library name...
  dyninstlib_addr = (Address) (codeBase + count);
  writeDataSpace((void *)(codeBase + count), dyninstRT_name.length()+1,
		 (caddr_t)const_cast<char*>(dyninstRT_name.c_str()));
  count += dyninstRT_name.length()+sizeof(instruction); // a little padding

  // Actually, we need to bump count up to a multiple of insnsize
  count += sizeof(instruction) - (count % sizeof(instruction));

  // Need a register space
  // make sure this syncs with inst-power.C
  Register liveRegList[] = {10, 9, 8, 7, 6, 5, 4, 3};
  Register deadRegList[] = {11, 12};
  unsigned liveRegListSize = sizeof(liveRegList)/sizeof(Register);
  unsigned deadRegListSize = sizeof(deadRegList)/sizeof(Register);

  registerSpace *dlopenRegSpace = new registerSpace(deadRegListSize, deadRegList, 
						    liveRegListSize, liveRegList);
  dlopenRegSpace->resetSpace();

  Address dyninst_count = 0; // size of generated code
  pdvector<AstNode*> dlopenAstArgs(2);
  AstNode *dlopenAst;

  dlopenAstArgs[0] = new AstNode(AstNode::Constant, (void *)(dyninstlib_addr));
  dlopenAstArgs[1] = new AstNode(AstNode::Constant, (void*)DLOPEN_MODE);
  dlopenAst = new AstNode("dlopen", dlopenAstArgs);
  removeAst(dlopenAstArgs[0]);
  removeAst(dlopenAstArgs[1]);
  // We need to push down the stack before we call this
  pushStack((char *)scratchCodeBuffer, dyninst_count);
  dlopenAst->generateCode(this, dlopenRegSpace, (char *)scratchCodeBuffer,
			  dyninst_count, true, true);
  popStack((char *)scratchCodeBuffer, dyninst_count);
  dlopencall_addr = codeBase + count;
  writeDataSpace((void *)dlopencall_addr, dyninst_count, 
		 (char *)scratchCodeBuffer);
  removeAst(dlopenAst);
  count += dyninst_count;

  instruction insnTrap;
  generateBreakPoint(insnTrap);
  dlopentrap_addr = codeBase + count;
  writeDataSpace((void *)dlopentrap_addr, sizeof(instruction),
		 (void *)(&insnTrap.raw));
  count += sizeof(instruction);

  dyninstlib_brk_addr = dlopentrap_addr;

  // save registers
  assert(savedRegs == NULL);
  savedRegs = new dyn_saved_regs;
  bool status = getRepresentativeLWP()->getRegisters(savedRegs);
  assert((status!=false) && (savedRegs!=(void *)-1));

  if (!getRepresentativeLWP()->changePC(dlopencall_addr, NULL)) {
    logLine("WARNING: changePC failed in loadDYNINSTlib\n");
    assert(0);
  }

  setBootstrapState(loadingRT);
  return true;
}

/*
 * Cleanup after dlopen()ing the runtime library. Since we don't overwrite
 * any existing functions, just restore saved registers. Cool, eh?
 */

bool process::loadDYNINSTlibCleanup()
{
  getRepresentativeLWP()->restoreRegisters(*savedRegs);
  delete savedRegs;
  savedRegs = NULL;
  // We was never here.... 
  
  // But before we go, reset the dyninstlib_brk_addr so we don't
  // accidentally trigger it, eh?
  dyninstlib_brk_addr = 0x0;
  return true;
}


//////////////////////////////////////////////////////////
// AIX /proc compatibility section
/////////////////////////////////////////////////////////

#define GETREG_GPR(regs,reg)   (regs.__gpr[reg])
// AIX system calls can vary in name and number. We need a way
// to decode this mess. So what we do is #define the syscalls we 
// want to numbers and use those to index into a mapping array.
// The first time we try and map a syscall we fill the array in.

int SYSSET_MAP(int syscall, int pid)
{
    static int syscall_mapping[NUM_SYSCALLS];
    static bool mapping_valid = false;
    
    if (mapping_valid)
        return syscall_mapping[syscall];
    
    for (int i = 0; i < NUM_SYSCALLS; i++)
        syscall_mapping[i] = -1;
    
    // Open and read the sysent file to find exit, fork, and exec.
    prsysent_t sysent;
    prsyscall_t *syscalls;
    int fd;
    char filename[256];
    char syscallname[256];
    sprintf(filename, "/proc/%d/sysent", pid);
    fd = open(filename, O_RDONLY, 0);
    if (read(fd, &sysent,
             sizeof(sysent) - sizeof(prsyscall_t))
        != sizeof(sysent) - sizeof(prsyscall_t))
        perror("AIX syscall_map: read");
    syscalls = (prsyscall_t *)malloc(sizeof(prsyscall_t)*sysent.pr_nsyscalls);
    if (read(fd, syscalls,
             sizeof(prsyscall_t)*sysent.pr_nsyscalls) !=
        (int) sizeof(prsyscall_t)*sysent.pr_nsyscalls)
        perror("AIX syscall_map: read2");
    for (unsigned int j = 0; j < sysent.pr_nsyscalls; j++) {
        lseek(fd, syscalls[j].pr_nameoff, SEEK_SET);
        read(fd, syscallname, 256);
        // Now comes the interesting part. We're interested in a list of
        // system calls. Compare the freshly read name to the list, and if
        // there is a match then set the syscall mapping.
        if (!strcmp(syscallname, "_exit")) {
            syscall_mapping[SYS_exit] = syscalls[j].pr_number;
        }
        else if (!strcmp(syscallname, "kfork")) {
            syscall_mapping[SYS_fork] = syscalls[j].pr_number;
        }
        else if (!strcmp(syscallname, "execve")) {    
            syscall_mapping[SYS_exec] = syscalls[j].pr_number;
        }
        
    }
    close(fd);
    free(syscalls);
    mapping_valid = true;
    return syscall_mapping[syscall];
}

// Bleah...
unsigned SYSSET_SIZE(sysset_t *x)
{
    // (pr_size - 1) because sysset_t is one uint64_t too large
    return sizeof(sysset_t) + (sizeof (uint64_t) * (x->pr_size-1));
}

sysset_t *SYSSET_ALLOC(int pid)
{
    static bool init = false;
    static int num_calls = 0;
    if (!init) {
        prsysent_t sysent;
        int fd;
        char filename[256];
        sprintf(filename, "/proc/%d/sysent", pid);
        fd = open(filename, O_RDONLY, 0);
        if (read(fd, &sysent,
                 sizeof(sysent) - sizeof(prsyscall_t))
            != (int) (sizeof(sysent) - sizeof(prsyscall_t)))
            perror("AIX syscall_alloc: read");
        num_calls = sysent.pr_nsyscalls;
        init = true;
        close(fd);
    }
    int size = 0; // Number of 64-bit ints we use for the bitmap
    // array size (*8 because we're bitmapping)
    size = ((num_calls / (8*sizeof(uint64_t))) + 1);
    sysset_t *ret = (sysset_t *)malloc(sizeof(sysset_t) 
                                       - sizeof(uint64_t) 
                                       + size*sizeof(uint64_t));

    ret->pr_size = size;
    
    return ret;
}

/*
 * The set operations (set_entry_syscalls and set_exit_syscalls) are defined
 * in sol_proc.C
 */

bool process::get_entry_syscalls(sysset_t *entry)
{
    pstatus_t status;
    if (!get_status(&status)) return false;
    
    // If the offset is 0, no syscalls are being traced
    if (status.pr_sysentry_offset == 0) {
        premptysysset(entry);
    }
    else {
        // The entry member of the status vrble is a pointer
        // to the sysset_t array.
        if(pread(getRepresentativeLWP()->status_fd(), entry, 
                 SYSSET_SIZE(entry), status.pr_sysentry_offset)
           != (int) SYSSET_SIZE(entry)) {
            perror("get_entry_syscalls: read");
            return false;
        }
    }
    return true;
}

bool process::get_exit_syscalls(sysset_t *exit)
{
    pstatus_t status;
    if (!get_status(&status)) return false;

    // If the offset is 0, no syscalls are being traced
    if(status.pr_sysexit_offset == 0) {
        premptysysset(exit);
    }
    else {
        if(pread(getRepresentativeLWP()->status_fd(), exit, 
                 SYSSET_SIZE(exit), status.pr_sysexit_offset)
           != (int) SYSSET_SIZE(exit)) {
            perror("get_exit_syscalls: read");
            return false;
        }
    }
    return true;
}



bool process::dumpCore_(const pdstring coreFile)
{
    pause();
    
    if (!dumpImage(coreFile))
        return false;
    
    continueProc();
    return true;
}

bool process::dumpImage(const pdstring outFile)
{
    // formerly OS::osDumpImage()
    const pdstring &imageFileName = symbols->file();
    // const Address codeOff = symbols->codeOffset();
    int i;
    int rd;
    int ifd;
    int ofd;
    int cnt;
    int ret;
    int total;
    int length;
    Address baseAddr;

    char buffer[4096];
    struct filehdr hdr;
    struct stat statBuf;
    struct aouthdr aout;
    struct scnhdr *sectHdr;
    bool needsCont = false;
    struct ld_info info[64];

    ifd = open(imageFileName.c_str(), O_RDONLY, 0);
    if (ifd < 0) {
      sprintf(errorLine, "Unable to open %s\n", imageFileName.c_str());
      logLine(errorLine);
      showErrorCallback(41, (const char *) errorLine);
      perror("open");
      return true;
    }

    rd = fstat(ifd, &statBuf);
    if (rd != 0) {
      perror("fstat");
      sprintf(errorLine, "Unable to stat %s\n", imageFileName.c_str());
      logLine(errorLine);
      showErrorCallback(72, (const char *) errorLine);
      return true;
    }
    length = statBuf.st_size;
    ofd = open(outFile.c_str(), O_WRONLY|O_CREAT, 0777);
    if (ofd < 0) {
      perror("open");
      exit(-1);
    }

    /* read header and section headers */
    cnt = read(ifd, &hdr, sizeof(struct filehdr));
    if (cnt != sizeof(struct filehdr)) {
	sprintf(errorLine, "Error reading header\n");
	logLine(errorLine);
	showErrorCallback(44, (const char *) errorLine);
	return false;
    }

    cnt = read(ifd, &aout, sizeof(struct aouthdr));

    sectHdr = (struct scnhdr *) calloc(sizeof(struct scnhdr), hdr.f_nscns);
    cnt = read(ifd, sectHdr, sizeof(struct scnhdr) * hdr.f_nscns);
    if ((unsigned) cnt != sizeof(struct scnhdr)* hdr.f_nscns) {
	sprintf(errorLine, "Section headers\n");
	logLine(errorLine);
	return false;
    }

    /* now copy the entire file */
    lseek(ofd, 0, SEEK_SET);
    lseek(ifd, 0, SEEK_SET);
    for (i=0; i < length; i += 4096) {
        rd = read(ifd, buffer, 4096);
        write(ofd, buffer, rd);
        total += rd;
    }

    if (!stopped) {
        // make sure it is stopped.
        pause();
        needsCont = true;
    }
    
    Address textorg = symbols->desc()->addr();
    baseAddr = aout.text_start - textorg;

    sprintf(errorLine, "seeking to %ld as the offset of the text segment \n",
            baseAddr);
    logLine(errorLine);
    sprintf(errorLine, "Code offset = 0x%lx\n", baseAddr);
    logLine(errorLine);
    

    lseek(ofd, aout.text_start, SEEK_SET);
    
    /* Copy the text segment over */
    unsigned uncopied_len = aout.tsize;
    for (i = 0; i < aout.tsize; i += 4096) {
        length = ((i + 4096) < aout.tsize) ? 4096 : aout.tsize-i;
        readDataSpace((void *) (baseAddr+i), length, (void *)buffer, false);

        
        write(ofd, buffer, length);
    }
    
    if (needsCont) {
        continueProc();
    }
    
    close(ofd);
    close(ifd);
    
    return true;

}

fileDescriptor *getExecFileDescriptor(pdstring filename, int &status, bool waitForTrap) {
    // AIX's /proc has a map file which contains data about
    // files loaded into the address space. The first two appear
    // to be the text and data from the process. We'll take it,
    // making sure that the filenames match.

    char tempstr[256];
    int pid = status;

    if (waitForTrap) {
        
        pstatus_t pstatus;
        int trapped = 0;
        int timeout = 0;
        int stat_fd = 0;
        sprintf(tempstr, "/proc/%d/status", pid);
        
        while (!trapped &&
               (timeout < 100) // 100: arbitrary number
               ) {
            
            // On slower machines (sp3-cw.cs.wisc.edu) we can enter
            // this code before the forked child has actually been created.
            // We attempt to re-open the FD if it failed the first time.
            if (stat_fd <= 0) 
                stat_fd = P_open(tempstr, O_RDONLY, 0);
            if (stat_fd > 0) {
                if (pread(stat_fd, &pstatus, sizeof(pstatus), 0) != sizeof(pstatus))
                    perror("pread failed while waiting for initial trap\n");
                
                if ((pstatus.pr_lwp.pr_why == PR_SYSEXIT))
                    trapped = 1;
            }
            timeout++;
            usleep(1000);
        }
        
        status = SIGTRAP;
        // Explicitly don't close the FD
    }
    
    int map_fd;
    sprintf(tempstr, "/proc/%d/map", pid);
    map_fd = P_open(tempstr, O_RDONLY, 0);

    if (map_fd <= 0)
        return NULL;

    prmap_t text_map;
    char text_name[512];
    prmap_t data_map;
    
    pread(map_fd, &text_map, sizeof(prmap_t), 0);
    pread(map_fd, text_name, 512, text_map.pr_pathoff);
    //assert(text_map.pr_mflags & MA_MAINEXEC);
    
    pread(map_fd, &data_map, sizeof(prmap_t), sizeof(prmap_t));
    if (!(data_map.pr_mflags & MA_MAINEXEC))
        data_map.pr_vaddr = 0;
    
    // We assume text = entry 0, data = entry 1
    // so they'll have the same names

    fileDescriptor *desc = 
    (fileDescriptor *) new fileDescriptor_AIX(filename,
                                              "",
                                              (Address) text_map.pr_vaddr,
                                              (Address) data_map.pr_vaddr,
                                              pid,
                                              true);
    

    return desc;
}


void process::copyDanglingMemory(process *child) {
    // Copy everything in a heap marked "uncopied" over by hand
    pdvector<heapItem *> items = heap.heapActive.values();
    for (unsigned i = 0; i < items.size(); i++) {
        if (items[i]->type == uncopiedHeap) {
            char buffer[items[i]->length];
            readDataSpace((void *)items[i]->addr, items[i]->length,
                          buffer, true);
            child->writeDataSpace((void *)items[i]->addr, 
                                  items[i]->length,
                                  buffer);
        }
    }
    // Odd... some changes _aren't_ copied. So add the base tramp
    // jumps as well (copy from parent)
    pdvector<trampTemplate *> tramps = baseMap.values();
    for (unsigned j = 0; j < tramps.size(); j++) {
        instruction insn;
        readDataSpace((void *)tramps[j]->location->absPointAddr(this),
                      sizeof(instruction),
                      (void *)&insn, true);
        child->writeDataSpace((void *)tramps[j]->location->absPointAddr(this),
                              sizeof(instruction),
                              (void *)&insn);
    }
    

}

        
