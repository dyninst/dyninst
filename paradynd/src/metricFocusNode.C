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

// $Id: metricFocusNode.C,v 1.258 2005/03/16 20:56:02 legendre Exp $

#include "common/h/headers.h"
#include "common/h/Types.h"
#include <assert.h>

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "pdutil/h/sampleAggregator.h"
#include "paradynd/src/comm.h"
#include "paradynd/src/internalMetrics.h"
#include "paradynd/src/init.h"
#include "paradynd/src/perfStream.h"
#include "paradynd/src/main.h"
#include "paradynd/src/dynrpc.h"
#include "paradynd/src/mdld.h"
#include "common/h/Timer.h"
#include "paradynd/src/costmetrics.h"
#include "paradynd/src/metricFocusNode.h"
#include "paradynd/src/machineMetFocusNode.h"
#include "paradynd/src/processMetFocusNode.h"
#include "paradynd/src/threadMetFocusNode.h"
#include "common/h/debugOstream.h"
#include "pdutil/h/pdDebugOstream.h"
#include "common/h/timing.h"
#include "pdutil/h/mdl_data.h"
#include "paradynd/src/focus.h"
#include "paradynd/src/processMgr.h"
#include "paradynd/src/pd_process.h"
#include "paradynd/src/context.h"
#include "paradynd/src/debug.h"

#ifdef FREEDEBUG
#  include <sstream>
#endif

#include "dyninstAPI/src/instPoint.h"

extern unsigned inferiorMemAvailable;
extern pdvector<Address> getAllTrampsAtPoint(miniTrampHandle *);

unsigned int metResPairsEnabled = 0; // PDSEP from stats.C

void flush_batch_buffer();
void batchSampleData(pdstring metname, int mid, timeStamp startTimeStamp, 
		     timeStamp endTimeStamp, pdSample value, 
		     bool internal_metric);

timeLength currentPredictedCost = timeLength::Zero();

unsigned mdnHash(const metricFocusNode *&mdn) {
  return ((unsigned)(Address)mdn) >> 2; // assume all addrs are 4-byte aligned
  //  return ((unsigned) mdn);
}

pdvector<internalMetric*> internalMetric::allInternalMetrics;

metricFocusNode::metricFocusNode()
{
}

void
metFocInstResponse::addResponse( u_int mi_id,
                                inst_insert_result_t res,
                                pdstring emsg )
{
    rinfo.push_back( T_dyninstRPC::indivInstResponse( mi_id, res, emsg ));
}


void
metFocInstResponse::updateResponse( u_int mi_id,
                                    inst_insert_result_t res,
                                    pdstring emsg )
{
    for( pdvector<T_dyninstRPC::indivInstResponse>::iterator iter = rinfo.begin();
            iter != rinfo.end();
            iter++ )
    {
        if( mi_id == iter->mi_id )
        {
            iter->status = res;
            iter->emsg = emsg;
            break;
        }
    }
}


void
metFocInstResponse::makeCallback( void )
{
    tp->enableDataCallback( *this );
}  



