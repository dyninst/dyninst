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

// $Id: metricFocusNode.C,v 1.237 2003/03/04 19:16:18 willb Exp $

#include "common/h/headers.h"
#include "common/h/Types.h"
#include <assert.h>

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "pdutil/h/sampleAggregator.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/dyninstP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "paradynd/src/comm.h"
#include "paradynd/src/internalMetrics.h"
#include "paradynd/src/init.h"
#include "paradynd/src/perfStream.h"
#include "paradynd/src/main.h"
#include "dyninstAPI/src/stats.h"
#include "paradynd/src/dynrpc.h"
#include "paradynd/src/mdld.h"
#include "common/h/Timer.h"
#include "dyninstAPI/src/showerror.h"
#include "paradynd/src/costmetrics.h"
#include "paradynd/src/metricFocusNode.h"
#include "paradynd/src/machineMetFocusNode.h"
#include "paradynd/src/processMetFocusNode.h"
#include "paradynd/src/threadMetFocusNode.h"
#include "common/h/debugOstream.h"
#include "pdutil/h/pdDebugOstream.h"
#include "common/h/timing.h"
#include "paradyn/src/met/mdl_data.h"
#include "paradynd/src/focus.h"
#include "paradynd/src/processMgr.h"
#include "paradynd/src/pd_process.h"

#ifdef FREEDEBUG
#if defined(i386_unknown_nt4_0)
#  include <strstrea.h>
#else
#  include <strstream.h>
#endif
//#include <strstream.h>  // in flush_batch_buffer
#endif

#include "dyninstAPI/src/instPoint.h"

// The following vrbles were defined in process.C:

extern unsigned enable_pd_attach_detach_debug;

#if ENABLE_DEBUG_CERR == 1
#define attach_cerr if (enable_pd_attach_detach_debug) cerr
#else
#define attach_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_inferior_rpc_debug;

#if ENABLE_DEBUG_CERR == 1
#define inferiorrpc_cerr if (enable_pd_inferior_rpc_debug) cerr
#else
#define inferiorrpc_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_shm_sampling_debug;

#if ENABLE_DEBUG_CERR == 1
#define shmsample_cerr if (enable_pd_shm_sampling_debug) cerr
#else
#define shmsample_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_fork_exec_debug;

#if ENABLE_DEBUG_CERR == 1
#define forkexec_cerr if (enable_pd_fork_exec_debug) cerr
#else
#define forkexec_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_metric_debug;

#if ENABLE_DEBUG_CERR == 1
#define metric_cerr if (enable_pd_metric_debug) cerr
#else
#define metric_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_samplevalue_debug;

#if ENABLE_DEBUG_CERR == 1
#define sampleVal_cerr if (enable_pd_samplevalue_debug) cerr
#else
#define sampleVal_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned inferiorMemAvailable;
extern pdvector<Address> getAllTrampsAtPoint(instInstance *instance);

void flush_batch_buffer();
void batchSampleData(string metname, int mid, timeStamp startTimeStamp, 
		     timeStamp endTimeStamp, pdSample value, 
		     bool internal_metric);

timeLength currentPredictedCost = timeLength::Zero();

unsigned mdnHash(const metricFocusNode *&mdn) {
  return ((unsigned)(Address)mdn) >> 2; // assume all addrs are 4-byte aligned
  //  return ((unsigned) mdn);
}

pdvector<internalMetric*> internalMetric::allInternalMetrics;

// used to indicate the mi is no longer used.
#define DELETED_MI 1
#define MILLION 1000000.0

/* No longer used
   bool mdl_internal_metric_data(const string& metric_name, mdl_inst_data& result) {
   unsigned size = internalMetric::allInternalMetrics.size();
   for (unsigned u=0; u<size; u++) {
   internalMetric *theIMetric = internalMetric::allInternalMetrics[u];
   if (theIMetric->name() == metric_name) {
   result.aggregate = theIMetric->aggregate();
   result.style = theIMetric->style();
   return true;
   }
   }
   
   for (unsigned u2=0; u2< costMetric::allCostMetrics.size(); u2++) {
   if (costMetric::allCostMetrics[u2]->name() == metric_name) {
   result.aggregate = costMetric::allCostMetrics[u2]->aggregate();
   result.style = costMetric::allCostMetrics[u2]->style();
   return true;
   } 
   }
   
   return (mdl_metric_data(metric_name, result));
   }
*/


// for NON_MT_THREAD:  PRIM_MDN is non-aggregate
// for MT_THREAD:  THR_LEV could be aggregate (component being PROC_COMP)
metricFocusNode::metricFocusNode()
{
}

