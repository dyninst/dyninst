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

// $Id: costmetrics.h,v 1.18 2003/07/15 22:46:45 schendel Exp $

#ifndef COST_METRICS_HDR 
#define COST_METRICS_HDR

#include "dyninstAPI/src/process.h"
#include "paradynd/src/im_preds.h"
#include "paradynd/src/internalMetrics.h"
#include "paradynd/src/resource.h"
#include "common/h/Time.h"
#include "pdutil/h/sampleAggregator.h"

class Focus;

class costMetric {
  machineMetFocusNode *node;

 public:
  costMetric(const pdstring n,
	     aggregateOp a,   // how paradyn combines values from daemons
	     const pdstring units,
	     im_pred_struct& preds,
	     bool developerMode,
	     daemon_MetUnitsType unitstype,
	     // how daemons combine values from processes
	     aggregateOp combiner_op); 
  ~costMetric(){}
  void enable(machineMetFocusNode *n) { node = n; }
  void disable() { node = NULL; }
  bool enabled() { return(node != NULL); }
  machineMetFocusNode *getNode() { return node; }
  metricStyle style() { return EventCounter; }
  pdstring name() const { return name_;}
  const char *getName() const { return name_.c_str();}
  aggregateOp aggregate() const { return agg_; }
  static costMetric *newCostMetric(const pdstring n, 
			       aggregateOp a,   // how paradyn combines values
			       const pdstring units,
			       im_pred_struct& preds,
			       bool developerMode,
			       daemon_MetUnitsType unitstype,
			       aggregateOp combiner_op); // how daemons combine values
  
  T_dyninstRPC::metricInfo getInfo() {
    T_dyninstRPC::metricInfo ret;
    ret.name = name_;
    ret.aggregate = agg_; ret.units = units_;
    ret.developerMode = developermode_;
    ret.style = EventCounter;
    if (unitstype_ == UnNormalized) ret.unitstype = 0;
    else if (unitstype_ == Normalized) ret.unitstype = 1; 
    else if (unitstype_ == Sampled) ret.unitstype = 2; 
    else ret.unitstype = 0;
    ret.handle = 0; // ignored by paradynd for now
    return ret;
  }

  bool legalToInst(const Focus& focus);
  bool isDeveloperMode() { return developermode_; }

  // add new entries to components, lastProcessTime, and parts
  bool addProcess(process *p);
  static bool addProcessToAll(process *newproc);
  // remove new entries to components, lastProcessTime, and parts
  bool removeProcess(process *p);
  static bool removeProcessFromAll(process *proc);

  timeStamp getLastSampleProcessTime(process *proc){
      for(unsigned i = 0; i < components.size(); i++){
	  if(proc == components[i]){
              return(lastProcessTime[i]);
      } }
      return timeStamp::ts1970();
  }
  pdSample getCumulativeValue(process *proc){
      for(unsigned i = 0; i < components.size(); i++){
	  if(proc == components[i]){
	      return(cumulative_values[i]);
      } }
      return pdSample::Zero();
  }

  // proc: process sending cost data
  void updateValue(process *proc, timeStamp timeOfSample, pdSample value, 
		   timeStamp processTime);
  // returns false when no more aggregation can be done
  bool aggregateAndBatch();
  static pdvector<costMetric*> allCostMetrics;

  static bool isCostMetric(const pdstring &metName) {
     for (unsigned lcv=0; lcv < allCostMetrics.size(); lcv++)
        if (allCostMetrics[lcv]->name_ == metName)
	   return true;
     return false;
  }

private:
  // list of processes and values contributing to metric value
  pdvector<process *> components;
  pdvector<aggComponent *> parts;
  sampleAggregator aggregator;

  pdvector<pdSample> cumulative_values;

  // process times associated with each component wall times are contained
  // in the parts pdvector
  pdvector<timeStamp> lastProcessTime;

  // why is there no mid stored in this class, as there is for the internalMetrics class?
  pdstring name_;
  aggregateOp agg_;
  pdstring units_;
  im_pred_struct pred;
  bool developermode_;
  daemon_MetUnitsType unitstype_;
  aggregateOp combinerop_;
};

#endif
