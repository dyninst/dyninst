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

// $Id: internalMetrics.C,v 1.9 2000/10/17 17:42:35 schendel Exp $

#include "dyninstAPI/src/process.h" // processVec
#include "internalMetrics.h"

internalMetric::eachInstance::eachInstance(sampleValueFunc f, metricDefinitionNode *n) {
   func = f;

   value = pdSample(0);
   cumulativeValue = pdSample(0);

   node = n;
}

internalMetric::eachInstance::eachInstance(const internalMetric::eachInstance &src) {
   func = src.func;
   if (func == NULL)
      value = src.value;
   cumulativeValue = src.cumulativeValue;
   node = src.node;
}

internalMetric::eachInstance &internalMetric::eachInstance::operator=(const internalMetric::eachInstance &src) {
   if (this == &src) return *this;

   // clean up "this" (nothing to do)...
   // assign to "this"...
   func = src.func;
   if (func == NULL)
      value = src.value;
   cumulativeValue = src.cumulativeValue;
   node = src.node;

   return *this;
}

void internalMetric::eachInstance::report(timeStamp start, timeStamp end,
					  pdSample valueToForward) {
   assert(node);
   node->forwardSimpleValue(start, end, valueToForward, 1, true);
      // 1 --> weight (?)
      // true --> this is an internal metric
}

/* ***************************************************************************** */

internalMetric::internalMetric(const string &n, metricStyle style, int a,
			       const string &units,
			       sampleValueFunc f, im_pred_struct& im_preds,
			       bool developermode, daemon_MetUnitsType unitstype) :
		       name_(n), units_(units), pred(im_preds)
{
   func = f;

   agg_ = a;
   style_ = style;
   developermode_ = developermode;
   unitstype_ = unitstype;

   // we leave "instances" initialized to an empty vector
}

void internalMetric::disableInstance(unsigned index) {
   // remove this guy from the array
   // shift guys after him 1 spot to the left
   for (unsigned lcv=index; lcv < instances.size()-1; lcv++)
      instances[lcv]=instances[lcv+1];
   instances.resize(instances.size()-1);
}

bool internalMetric::disableByMetricDefinitionNode(metricDefinitionNode *diss_me) {
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

int internalMetric::aggregate() const {
   return agg_;
}

bool internalMetric::isDeveloperMode() const {
   return developermode_;
}

T_dyninstRPC::metricInfo internalMetric::getInfo() {
    T_dyninstRPC::metricInfo ret;
    ret.name = name_;
    ret.style = style_;
    ret.aggregate = agg_;
    ret.units = units_;
    ret.developerMode = developermode_;
    if (unitstype_ == UnNormalized) ret.unitstype = 0;
    else if (unitstype_ == Normalized) ret.unitstype = 1; 
    else if (unitstype_ == Sampled) ret.unitstype = 2; 
    else ret.unitstype = 0;
    ret.handle = 0; // ignored by paradynd for now
    return ret;
}

extern vector<process*> processVec;

bool internalMetric::legalToInst(const vector< vector<string> >& focus) const {
  // returns true iff this internal metric may be enabled for the given focus.
  if (processVec.size()==0) {
    // we don't enable internal metrics if there are no process to run
    return false;
  }

  // How many components are there for the /Machine part of the focus?
  switch (focus[resource::machine].size()) {
  case 1: break;
  case 2:
  case 3:
  case 4:
    switch(pred.machine) {
    case pred_invalid: return false;
    case pred_null: break;
    default: return false;
    }
    break;
  default: return false;
  }

  // How many components are there for the /Code part of the focus?
  switch (focus[resource::procedure].size()) {
  case 1: break;
  case 2:
  case 3:
    switch(pred.procedure) {
    case pred_invalid: return false;
    case pred_null: break;
    default: return false;
    }
    break;
  default: return false;
  }

  // How many components are there for the /SyncObject part of the focus?
  switch (focus[resource::sync_object].size()) {
  case 1: break;
  case 2:
  case 3:
  case 4:
    switch(pred.sync) {
    case pred_invalid: return false;
    case pred_null: break;
    default: return false;
    }
    break;
  default: return false;
  }

  // If we've passed all the tests up to now, then it's okay to
  // instrument this metric...
  return true;
}

internalMetric *internalMetric::newInternalMetric(const string &n,
                                                  metricStyle style,
                                                  int a, 
                                                  const string &units, 
                                                  sampleValueFunc f,
                                                  im_pred_struct& im_pred,
                                                  bool developerMode,
                                                  daemon_MetUnitsType unitstype) {
  internalMetric *im = new internalMetric(n, style, a, units, f, im_pred,
                                          developerMode, unitstype);
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