void metricFocusRequestCallbackInfo::makeCallback(const pdvector<int> &returnIDs,
						  const pdvector<u_int> &mi_ids)
{
   assert(returnIDs.size() == mi_ids.size());
   tp->enableDataCallback(daemon_id, returnIDs, mi_ids, request_id);
}  

// check for "special" metrics that are computed directly by paradynd 
// if a cost of an internal metric is asked for, enable=false
machineMetFocusNode *doInternalMetric(int mid, 
				      const Focus &focus,
				      const string& metric_name, 
				      bool enable, bool& matched)
{
  // called by createMetricInstance, below.
  // return values:
  //   a valid metricFocusNode* when successful
  //   -1 --> enable was false
  //   -2 --> not legal to instrument this focus
  //   NULL --> a more serious error (probably metric-is-unknown)

  matched = false;
  machineMetFocusNode *mn = 0; 

  // check to see if this is an internal metric
  unsigned im_size = internalMetric::allInternalMetrics.size();

  for (unsigned im_index=0; im_index<im_size; im_index++){
    internalMetric *theIMetric = internalMetric::allInternalMetrics[im_index];
    if (theIMetric->name() == metric_name) {
      matched = true;
      if (!enable)
	return (machineMetFocusNode*)-1;

      if (!theIMetric->legalToInst(focus))
	// Paradyn will handle this case and report appropriate error msg
	return (machineMetFocusNode*)-2;

      // it's required that the internal metric's mdn be a "top level node"
      // (ie. AGG_MDN or AGG_MDN) in order for setInitialActualValue to send
      // the value the the front-end
      pdvector<processMetFocusNode*> noParts;
      mn = new machineMetFocusNode(mid, metric_name, focus, noParts, 
				   theIMetric->aggregate(), enable);
      assert(mn);
      
      unsigned instIndex = theIMetric->enableNewInstance(mn);
      theIMetric->getEnabledInstance(instIndex).setStartTime(getWallTime());
      return(mn);
    }
  }

  // check to see if this is a cost metric
  for (unsigned i=0; i < costMetric::allCostMetrics.size(); i++){
    if(costMetric::allCostMetrics[i]->name() == metric_name){
      matched = true;
      if (!enable)
	return (machineMetFocusNode*)-1;

      costMetric *nc = costMetric::allCostMetrics[i];

      if (!nc->legalToInst(focus))
	return (machineMetFocusNode*)-2;
      pdvector<processMetFocusNode*> noParts;
      mn = new machineMetFocusNode(mid, metric_name, focus, noParts, 
				   nc->aggregate(), enable);
      assert(mn);

      nc->enable(mn); 

      return(mn);
    }
  }

  // No matches found among internal or cost metrics
  return NULL;
}

machineMetFocusNode *createMetricInstance(int mid, string& metric_name, 
		        pdvector<u_int>& focusData,
		        bool enable) // true if for real; false for guessCost()
{
   // we make third parameter false to avoid printing warning messages in
   // focus2CanonicalFocus ("enable" was here previously) - naim
   
   bool errFlag = false;
   Focus &focus = *(new Focus(focusData, &errFlag));
   if(errFlag) {
      return NULL;
   }

   if (mdl_can_do(metric_name)) {
      /* select the processes that should be instrumented. We skip process
	 that have exited, and processes that have been created but are not
	 completely initialized yet.  If we try to insert instrumentation in
	 a process that is not ready yet, we get a core dump.  A process is
	 ready when it is not in neonatal state and the isBootstrappedYet
	 returns true.
      */
      pdvector<pd_process*> procs;

      processMgr::procIter itr = getProcMgr().begin();
      while(itr != getProcMgr().end()) {
	 pd_process *curProc = *itr++;
	 if(curProc->status()==exited || curProc->status()==neonatal || 
	    curProc->isBootstrappedYet())
	 {
	    procs += curProc;
	 }
      }
      
      if (procs.size() == 0)
	 // there are no processes to instrument
      {	    
	 //printf("createMetricInstance failed, no processes to instrument\n");
	 return NULL;
      }
      
      machineMetFocusNode *machNode = 
	 makeMachineMetFocusNode(mid, focus, metric_name, procs, false,enable);
      
      if (machNode == NULL) {
	 metric_cerr << "createMetricInstance failed since mdl_do failed\n";
	 metric_cerr << "metric name was " << metric_name << "; focus was ";
	 metric_cerr << "createMetricInstance failed since mdl_do failed\n";
      }
      return machNode;
   } else {
      bool matched;
      machineMetFocusNode *machNode = 
	 doInternalMetric(mid, focus, metric_name, enable, matched);
      // NULL on serious error; -1 if enable was false; -2 if illegal to
      // instr with given focus [many internal metrics work only for whole
      // program]

      if (machNode == (machineMetFocusNode*)-2) {
	 metric_cerr << "createMetricInstance: internal metric " 
		     << metric_name << " isn't defined for focus: " 
		     << focus.getName() << "\n";
	 machNode = NULL; // straighten up the return value
      }
      else if (machNode == (machineMetFocusNode*)-1) {
	 metric_cerr << " createMetricInstance: internal metric not enable: " 
		     << metric_name << endl;
	 assert(!enable); // no error msg needed
	 machNode = NULL; // straighten up the return value
      }
      else if (machNode == NULL) {
	 // more serious error...do a printout
	 metric_cerr 
	    << "createMetricInstance failed since doInternalMetric failed\n";
	 metric_cerr << "metric name was " << metric_name << "; focus was "
		     << focus.getName() << "\n";
      }
      if(machNode)  machNode->markAsInternalMetric();
      return machNode;
   }
}


