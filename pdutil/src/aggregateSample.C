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

// $Id: aggregateSample.C,v 1.21 2000/10/26 17:02:36 schendel Exp $

#include <assert.h>
#include <math.h>
#include <iostream.h>
#include "pdutil/h/aggregateSample.h"
#include "pdutil/h/pdDebugOstream.h"


#ifdef AGGREGATE_DEBUG
pdDebug_ostream aggu_cerr(cerr, true);
#else
pdDebug_ostream aggu_cerr(cerr, false);
#endif


// void sampleInfo::startTime(timeStamp time) {
//   assert(numAggregators > 0);
//   firstSampleReceived = true;
//   lastSampleStart = time;
//   // The remaining fields should be zero.
//   // It is important that the value of lastSampleEnd is zero, otherwise the
//   // aggregation code in newValue will not work.
// }

void sampleInfo::firstTimeAndValue(timeStamp time, pdSample firstValue) {
  aggu_cerr << "sampleInfo::firstTimeAndValue - timeStamp: " << time
	    << ", firstValue: " << firstValue << "\n";
  assert(numAggregators > 0);
  firstSampleReceived = true;
  lastSampleStart = time;

  assert(lastSample.getValue() == 0);
  lastSample = firstValue;
  
  // The remaining fields should be zero.
  // It is important that the value of lastSampleEnd is zero, otherwise the
  // aggregation code in newValue will not work.
}

void sampleInfo::newValue(timeStamp sampleTime, 
                          pdSample newVal, 
                          unsigned weight_) {
    // why does this routine return a value (which is essentially useless)?
    aggu_cerr << "sampleInfo::newValue - sampleTime: " << sampleTime
	      << ", pdSample: " << newVal << ", wt: " << weight << "\n";
    assert(firstSampleReceived);
    assert(sampleTime >= lastSampleEnd);

    // used when it's a component of an aggregate.
    switch(valueUpdateStyle) {
    case add:
      lastSample += newVal;
      break;
    case assign:
      lastSample = newVal;
      break;
    }
    lastSampleEnd = sampleTime;
    weight = weight_;
}

ostream& operator<<(ostream&s, const sampleInfo &info) {
  s << "[sampleInfo - firstSampleReceived: " << info.firstSampleReceived
    << ", lastSampleStart: " << info.lastSampleStart << ", lastSampleEnd: "
    << info.lastSampleEnd << ", lastSample: " << info.lastSample
    << ", numAgg: " << info.numAggregators << ", wt: " << info.weight << "\n";

  return s;
}

struct sampleInterval aggregateSample::aggregateValues() {
    struct sampleInterval ret(timeStamp::ts1970(), timeStamp::ts1970(), 
			      pdSample::Zero());
    const timeLength ten_ms(10, timeUnit::ms());
    timeStamp earlyestTime = timeStamp::ts2200();
    ret.valid = false;

    aggu_cerr << "aggValues() - step1   earlyestTime: " << earlyestTime <<"\n";

    if (newParts.size()) {
      // The new components do not need to have the same start time.
      // We must wait until we have samples from all components, and then
      // we need to find the first and second min start time for the new components
      // and if first min == lastSampleEnd, we can aggregate from first to second.
      // But we don't want to generate lots of very small samples in the case we have
      // a large number of components, so we round the start times that are close
      // enough to the min start time.

      timeStamp newStart = timeStamp::ts2200();
      timeStamp secondStart = timeStamp::ts2200();
      for (unsigned u = 0; u < newParts.size(); u++) {
        if ((!newParts[u]->firstValueReceived())) {
          return ret;
        }
        if (newParts[u]->lastSampleStart < lastSampleEnd + ten_ms) {
          // round lastSampleStart to avoid generating very small aggregate
          // samples. I'm using 0.01 as an arbitrary value. We should
          // do some measurements to find a good value -- mjrg
          newParts[u]->lastSampleStart = lastSampleEnd;
        }
        if (newParts[u]->lastSampleStart < newStart) {
          newStart = newParts[u]->lastSampleStart;
        }
      }

      aggu_cerr << "aggValues() - step2\n";
      aggu_cerr << "earlyestTime: " << earlyestTime << ", newStart: " 
		<< newStart << ", secondStart: " << secondStart << "\n";

      assert (newStart >= lastSampleEnd);

      if (parts.size() == 0)
        lastSampleEnd = newStart;

      if (newStart > lastSampleEnd) {
        // new parts can't be aggregated yet
        earlyestTime = newStart;
      }
      else { // newStart == lastSampleEnd
        // find the new parts that are ready to be aggregated and move them to parts.
        vector<sampleInfo *> temp;
        vector<bool> rtemp;
        for (unsigned u = 0; u < newParts.size(); u++) {
          if (newParts[u]->lastSampleStart == newStart) {
            parts += newParts[u];
            removedParts += removedNewParts[u];
          }
          else {
            if (newParts[u]->lastSampleStart < secondStart)
              secondStart = newParts[u]->lastSampleStart;
            temp += newParts[u];
            rtemp += removedNewParts[u];
          }
        }
        if (temp.size()) {
          newParts = temp;
          removedNewParts = rtemp;
          earlyestTime = secondStart;
        }
        else {
          newParts.resize(0);
          removedNewParts.resize(0);
        }
      }
    }  // ending for: if (newParts.size())

    aggu_cerr << "aggValues() - step3   earlyestTime: " << earlyestTime <<"\n";
    bool partsToRemove = false;

