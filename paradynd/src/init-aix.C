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

// $Id: init-aix.C,v 1.34 2004/10/07 00:45:57 jaw Exp $

#include "paradynd/src/internalMetrics.h"
#include "paradynd/src/pd_process.h"
#include "paradynd/src/init.h"
#include "paradynd/src/metricDef.h"
#include "common/h/timing.h"

/* For read_real_time */
#include <sys/systemcfg.h>


bool initOS()
{
// NOTE - the tagArg integer number starting with 0.  
  static BPatch_paramExpr tagArg(1);
  static BPatch_paramExpr argvArg(1);
  static BPatch_paramExpr cmdArg(4);

  static BPatch_paramExpr mpiNormTagArg(4);
  static BPatch_paramExpr mpiNormCommArg(5);
  static BPatch_paramExpr mpiSRSendTagArg(4);
  static BPatch_paramExpr mpiSRCommArg(10);
  static BPatch_paramExpr mpiSRRSendTagArg(4);
  static BPatch_paramExpr mpiSRRCommArg(7);

  static BPatch_paramExpr mpiBcastCommArg(4);
  static BPatch_paramExpr mpiAlltoallCommArg(6);
  static BPatch_paramExpr mpiAlltoallvCommArg(8);
  static BPatch_paramExpr mpiGatherCommArg(7);
  static BPatch_paramExpr mpiGathervCommArg(8);
  static BPatch_paramExpr mpiAllgatherCommArg(6);
  static BPatch_paramExpr mpiAllgathervCommArg(7);
  static BPatch_paramExpr mpiReduceCommArg(6);
  static BPatch_paramExpr mpiAllreduceCommArg(5);
  static BPatch_paramExpr mpiReduceScatterCommArg(5);
  static BPatch_paramExpr mpiScatterCommArg(7);
  static BPatch_paramExpr mpiScattervCommArg(8);
  static BPatch_paramExpr mpiScanCommArg(5);


    initialRequestsPARADYN += new pdinstMapping("DYNINSTsampleValues", 
                                                "DYNINSTreportNewTags", FUNC_ENTRY);

    initialRequestsPARADYN += new pdinstMapping("system", "DYNINSTsystem", 
                                                FUNC_ENTRY);
    //  mpi inst mappings are set to not be noisy if they fail. 
    static const bool no_warn = false;
    pdvector<BPatch_snippet*> argList(2);
    argList[0] = &mpiNormTagArg;
    argList[1] = &mpiNormCommArg;
    initialRequestsPARADYN += new pdinstMapping("MPI_Send", "DYNINSTrecordTagAndGroup",
                                              FUNC_ENTRY|FUNC_ARG, argList, no_warn);
    initialRequestsPARADYN += new pdinstMapping("MPI_Bsend", "DYNINSTrecordTagAndGroup",
                                              FUNC_ENTRY|FUNC_ARG, argList, no_warn);
    initialRequestsPARADYN += new pdinstMapping("MPI_Ssend", "DYNINSTrecordTagAndGroup",
                                              FUNC_ENTRY|FUNC_ARG, argList, no_warn);
    initialRequestsPARADYN += new pdinstMapping("MPI_Isend", "DYNINSTrecordTagAndGroup",
                                              FUNC_ENTRY|FUNC_ARG, argList, no_warn);
    initialRequestsPARADYN += new pdinstMapping("MPI_Issend", "DYNINSTrecordTagAndGroup",
                                              FUNC_ENTRY|FUNC_ARG, argList, no_warn);
    argList[0] = &mpiSRSendTagArg;
    argList[1] = &mpiSRCommArg;
    initialRequestsPARADYN += new pdinstMapping("MPI_Sendrecv", "DYNINSTrecordTagAndGroup",
                                              FUNC_ENTRY|FUNC_ARG, argList, no_warn);
    argList[0] = &mpiSRRSendTagArg;
    argList[1] = &mpiSRRCommArg;
    initialRequestsPARADYN += new pdinstMapping("MPI_Sendrecv_replace",
                                              "DYNINSTrecordTagAndGroup",
                                              FUNC_ENTRY|FUNC_ARG, argList, no_warn);
    initialRequestsPARADYN += new pdinstMapping("MPI_Bcast", "DYNINSTrecordGroup",
                                              FUNC_ENTRY|FUNC_ARG, &mpiBcastCommArg, no_warn);
    initialRequestsPARADYN += new pdinstMapping("MPI_Alltoall", "DYNINSTrecordGroup",
                                              FUNC_ENTRY|FUNC_ARG, &mpiAlltoallCommArg, no_warn);
    initialRequestsPARADYN += new pdinstMapping("MPI_Alltoallv", "DYNINSTrecordGroup",
                                              FUNC_ENTRY|FUNC_ARG, &mpiAlltoallvCommArg, no_warn);
    initialRequestsPARADYN += new pdinstMapping("MPI_Gather", "DYNINSTrecordGroup",
                                              FUNC_ENTRY|FUNC_ARG, &mpiGatherCommArg, no_warn);
    initialRequestsPARADYN += new pdinstMapping("MPI_Gatherv", "DYNINSTrecordGroup",
                                              FUNC_ENTRY|FUNC_ARG, &mpiGathervCommArg, no_warn);
    initialRequestsPARADYN += new pdinstMapping("MPI_Allgather", "DYNINSTrecordGroup",
                                              FUNC_ENTRY|FUNC_ARG, &mpiAllgatherCommArg, no_warn);
    initialRequestsPARADYN += new pdinstMapping("MPI_Allgatherv", "DYNINSTrecordGroup",
                                              FUNC_ENTRY|FUNC_ARG, &mpiAllgathervCommArg, no_warn);
    initialRequestsPARADYN += new pdinstMapping("MPI_Reduce", "DYNINSTrecordGroup",
                                              FUNC_ENTRY|FUNC_ARG, &mpiReduceCommArg, no_warn);
    initialRequestsPARADYN += new pdinstMapping("MPI_Allreduce", "DYNINSTrecordGroup",
                                              FUNC_ENTRY|FUNC_ARG, &mpiAllreduceCommArg, no_warn);
    initialRequestsPARADYN += new pdinstMapping("MPI_Reduce_scatter", "DYNINSTrecordGroup",
                                              FUNC_ENTRY|FUNC_ARG, &mpiReduceScatterCommArg, no_warn);
    initialRequestsPARADYN += new pdinstMapping("MPI_Scatter", "DYNINSTrecordGroup",
                                              FUNC_ENTRY|FUNC_ARG, &mpiScatterCommArg, no_warn);
    initialRequestsPARADYN += new pdinstMapping("MPI_Scatterv", "DYNINSTrecordGroup",
                                              FUNC_ENTRY|FUNC_ARG, &mpiScattervCommArg, no_warn);
    initialRequestsPARADYN += new pdinstMapping("MPI_Scan", "DYNINSTrecordGroup",
                                              FUNC_ENTRY|FUNC_ARG, &mpiScanCommArg, no_warn);
    
    // ===  MULTI-THREADED FUNCTIONS  ======================================
    // Official gotten-from-tracing name. While pthread_create() is the
    // call made from user space, _pthread_body is the parent of any created
    // thread, and so is a good place to instrument.
    pdinstMapping *mapping;
    mapping = new pdinstMapping("_pthread_body", "DYNINST_dummy_create",
                              FUNC_ENTRY, BPatch_callBefore, BPatch_firstSnippet);
    mapping->markAs_MTonly();
    initialRequestsPARADYN.push_back(mapping);


    mapping = new pdinstMapping("pthread_exit", "DYNINSTthreadDelete", 
                              FUNC_ENTRY, BPatch_callBefore, BPatch_lastSnippet);
    mapping->markAs_MTonly();
    initialRequestsPARADYN.push_back(mapping);


    // Should really be the longjmp in the pthread library
    mapping = new pdinstMapping("_longjmp", "DYNINSTthreadStart",
                              FUNC_ENTRY, BPatch_callBefore, BPatch_lastSnippet) ;
    mapping->markAs_MTonly();
    initialRequestsPARADYN.push_back(mapping);


    mapping = new pdinstMapping("_usched_swtch", "DYNINSTthreadStop",
                              FUNC_ENTRY, BPatch_callBefore, BPatch_lastSnippet) ;
    mapping->markAs_MTonly();
    initialRequestsPARADYN.push_back(mapping);


    // Thread SyncObjects
    // mutex
    BPatch_snippet* arg0 = new BPatch_paramExpr(0);
    mapping = new pdinstMapping("pthread_mutex_init", "DYNINSTreportNewMutex", 
                              FUNC_ENTRY|FUNC_ARG, arg0);
    mapping->markAs_MTonly();
    initialRequestsPARADYN.push_back(mapping);

    
    // rwlock
    //
    arg0 = new BPatch_paramExpr(0);
    mapping = new pdinstMapping("pthread_rwlock_init", "DYNINSTreportNewRwLock", 
                              FUNC_ENTRY|FUNC_ARG, arg0);
    mapping->markAs_MTonly();
    initialRequestsPARADYN.push_back(mapping);

    
    //Semaphore
    //
    arg0 = new BPatch_paramExpr(0);
    mapping = new pdinstMapping("i_need_a_name", "DYNINSTreportNewSema", 
                              FUNC_ENTRY|FUNC_ARG, arg0);
    mapping->markAs_MTonly();
    initialRequestsPARADYN.push_back(mapping);

    
    // Conditional variable
    //
    arg0 = new BPatch_paramExpr(0); 
    mapping = new pdinstMapping("pthread_cond_init", "DYNINSTreportNewCondVar", 
                              FUNC_ENTRY|FUNC_ARG, arg0);
    mapping->markAs_MTonly();
    initialRequestsPARADYN.push_back(mapping);
    // =====================================================================
    
    return true;
};