// propagate this metric instance to process p.
// p is a process that started after the metric instance was created
// note: don't call this routine for a process started via fork or exec, just
// for processes started the "normal" way.
// "this" is an aggregate(AGG_MDN or AGG_MDN) mi, not a component one.

void metricFocusNode::handleNewProcess(process *p) {
   pdvector<machineMetFocusNode *> allMachNodes;
   machineMetFocusNode::getMachineNodes(&allMachNodes);

   pd_process *pd_proc = getProcMgr().find_pd_process(p);

   for (unsigned j=0; j < allMachNodes.size(); j++) {
      machineMetFocusNode *curNode = allMachNodes[j];
      curNode->propagateToNewProcess(pd_proc);
   }
}

// Remove the aggregate metric instances that don't have any components left
void metricFocusNode::handleDeletedProcess(pd_process *proc) {
   metric_cerr << "removeFromMetricInstances- proc: " << proc << ", pid: " 
	       << proc->getPid() << "\n";

   pdvector<processMetFocusNode *> greppedProcNodes;
   processMetFocusNode::getProcNodes(&greppedProcNodes, proc->getPid());
   for(unsigned i=0; i<greppedProcNodes.size(); i++) {
      if (greppedProcNodes[i]->isBeingDeleted()) continue;
      machineMetFocusNode *machNode = greppedProcNodes[i]->getParent();
      machNode->deleteProcNode(greppedProcNodes[i]);
      // what about internal metrics?
      costMetric::removeProcessFromAll(proc->get_dyn_process());
   }
}

void metricFocusNode::handleNewThread(pd_process *proc, pd_thread *thr) {
   pdvector<processMetFocusNode *> procNodes;
   assert(proc->multithread_ready());
   processMetFocusNode::getProcNodes(&procNodes, proc->getPid());
   for(unsigned i=0; i<procNodes.size(); i++) {
      procNodes[i]->propagateToNewThread(thr);
   }
}

void metricFocusNode::handleDeletedThread(pd_process *proc, pd_thread *thr) {
   pdvector<processMetFocusNode *> procNodes;
   assert(proc->multithread_ready());
   processMetFocusNode::getProcNodes(&procNodes, proc->getPid());
   for(unsigned i=0; i<procNodes.size(); i++) {
      procNodes[i]->updateForDeletedThread(thr);
   }
}

// called by forkProcess of context.C, just after the fork-constructor was
// called for the child process.
void metricFocusNode::handleFork(const pd_process *parent, pd_process *child)
{
   pdvector<machineMetFocusNode *> allMachNodes;
   machineMetFocusNode::getMachineNodes(&allMachNodes);

   pdvector<processMetFocusNode *> procNodesToUnfork;

   for (unsigned j=0; j < allMachNodes.size(); j++) {
      machineMetFocusNode *curNode = allMachNodes[j];
      curNode->propagateToForkedProcess(parent, child, &procNodesToUnfork);
   }
   for(unsigned k=0; k<procNodesToUnfork.size(); k++) {
      // don't want to use machineMetFocusNode::deleteProcNode because
      // this procNode hasn't been registered yet with a machineMetFocusNode
      procNodesToUnfork[k]->unFork();
   }
}

void metricFocusNode::handleExec(pd_process *pd_proc) {
   pdvector<machineMetFocusNode *> allMachNodes;
   machineMetFocusNode::getMachineNodes(&allMachNodes);

   for (unsigned j=0; j < allMachNodes.size(); j++) {
      machineMetFocusNode *curNode = allMachNodes[j];
      curNode->adjustForExecedProcess(pd_proc);
   }
}

