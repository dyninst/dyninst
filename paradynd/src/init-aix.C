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

// $Id: init-aix.C,v 1.17 2001/10/11 23:58:01 schendel Exp $

#include "paradynd/src/metric.h"
#include "paradynd/src/internalMetrics.h"
#include "dyninstAPI/src/inst.h"
#include "paradynd/src/init.h"
#include "paradynd/src/metricDef.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/os.h"
#include "common/h/timing.h"

// NOTE - the tagArg integer number starting with 0.  
static AstNode tagArg(AstNode::Param, (void *) 1);
static AstNode argvArg(AstNode::Param, (void *) 1);
static AstNode cmdArg(AstNode::Param, (void *) 4);

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


bool initOS()
{
  //  initialRequests += new instMapping("main", "DYNINSTinit", FUNC_ENTRY);
  // (obsoleted by installBootstrapInst() --ari)

  // A problem exists in mapping these two functions, since FUNC_EXIT
  // is not found very well in the aix version, as of now.
  initialRequests += new instMapping("main", "DYNINSTexit", FUNC_EXIT);

  initialRequests += new instMapping(EXIT_NAME, "DYNINSTexit", FUNC_ENTRY);

  initialRequests += new instMapping("DYNINSTsampleValues", 
				     "DYNINSTreportNewTags", FUNC_ENTRY);

  // we need to instrument __fork instead of fork. fork makes a tail call
  // to __fork, and we can't get the correct return value from fork.
  AstNode *tidArg = new AstNode(AstNode::Param, (void *) 0);
  initialRequests += new instMapping("__fork", "DYNINSTfork", 
				     FUNC_EXIT|FUNC_ARG, tidArg);

  // Trap whenever you hit a dlopen()
  //initialRequests += new instMapping("dlopen", "DYNINSTtrap",
  //FUNC_EXIT);
  // We apparently get that one for free -- any change to the link
  // map shows up as a trap. Cool.
  

  // None of the execs work very well on AIX, this needs to be looked
  // into.
  // initialRequests += new instMapping("rexec", "DYNINSTrexec",
  //                                    FUNC_ENTRY|FUNC_ARG, &cmdArg);
  tidArg = new AstNode(AstNode::Param, (void *) 0);
  initialRequests += new instMapping("execve", "DYNINSTexec", 
                                     FUNC_ENTRY|FUNC_ARG, tidArg);
  // Instrumenting DYNINSTexecFailed at the end of execve is not working
  // well on the aix version, this needs to be looked into more seriously
  // to find out if DYNINSTexecFailed should really be instrumented at the
  // end of execve!
  // We are installing a trampoline at the end of execve, before the last
  // branch out of the function, which branches to a __start function 
  // which calls the new executable; so we are calling DYNINSTexecFailed
  // way to early; since I am not sure where it should go, just by looking
  // at the execve code, I will leave it out for now.
  // initialRequests += new instMapping("execve", "DYNINSTexecFailed", 
  //                                    FUNC_EXIT);

  // ----------------------------------------------------------------------

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

  initialRequests += new instMapping("pvm_send", "DYNINSTrecordTag",
				 FUNC_ENTRY|FUNC_ARG, &tagArg);

  // kludge to get Critical Path to work.
  // XXX - should be tunable constant.
  doPiggy = getenv("DYNINSTdoPiggy");
  if (doPiggy) {
      initialRequests += new instMapping("main", "DYNINSTpvmPiggyInit", FUNC_ENTRY);
      tidArg = new AstNode(AstNode::Param, (void *) 0);
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

rawTime64 getRawTime1970_ns() {
  return getRawTime1970() * 1000;
}

void initWallTimeMgrPlt() {
  getWallTimeMgr().installLevel(wallTimeMgr_t::LEVEL_TWO, yesFunc,
				timeUnit::ns(), timeBase::b1970(),
				&getRawTime1970_ns, "swWallTimeFPtrInfo");
}
