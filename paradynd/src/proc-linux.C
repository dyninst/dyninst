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

rawTime64 pd_process::getAllLwpRawCpuTime_hw() {
   rawTime64 total = 0;
   threadMgr::thrIter itr = thrMgr().begin();
   for(; itr != thrMgr().end(); itr++) {
      pd_thread *thr = *itr;
      dyn_lwp *lwp = thr->get_dyn_thread()->get_lwp();
      total += lwp->getRawCpuTime_hw();
   }
   return total;
}

rawTime64 pd_process::getAllLwpRawCpuTime_sw() {
   rawTime64 total = 0;
   threadMgr::thrIter itr = thrMgr().begin();

   for(; itr != thrMgr().end(); itr++) {
      pd_thread *thr = *itr;
      dyn_lwp *lwp = thr->get_dyn_thread()->get_lwp();
      total += lwp->getRawCpuTime_sw();
   }
   return total;
}


timeUnit calcJiffyUnit() {
   // Determine the number of jiffies/sec by checking the clock idle time in
   // /proc/uptime against the jiffies idle time in /proc/stat
   
   FILE *tmp = P_fopen( "/proc/uptime", "r" );
   assert( tmp );
   double uptimeReal;
   assert( 1 == fscanf( tmp, "%*f %lf", &uptimeReal ) );
   fclose( tmp );
   tmp = P_fopen( "/proc/stat", "r" );
   assert( tmp );
   int uptimeJiffies;
   assert( 1 == fscanf( tmp, "%*s %*d %*d %*d %d", &uptimeJiffies ) );
   
   if (sysconf(_SC_NPROCESSORS_CONF) > 1) {
      // on SMP boxes, the first line is cumulative jiffies, the second line
      // is jiffies for cpu0 - on uniprocessors, this fscanf will fail as
      // there is only a single cpu line
      assert (1 == fscanf(tmp, "\ncpu0 %*d %*d %*d %d", &uptimeJiffies));
   }
   
   fclose( tmp );
   int intJiffiesPerSec = static_cast<int>( static_cast<double>(uptimeJiffies) 
                                            / uptimeReal + 0.5 );
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