// startCollecting is called by dynRPC::enableDataCollection 
// (or enableDataCollection2) in dynrpc.C
// startCollecting is a friend of metricFocusNode; can it be
// made a member function of metricFocusNode instead?
// Especially since it clearly is an integral part of the class;
// in particular, it sets the crucial vrble "id_"
//
instr_insert_result_t startCollecting(string& metric_name, 
      pdvector<u_int>& focus, int mid, metricFocusRequestCallbackInfo *cbi)
{
   // Make the unique ID for this metric/focus visible in MDL.
   string vname = "$globalId";
   mdl_env::add(vname, false, MDL_T_INT);
   mdl_env::set(mid, vname);

   machineMetFocusNode *machNode = 
     createMetricInstance(mid, metric_name, focus, true);

   if (!machNode) {
      metric_cerr << "startCollecting for " << metric_name 
		  << " failed because createMetricInstance failed" << endl;
      return insert_failure;
   }

   //cerr << "created metric-focus " << machNode->getFullName() << "\n";   
   addCurrentPredictedCost(machNode->cost());
   metResPairsEnabled++;
   
   if (machNode->isInternalMetric()) {
      return insert_success;
   }

   instr_insert_result_t insert_status =  machNode->insertInstrumentation();
   if(insert_status == insert_deferred) {
      machNode->setMetricFocusRequestCallbackInfo(cbi);
      return insert_deferred;
   } else if(insert_status == insert_failure) {
      // error message already displayed in processMetFocusNode::insertInstrum.
      delete machNode;
      return insert_failure;
   }

   // This has zero for an initial value.  This is because for cpu_time and
   // wall_time, we just want to total the cpu_time and wall_time for this
   // process and no others (but if we want someone to get an actual cpu time
   // for this program even if they start the cpu_time metric after the start
   // of the process, the initial actual value could be the actual cpu time
   // at the start of this metric).  For the counter metrics (eg. proc_calls,
   // io_bytes), we also want zero (we have no way of getting the total
   // proc_calls & io_bytes of the process before the metric was enabled, so
   // we have to use zero).  However, it is possible that in the future we'll
   // create a metric where it makes sense to send an initial actual value.
   machNode->initializeForSampling(getWallTime(), pdSample::Zero());

   return insert_success;
}

timeLength guessCost(string& metric_name, pdvector<u_int>& focus) {
    // called by dynrpc.C (getPredictedDataCost())
   static int tempMetFocus_ID = -1;

   machineMetFocusNode *mi = 
      createMetricInstance(tempMetFocus_ID, metric_name, focus, false);
   tempMetFocus_ID--;

    if (!mi) {
       metric_cerr << "guessCost returning 0.0 since createMetricInstance failed" << endl;
       return timeLength::Zero();
    }

    timeLength cost = mi->cost();
    delete mi;

    return cost;
}

// NOTE: This stuff (flush_batch_buffer() and batchSampleData()) belongs
//       in perfStream.C; this is an inappropriate file.

//////////////////////////////////////////////////////////////////////////////
// Buffer the samples before we actually send it                            //
//      Send it when the buffers are full                                   //
//      or, send it when the last sample in the interval has arrived.       //
//////////////////////////////////////////////////////////////////////////////

const unsigned SAMPLE_BUFFER_SIZE = (1*1024)/sizeof(T_dyninstRPC::batch_buffer_entry);
bool BURST_HAS_COMPLETED = false;
   // set to true after a burst (after a processTraceStream(), or sampleNodes for
   // the CM5), which will force the buffer to be flushed before it fills up
   // (if not, we'd have bad response time)

pdvector<T_dyninstRPC::batch_buffer_entry> theBatchBuffer (SAMPLE_BUFFER_SIZE);
unsigned int batch_buffer_next=0;

// The following routines (flush_batch_buffer() and batchSampleData() are
// in an inappropriate src file...move somewhere more appropriate)
void flush_batch_buffer() {
   // don't need to flush if the batch had no data (this does happen; see
   // perfStream.C)

  if (batch_buffer_next == 0) {
    return;
  }


   // alloc buffer of the exact size to make communication
   // more efficient.  Why don't we send theBatchBuffer with a count?
   // This would work but would always (in the igen call) copy the entire
   // vector.  This solution has the downside of calling new but is not too bad
   // and is clean.
   pdvector<T_dyninstRPC::batch_buffer_entry> copyBatchBuffer(batch_buffer_next);
   assert(copyBatchBuffer.size() <= theBatchBuffer.size());
   for (unsigned i=0; i< batch_buffer_next; i++) {
      copyBatchBuffer[i] = theBatchBuffer[i];
   }

#ifdef FREEDEBUG
   timeStamp t1 = getWallTime();
#endif

   // Now let's do the actual igen call!
   tp->batchSampleDataCallbackFunc(0, copyBatchBuffer);

#ifdef FREEDEBUG
   timeStamp t2 = getWallTime();
   if (t2-t1 > 15*timeLength::sec()) {
     ostrstream errorLine;
     errorLine << "++--++ TEST ++--++ batchSampleDataCallbackFunc took " << 
       t2-t1 << ", size= << " << sizeof(T_dyninstRPC::batch_buffer_entry) << 
       ", Kbytes=", << (sizeof(T_dyninstRPC::batch_buffer_entry) * 
			copyBatchBuffer.size()/1024.0F);
     logLine(errorLine);
   }
#endif

   BURST_HAS_COMPLETED = false;
   batch_buffer_next = 0;
}

