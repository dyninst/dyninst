/*
 * Copyright (c) 1999 Barton P. Miller
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

// $Id: init-irix.C,v 1.5 2000/10/17 17:42:34 schendel Exp $

#include "paradynd/src/metric.h"
#include "paradynd/src/internalMetrics.h"
#include "dyninstAPI/src/inst.h"
#include "paradynd/src/init.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/os.h"
#include "common/h/Time.h"
#include "common/h/timing.h"
#include "paradynd/src/timeMgr.h"

static AstNode mpiNormTagArg(AstNode::Param, (void *) 4);
static AstNode mpiNormCommArg(AstNode::Param, (void *) 5);
static AstNode mpiSRSendTagArg(AstNode::Param, (void *) 4);
static AstNode mpiSRCommArg(AstNode::Param, (void *) 10);
static AstNode mpiSRRSendTagArg(AstNode::Param, (void *) 4);
static AstNode mpiSRRCommArg(AstNode::Param, (void *) 7);

static AstNode mpiBcastCommArg(AstNode::Param, (void *) 4);
static AstNode mpiAlltoallCommArg(AstNode::Param, (void *) 6);
static AstNode mpiAlltoallvCommArg(AstNode::Param, (void *) 8);
static AstNode mpiGatherCommArg(AstNode::Param, (void *) 7);
static AstNode mpiGathervCommArg(AstNode::Param, (void *) 8);
static AstNode mpiAllgatherCommArg(AstNode::Param, (void *) 6);
static AstNode mpiAllgathervCommArg(AstNode::Param, (void *) 7);
static AstNode mpiReduceCommArg(AstNode::Param, (void *) 6);
static AstNode mpiAllreduceCommArg(AstNode::Param, (void *) 5);
static AstNode mpiReduceScatterCommArg(AstNode::Param, (void *) 5);
static AstNode mpiScatterCommArg(AstNode::Param, (void *) 7);
static AstNode mpiScattervCommArg(AstNode::Param, (void *) 8);
static AstNode mpiScanCommArg(AstNode::Param, (void *) 5);

// NOTE - the tagArg integer number starting with 0.  
bool initOS() {

//  initialRequests += new instMapping("main", "DYNINSTinit", FUNC_ENTRY);
// (obsoleted by installBootstrapInst() --ari)


  initialRequests += new instMapping("main", "DYNINSTexit", FUNC_EXIT);

  initialRequests += new instMapping(EXIT_NAME, "DYNINSTexit", FUNC_ENTRY);
  AstNode *retVal;
    // TODO: use "___tp_fork_XXX" hooks?

  retVal = new AstNode(AstNode::ReturnVal, (void *) 0);
  initialRequests += new instMapping("fork", "DYNINSTfork", 
				       FUNC_EXIT|FUNC_ARG, retVal);

  retVal = new AstNode(AstNode::ReturnVal, (void *) 0);
  initialRequests += new instMapping("_fork", "DYNINSTfork", 
				       FUNC_EXIT|FUNC_ARG, retVal);
#if defined(SHM_SAMPLING) && defined(MT_THREAD)
  AstNode *THRidArg = new AstNode(AstNode::Param, (void *) 5);
  initialRequests += new instMapping("pthread_create", "DYNINSTthreadCreate", 
                                     FUNC_EXIT|FUNC_ARG, THRidArg);
#endif

  {
    //initialRequests += new instMapping("execve", "DYNINSTexec",
    //			     FUNC_ENTRY|FUNC_ARG, tidArg);
    //initialRequests += new instMapping("execve", "DYNINSTexecFailed", FUNC_EXIT);
    AstNode *tidArg = new AstNode(AstNode::Param, (void *) 0);
    initialRequests += new instMapping("_execve", "DYNINSTexec",
				     FUNC_ENTRY|FUNC_ARG, tidArg);
    initialRequests += new instMapping("_execve", "DYNINSTexecFailed", FUNC_EXIT);
  }

#ifndef SHM_SAMPLING
  initialRequests += new instMapping("DYNINSTsampleValues", "DYNINSTreportNewTags",
				 FUNC_ENTRY);
#endif

  AstNode *cmdArg = new AstNode(AstNode::Param, (void *) 4);
  initialRequests += new instMapping("_rexec", "DYNINSTrexec",
				 FUNC_ENTRY|FUNC_ARG, cmdArg);
//   initialRequests += new instMapping("PROCEDURE_LINKAGE_TABLE","DYNINSTdynlinker",FUNC_ENTRY);

  vector<AstNode*> argList(2);
  argList[0] = &mpiNormTagArg;
  argList[1] = &mpiNormCommArg;
  initialRequests += new instMapping("PMPI_Send", "DYNINSTrecordTagAndGroup",
				     FUNC_ENTRY|FUNC_ARG, argList);
  initialRequests += new instMapping("PMPI_Bsend", "DYNINSTrecordTagAndGroup",
				     FUNC_ENTRY|FUNC_ARG, argList);
  initialRequests += new instMapping("PMPI_Ssend", "DYNINSTrecordTagAndGroup",
				     FUNC_ENTRY|FUNC_ARG, argList);
  initialRequests += new instMapping("PMPI_Isend", "DYNINSTrecordTagAndGroup",
				     FUNC_ENTRY|FUNC_ARG, argList);
  initialRequests += new instMapping("PMPI_Issend", "DYNINSTrecordTagAndGroup",
				     FUNC_ENTRY|FUNC_ARG, argList);
  argList[0] = &mpiSRSendTagArg;
  argList[1] = &mpiSRCommArg;
  initialRequests += new instMapping("PMPI_Sendrecv", "DYNINSTrecordTagAndGroup",
				     FUNC_ENTRY|FUNC_ARG, argList);
  argList[0] = &mpiSRRSendTagArg;
  argList[1] = &mpiSRRCommArg;
  initialRequests += new instMapping("PMPI_Sendrecv_replace",
				     "DYNINSTrecordTagAndGroup",
				     FUNC_ENTRY|FUNC_ARG, argList);
  initialRequests += new instMapping("PMPI_Bcast", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiBcastCommArg);
  initialRequests += new instMapping("PMPI_Alltoall", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiAlltoallCommArg);
  initialRequests += new instMapping("PMPI_Alltoallv", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiAlltoallvCommArg);
  initialRequests += new instMapping("PMPI_Gather", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiGatherCommArg);
  initialRequests += new instMapping("PMPI_Gatherv", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiGathervCommArg);
  initialRequests += new instMapping("PMPI_Allgather", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiAllgatherCommArg);
  initialRequests += new instMapping("PMPI_Allgatherv", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiAllgathervCommArg);
  initialRequests += new instMapping("PMPI_Reduce", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiReduceCommArg);
  initialRequests += new instMapping("PMPI_Allreduce", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiAllreduceCommArg);
  initialRequests += new instMapping("PMPI_Reduce_scatter", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiReduceScatterCommArg);
  initialRequests += new instMapping("PMPI_Scatter", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiScatterCommArg);
  initialRequests += new instMapping("PMPI_Scatterv", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiScattervCommArg);
  initialRequests += new instMapping("PMPI_Scan", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiScanCommArg);

#ifdef PARADYND_PVM
  char *doPiggy;

  AstNode *tagArg = new AstNode(AstNode::Param, (void *) 1);
  initialRequests += new instMapping("pvm_send", "DYNINSTrecordTag",
				 FUNC_ENTRY|FUNC_ARG, tagArg);

  // kludge to get Critical Path to work.
  // XXX - should be tunable constant.
  doPiggy = getenv("DYNINSTdoPiggy");
  if (doPiggy) {
      initialRequests += new instMapping("main", "DYNINSTpvmPiggyInit", FUNC_ENTRY);
      AstNode *tidArg = new AstNode(AstNode::Param, (void *) 0);
      initialRequests+= new instMapping("pvm_send", "DYNINSTpvmPiggySend",
                           FUNC_ENTRY|FUNC_ARG, tidArg);
      initialRequests += new instMapping("pvm_recv", "DYNINSTpvmPiggyRecv", FUNC_EXIT);
      tidArg = new AstNode(AstNode::Param, (void *) 0);
      initialRequests += new instMapping("pvm_mcast", "DYNINSTpvmPiggyMcast",
                           FUNC_ENTRY|FUNC_ARG, tidArg);
  }
#endif

  return true;
};

bool isFreeRunningHwCounterAvail() {
  timespec val;
  int result = clock_gettime(CLOCK_SGI_CYCLE, &val);
  if(result == 0) return true;
  else            return false;
}

rawTime64 getRawWallTime_frhc() {   // free running hardware counter
  timespec timestr;
  int nsec_per_rawval = 0;
  if(nsec_per_rawval == 0) {
    clock_getres(CLOCK_SGI_CYCLE, &timestr);
    nsec_per_rawval = timestr.tv_nsec;
  }
  assert(clock_gettime(CLOCK_SGI_CYCLE, &timestr)==0);
  int64_t val = timestr.tv_sec;
  val *= I64_C(1000000000);
  val += timestr.tv_nsec;
  // currently the timeMgr conversion functions expect the daemon &
  // rtinst libraries to return values in the same raw units
  val /= nsec_per_rawval;  
  return static_cast<rawTime64>(val);
}

void initWallTimeMgrPlt() {
  timespec timestr;
  if(isFreeRunningHwCounterAvail()) {
    clock_getres(CLOCK_SGI_CYCLE, &timestr);
    timeUnit frcRes(fraction(timestr.tv_nsec));
    timeStamp curTime = getCurrentTime();  // general util one
    timeLength hrtimeLength(getRawWallTime_frhc(), frcRes);
    timeStamp beghrtime = curTime - hrtimeLength;
    timeBase hrtimeBase(beghrtime);
    getWallTimeMgr().installLevel(
         wallTimeMgr_t::LEVEL_ONE, &isFreeRunningHwCounterAvail, frcRes, 
         hrtimeBase, &getRawWallTime_frhc, "DYNINSTgetWalltime_hw");
  }

  getWallTimeMgr().installLevel(wallTimeMgr_t::LEVEL_TWO, &yesFunc,
				timeUnit::us(), timeBase::b1970(), 
				&getRawTime1970, "DYNINSTgetWalltime_sw");
}

