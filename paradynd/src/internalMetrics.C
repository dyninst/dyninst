// internalMetrics.C

#include "internalMetrics.h"

internalMetric::eachInstance::eachInstance(sampleValueFunc f, metricDefinitionNode *n) {
   func = f;

   value = 0.0;
   cumulativeValue = 0.0;

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
					  sampleValue valueToForward) {
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

  // How many components are there for the /Process part of the focus?
  switch (focus[resource::process].size()) {
  case 1: break;
  case 2:
    switch(pred.process) {
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