// temporary until front-end's pipeline gets converted
u_int isMetricTimeType(const string& met_name) {
  unsigned size = mdl_data::all_metrics.size();
  T_dyninstRPC::metricInfo element;
  unsigned u;
  for (u=0; u<size; u++) {
    //cerr << "checking " << met_name << " against " << mdl_data::all_metrics[u]->name_ << "\n";
    if (mdl_data::all_metrics[u]->name_ == met_name) {
      u_int mtype = mdl_data::all_metrics[u]->type_;
      return (mtype == MDL_T_PROC_TIMER || mtype == MDL_T_WALL_TIMER);
    }
  }

  unsigned isize = internalMetric::allInternalMetrics.size();
  for (u=0; u<isize; u++) {
    T_dyninstRPC::metricInfo metInfo;
    metInfo = internalMetric::allInternalMetrics[u]->getInfo();
    //cerr << "checking " << met_name << " against " << metInfo.name << "\n";
    if(met_name == metInfo.name) {
      return (metInfo.unitstype == Normalized);
    }
  }
  for (unsigned u2=0; u2< costMetric::allCostMetrics.size(); u2++) {
    T_dyninstRPC::metricInfo metInfo;
    metInfo = costMetric::allCostMetrics[u2]->getInfo();
    //cerr << "checking " << met_name << " against " << metInfo.name << "\n";
    if(met_name == metInfo.name) {
      return (metInfo.unitstype == Normalized);
    }
  }

  //  cerr << "mdl_get_type: mid " << met_name << " not found\n";
  assert(0);
  return 0;
}

// the metname is temporary, get rid of this 
void batchSampleData(string metname, int mid, timeStamp startTimeStamp, 
		     timeStamp endTimeStamp, pdSample value) 
{
   // This routine is called where we used to call tp->sampleDataCallbackFunc.
   // We buffer things up and eventually call tp->batchSampleDataCallbackFunc

#ifdef notdef
   char myLogBuffer[120] ;
   sprintf(myLogBuffer, "mid %d, value %g\n", mid, value.getValue()) ;
   logLine(myLogBuffer) ;
#endif

   sampleVal_cerr << "batchSampleData - metric: " << metname.c_str() 
		  << "  mid: " << mid << ", startTimeStamp: " <<startTimeStamp
		  << ", endTimeStamp: " << endTimeStamp << "value: " 
		  << value << "\n";

   // Flush the buffer if (1) it is full, or (2) for good response time, after
   // a burst of data:
   if (batch_buffer_next >= SAMPLE_BUFFER_SIZE || BURST_HAS_COMPLETED)
      flush_batch_buffer();

   // Now let's batch this entry.
   T_dyninstRPC::batch_buffer_entry &theEntry = theBatchBuffer[batch_buffer_next];
   theEntry.mid = mid;
   theEntry.startTimeStamp = startTimeStamp.getD(timeUnit::sec(), 
						 timeBase::bStd());
   theEntry.endTimeStamp = endTimeStamp.getD(timeUnit::sec(),timeBase::bStd());

   double bval = static_cast<double>(value.getValue());
   if(isMetricTimeType(metname)) {
     sampleVal_cerr << metname.c_str() << " is a time metric type: normalizing\n";
     bval /= 1000000000.0;
   }
   theEntry.value = bval;

   sampleVal_cerr << ">b2 startTimeStamp d: " << theEntry.startTimeStamp
   		  << ", endTimeStamp d: " << theEntry.endTimeStamp
   		  << ", value d: " << theEntry.value << "\n";

   batch_buffer_next++;
}

//////////////////////////////////////////////////////////////////////////////
// Buffer the traces before we actually send it                            //
//      Send it when the buffers are full                                   //
//      or, send it when the last sample in the interval has arrived.       //
//////////////////////////////////////////////////////////////////////////////

