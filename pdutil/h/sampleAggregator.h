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

#ifndef SAMPLE_AGGREGATOR_H
#define SAMPLE_AGGREGATOR_H

#include <assert.h>
#include "common/h/Vector.h"
#include "common/h/Time.h"
#include "pdutil/h/pdSample.h"
#include "pdutil/h/aggregationDefines.h"
#include "pdutil/h/PriorityQueue.h"


//
// What gets returned when a newValue is called in aggComponent;
struct sampleInterval {
  timeStamp start;
  timeStamp end;
  pdSample  value;
};

// class aggComponent: define a class for sample values. This class should be
// used with class sampleAggregator. aggComponent objects are allocated and
// deallocated by class sampleAggregator.  Objects are allocated by
// sampleAggregator::newSampleInfo.  All aggComponent object must be a
// component of at least one sampleAggregator, and they can of more many
// samplesAggregator.

class aggComponent {
  friend class sampleAggregator;
 public:
  void setInitialStartTime(timeStamp initialStTime);
  void resetInitialStartTime(timeStamp initialStTime);
  timeStamp getInitialStartTime() const {
    return lastProcessedSampleTime;
  }
  bool isInitialStartTimeSet() const {
    return lastProcessedSampleTime.isInitialized();
  }
  bool isReadyToReceiveSamples() const {
    return isInitialStartTimeSet() && (!curActualVal.isNaN());
  }
  bool isInitialActualValueSet() const {
    return !initActualVal.isNaN();
  }
  void setInitialActualValue(pdSample v) {  
    assert(initActualVal.isNaN());
    initActualVal = v;  
    curActualVal = v;
  }
  pdSample getInitialActualValue() const { return initActualVal; }
  void addSamplePt(timeStamp timeOfSample, pdSample value);
  void requestRemove() {  _requestRemove = true;  }
  const sampleAggregator *getParentAggregator() { return &parentAggregator; }
 private:
  aggComponent(const sampleAggregator &parent) : curIntvlVal(pdSample::Zero()),
    parentAggregator(parent), _requestRemove(false) { }
  ~aggComponent() { }
  timeStamp  startIntvl() const;
  timeStamp  endIntvl()   const;
  timeLength intvlWidth() const;
  bool curIntvlFilled()   const;
  pdSample   getCurIntvlVal()  const { return curIntvlVal; }
  pdSample   getCurActualVal() const { return curActualVal + curIntvlVal; }
  bool filledUpto(timeStamp timeMark) const {
    return (lastProcessedSampleTime >= timeMark);
  }

  void processSamplePt(timeStamp timeOfSample, pdSample value);
  void updateActualValWithIntvlVal() {
    curActualVal += curIntvlVal;
    curIntvlVal  = pdSample::Zero();
  }
  void updateWithPriorQueuedSamples(timeStamp curTimeStamp);
  void updateCurIntvlWithQueuedSamples();
  bool readyToProcessSamples() const;
  bool isRemoveRequested() const { return _requestRemove; }
  unsigned numQueuedSamples() { return futureSamples.size(); }

  timeStamp lastProcessedSampleTime;
  pdSample  curIntvlVal;
  pdSample  curActualVal;      // used when calculating max's and min's
  pdSample  initActualVal;     // the initial actual value
  PriorityQueue<timeStamp, pdSample> futureSamples;
  const sampleAggregator &parentAggregator;
  bool _requestRemove;
  bool bIsInitialStartTimeSet;
  
  friend ostream& operator<<(ostream&s, const aggComponent &comp);
};

ostream& operator<<(ostream&s, const aggComponent &comp);



class sampleAggregator {
 public:
  // a sentinal value, for aggregate function
  static const timeStamp DONTWAITTOAGGREGATE;

  sampleAggregator(aggregateOp aggregateOp, timeLength initialIntervalWidth) : 
    aggOp(aggregateOp), aggIntervalWidth(initialIntervalWidth), 
    newAggIntervalWidth(initialIntervalWidth), 
    bCachedAllStartTimesReceived(false) 
    {  
    assert(initialIntervalWidth > timeLength::Zero());  
    }  

  ~sampleAggregator() { 
    for(unsigned i=0; i<componentBuf.size(); i++) {
      componentBuf[i]->requestRemove();
    }
    removeComponentsRequestedToRemove();
  }
  
  unsigned numComponents() const {
    return componentBuf.size();
  }
  
  // no sharing of components between sampleAggregators is allowed
  aggComponent *newComponent();

  aggComponent *getComponent(int index) {
    return componentBuf[index];
  }
    
  // aggregate the values for all components.  If not ready to aggregate
  // will return false and sampleInterval argument will be unchanged
  bool aggregate(struct sampleInterval *ret, 
		 timeStamp curTime = DONTWAITTOAGGREGATE);
  
  // won't really get changed until current aggregation interval is aggregated
  void changeAggIntervalWidth(timeLength _aggIntervalWidth) {
    assert(_aggIntervalWidth > timeLength::Zero());
    newAggIntervalWidth = _aggIntervalWidth;
  }

  timeStamp  getCurIntvlStart() const {  return curIntvlStart;  }
  timeStamp  getCurIntvlEnd()   const {  return curIntvlEnd;    }
  timeLength getAggIntvlWidth() const {  return aggIntervalWidth; }
  pdSample   getInitialActualValue() const { return initActualVal; }
  //static timeLength getAggDelayTime() {  return aggDelayTime;   }
  //static void setAggDelayTime(timeLength t) { aggDelayTime = t; }
  
 private:
  void setCurIntvlStart(timeStamp t) {  
    curIntvlStart    = t;  
    curIntvlEnd      = curIntvlStart + newAggIntervalWidth;
    aggIntervalWidth = newAggIntervalWidth;
  }
  timeStamp earliestCompInitialStartTime() const;
  void tryToSetupIntvls();
  bool allCompsReadyToReceiveSamples() const;    // for addSamplePt
  bool allCompsCompleteForInterval() const;
  void updateCompValues();
  void updateQueuedSamples();
  bool readyToAggregate(timeStamp curTime);
  void calcInitialActualVal();
  void actuallyAggregate(struct sampleInterval *ret);
  void removeComponentsRequestedToRemove();

  static timeLength aggDelayTime;
  const aggregateOp aggOp;       // how to combine component values
  timeStamp  curIntvlStart;
  timeStamp  curIntvlEnd;
  timeLength aggIntervalWidth;  // components will get resampled every interval
  timeLength newAggIntervalWidth;  // to change aggInterval, update this
                                   // can only change aggInterval 
  pdSample lastActualVal;  // the actual value as determined by the last
                           // call to the aggregation function
  pdSample initActualVal;       // the initial actual value for this 
                                // aggregate (derivative) graph
  mutable bool bCachedAllStartTimesReceived;
  vector<aggComponent *> componentBuf;
  
  friend ostream& operator<<(ostream&s, const sampleAggregator &ag);
};

ostream& operator<<(ostream&s, const sampleAggregator &ag);

inline timeStamp aggComponent::startIntvl() const {  
  return parentAggregator.getCurIntvlStart(); 
}

inline timeStamp aggComponent::endIntvl()   const {  
  return parentAggregator.getCurIntvlEnd();   
}

inline timeLength aggComponent::intvlWidth() const {  
  return parentAggregator.getAggIntvlWidth(); 
}

inline bool aggComponent::readyToProcessSamples() const {
  //  return (parentAggregator.allCompsReadyToReceiveSamples() &&
  return  startIntvl().isInitialized()  &&  endIntvl().isInitialized();
}



#endif

