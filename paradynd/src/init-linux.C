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

// $Id: init-linux.C,v 1.12 2002/02/21 21:48:32 bernat Exp $

#include "paradynd/src/metric.h"
#include "paradynd/src/internalMetrics.h"
#include "dyninstAPI/src/inst.h"
#include "paradynd/src/init.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/os.h"
#include "paradynd/src/init.h"
#include "common/h/Time.h"
#include "common/h/timing.h"
#include "paradynd/src/timeMgr.h"
#include "rtinst/h/RThwtimer-linux.h"
#include "dyninstAPI/src/process.h"

bool initOS() {
#ifdef PARADYND_PVM
  AstNode *tagArg;
#endif
  AstNode *cmdArg;
  AstNode *tidArg;
  AstNode *retVal;

  initialRequests += new instMapping("main", "DYNINSTexit", FUNC_EXIT);

  initialRequests += new instMapping(EXIT_NAME, "DYNINSTexit", FUNC_ENTRY);

  if (process::pdFlavor == "mpi") {
	  instMPI();

	  retVal = new AstNode(AstNode::ReturnVal, (void *) 0);
	  initialRequests += new instMapping("__libc_fork", "DYNINSTmpi_fork", 
					     FUNC_EXIT|FUNC_ARG, retVal);
  } else { /* Fork and exec */
	  retVal = new AstNode(AstNode::ReturnVal, (void *) 0);
	  initialRequests += new instMapping("__libc_fork", "DYNINSTfork", 
					     FUNC_EXIT|FUNC_ARG, retVal);
  
	  tidArg = new AstNode(AstNode::Param, (void *) 0);
	  initialRequests += new instMapping("__execve", "DYNINSTexec",
					     FUNC_ENTRY|FUNC_ARG, tidArg);
	  initialRequests += new instMapping("__execve", "DYNINSTexecFailed", 
					     FUNC_EXIT);
  }

  cmdArg = new AstNode(AstNode::Param, (void *) 4);
  initialRequests += new instMapping("rexec", "DYNINSTrexec",
				     FUNC_ENTRY|FUNC_ARG, cmdArg);

#ifdef PARADYND_PVM
  char *doPiggy;

  tagArg = new AstNode(AstNode::Param, (void *) 1);
  initialRequests += new instMapping("pvm_send", "DYNINSTrecordTag",
				     FUNC_ENTRY|FUNC_ARG, tagArg);

  // kludge to get Critical Path to work.
  // XXX - should be tunable constant.
  doPiggy = getenv("DYNINSTdoPiggy");
  if (doPiggy) {
      initialRequests += new instMapping("main", "DYNINSTpvmPiggyInit", 
					 FUNC_ENTRY);
      tidArg = new AstNode(AstNode::Param, (void *) 0);
      initialRequests+= new instMapping("pvm_send", "DYNINSTpvmPiggySend",
					FUNC_ENTRY|FUNC_ARG, tidArg);
      initialRequests += new instMapping("pvm_recv", "DYNINSTpvmPiggyRecv", 
					 FUNC_EXIT);
      tidArg = new AstNode(AstNode::Param, (void *) 0);
      initialRequests += new instMapping("pvm_mcast", "DYNINSTpvmPiggyMcast",
					 FUNC_ENTRY|FUNC_ARG, tidArg);
  }
#endif

  // Protect our signal handler by overriding any which the application
  // may already have or subsequently install.
  // Note that this is currently replicated in dyninstAPI_init until
  // dyninst is updated to refer to this (or a similar) initialization.
  const char *sigactionF="__sigaction";
  vector<AstNode*> argList(3);
  static AstNode  sigArg(AstNode::Param, (void*) 0); argList[0] = &sigArg;
  static AstNode  actArg(AstNode::Param, (void*) 1); argList[1] = &actArg;
  static AstNode oactArg(AstNode::Param, (void*) 2); argList[2] = &oactArg;
  
  initialRequests += new instMapping(sigactionF, "DYNINSTdeferSigHandler",
                                     FUNC_ENTRY|FUNC_ARG, argList);
  initialRequests += new instMapping(sigactionF, "DYNINSTresetSigHandler",
                                     FUNC_EXIT|FUNC_ARG, argList);

  return true;
};

bool dm_isTSCAvail() {
  return isTSCAvail() != 0;
}

rawTime64 dm_getTSC() {
  rawTime64 v;
  getTSC(v);
  return v;
}

void initWallTimeMgrPlt() {
  if(dm_isTSCAvail()) {
    timeStamp curTime = getCurrentTime();  // general util one
    timeLength hrtimeLength(dm_getTSC(), getCyclesPerSecond());
    timeStamp beghrtime = curTime - hrtimeLength;
    timeBase hrtimeBase(beghrtime);
    getWallTimeMgr().installLevel(wallTimeMgr_t::LEVEL_ONE, &dm_isTSCAvail,
				  getCyclesPerSecond(), hrtimeBase,
				  &dm_getTSC, "hwWallTimeFPtrInfo");
  }

  getWallTimeMgr().installLevel(wallTimeMgr_t::LEVEL_TWO, yesFunc,
				timeUnit::us(), timeBase::b1970(), 
				&getRawTime1970, "swWallTimeFPtrInfo");
}