const unsigned TRACE_BUFFER_SIZE = 10;
bool TRACE_BURST_HAS_COMPLETED = false;
   // set to true after a burst (after a processTraceStream(), or sampleNodes for
   // the CM5), which will force the buffer to be flushed before it fills up
   // (if not, we'd have bad response time)

pdvector<T_dyninstRPC::trace_batch_buffer_entry> theTraceBatchBuffer (TRACE_BUFFER_SIZE);
unsigned int trace_batch_buffer_next=0;

void flush_trace_batch_buffer(int program) {
   // don't need to flush if the batch had no data (this does happen; see
   // perfStream.C)
   if (trace_batch_buffer_next == 0)
      return;

   pdvector<T_dyninstRPC::trace_batch_buffer_entry> copyTraceBatchBuffer(trace_batch_buffer_next);
   for (unsigned i=0; i< trace_batch_buffer_next; i++)
      copyTraceBatchBuffer[i] = theTraceBatchBuffer[i];

   // Now let's do the actual igen call!

   tp->batchTraceDataCallbackFunc(program, copyTraceBatchBuffer);

   TRACE_BURST_HAS_COMPLETED = false;
   trace_batch_buffer_next = 0;
}

void batchTraceData(int program, int mid, int recordLength,
                     char *recordPtr)
{
   // Now let's batch this entry.
   T_dyninstRPC::trace_batch_buffer_entry &theEntry = theTraceBatchBuffer[trace_batch_buffer_next];
   theEntry.mid = mid;
   theEntry.length = recordLength;
   theEntry.traceRecord = byteArray(recordPtr,recordLength);
   trace_batch_buffer_next++;

   // We buffer things up and eventually call tp->batchTraceDataCallbackFunc

   // Flush the buffer if (1) it is full, or (2) for good response time, after
   // a burst of data:
   if (trace_batch_buffer_next >= TRACE_BUFFER_SIZE || TRACE_BURST_HAS_COMPLETED) {
      flush_trace_batch_buffer(program);
   }

}


//
// Costs are now reported to paradyn like other metrics (ie. we are not
// calling reportInternalMetrics to deliver cost values, instead we wait
// until we have received a new interval of cost data from each process)
// note: this only works for the CM5 because all cost metrics are sumed
// at the daemons and at paradyn, otherwise the CM5 needs its own version
// of this routine that uses the same aggregate method as the one for paradyn 
//

/* ************************************************************************* */

/* **************************** */

void reportInternalMetrics(bool force) 
{
  if (isApplicationPaused())
    return; // we don't sample when paused (is this right?)

  // see if we have a sample to establish time base.
  if (!isInitFirstRecordTime())
    return;

  static timeStamp lastSampleTime;
  const  timeStamp now = getWallTime();

  if(! lastSampleTime.isInitialized()) {
    lastSampleTime = now;
    return;
  }

  //  check if it is time for a sample
  if (!force && now < lastSampleTime + getCurrSamplingRate())
    return;

  unsigned ai_size = internalMetric::allInternalMetrics.size();
  for (unsigned u2=0; u2<ai_size; u2++) {
    internalMetric *theIMetric = internalMetric::allInternalMetrics[u2];
    // Loop thru all enabled instances of this internal metric...
    
    for (unsigned v=0; v < theIMetric->num_enabled_instances(); v++) {
      internalMetric::eachInstance &theInst =theIMetric->getEnabledInstance(v);
      theInst.updateValue(now, theInst.calcValue());
    }
  }
  lastSampleTime = now;
}

void disableAllInternalMetrics() {
    flush_batch_buffer();
    for (unsigned u=0; u < internalMetric::allInternalMetrics.size(); u++) {
      internalMetric *theIMetric = internalMetric::allInternalMetrics[u];

      // Now loop thru all the enabled instances of this internal metric...
      while (theIMetric->num_enabled_instances() > 0) {
 	internalMetric::eachInstance &theInstance = theIMetric->getEnabledInstance(0);
	tp->endOfDataCollection(theInstance.getMetricID());
	theIMetric->disableInstance(0);
      }
    }  
}




// ======================


//#if !defined(MT_THREAD)
// going to dyninstAPI/src/instPoint-alpha, mips, power, sparc, x86.h

// instPoint-alpha.h, instPoint-power.h, instPoinst-sparc.h
// addr_ in instPoint-mips.h and instPoint-x86.h
// moved to individual files
/*
bool instPoint::match(instPoint *p)
{
  if (this == p)
    return true;

  // should we check anything else?
  if (addr == p->addr)
    return true;
  
  return false;
}
*/

