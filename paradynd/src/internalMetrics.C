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

// $Id: internalMetrics.C,v 1.14 2002/10/15 17:11:48 schendel Exp $

#include "paradynd/src/processMgr.h"
#include "internalMetrics.h"
#include "paradynd/src/init.h"
#include "paradynd/src/machineMetFocusNode.h"
#include "paradynd/src/focus.h"

internalMetric::eachInstance::eachInstance(internalMetric *_parent,
		     sampleValueFunc f, machineMetFocusNode *n) :
                         parent(_parent), func(f), node(n) { 
}

int internalMetric::eachInstance::getMetricID() const {
  assert(node);
  return node->getMetricID();
}

void internalMetric::eachInstance::updateValue(timeStamp timeOfSample,
					       pdSample value) {
  //  cerr << "intM::updateValue- time: " << timeOfSample << "val: " 
  //   << value << " lastSampleTime: " << lastSampleTime 
  //   << ", inst: " << this << ", mdn: " << node->getFullName() << "\n";
  assert(node);
  // lastSampleTime needs to be initially set with setStartTime()
  assert(lastSampleTime.isInitialized());

  if(cumulativeValue.isNaN()) {
    assert(!node->sentInitialActualValue());
    pdSample initActVal;
    if(getInitActualValuePolicy() == zero_ForInitActualValue) {
      // we set the cumulativeValue to the value of this first sample
      // because we want to "throw away" any value of this metric before
      // this.  eg. for pause_time, we want the pause_time to graph
      // starting at zero, even if the application was paused and
      // it's first sample is greater than zero.
      cumulativeValue = value;
      initActVal = pdSample::Zero();
    }
    else if(getInitActualValuePolicy() == firstSample_ForInitActualValue) {
      cumulativeValue = value;
      initActVal = value;
    }
    else assert(0);  // only two initActualValuePolicies so far

    node->initializeForSampling(getWallTime(), initActVal);
    node->sendInitialActualValue(initActVal);
  } else {
    value -= cumulativeValue;
    cumulativeValue += value;
    assert(timeOfSample > lastSampleTime);
    node->forwardSimpleValue(lastSampleTime, timeOfSample, value);
  }
  lastSampleTime = timeOfSample;
  // 1 --> weight (?)
  // true --> this is an internal metric
}

/* ***************************************************************************** */

internalMetric::internalMetric(const string &n, aggregateOp a, 
			       const string &units, sampleValueFunc f, 
			       im_pred_struct& im_preds, bool developermode, 
			       daemon_MetUnitsType unitstype,
			       initActualValuePolicy_t iavPolicy) :
  name_(n), agg_(a), units_(units), pred(im_preds), 
  developermode_(developermode), unitstype_(unitstype), func(f), 
  initActualValuePolicy(iavPolicy)
{
   // we leave "instances" initialized to an empty vector
}

void internalMetric::disableInstance(unsigned index) {
   // remove this guy from the array
   // shift guys after him 1 spot to the left
   for (unsigned lcv=index; lcv < instances.size()-1; lcv++)
      instances[lcv]=instances[lcv+1];
   instances.resize(instances.size()-1);
}

bool internalMetric::disableByMetricDefinitionNode(machineMetFocusNode *diss_me) {
   for (unsigned index=0; index < instances.size(); index++) {
      if (instances[index].matchMetricDefinitionNode(diss_me)) {
	 disableInstance(index);
	 return true;
      }
   }
   return false;
}

metricStyle internalMetric::style() const {
  return style_;
}

const string &internalMetric::name() const {
   return name_;
}

aggregateOp internalMetric::aggregate() const {
   return agg_;
}

bool internalMetric::isDeveloperMode() const {
   return developermode_;
}

T_dyninstRPC::metricInfo internalMetric::getInfo() {
    T_dyninstRPC::metricInfo ret;
    ret.name = name_;
    ret.aggregate = agg_;
    ret.units = units_;
    ret.style = int(style_);
    ret.developerMode = developermode_;
    if (unitstype_ == UnNormalized) ret.unitstype = 0;
    else if (unitstype_ == Normalized) ret.unitstype = 1; 
    else if (unitstype_ == Sampled) ret.unitstype = 2; 
    else ret.unitstype = 0;
    ret.handle = 0; // ignored by paradynd for now
    return ret;
}

bool internalMetric::legalToInst(const Focus &focus) const {
  // returns true iff this internal metric may be enabled for the given focus.
  if(getProcMgr().size() == 0) {
    // we don't enable internal metrics if there are no process to run
    return false;
  }

  // Is the /Machine part of the focus allowed?
  if(pred.machine == pred_invalid   && !focus.allMachines())  return false;

  // Is the /Code part of the focus allowed?
  if(pred.procedure == pred_invalid && !focus.allCode())      return false;

  // Is the /SyncObject part of the focus allowed?
  if(pred.sync == pred_invalid      && !focus.allSync())      return false;

  // If we've passed all the tests up to now, then it's okay to
  // instrument this metric...
  return true;
}

internalMetric *internalMetric::newInternalMetric(
          const string &n, aggregateOp a, const string &units, 
	  sampleValueFunc f, im_pred_struct& im_pred, bool developerMode,
	  daemon_MetUnitsType unitstype, initActualValuePolicy_t iavPolicy) {
  internalMetric *im = new internalMetric(n, a, units, f, im_pred,
					  developerMode, unitstype, iavPolicy);
  assert(im);

  // It looks like we are checking to see if an internal metric with
  // this name already exists; if so, we replace it.  Why is this being done?
  unsigned size = allInternalMetrics.size();
  for (unsigned u=0; u<size; u++)
    if (allInternalMetrics[u]->name() == n) {
      allInternalMetrics[u] = im; // shouldn't we 'delete' the old entry first?
      return im;
    }

  allInternalMetrics += im;
  return im;
}