// check for "special" metrics that are computed directly by paradynd 
// if a cost of an internal metric is asked for, enable=false
machineMetFocusNode *doInternalMetric(int mid, 
				      const Focus &focus,
				      const pdstring& metric_name, 
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

machineMetFocusNode *createMetricInstance(int mid, pdstring& metric_name, 
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
         if(!curProc) continue;
#ifdef NOTDEF // PDSEP
   //  I think there must be an error in this predicate?
   //  Doesn't match the comment above
         if(curProc->status()==exited || curProc->status()==neonatal || 
            curProc->isBootstrappedYet())
#endif
         if (!curProc->isTerminated() 
             && !curProc->isDetached()
             && curProc->isBootstrappedYet())
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

      //cerr << "makeMachineMetFocusNode START" << endl;
      machineMetFocusNode *machNode = 
         makeMachineMetFocusNode(mid, focus, metric_name, procs, false,enable);
      //cerr << "makeMachineMetFocusNode END" << endl;

      if (machNode == NULL) {
//          cerr << "createMetricInstance failed since mdl_do failed\n";
//          cerr << "metric name was " << metric_name << "; focus was ";
//          cerr << "createMetricInstance failed since mdl_do failed\n";
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
//          cerr << "createMetricInstance: internal metric " 
//                      << metric_name << " isn't defined for focus: " 
//                      << focus.getName() << "\n";
         machNode = NULL; // straighten up the return value
      }
      else if (machNode == (machineMetFocusNode*)-1) {
//          cerr << " createMetricInstance: internal metric not enable: " 
//                      << metric_name << endl;
         assert(!enable); // no error msg needed
         machNode = NULL; // straighten up the return value
      }
      else if (machNode == NULL) {
         // more serious error...do a printout
//          cerr << "createMetricInstance failed since doInternalMetric failed\n";
//          cerr << "metric name was " << metric_name << "; focus was "
//                      << focus.getName() << "\n";
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

void metricFocusNode::handleNewProcess(pd_process *pd_proc) {
   pdvector<machineMetFocusNode *> allMachNodes;
   machineMetFocusNode::getMachineNodes(&allMachNodes);

   for (unsigned j=0; j < allMachNodes.size(); j++) {
      machineMetFocusNode *curNode = allMachNodes[j];
      curNode->propagateToNewProcess(pd_proc, false);
   }
}

// Remove the aggregate metric instances that don't have any components left
void metricFocusNode::handleExitedProcess(pd_process *proc) {
   metric_cerr << "removeFromMetricInstances- proc: " << proc << ", pid: " 
	       << proc->getPid() << "\n";

   pdvector<machineMetFocusNode *> all_mach_nodes;
   machineMetFocusNode::getMachineNodes(&all_mach_nodes);
   for(unsigned i=0; i<all_mach_nodes.size(); i++) {
      machineMetFocusNode *curMachNode = all_mach_nodes[i];
      curMachNode->adjustForExitedProcess(proc);
   }

   // what about internal metrics?
   process *llproc = proc->get_dyn_process()->lowlevel_process();
   costMetric::removeProcessFromAll(llproc);
}

void metricFocusNode::handleNewThread(pd_process *proc, pd_thread *thr) {
   assert(proc->multithread_ready());
   pdvector<machineMetFocusNode *> all_mach_nodes;
   machineMetFocusNode::getMachineNodes(&all_mach_nodes);
   for(unsigned i=0; i<all_mach_nodes.size(); i++) {
      machineMetFocusNode *curMachNode = all_mach_nodes[i];
      curMachNode->adjustForNewThread(proc, thr);
   }
}

void metricFocusNode::handleExitedThread(pd_process *proc, pd_thread *thr) {
   assert(proc->multithread_ready());   
   pdvector<machineMetFocusNode *> all_mach_nodes;
   machineMetFocusNode::getMachineNodes(&all_mach_nodes);
   for(unsigned i=0; i<all_mach_nodes.size(); i++) {
      machineMetFocusNode *curMachNode = all_mach_nodes[i];
      curMachNode->adjustForExitedThread(proc, thr);
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
// The only difference between sync mode and async mode is
// whether we indicate deferred instrumentation as a failure in 
// the response object or not.
//
void startCollecting(pdstring& metric_name, pdvector<u_int>& focus,
                                        int mid,
                                        metFocInstResponse *cbi)
{
    pdstring temp = metric_name;
    for (unsigned i = 0; i < focus.size(); i++) {
        temp += (pdstring(" ") + pdstring(focus[i]));
    }

   assert( cbi != NULL );

   // Make the unique ID for this metric/focus visible in MDL.
   pdstring vname = "$globalId";
   mdl_data::cur_mdl_data->env->add(vname, false, MDL_T_INT);
   mdl_data::cur_mdl_data->env->set(mid, vname);

   machineMetFocusNode *machNode = 
     createMetricInstance(mid, metric_name, focus, true);

   if (!machNode) {
//       cerr << "startCollecting for " << metric_name 
// 		  << " failed because createMetricInstance failed" << endl;
      cbi->addResponse( mid,
                        inst_insert_failure,
                        mdl_data::cur_mdl_data->env->getSavedErrorString() );
      return;
   }


   //cerr << "created metric-focus " << machNode->getFullName() << "\n";   
   addCurrentPredictedCost(machNode->cost());
   metResPairsEnabled++;
   
   if (machNode->isInternalMetric()) {
      cbi->addResponse( mid, inst_insert_success );
      return;
   }

   inst_insert_result_t insert_status =  machNode->insertInstrumentation();

   if(insert_status == inst_insert_deferred) {
      machNode->setMetricFocusResponse(cbi);
      cbi->addResponse( mid, inst_insert_deferred );
      return;
   } else if(insert_status == inst_insert_failure) {
      // error message already displayed in processMetFocusNode::insertInstrum.
      delete machNode;
      cbi->addResponse( mid,
                        inst_insert_failure,
                        mdl_data::cur_mdl_data->env->getSavedErrorString() );
      return;
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

   cbi->addResponse( mid, inst_insert_success );
}


timeLength guessCost(pdstring& metric_name, pdvector<u_int>& focus) {
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
     std::ostringstream errorLine;
     errorLine << "++--++ TEST ++--++ batchSampleDataCallbackFunc took "
               << t2-t1 << ", size= << "
               << sizeof(T_dyninstRPC::batch_buffer_entry) << ", Kbytes="
               << (sizeof(T_dyninstRPC::batch_buffer_entry) * 
                   copyBatchBuffer.size()/1024.0F);
     logLine(errorLine.str().c_str());
   }
#endif

   BURST_HAS_COMPLETED = false;
   batch_buffer_next = 0;
}

// temporary until front-end's pipeline gets converted
u_int isMetricTimeType(const pdstring& met_name) {
  unsigned size = mdl_data::cur_mdl_data->all_metrics.size();
  T_dyninstRPC::metricInfo element;
  unsigned u;
  for (u=0; u<size; u++) {
    //cerr << "checking " << met_name << " against " 
    //    << mdl_data::cur_mdl_data->all_metrics[u]->name_ << "\n";
    if (mdl_data::cur_mdl_data->all_metrics[u]->name_ == met_name) {
      u_int mtype = mdl_data::cur_mdl_data->all_metrics[u]->type_;
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
void batchSampleData(pdstring metname, int mid, timeStamp startTimeStamp, 
		     timeStamp endTimeStamp, pdSample value) 
{
   // This routine is called where we used to call tp->sampleDataCallbackFunc.
   // We buffer things up and eventually call tp->batchSampleDataCallbackFunc

#ifdef notdef
   char myLogBuffer[120] ;
   sprintf(myLogBuffer, "mid %d, value %g\n", mid, value.getValue()) ;
   logLine(myLogBuffer) ;
#endif

   sample_cerr << "batchSampleData - metric: " << metname.c_str() 
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
     sample_cerr << metname.c_str() << " is a time metric type: normalizing\n";
     bval /= 1000000000.0;
   }
   theEntry.value = bval;

   sample_cerr << ">b2 startTimeStamp d: " << theEntry.startTimeStamp
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



