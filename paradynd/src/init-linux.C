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

// $Id: init-linux.C,v 1.21 2003/07/31 19:01:09 schendel Exp $

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
   AstNode *cmdArg;
   
   if (process::pdFlavor == "mpi") {
      instMPI();
   } 
   cmdArg = new AstNode(AstNode::Param, (void *) 4);
   initialRequestsPARADYN += new instMapping("rexec", "DYNINSTrexec",
                                             FUNC_ENTRY|FUNC_ARG, cmdArg);
   
   // ===  MULTI-THREADED FUNCTIONS  ======================================
   // Official gotten-from-tracing name. While pthread_create() is the
   // call made from user space, _pthread_body is the parent of any created
   // thread, and so is a good place to instrument.
   instMapping *mapping;
   mapping = new instMapping("pthread_start_thread", "DYNINST_dummy_create",
                             FUNC_ENTRY, callPreInsn, orderFirstAtPoint);
   mapping->markAs_MTonly();
   initialRequestsPARADYN.push_back(mapping);
   

   mapping = new instMapping("pthread_exit", "DYNINSTthreadDelete", 
                             FUNC_ENTRY, callPreInsn, orderLastAtPoint);
   mapping->markAs_MTonly();
   initialRequestsPARADYN.push_back(mapping);
   
   
   // Should really be the longjmp in the pthread library
   mapping = new instMapping("_longjmp", "DYNINSTthreadStart",
                             FUNC_ENTRY, callPreInsn, orderLastAtPoint) ;
   mapping->markAs_MTonly();
   initialRequestsPARADYN.push_back(mapping);
   
   
   mapping = new instMapping("_usched_swtch", "DYNINSTthreadStop",
                             FUNC_ENTRY, callPreInsn, orderLastAtPoint) ;
   mapping->markAs_MTonly();
   initialRequestsPARADYN.push_back(mapping);
   

   // Thread SyncObjects
   // mutex
   AstNode* arg0 = new AstNode(AstNode::Param, (void*) 0);
   mapping = new instMapping("pthread_mutex_init", "DYNINSTreportNewMutex", 
                             FUNC_ENTRY|FUNC_ARG, arg0);
   mapping->markAs_MTonly();
   initialRequestsPARADYN.push_back(mapping);
   
    
#ifdef none    // rwlocks don't appear to exist on linux
   // rwlock
   //
   arg0 = new AstNode(AstNode::Param, (void*) 0);
   mapping = new instMapping("pthread_rwlock_init", "DYNINSTreportNewRwLock", 
                             FUNC_ENTRY|FUNC_ARG, arg0);
   mapping->markAs_MTonly();
   initialRequestsPARADYN.push_back(mapping);
#endif
    
   //Semaphore
   //
   arg0 = new AstNode(AstNode::Param, (void*) 0);
   mapping = new instMapping("i_need_a_name", "DYNINSTreportNewSema", 
                             FUNC_ENTRY|FUNC_ARG, arg0);
   mapping->markAs_MTonly();
   initialRequestsPARADYN.push_back(mapping);
   
   
   // Conditional variable
   //
   arg0 = new AstNode(AstNode::Param, (void*) 0);
   mapping = new instMapping("pthread_cond_init", "DYNINSTreportNewCondVar", 
                             FUNC_ENTRY|FUNC_ARG, arg0);
   mapping->markAs_MTonly();
   initialRequestsPARADYN.push_back(mapping);
   // =======

   // Protect our signal handler by overriding any which the application
   // may already have or subsequently install.
   // Note that this is currently replicated in dyninstAPI_init until
   // dyninst is updated to refer to this (or a similar) initialization.
   const char *sigactionF="__sigaction";
   pdvector<AstNode*> argList(3);
   static AstNode  sigArg(AstNode::Param, (void*) 0); argList[0] = &sigArg;
   static AstNode  actArg(AstNode::Param, (void*) 1); argList[1] = &actArg;
   static AstNode oactArg(AstNode::Param, (void*) 2); argList[2] = &oactArg;

   mapping = new instMapping(sigactionF, "DYNINSTdeferSigHandler",
                             FUNC_ENTRY|FUNC_ARG, argList);
   initialRequestsPARADYN.push_back(mapping);

   mapping = new instMapping(sigactionF, "DYNINSTresetSigHandler",
                             FUNC_EXIT|FUNC_ARG, argList);
   initialRequestsPARADYN.push_back(mapping);

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




