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

// $Id: proc-solaris.C,v

#include "paradynd/src/pd_process.h"
#include "paradynd/src/pd_thread.h"


void pd_process::initCpuTimeMgrPlt() {
   cpuTimeMgr->installLevel(cpuTimeMgr_t::LEVEL_TWO, &pd_process::yesAvail, 
                            timeUnit::ns(), timeBase::bNone(), 
                            &pd_process::getRawCpuTime_sw,"swCpuTimeFPtrInfo");
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
  return 0;
}

/* return unit: nsecs */
rawTime64 pd_thread::getRawCpuTime_sw() 
{
  // returns user time from the u or proc area of the inferior process,
  // which in turn is presumably obtained by using a /proc ioctl to obtain
  // it (solaris).  It must not stop the inferior process in order to obtain
  // the result, nor can it assure that the inferior has been stopped.  The
  // result MUST be "in sync" with rtinst's DYNINSTgetCPUtime().
  
  rawTime64 result;
  prusage_t theUsage;

#ifdef PURE_BUILD
  // explicitly initialize "theUsage" struct (to pacify Purify)
  memset(&theUsage, '\0', sizeof(prusage_t));
#endif

  // compute the CPU timer for the whole process
  if(pread(dyninst_thread->os_handle(), &theUsage, sizeof(prusage_t), 0) 
     != sizeof(prusage_t))
  {
     perror("getInfCPU: read");
     return -1;  // perhaps the process ended
  }

  result =  (theUsage.pr_utime.tv_sec + theUsage.pr_stime.tv_sec) * 1000000000LL;
  result += (theUsage.pr_utime.tv_nsec+ theUsage.pr_stime.tv_nsec);

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
