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


#include "paradynd/src/pd_process.h"
#include "paradynd/src/pd_thread.h"
#include "paradynd/src/init.h"
#include "paradynd/src/metricFocusNode.h"


void pd_process::init() {
   for(unsigned i=0; i<dyninst_process->threads.size(); i++) {
      pd_thread *thr = new pd_thread(dyninst_process->threads[i]);
      addThread(thr);
   }

   theVariableMgr = new variableMgr(this, getSharedMemMgr(),
				    maxNumberOfThreads());
}

// fork constructor
pd_process::pd_process(const pd_process &parent, process *childDynProc) :
   dyninst_process(childDynProc), bufStart(0), bufEnd(0)
{
   for(unsigned i=0; i<childDynProc->threads.size(); i++) {
      pd_thread *thr = new pd_thread(childDynProc->threads[i]);
      thr_mgr.addThread(thr);
   }

   theVariableMgr = new variableMgr(*parent.theVariableMgr, this,
				    getSharedMemMgr());
   theVariableMgr->initializeVarsAfterFork();
}

pd_process::~pd_process() {
   delete theVariableMgr;
   delete dyninst_process;
}

bool pd_process::doMajorShmSample() {
   if( !isBootstrappedYet() || !isPARADYNBootstrappedYet()) { //SPLIT ccw 4 jun 2002
      return false;
   }

   bool result = true; // will be set to false if any processAll() doesn't complete
                       // successfully.
   process *dyn_proc = get_dyn_process();

   if(! getVariableMgr().doMajorSample())
      result = false;
   
   if(status() == exited) {
      return false;
   }

   // inferiorProcessTimers used to take in a non-dummy process time as the
   // 2d arg, but it looks like that we need to re-read the process time for
   // each proc timer, at the time of sampling the timer's value, to avoid
   // ugly jagged spikes in histogram (i.e. to avoid incorrect sampled 
   // values).  Come to think of it: the same may have to be done for the 
   // wall time too!!!

   const timeStamp theProcTime = dyn_proc->getCpuTime(0);
   const timeStamp curWallTime = getWallTime();
   // Now sample the observed cost.

   unsigned *costAddr = (unsigned *)dyn_proc->getObsCostLowAddrInParadyndSpace();
   const unsigned theCost = *costAddr; // WARNING: shouldn't we be using a mutex?!

   dyn_proc->processCost(theCost, curWallTime, theProcTime);

   return result;
}

bool pd_process::doMinorShmSample() {
   // Returns true if the minor sample has successfully completed all
   // outstanding samplings.
   bool result = true; // so far...
   
   if(! getVariableMgr().doMinorSample())
      result = false;

   return result;
}

extern pdRPC *tp;

void pd_process::handleExit(int exitStatus) {
   // don't do a final sample for terminated processes
   // this is because there could still be active process timers
   // we can't get a current process time since the process no longer
   // exists, so can't sample these active process timers   
   if(exitStatus == 0) {
      doMajorShmSample();
   }

   reportInternalMetrics(true);

   // close down the trace stream:
   if(getTraceLink() >= 0) {
      //processTraceStream(proc); // can't do since it's a blocking read 
                                  // (deadlock)
      P_close(getTraceLink());      
      setTraceLink(-1);
   }
   metricFocusNode::handleDeletedProcess(this);

   if(multithread_ready()) {
      // retire any thread resources which haven't been retired yet
      threadMgr::thrIter itr = beginThr();
      while(itr != endThrMark()) {
         pd_thread *thr = *itr;
         itr++;
         assert(thr->get_dyn_thread()->get_rid() != NULL);
         tp->retiredResource(thr->get_dyn_thread()->get_rid()->full_name());
      }
   }

   assert(get_rid() != NULL);
   tp->retiredResource(get_rid()->full_name());
   tp->processStatus(getPid(), procExited);
}


