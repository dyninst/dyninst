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

/* $Id: init-sunos.C,v 1.37 2002/06/17 21:31:16 chadd Exp $ */

#include <sys/time.h>
#include "paradynd/src/internalMetrics.h"
#include "dyninstAPI/src/inst.h"
#include "paradynd/src/init.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/os.h"
#include "common/h/timing.h"
#include "dyninstAPI/src/process.h"


//ccw 19 apr 2002 : SPLIT
//i changed initialRequests to initialRequestsPARADYN (28 times)

// NOTE - the tagArg integer number starting with 0.  
bool initOS() {

//  initialRequestsPARADYN += new instMapping("main", "DYNINSTinit", FUNC_ENTRY);
// (obsoleted by installBootstrapInst() --ari)


  AstNode *retVal;

  initialRequestsPARADYN += new instMapping("main", "DYNINSTexit", FUNC_EXIT);

  initialRequestsPARADYN += new instMapping(EXIT_NAME, "DYNINSTexit", FUNC_ENTRY);

  if (process::pdFlavor == "mpi") {
#ifndef sparc_sun_solaris2_4
	  instMPI();
#endif
	  retVal = new AstNode(AstNode::ReturnVal, (void *) 0);
	  initialRequestsPARADYN += new instMapping("fork", "DYNINSTmpi_fork", 
					     FUNC_EXIT|FUNC_ARG, retVal);
	  retVal = new AstNode(AstNode::ReturnVal, (void *) 0);
	  initialRequestsPARADYN += new instMapping("_fork", "DYNINSTmpi_fork", 
					     FUNC_EXIT|FUNC_ARG, retVal);
  } else { /* Fork and exec */
	  retVal = new AstNode(AstNode::ReturnVal, (void *) 0);
	  initialRequestsPARADYN += new instMapping("fork", "DYNINSTfork", 
					     FUNC_EXIT|FUNC_ARG, retVal);

	  //libthread _fork
	  retVal = new AstNode(AstNode::ReturnVal, (void *) 0);
	  initialRequestsPARADYN += new instMapping("_fork", "DYNINSTfork", 
					     FUNC_EXIT|FUNC_ARG, retVal);


	  //initialRequestsPARADYN += new instMapping("execve", "DYNINSTexec",
	  //			               FUNC_ENTRY|FUNC_ARG, tidArg);
	  //initialRequestsPARADYN += new instMapping("execve", "DYNINSTexecFailed", 
	  //                                   FUNC_EXIT);
	  AstNode *tidArg = new AstNode(AstNode::Param, (void *) 0);
	  initialRequestsPARADYN += new instMapping("_execve", "DYNINSTexec",
					     FUNC_ENTRY|FUNC_ARG, tidArg);
	  initialRequestsPARADYN += new instMapping("_execve", "DYNINSTexecFailed", 
					     FUNC_EXIT);
  }
  
#if defined(MT_THREAD)
  initialRequestsPARADYN += new instMapping("_thread_start",
				     "DYNINST_VirtualTimerCREATE",
				     FUNC_ENTRY, callPreInsn,
				     orderLastAtPoint) ;

  initialRequestsPARADYN += new instMapping("_thr_exit_common", "DYNINSTthreadDelete", 
                                     FUNC_ENTRY, callPreInsn, 
				     orderLastAtPoint);

  initialRequestsPARADYN += new instMapping("_resume_ret",
				     "DYNINSTthreadStart",
				     FUNC_ENTRY, callPreInsn,
				     orderLastAtPoint) ;

  initialRequestsPARADYN += new instMapping("_resume",
				     "DYNINSTthreadStop",
				     FUNC_ENTRY, callPreInsn,
				     orderLastAtPoint) ;

  // Thread SyncObjects
  // mutex
  AstNode* arg0 = new AstNode(AstNode::Param, (void*) 0);
  initialRequestsPARADYN += new instMapping("_cmutex_lock", 
  				     "DYNINSTreportNewMutex", 
                                     FUNC_ENTRY|FUNC_ARG, 
  				     arg0);

  // rwlock
  //
  arg0 = new AstNode(AstNode::Param, (void*) 0);
  initialRequestsPARADYN += new instMapping("_lrw_rdlock", 
  				     "DYNINSTreportNewRwLock", 
                                     FUNC_ENTRY|FUNC_ARG, 
  				     arg0);

  //Semaphore
  //
  arg0 = new AstNode(AstNode::Param, (void*) 0);
  initialRequestsPARADYN += new instMapping("_sema_wait_cancel", 
  				     "DYNINSTreportNewSema", 
                                     FUNC_ENTRY|FUNC_ARG, 
  				     arg0);

  // Conditional variable
  //
  arg0 = new AstNode(AstNode::Param, (void*) 0);
  initialRequestsPARADYN += new instMapping("_cond_wait_cancel", 
  				     "DYNINSTreportNewCondVar", 
                                     FUNC_ENTRY|FUNC_ARG, 
  				     arg0);
#endif

  
  AstNode *cmdArg = new AstNode(AstNode::Param, (void *) 4);
  initialRequestsPARADYN += new instMapping("rexec", "DYNINSTrexec",
				     FUNC_ENTRY|FUNC_ARG, cmdArg);
  //   initialRequestsPARADYN += new instMapping("PROCEDURE_LINKAGE_TABLE","DYNINSTdynlinker",FUNC_ENTRY);
  
  
#ifdef PARADYND_PVM
  char *doPiggy;

  AstNode *tagArg = new AstNode(AstNode::Param, (void *) 1);
  initialRequestsPARADYN += new instMapping("pvm_send", "DYNINSTrecordTag",
				 FUNC_ENTRY|FUNC_ARG, tagArg);

  // kludge to get Critical Path to work.
  // XXX - should be tunable constant.
  doPiggy = getenv("DYNINSTdoPiggy");
  if (doPiggy) {
      initialRequestsPARADYN += new instMapping("main", "DYNINSTpvmPiggyInit", FUNC_ENTRY);
      AstNode *tidArg = new AstNode(AstNode::Param, (void *) 0);
      initialRequestsPARADYN+= new instMapping("pvm_send", "DYNINSTpvmPiggySend",
                           FUNC_ENTRY|FUNC_ARG, tidArg);
      initialRequestsPARADYN += new instMapping("pvm_recv", "DYNINSTpvmPiggyRecv", FUNC_EXIT);
      tidArg = new AstNode(AstNode::Param, (void *) 0);
      initialRequestsPARADYN += new instMapping("pvm_mcast", "DYNINSTpvmPiggyMcast",
                           FUNC_ENTRY|FUNC_ARG, tidArg);
  }
#endif

#if defined(i386_unknown_solaris2_5)
      // Protect our signal handler by overriding any which the application
      // may already have or subsequently install.
      // Note that this is currently replicated in dyninstAPI_init until
      // dyninst is updated to refer to this (or a similar) initialization.
      const char *sigactionF="_libc_sigaction";
      vector<AstNode*> argList(3);
      static AstNode  sigArg(AstNode::Param, (void*) 0); argList[0] = &sigArg;
      static AstNode  actArg(AstNode::Param, (void*) 1); argList[1] = &actArg;
      static AstNode oactArg(AstNode::Param, (void*) 2); argList[2] = &oactArg;
      
      initialRequestsPARADYN += new instMapping(sigactionF, "DYNINSTdeferSigHandler",
                                         FUNC_ENTRY|FUNC_ARG, argList);
      initialRequestsPARADYN += new instMapping(sigactionF, "DYNINSTresetSigHandler",
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

