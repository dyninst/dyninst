/*
 * Copyright (c) 1996-2003 Barton P. Miller
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

// $Id: costmetrics.C,v 1.28 2003/03/10 21:08:37 pcroth Exp $

#include "common/h/Types.h"
#include "common/h/int64iostream.h"
#include "paradynd/src/costmetrics.h"
#include "paradynd/src/machineMetFocusNode.h"
#include "dyninstAPI/src/process.h"
#include "pdutil/h/pdDebugOstream.h"
#include "paradynd/src/dynrpc.h"
#include "paradynd/src/focus.h"
#include "paradynd/src/processMgr.h"
#include "paradynd/src/pd_process.h"

pdvector<costMetric*> costMetric::allCostMetrics;

extern unsigned enable_pd_samplevalue_debug;

#if ENABLE_DEBUG_CERR == 1
#define sampleVal_cerr if (enable_pd_samplevalue_debug) cerr
#else
#define sampleVal_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

costMetric::costMetric(const string n, aggregateOp a, const string units,
		       im_pred_struct& preds, bool developerMode,
		       daemon_MetUnitsType unitstype, aggregateOp combiner_op):
  node(NULL), aggregator(combiner_op, getCurrSamplingRate()), name_(n), 
  agg_(a), units_(units), pred(preds), developermode_(developerMode), 
  unitstype_(unitstype)
{
   processMgr::procIter itr = getProcMgr().begin();
   while(itr != getProcMgr().end()) {
      pd_process *p = *itr++;
      if(p->status() != exited) {
	 components += p->get_dyn_process();
	 //aggComponent *s = new aggComponent;
	 aggComponent *s = aggregator.newComponent();
	 parts += s;
	 lastProcessTime += timeStamp::ts1970(); 
	 cumulative_values += pdSample(0);
      }
   }
}


costMetric *costMetric::newCostMetric(const string n, aggregateOp a,
				      const string units,
				      im_pred_struct& preds,
				      bool developerMode,
				      daemon_MetUnitsType unitstype,
				      aggregateOp combiner_op)
{
    costMetric *newcm = new costMetric(n, a, units, preds, developerMode,
				       unitstype, combiner_op);
    allCostMetrics += newcm; 
    return newcm;
}


bool costMetric::legalToInst(const Focus& focus) {

    if(! getProcMgr().size()) {
        // we don't enable metrics if there are no process to run
	return false;
    }

    bool all_exited = true;
    processMgr::procIter itr = getProcMgr().begin();
    while(itr != getProcMgr().end()) {
       if((*itr)->status() != exited) {
	  all_exited = false;
       }
       itr++;
    }
    if(all_exited) return false;

    // Is the /Machine part of the focus allowed?
    if(pred.machine == pred_invalid   && !focus.allMachines()) return false;
    
    // Is the /Code part of the focus allowed?
    if(pred.procedure == pred_invalid && !focus.allCode())     return false;
    
    // Is the /SyncObject part of the focus allowed?
    if(pred.sync == pred_invalid      && !focus.allSync())     return false;
    
    return true;
}

bool costMetric::addProcess(process *p){

    for(unsigned i = 0; i < components.size(); i++){
       if(p == components[i])
	 return false;
    }
    components += p;
    aggComponent *s = aggregator.newComponent();
    parts += s;
    lastProcessTime += timeStamp::ts1970();
    cumulative_values += pdSample(0);
    return true;
}

bool costMetric::removeProcess(process *p){
    unsigned size = components.size();
    for(unsigned i = 0; i < size; i++){
       if(p == components[i]){
	   //aggComponent *olddata = parts[i];
	   parts[i]->requestRemove();
	   for(unsigned j = i; j < size-1; j++){
	       components[j] = components[j+1];
	       parts[j] = parts[j+1];
	       lastProcessTime[j] = lastProcessTime[j+1];
	       cumulative_values[j] = cumulative_values[j+1];
	   }
	   components.resize(size -1);
	   parts.resize(size -1);
	   lastProcessTime.resize(size -1);
	   cumulative_values.resize(size -1);
	   // delete olddata;
	   assert(components.size() == parts.size());
	   assert(lastProcessTime.size() == parts.size());
	   assert(cumulative_values.size() == parts.size());
	   return true;
       }
    }
    return false;
}


bool costMetric::addProcessToAll(process *p){

    for(unsigned i=0; i < allCostMetrics.size(); i++){
        costMetric *next_met = allCostMetrics[i];
	next_met->addProcess(p);
    }
    return true;
}

bool costMetric::removeProcessFromAll(process *p){
    for(unsigned i=0; i < allCostMetrics.size(); i++){
        costMetric *next_met = allCostMetrics[i];
	next_met->removeProcess(p);
    }
    return true;
}

void costMetric::updateValue(process *proc, timeStamp timeOfSample, 
			     pdSample value, timeStamp processTime) 
{
  int proc_num = -1;
  //sampleVal_cerr << "costMetric::updateValue- val: " << value << "\n";
  for(unsigned i=0; i < components.size(); i++) {
    if(proc == components[i]) proc_num = i;
  }
  if(proc_num == -1) return;
  
  lastProcessTime[proc_num] = processTime;
  // only use delta from last sample
  if (value < cumulative_values[proc_num]) {
    value = cumulative_values[proc_num];
  }
  value -= cumulative_values[proc_num];
  cumulative_values[proc_num] += value;
  aggComponent *aggComp = parts[proc_num];
  // update the sample value associated with the process proc
  if(node == NULL)  
    return;

  // set the initial start time and actual value
  if(! aggComp->isInitialStartTimeSet()) {
    aggComp->setInitialStartTime(timeOfSample);
    // addSamplePt requires samples to be later than last sample
    timeOfSample += timeLength::us();
  }
  if((aggComp->getInitialActualValue()).isNaN()) {
    aggComp->setInitialActualValue(pdSample::Zero());
  }

  aggComp->addSamplePt(timeOfSample, value);
  while(aggregateAndBatch());
}

bool costMetric::aggregateAndBatch() {
  // update value if we have a new value from each part
  struct sampleInterval aggSample;
  bool valid = aggregator.aggregate(&aggSample);

  if(node==NULL || !valid) return false;

  if(! node->sentInitialActualValue())
    node->sendInitialActualValue(aggregator.getInitialActualValue());

  timeLength timeSpan = aggSample.end - aggSample.start;
  int64_t span_ns = timeSpan.getI(timeUnit::ns());
  int64_t rval = aggSample.value.getValue();

  sampleVal_cerr << "costMetric- end: " << aggSample.end << "  start: " 
		 << aggSample.start << "  span_ns: " 
		 << span_ns << "  rval: " << rval << "\n";

  pdSample adjSampleVal;
  if(rval < span_ns) {
    // cost = total execution / computation 
    //      = computation + instr / computation
    //      = ratio of program time for instrumented program to
    //                 program time for uninstrumented program
    double adjRatio = static_cast<double>(span_ns) / (span_ns - rval);
    adjSampleVal.assign(static_cast<int64_t>(span_ns * adjRatio));
  } else {
    // these sample values might need to be aggregated in front-end;
    // the chosen value allows for 100,000 daemons (ie. nodes)
    const int64_t largestCostVal = I64_MAX / 100000;
    adjSampleVal.assign(largestCostVal);
  }
  sampleVal_cerr << "costMetric- adjSampleVal: " << adjSampleVal << "\n";

  node->forwardSimpleValue(aggSample.start, aggSample.end, adjSampleVal);
  return true;
}


