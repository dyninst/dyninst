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

//
// Define a class for a sample value.
//    Sample values can be a single value or may be the result of an aggregate.
//    This is in the util library because both paradynd's and paradyn need to
//      use it.
//

#ifndef UTIL_SAMPLE
#define UTIL_SAMPLE

#include "pdutil/h/hist.h"
#include "common/h/Vector.h"
#include "pdutil/h/aggregation.h"

//
// What gets returned when a newValue is called in sampleInfo;
struct sampleInterval {
    bool valid;
    timeStamp	start;
    timeStamp	end;
    pdSample    value;
};

// class sampleInfo: define a class for sample values. This class should be used
// with class aggregatesample. sampleInfo objects are allocated and deallocated
// by class aggregateSample.
// Objects are allocated by aggregateSample::newSampleInfo.
// All sampleInfo object must be a component of at least one aggregateSample, 
// and they can of more many aggregateSamples.

class sampleInfo {
  friend class aggregateSample;

  public:
 
    bool firstValueReceived() const { return firstSampleReceived; }

//    void startTime(timeStamp startTime_);
    void firstTimeAndValue(timeStamp, pdSample firstValue);

    void newValue(timeStamp wallTime, pdSample value, unsigned weight_ = 1);

    timeStamp lastSampleTime() { return lastSampleEnd; }

    // constructor and destructor are private. 
    // They should only be used by class aggregateSample.
    typedef enum { add, assign } updateStyle;

private:

    sampleInfo(updateStyle style) : valueUpdateStyle(style) {
      firstSampleReceived = false;
      weight = 1;
      numAggregators = 0;
    }

    ~sampleInfo() {};

    sampleInfo &operator=(const sampleInfo &src) {
       if (&src == this)
          return *this;
       
       firstSampleReceived = src.firstSampleReceived;
       lastSampleStart = src.lastSampleStart;
       lastSampleEnd = src.lastSampleEnd;
       lastSample = src.lastSample;
       weight = src.weight;
       numAggregators = src.numAggregators;
       valueUpdateStyle = src.valueUpdateStyle;
       return *this;
    }

    bool firstSampleReceived;        // has first sample been recorded
    timeStamp   lastSampleStart;        // start time for last sample
    timeStamp   lastSampleEnd;          // end time for last sample
    pdSample    lastSample;             // what was the last sample increment
    unsigned numAggregators;            // number of aggregateSample this is a part of
    unsigned weight;                    // weight of this sample
    updateStyle valueUpdateStyle;       // should the sampleInfo be updated
                                        // with an addition or assignment

    friend ostream& operator<<(ostream&s, const sampleInfo &info);
};

ostream& operator<<(ostream&s, const sampleInfo &info);


// aggregateSample: aggregate values for samples. aggregate samples can have
// one or more components. Components can be added or removed at any time.

class aggregateSample {

public:
  aggregateSample(int aggregateOp, bool proportionCalc) : 
    doProportionCalc(proportionCalc) {
    assert(aggregateOp == aggSum || aggregateOp == aggAvg || 
	   aggregateOp == aggMin || aggregateOp == aggMax);
    aggOp = aggregateOp;
  }
  
  ~aggregateSample() {
    unsigned u;
    for (u = 0; u < newParts.size(); u++)
      if (--newParts[u]->numAggregators == 0)
        delete newParts[u];
    for (u = 0; u < parts.size(); u++)
      if (--parts[u]->numAggregators == 0)
        delete parts[u];
  }

  sampleInfo *newComponent(sampleInfo::updateStyle st) {
    sampleInfo *comp = new sampleInfo(st);
    addComponent(comp);
    return comp;
  }

  void addComponent(sampleInfo *comp) {
    newParts += comp;
    removedNewParts += false;
    comp->numAggregators++;
  }

  // remove a component. The sampleInfo object will be deallocated once
  // its value has been aggregated.
  void removeComponent(sampleInfo *comp) {
    for (unsigned u1 = 0; u1 < parts.size(); u1++) {
      if (parts[u1] == comp) {
        removedParts[u1] = true;
        return;
      }
    }
    for (unsigned u2 = 0; u2 < newParts.size(); u2++) {
      if (newParts[u2] == comp) {
        removedNewParts[u2] = true;
        return;
      }
    }
    assert(0);
  }

  // aggregate the values for all components.
  struct sampleInterval aggregateValues();

  // return the number of components included in the last valid aggregate value.
  inline unsigned numComponents() {
    return parts.size();
  }

  timeStamp currentTime() { return lastSampleEnd; }

private:

  int aggOp;                        // the aggregate operator (sum, avg, min, max)
  bool doProportionCalc;            // if set, will combine the proportion
                                    // of the sample value that is within the 
                                    // aggregated interval
  timeStamp lastSampleStart;        // start time of last sample
  timeStamp lastSampleEnd;          // end time of last sample
  vector<sampleInfo *> parts;       // the parts that are being aggregated.
                                    // For all u, 
                                    //    parts[u]->lastSampleStart == lastSampleEnd
  vector<bool> removedParts;        // true if parts[u] has been removed.
                                    // Once parts[u]->lastSampleStart ==
                                    //                 parts[u]->lastSampleEnd
                                    // parts[u] will be deleted.
  vector<sampleInfo *> newParts;    // new parts that have been added to the 
                                    // aggregation, but cannot be aggregated yet
                                    // because their start time is not aligned with
                                    // lastSampleEnd.
  vector<bool> removedNewParts;     // new parts that have been removed.

  friend ostream& operator<<(ostream&s, const aggregateSample &ag);
};

ostream& operator<<(ostream&s, const aggregateSample &ag);



class metricAggInfo {
  enum { numMetricStyles = 2 };

  bool aggSample_doProportionCalc[numMetricStyles];
  sampleInfo::updateStyle sampleInfo_updateStyle[numMetricStyles];

 public:
  metricAggInfo() {
    init();
  }
  void init();
  bool get_proportionCalc(metricStyle s) {
    return aggSample_doProportionCalc[s];
  }
  sampleInfo::updateStyle get_updateStyle(metricStyle s) {
    return sampleInfo_updateStyle[s];
  }
};

extern metricAggInfo metAggInfo;


#endif

