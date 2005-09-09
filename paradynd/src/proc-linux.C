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

// $Id: proc-linux.C,v

#include "paradynd/src/pd_process.h"
#include "paradynd/src/init.h"
#include "paradynd/src/pd_thread.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include <unistd.h>

rawTime64 pd_process::getAllLwpRawCpuTime_hw() {
   rawTime64 total = 0;
   threadMgr::thrIter itr = thrMgr().begin();
   for(; itr != thrMgr().end(); itr++) {
      pd_thread *thr = *itr;
      total += thr->getRawCpuTime_hw();
   }
   return total;
}

rawTime64 pd_process::getAllLwpRawCpuTime_sw() {
   rawTime64 total = 0;
   threadMgr::thrIter itr = thrMgr().begin();

   for(; itr != thrMgr().end(); itr++) {
      pd_thread *thr = *itr;
      total += thr->getRawCpuTime_sw();
   }
   return total;
}



timeUnit calcJiffyUnit() 
{
  int intJiffiesPerSec = sysconf(_SC_CLK_TCK);
  timeUnit jiffy(fraction(1000000000LL, intJiffiesPerSec));
  return jiffy;
}


bool pd_process::isPapiAvail() {
   return isPapiInitialized();
}

void pd_process::initCpuTimeMgrPlt() {
#ifdef PAPI
   cpuTimeMgr->installLevel(cpuTimeMgr_t::LEVEL_ONE, &pd_process::isPapiAvail,
                            getCyclesPerSecond(), timeBase::bNone(), 
                            &pd_process::getRawCpuTime_hw,"hwCpuTimeFPtrInfo");
   
#endif

   cpuTimeMgr->installLevel(cpuTimeMgr_t::LEVEL_TWO, &pd_process::yesAvail, 
                            calcJiffyUnit(), timeBase::bNone(), 
                            &pd_process::getRawCpuTime_sw,"swCpuTimeFPtrInfo");
}

rawTime64 pd_thread::getRawCpuTime_hw()
{
   rawTime64 result = 0;
  
#ifdef PAPI
   result = papi()->getCurrentVirtCycles();
#endif
  
   if (result < hw_previous_) 
   {
      logLine("********* time going backwards in paradynd **********\n");
      result = hw_previous_;
   }
   else 
      hw_previous_ = result;
   
   return result;
}

rawTime64 pd_thread::getRawCpuTime_sw()
{
   rawTime64 result = 0;
   int bufsize = 255;
   unsigned long utime, stime;
   char procfn[bufsize], *buf;
   
   sprintf( procfn, "/proc/%d/stat", dyninst_thread->getLWP());
   
   int fd;

   // The reason for this complicated method of reading and sseekf-ing is
   // to ensure that we read enough of the buffer 'atomically' to make sure
   // the data is consistent.  Is this necessary?  I *think* so. - nash
   do {
      fd = P_open(procfn, O_RDONLY, 0);
      if (fd < 0) {
         perror("getInferiorProcessCPUtime (open)");
         return false;
      }
      
      buf = new char[ bufsize ];
      
      if ((int)P_read( fd, buf, bufsize ) < 0) {
         perror("getInferiorProcessCPUtime");
         return false;
      }
      
      /* While I'd bet that any of the numbers preceding utime and stime 
         could overflow a signed int on IA-64, the compiler whines if you 
         add length specifiers to elements whose conversion has been 
         surpressed. */
      if(2==sscanf(buf,
            "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu ",
             &utime, &stime ) ) 
      {
         // These numbers are in 'jiffies' or timeslices.
         // Oh, and I'm also assuming that process time includes system time
         result = static_cast<rawTime64>(utime) + static_cast<rawTime64>(stime);
         break;
      }

      delete [] buf;
      bufsize = bufsize * 2;
      
      P_close( fd );
   } while ( true );
   
   delete [] buf;
   P_close(fd);
   
   if (result < sw_previous_) {
      logLine("********* time going backwards in paradynd **********\n");
      result = sw_previous_;
   }
   else 
      sw_previous_ = result;
   
   return result;
}