/*
#if defined(MT_THREAD)
bool level_index_match(unsigned level1, unsigned level2,
		       unsigned index1, unsigned index2,
		       pdvector<dataReqNode*> &data_tuple1, // initialization?
		       pdvector<dataReqNode*> &data_tuple2,
		       pdvector<dataReqNode*> datareqs1,
		       pdvector<dataReqNode*> datareqs2)
{
  // defined in mdl.C
  extern int index_in_data(unsigned lev, unsigned ind, pdvector<dataReqNode*>& data);

  int match_index1 = index_in_data(level1, index1, data_tuple1);
  int match_index2 = index_in_data(level2, index2, data_tuple2);
  
  // CHECK IS THE SAME AS IN variable_address_match
  // v1 get matched in data_tuple1
  if (match_index1 >= 0) {
    // v2 get matched in data_tuple2
    if (match_index2 >= 0) {
      return (match_index1 == match_index2);
    }
    // v2 not get matched in data_tuple2
    else {
      return false;
    }
  }
  
  // now v1 not matched in data_tuple1
  match_index1 = index_in_data(level1, index1, datareqs1);
  match_index2 = index_in_data(level2, index2, datareqs2);
  

  // CHECK IS THE SAME AS IN variable_address_match
  // v1 get matched in datareqs1
  if (match_index1 >= 0) {
    // v2 get matched in datareqs2
    if (match_index2 >= 0) {
      // get a new match pair @@
      data_tuple1 += datareqs1[match_index1];
      data_tuple2 += datareqs2[match_index2];
      return true;
    }
    // v2 not get matched in datareqs2
    else {
      return false;
    }
  }
  
  // neither v1 or v2 get matched in their data tuples or reqs, respectively
  return ((level1 == level2) && (index1 == index2));
}
#else
bool variable_address_match(Address v1, Address v2, 
			    pdvector<dataReqNode*> &data_tuple1, // initialization?
			    pdvector<dataReqNode*> &data_tuple2,
			    pdvector<dataReqNode*> datareqs1,
			    pdvector<dataReqNode*> datareqs2)
{
  // defined in mdl.C
  extern int index_in_data(Address v, pdvector<dataReqNode*>& data);

  int match_index1 = index_in_data(v1, data_tuple1);
  int match_index2 = index_in_data(v2, data_tuple2);
  
  // v1 get matched in data_tuple1
  if (match_index1 >= 0) {
    // v2 get matched in data_tuple2
    if (match_index2 >= 0) {
      return (match_index1 == match_index2);
    }
    // v2 not get matched in data_tuple2
    else {
      return false;
    }
  }
  
  // now v1 not matched in data_tuple1
  match_index1 = index_in_data(v1, datareqs1);
  match_index2 = index_in_data(v2, datareqs2);
  
  // v1 get matched in datareqs1
  if (match_index1 >= 0) {
    // v2 get matched in datareqs2
    if (match_index2 >= 0) {
      // get a new match pair @@
      data_tuple1 += datareqs1[match_index1];
      data_tuple2 += datareqs2[match_index2];
      return true;
    }
    // v2 not get matched in datareqs2
    else {
      return false;
    }
  }
  
  // neither v1 or v2 get matched in their data tuples or reqs, respectively
  return (v1 == v2);
}
#endif
*/

