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

// $Id: proc-aix.C,v

#include "paradynd/src/pd_process.h"
#include "paradynd/src/pd_thread.h"
#include "paradynd/src/init.h"
#include "paradynd/src/instReqNode.h"
#include "common/h/timing.h"
#include "common/h/Time.h"
#include <procinfo.h>

#ifdef USES_PMAPI
#include <pmapi.h>
#include "rtinst/h/rthwctr-aix.h"
#endif

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

bool pd_process::isPmapiAvail() {
#ifdef USES_PMAPI
  return true;
#else
  return false;
#endif
}

void pd_process::initCpuTimeMgrPlt() {
  cpuTimeMgr->installLevel(cpuTimeMgr_t::LEVEL_ONE, &pd_process::isPmapiAvail, 
                           getCyclesPerSecond(), timeBase::bNone(), 
                           &pd_process::getRawCpuTime_hw, "hwCpuTimeFPtrInfo");
  cpuTimeMgr->installLevel(cpuTimeMgr_t::LEVEL_TWO, &pd_process::yesAvail, 
                           timeUnit::us(), timeBase::bNone(), 
                           &pd_process::getRawCpuTime_sw, "swCpuTimeFPtrInfo");
}

rawTime64 pd_process::getAllLwpRawCpuTime_hw() {
   pd_thread *pt = *beginThr();
   return pt->getRawCpuTime_hw();
}

rawTime64 pd_process::getAllLwpRawCpuTime_sw() {
   pd_thread *pt = *beginThr();
   return pt->getRawCpuTime_sw();
}


rawTime64 pd_thread::getRawCpuTime_hw() 
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

   if (get_lwp() > 0) 
      lwp_to_use = get_lwp();
   else {
      /* If we need to get the data for the entire process (ie. lwp_id_ == 0)
         then we need to the pm_get_data_group function requires any lwp in
         the group, so we'll grab the lwp of any active thread in the
         process */
      if(getthrds(pd_proc->, &thrd_buf, sizeof(struct thrdsinfo), 
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

rawTime64 pd_thread::getRawCpuTime_sw()
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


