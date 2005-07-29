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

/* $Id: init-sunos.C,v 1.52 2005/07/29 19:20:06 bernat Exp $ */

#include <sys/time.h>
#include "paradynd/src/internalMetrics.h"
#include "paradynd/src/pd_process.h"
#include "paradynd/src/init.h"
#include "common/h/timing.h"


//ccw 19 apr 2002 : SPLIT
//i changed initialRequests to initialRequestsPARADYN (28 times)

// NOTE - the tagArg integer number starting with 0.  
bool initOS() {
   BPatch_snippet *retVal;

   // the use of system on Solaris needs to be implemented
   // /bin/sh uses it's own version of malloc which is causing us a problem
   initialRequestsPARADYN += new pdinstMapping("system", "DYNINSTsystem",
                                               FUNC_ENTRY);

   if (pd_process::pdFlavor == "mpi") {
      instMPI();
      retVal = new BPatch_retExpr();
      initialRequestsPARADYN += new pdinstMapping("fork", "DYNINSTmpi_fork", 
                                                FUNC_EXIT|FUNC_ARG, retVal);
      retVal = new BPatch_retExpr();
      initialRequestsPARADYN += new pdinstMapping("_fork", "DYNINSTmpi_fork", 
                                                FUNC_EXIT|FUNC_ARG, retVal);
   } else { /* Fork and exec */
   }

   // ===  MULTI-THREADED FUNCTIONS  ======================================  
   pdinstMapping *mapping;
   mapping = new pdinstMapping("_thread_start", "DYNINST_dummy_create",
                               FUNC_ENTRY, BPatch_callBefore, BPatch_lastSnippet);
   mapping->markAs_MTonly();
   initialRequestsPARADYN.push_back(mapping);


   mapping = new pdinstMapping("_thr_exit_common", "DYNINSTthreadDelete", 
                               FUNC_ENTRY, BPatch_callBefore, BPatch_lastSnippet);
   mapping->markAs_MTonly();
   initialRequestsPARADYN.push_back(mapping);

#if 0
   // Unsupported
   mapping = new pdinstMapping("_resume_ret", "DYNINSTthreadStart",
                               FUNC_ENTRY, callPreInsn, orderLastAtPoint) ;
   mapping->markAs_MTonly();
   initialRequestsPARADYN.push_back(mapping);


   mapping = new pdinstMapping("_resume", "DYNINSTthreadStop",
                               FUNC_ENTRY, callPreInsn, orderLastAtPoint) ;
   mapping->markAs_MTonly();
   initialRequestsPARADYN.push_back(mapping);
#endif

   // Thread SyncObjects
   // mutex
   BPatch_paramExpr *arg0 = new BPatch_paramExpr(0);
   mapping = new pdinstMapping("_cmutex_lock",  "DYNINSTreportNewMutex", 
                             FUNC_ENTRY|FUNC_ARG, arg0);
   mapping->markAs_MTonly();
   initialRequestsPARADYN.push_back(mapping);


   arg0 = new BPatch_paramExpr(0);
   mapping = new pdinstMapping("pthread_mutex_init", "DYNINSTreportNewMutex",
                             FUNC_ENTRY|FUNC_ARG, arg0);
   mapping->markAs_MTonly();
   initialRequestsPARADYN.push_back(mapping);


   arg0 = new BPatch_paramExpr(0);
   mapping = new pdinstMapping("pthread_mutex_lock", "DYNINSTreportNewMutex",
                             FUNC_ENTRY|FUNC_ARG, arg0);
   mapping->markAs_MTonly();
   initialRequestsPARADYN.push_back(mapping);


   // rwlock
   //
   arg0 = new BPatch_paramExpr(0);
   mapping = new pdinstMapping("_lrw_rdlock", "DYNINSTreportNewRwLock", 
                             FUNC_ENTRY|FUNC_ARG, arg0);
   mapping->markAs_MTonly();
   initialRequestsPARADYN.push_back(mapping);


   //Semaphore
   //
   arg0 = new BPatch_paramExpr(0);
   mapping = new pdinstMapping("_sema_wait_cancel", "DYNINSTreportNewSema", 
                             FUNC_ENTRY|FUNC_ARG, arg0);
   mapping->markAs_MTonly();
   initialRequestsPARADYN.push_back(mapping);


   // Conditional variable
   //
   arg0 = new BPatch_paramExpr(0);
   mapping = new pdinstMapping("_cond_wait_cancel", "DYNINSTreportNewCondVar", 
                             FUNC_ENTRY|FUNC_ARG, arg0);
   mapping->markAs_MTonly();
   initialRequestsPARADYN.push_back(mapping);
   // =====================================================================

  
   BPatch_paramExpr *cmdArg = new BPatch_paramExpr(4);
   initialRequestsPARADYN += new pdinstMapping("rexec", "DYNINSTrexec",
                                             FUNC_ENTRY|FUNC_ARG, cmdArg);
   //   initialRequestsPARADYN += new pdinstMapping("PROCEDURE_LINKAGE_TABLE","DYNINSTdynlinker",FUNC_ENTRY);
  
  
#if defined(i386_unknown_solaris2_5)
   // Protect our signal handler by overriding any which the application
   // may already have or subsequently install.
   // Note that this is currently replicated in dyninstAPI_init until
   // dyninst is updated to refer to this (or a similar) initialization.
   const char *sigactionF="_libc_sigaction";
   pdvector<BPatch_snippet*> argList(3);
   static BPatch_paramExpr  sigArg(0); argList[0] = &sigArg;
   static BPatch_paramExpr  actArg(1); argList[1] = &actArg;
   static BPatch_paramExpr oactArg(2); argList[2] = &oactArg;
      
   initialRequestsPARADYN += new pdinstMapping(sigactionF, "DYNINSTdeferSigHandler",
                                               FUNC_ENTRY|FUNC_ARG, argList);
   initialRequestsPARADYN += new pdinstMapping(sigactionF, "DYNINSTresetSigHandler",
                                               FUNC_EXIT|FUNC_ARG, argList);
#endif

   return true;
};

rawTime64 getRawWallTime_hrtime() {
  hrtime_t cur;
  cur = gethrtime();
  return static_cast<rawTime64>(cur);
}

void initWallTimeMgrPlt() {
  //getWallTimeMgr().installLevel(wallTimeMgr_t::LEVEL_THREE,yesFunc,
  //                       timeUnit::ms(), timeBase::b1970(), &getRawTime1970);

  timeStamp curTime = getCurrentTime();  // general util one
  timeLength hrtimeLength(getRawWallTime_hrtime(), timeUnit::ns());
  timeStamp beghrtime = curTime - hrtimeLength;
  timeBase hrtimeBase(beghrtime);
  getWallTimeMgr().installLevel(wallTimeMgr_t::LEVEL_TWO, yesFunc,
                                timeUnit::ns(), hrtimeBase, 
                                &getRawWallTime_hrtime,"swWallTimeFPtrInfo");
}