/*
bool AstNode::condMatch(AstNode* a,
			pdvector<dataReqNode*> &data_tuple1, // initialization?
			pdvector<dataReqNode*> &data_tuple2,
			pdvector<dataReqNode*> datareqs1,
			pdvector<dataReqNode*> datareqs2)
{

  unsigned i;


  if (this == a)
    return true;
  
  // compare node type
  if (type != a->type)
    return false;
  
  // what to do with "size" (size of the operations in bytes)
  
  switch (type)
    {
    case sequenceNode:
      assert ((loperand != NULL) &&
	      (roperand != NULL) &&
	      (a->loperand != NULL) &&
	      (a->roperand != NULL));
      
      if ((loperand->condMatch(a->loperand, data_tuple1, data_tuple2, datareqs1, datareqs2)) &&
	  (roperand->condMatch(a->roperand, data_tuple1, data_tuple2, datareqs1, datareqs2)))
	return true;
      else
	return false;

      break;
      
    case opCodeNode:
      // compare operator code
      if (op != a->op)
	return false;
      
      switch (op)
	{
	case funcJumpOp:
	  if ((callee == a->callee) &&
	      (calleefunc->match(a->calleefunc)))  // NEED WORK
	    return true;
	  else
	    return false;
	  
	  break;

	default:
	  // assume:
	  // if 1 operand,  it is loperand
	  // if 2 operands, they are loperand and roperand
	  // if 3 operands, they are loperand, roperand and eoperand
	  if ((loperand != NULL) && (a->loperand != NULL) &&
	      (roperand == NULL) && (a->roperand == NULL) &&
	      (eoperand == NULL) && (a->eoperand == NULL))
	    return (loperand->condMatch(a->loperand, data_tuple1, data_tuple2, 
					datareqs1, datareqs2));
	  
	  else if ((loperand != NULL) && (a->loperand != NULL) &&
		   (roperand != NULL) && (a->roperand != NULL) &&
		   (eoperand == NULL) && (a->eoperand == NULL))
	    return ((loperand->condMatch(a->loperand, data_tuple1, data_tuple2, datareqs1, datareqs2)) &&
		    (roperand->condMatch(a->roperand, data_tuple1, data_tuple2, datareqs1, datareqs2)));

	  else if ((loperand != NULL) && (a->loperand != NULL) &&
		   (roperand != NULL) && (a->roperand != NULL) &&
		   (eoperand != NULL) && (a->eoperand != NULL))
	    return ((loperand->condMatch(a->loperand, data_tuple1, data_tuple2, datareqs1, datareqs2)) &&
		    (roperand->condMatch(a->roperand, data_tuple1, data_tuple2, datareqs1, datareqs2)) &&
		    (eoperand->condMatch(a->eoperand, data_tuple1, data_tuple2, datareqs1, datareqs2)));
	  else
	    return false;

	  break;
	}

      break;
      
    case operandNode:
      // compare operand type
      if (oType != a->oType)
	return false;

      switch (oType)
	{ // need to know the EXACT ovalue type to compare
	case Constant:  // NEED WORK, WHAT ABOUT ADDRESSES, or other types
	  return ((int) oValue == (int) a->oValue);
	  
	case ConstantPtr:  // doesn't seem to be used anywhere
	  break;

	case ConstantString:
	  return (strcmp((char *)oValue, (char *)a->oValue) == 0);
	  
#if defined(MT_THREAD)
	case OffsetConstant:
	  return ((isLevel == a->isLevel) &&
		  (level_index_match(lvl, a->lvl, idx, a->idx,
				     data_tuple1, data_tuple2, datareqs1, datareqs2)));
#else
	  // restore DataValue and DataPtr
	case DataPtr:  // used in: createTimer, mdl.C MDL_ADDRESS &v
	  return (variable_address_match((Address)oValue, (Address)a->oValue,
					 data_tuple1, data_tuple2, datareqs1, datareqs2));
	  
	case DataValue:  // used in: createCounter, mdl.C MDL_T_DRN, if (flag)
	  return (variable_address_match((Address)oValue, (Address)a->oValue,
					 data_tuple1, data_tuple2, datareqs1, datareqs2));
#endif

	case DataId:  // doesn't seem to be used anywhere
	  break;

	case DataIndir:
	  assert((loperand != NULL) &&
		 (a->loperand != NULL));

	  return (loperand->condMatch(a->loperand, data_tuple1, data_tuple2, 
				      datareqs1, datareqs2));
	  
	case DataReg:  // used once in ast.C (computeAddress)
	  return ((unsigned) oValue == (unsigned) a->oValue);

	case Param:  // or should it be "unsigned"
	  return ((int) oValue == (int) a->oValue);

	case ReturnVal:  // or should it be "unsigned"
	  return ((int) oValue == (int) a->oValue);
	  
#if defined(MT_THREAD) // Need work: check type!!
	case DataAddr:
	  return ((void *) oValue == (void *) a->oValue);
#else  // Need work: check for first 2 or 3 cases!!!
	case DataAddr:  // Address is unsigned; used in mdl.C -- arg to func
	  // return ((Address) oValue == (Address) a->oValue);
	  return (variable_address_match((Address)oValue, (Address)a->oValue,
					 data_tuple1, data_tuple2, datareqs1, datareqs2));
#endif
	  
	case FrameAddr:  // used once in BPatch_snippet.C (BPatch_variableExpr constructor)
	  return ((void *) oValue == (void *) a->oValue);

	case SharedData:  // doesn't seem to be used anywhere
	  break;

	case PreviousStackFrameDataReg:  // Register or some other unsigned type
	  return ((unsigned) oValue == (unsigned) a->oValue);

	default:
	  return false;
	}

      break;
      
    case callNode:
      if (callee != a->callee)
	return false;
      
      if (!calleefunc->match(a->calleefunc))
	return false;
      
      // unsigned osize = operands.size();
      if (operands.size() != a->operands.size())
	return false;
      
      for (i=0; i<operands.size(); i++) {
	if (!operands[i]->condMatch(a->operands[i], data_tuple1, data_tuple2,
				    datareqs1, datareqs2))
	  return false;

	  }
      
      return true;
      
    default:
      return false;
    }

  return false;
}
*/