    for (unsigned u = 0; u < parts.size(); u++) {
      if (removedParts[u]) {
        if (parts[u]->lastSampleEnd == parts[u]->lastSampleStart) {
          partsToRemove = true;
          continue;
        }
        if (parts[u]->lastSampleEnd < lastSampleEnd + ten_ms) {
           // round to avoid very small intervals;
           parts[u]->lastSampleEnd = lastSampleEnd + ten_ms;
         }
      }
      if (parts[u]->lastSampleEnd < earlyestTime)
        earlyestTime = parts[u]->lastSampleEnd;
    }

    aggu_cerr << "aggValues() - step4   earlyestTime: " << earlyestTime <<"\n";
    
    if (partsToRemove) {
      vector<sampleInfo *> temp;
      vector<bool> rtemp;
      for (unsigned u = 0; u < parts.size(); u++) {
        if (!(removedParts[u] && parts[u]->lastSampleEnd == parts[u]->lastSampleStart)) {
          temp += parts[u];
          rtemp += removedParts[u];
        }
        else {
          --(parts[u]->numAggregators);
          if (parts[u]->numAggregators == 0) {
            delete parts[u];
          }
        }
      }
      parts = temp;
      removedParts = rtemp;
      if (parts.size() == 0) {
	aggu_cerr << "returning from aggregateValues, parts.size()==0\n";
        return ret;
      }
    }

    aggu_cerr << "aggValues() - step5   earlyestTime: " << earlyestTime <<"\n";

    pdSample aggregateVal;
    unsigned total_weight;

    if (earlyestTime > lastSampleEnd + timeLength::ms()) {
    // we can aggregate up to earlyest time.

            aggregateVal.assign(0);

            int first = 1;
            total_weight = 0;

            for (unsigned u = 0; u < parts.size(); u++) {
                // assert(earlyestTime >= parts[u]->lastSampleStart);

	        double fract;
                pdSample component;
                if(doProportionCalc) {
		  fract = (earlyestTime - lastSampleEnd)/
		    (parts[u]->lastSampleEnd - parts[u]->lastSampleStart);
		  assert(fract > 0.0);
		  assert(fract <= 1.0);
		  component = parts[u]->lastSample * fract;
		  parts[u]->lastSample -= component;
                } else {
	          component = parts[u]->lastSample;
                }

		//assert(component >= -0.01);
		assert(component >= pdSample::Zero());

                // each list entry comes from a separate reporter
                switch (aggOp)
                  {
                  case aggSum:
                    aggregateVal += component;
                    break;
                  case aggAvg:
                    aggregateVal += component * 
		                    static_cast<double>(parts[u]->weight);
                    total_weight += parts[u]->weight;
                    break;
                  case aggMin:
                    if (first) {
                      aggregateVal = component;
                      first = 0;
                    } else if (component < aggregateVal)
                      aggregateVal = component;
                    break;
                  case aggMax:
                    if (component > aggregateVal)
                      aggregateVal = component;
                    break;
                  }

                /* move forward our time of our earliest sample */
                parts[u]->lastSampleStart = earlyestTime;

              }

            if (aggOp == aggAvg)
              aggregateVal = aggregateVal / static_cast<double>(total_weight);

            ret.valid = true;
            ret.start = lastSampleEnd;
            ret.end = earlyestTime;
            ret.value = aggregateVal;
            assert(ret.value >= pdSample::Zero());

            lastSampleStart = lastSampleEnd;
            lastSampleEnd = earlyestTime;

          }

    aggu_cerr << "aggValues() - step6\n";
    return ret;
}


ostream& operator<<(ostream&s, const aggregateSample &ag) {
  const char *aggStr[] = { "aggSum", "aggMin", "aggMax", "aggAvg" };

  s << "------------------------------------------------------\n";
  s << "[aggregateSample - aggOp: " << aggStr[ag.aggOp] << ", lastSampleStart: "
    << ag.lastSampleStart << ", lastSampleEnd: " << ag.lastSampleEnd << "\n";
  s << "++ vector<sampleInfo *> parts: ";

  unsigned int i=0;
  for(i=0; i<ag.parts.size(); i++) {
    s << (i>0?",":"") << "[" << i << "]: " << *ag.parts[i];
  }
  if(ag.parts.size() == 0) s << "\n";

  s << "++ vector<bool> removedParts: ";
  for(i=0; i<ag.removedParts.size(); i++) {
    s << (i>0?",":"") << "[" << i << "]: " << ag.removedParts[i];
  }
  if(ag.removedParts.size() == 0) s << "\n";

  s << "++ vector<sampleInfo *> newParts: ";
  for(i=0; i<ag.newParts.size(); i++) {
    s << (i>0?",":"") << "[" << i << "]: " << *ag.newParts[i];
  }
  if(ag.newParts.size() == 0) s << "\n";

  s << "++ vector<bool> removedNewParts: ";
  for(i=0; i<ag.removedNewParts.size(); i++) {
    s << (i>0?",":"") << "[" << i << "]: " << ag.removedNewParts[i];
  }
  if(ag.removedNewParts.size() == 0) s << "\n";
  s << "------------------------------------------------------\n";

  return s;
}

metricAggInfo metAggInfo;

void metricAggInfo::init() {
  aggSample_doProportionCalc[EventCounter] = true;
  sampleInfo_updateStyle[EventCounter] = sampleInfo::add;

  aggSample_doProportionCalc[SampledFunction] = true;
  sampleInfo_updateStyle[SampledFunction] = sampleInfo::assign;
}
