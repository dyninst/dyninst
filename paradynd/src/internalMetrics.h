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

#ifndef INTERNAL_MET_HDR
#define INTERNAL_MET_HDR

// $Id: internalMetrics.h,v 1.25 2004/03/23 01:12:34 eli Exp $

#include "im_preds.h"
#include "dyninstRPC.xdr.h" // T_dyninstRPC
#include "pdutil/h/pdSample.h"
#include "pdutil/h/metricStyle.h"


class machineMetFocusNode;

typedef pdSample (*sampleValueFunc)(const machineMetFocusNode *);

typedef enum {UnNormalized, Normalized, Sampled} daemon_MetUnitsType;

//
// Metrics that are internal to a paradyn daemon.
//

class Focus;

class internalMetric {
 public:
  // the costMetrics and the normal metrics don't need this choice in
  // policy since they have it hardcoded right now to use zero as the
  // initial actual value
  enum initActualValuePolicy_t { zero_ForInitActualValue, 
				 firstSample_ForInitActualValue };
 private:
  pdstring name_;
  aggregateOp agg_;
  pdstring units_;
  im_pred_struct pred;
  bool developermode_;
  daemon_MetUnitsType unitstype_;
  metricStyle style_;
  sampleValueFunc func; // a func taking in no params and returning float
  initActualValuePolicy_t initActualValuePolicy;

 public:
  class eachInstance {
   private:
     // If func!=NULL, then it is used to indirectly obtain the value of
     // the internal metric (getValue()).  Otherwise, the vrble "value" is used.
     internalMetric *parent;
     sampleValueFunc func; // a func taking in no params and returning float
     timeStamp lastSampleTime;
     pdSample cumulativeValue;
     machineMetFocusNode *node;

   public:
     eachInstance() {} // needed by Vector class.
     eachInstance(internalMetric *_parent,
		  sampleValueFunc f, machineMetFocusNode *n);
    ~eachInstance() {}
     
     bool matchMetricDefinitionNode(const machineMetFocusNode *match_me) const
     {
        return (node == match_me);
     }

     pdSample calcValue() const {
       assert(func);
       return func(node);
     }
     initActualValuePolicy_t getInitActualValuePolicy() {
       return parent->getInitActualValuePolicy();
     }
     int getMetricID() const;
     void setStartTime(timeStamp t) {
       lastSampleTime = t;
     }
     void updateValue(timeStamp timeOfSample, pdSample valueToForward);
  };

 private:
  // enabled instances of this internal metric are here:
  pdvector<eachInstance> instances;

 public:
  internalMetric(const pdstring &n, aggregateOp a, const pdstring &units,
		 sampleValueFunc f, im_pred_struct& im_preds,
		 bool developermode, daemon_MetUnitsType unitstype,
		 initActualValuePolicy_t initActualValuePolicy);

  static bool isInternalMetric(const pdstring &metName) {
     for (unsigned lcv=0; lcv < allInternalMetrics.size(); lcv++)
        if (allInternalMetrics[lcv]->name_ == metName)
	   return true;
     return false;
  }

  unsigned num_enabled_instances() const {
     return instances.size();
  }
  eachInstance &getEnabledInstance(unsigned index) {
     return instances[index];
  }
  const eachInstance &getEnabledInstance(unsigned index) const {
     return instances[index];
  }
  void setStyle(metricStyle st) {
    style_ = st;
  }
  initActualValuePolicy_t getInitActualValuePolicy() {
    return initActualValuePolicy;
  }
  // returns the index of the newly enabled instance
  unsigned enableNewInstance(machineMetFocusNode *n) {
     eachInstance newGuy(this, func, n);
     instances += newGuy;
     return instances.size()-1;
  }
  void disableInstance(unsigned index);
  bool disableByMetricDefinitionNode(machineMetFocusNode *diss_me);
  
  int getMetricID(unsigned index) const {
     // returns the mid from the machineMetFocusNode.  Used in metric.C
     return instances[index].getMetricID();
  }

  metricStyle style() const;
  const pdstring &name() const;
  aggregateOp aggregate() const;
  bool isDeveloperMode() const;

  T_dyninstRPC::metricInfo getInfo();

  bool legalToInst(const Focus& focus) const;

  static pdvector<internalMetric*> allInternalMetrics; // should be private!
  static internalMetric *newInternalMetric(const pdstring &n, aggregateOp a,
	     const pdstring &units, sampleValueFunc f, im_pred_struct& preds,
	     bool developerMode, daemon_MetUnitsType unitstype,
	     initActualValuePolicy_t iavPolicy);
};

#endif