union bigWord {
  uint64_t b64;
  unsigned b32[2];
} bitGrabber;

rawTime64 getRawWallTime_fast() {
  struct timebasestruct timestruct;
  read_real_time(&timestruct, TIMEBASE_SZ);
  bitGrabber.b32[0] = timestruct.tb_high;
  bitGrabber.b32[1] = timestruct.tb_low;
  return bitGrabber.b64;
}

rawTime64 fastUnits2ns(rawTime64 fastUnits) {
  struct timebasestruct timestruct;
  timestruct.flag = 2;
  bitGrabber.b64 = fastUnits;
  timestruct.tb_high = bitGrabber.b32[0];
  timestruct.tb_low = bitGrabber.b32[1];    
  time_base_to_time(&timestruct, TIMEBASE_SZ);
  rawTime64 now = static_cast<rawTime64>(timestruct.tb_high);
  now *= I64_C(1000000000);
  now += static_cast<rawTime64>(timestruct.tb_low);
  return now;
}

rawTime64 getRawWallTime_ns() {
  struct timebasestruct timestruct;
  read_real_time(&timestruct, TIMEBASE_SZ);
  time_base_to_time(&timestruct, TIMEBASE_SZ);
  /*   ts.tb_high is seconds, ts.tb_low is nanos */
  rawTime64 now = static_cast<rawTime64>(timestruct.tb_high);
  now *= I64_C(1000000000);
  now += static_cast<rawTime64>(timestruct.tb_low);
  return now;
}

void initWallTimeMgrPlt() {
  timeStamp curTime = getCurrentTime();  // general util one
  int64_t fastUnits = getRawWallTime_fast();
  int64_t cur_ns = fastUnits2ns(fastUnits);
  timeLength fast_timeLength(cur_ns, timeUnit::ns());
  timeStamp beg_fasttime = curTime - fast_timeLength;
  timeBase fastTimeBase(beg_fasttime);

  getWallTimeMgr().installLevel(wallTimeMgr_t::LEVEL_ONE, yesFunc,
				&fastUnits2ns, fastTimeBase,
				&getRawWallTime_fast, "hwWallTimeFPtrInfo");

  getWallTimeMgr().installLevel(wallTimeMgr_t::LEVEL_TWO, yesFunc,
				timeUnit::ns(), timeBase::b1970(),
				&getRawWallTime_ns, "swWallTimeFPtrInfo");
}
