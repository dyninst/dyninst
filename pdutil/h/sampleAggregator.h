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
// deallocated by class sampleAggregator.  All aggComponent object must be a
// component of one and only one sampleAggregator.  Sharing of aggComponents
// between sampleAggregator's is not allowed.  This is because the
// aggComponents have information about the state of aggregation for a
// particular sampleAggregator.

// To interact with the aggregation code, here's what one does.  The sequence
// is important:
//   1) create a sampleAggregator object
//   2) create a new aggComponent with newComponent() of a sampleAggregator
//   3) call setInitialStartTime() on this aggComponent
//   4) call setInitialActualValue() on this aggComponent
//          note: (3) and (4) can be done in either order
//                isReadyToReceiveSamples() should now return true
//   5) call addSamplePt() on this aggComponent
//
//   6) aggregate() of the sampleAggregator object can be called at any time
//      it will return true when it is able to do the aggregation, otherwise
//      it will return false.  The aggregation result is returned in the
//      modified sampleInterval argument.  The calling of aggregate will not
//      break (that is it will return false) even if some of it's constituent
//      aggComponents aren't ready to receive samples yet (indicated with
//      function isReadyToReceiveSamples).
// 
//   7) after a call to aggregate() returns true, one can call
//      getInitialActualValue() to get the calculated initial actual value
//      for these aggComponents.  This initial actual value for the
//      sampleAggregator is determined by the sampleAggregator's aggregation
//      operator.  If it's aggSum, then the sampleAggregator initial actual
//      value will be the sum of the initial actual values of the
//      aggComponents.  If it's aggMax, the resultant initial actual value
//      will be the maximum of the initial actual values of the
//      aggComponents.
//
//
// Aggregation (aggSum, aggMax, or aggMin) is done in user specified
// intervals.  An example might help of what it's doing.  Let's say there are
// three aggComponents (A, B, and C) attached to a sampleAggregator.  Let's
// say the sampleAggregator has an interval that it's collecting data for of
// length 200 milliseconds.  Let's say aggComponent A has filled this
// interval with a value of 5.  Let's say aggComponent B has filled this
// interval with a value of 9.  Let's say C has filled the interval halfway
// with a value of 2.  If one calls aggregate() at this point, false will be
// returned since C hasn't filled the interval yet.  Let's say a addSamplePt
// gets called on C so C gets filled.  Let's say C has a final value of 6
// now.  Now if aggregate() gets called it will return true since it has data
// for all of it's components.  If the aggregation operator is aggSum, the
// resulting interval with length 200 milliseconds will have a value of 20.
// If the aggregation operator is aggMax or aggMin, then the resulting value
// is not necesarily 9 or 5 respectively.  This is because the
// sampleAggregator returns the change in the actual sample value (ie. the
// derivative) of the resulting graph.  Think of it like this, the graphs for
// A, B, and C are either summed, maxed, or mined (at each point).  Then the
// derivative of this resulting graph is taken and returned from the
// aggregate() function.
//
// If one wants to change the interval length by which the sampleAggregator
// is doing aggregation, call the member function of sampleAggregator,
// changeAggIntervalWidth.  

// If one wants to remove an aggComponent, one should NOT delete (ie. with
// the C++ delete keyword) it themselves.  One should call the requestRemove
// on this aggComponent and the parent sampleAggregator will delete this
// aggComponent after it has aggregated all the sampling data for this
// aggComponent.

// However, if one wants to finish sending samples to an aggComponent because
// the aggComponent has finished it's processing, then one should call
// markAsFinished on this aggComponent.  This will allow any
// sampleAggregators which are managing this aggComponent to not wait for
// samples on this aggComponent and hold up aggregation.  However, the
// aggComponent won't be deleted, but will stay around so it's values can
// still be aggregated with it's other sibling aggComponents.


class aggComponent {
  friend class sampleAggregator;
 public:
  // requires that initial start time hasn't already been set 

  // requires that the initial start time is greater than the
  // end of the last sampleAggregator interval.  That is the initial start
  // time needs to be in or after the current interval
  void setInitialStartTime(timeStamp initialStTime);

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

  // note the value here is a change in sample value that got added at the
  // specified timeOfSample
  void addSamplePt(timeStamp timeOfSample, pdSample value);

  // This will keep the aggComponent around and not remove the aggComponent.
  // This allows a sampleAggregator to still total the values in this
  // aggComponent when aggregating.  However, the aggComponent won't wait
  // for new samples from this aggComponent, since it has "finished".
  void markAsFinished() {
    bIsFinished = true;
  }
  void requestRemove() {  _requestRemove = true;  }
  const sampleAggregator *getParentAggregator() { return &parentAggregator; }
 private:
  aggComponent(const sampleAggregator &parent) : curIntvlVal(pdSample::Zero()),
    parentAggregator(parent), _requestRemove(false), bIsFinished(false) { }
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

  // used by processSamplePt, when a portion of the sample can be processed
  // but the leftover part of the sample should be added to the queue
  void internalAddSamplePt(timeStamp timeOfSample, pdSample value);
  void updateActualValWithIntvlVal() {
    curActualVal += curIntvlVal;
    curIntvlVal  = pdSample::Zero();
  }
  void updateWithPriorQueuedSamples(timeStamp curTimeStamp);
  void updateCurIntvlWithQueuedSamples();
  bool readyToProcessSamples() const;
  bool hasFinished() const { return bIsFinished; }
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
  bool bIsFinished;
  
  friend ostream& operator<<(ostream&s, const aggComponent &comp);
};

ostream& operator<<(ostream&s, const aggComponent &comp);


// read comments above
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

  aggComponent *getComponent(unsigned index) {
    return componentBuf[index];
  }
    
  // aggregate the values for all components.  If not ready to aggregate
  // will return false and sampleInterval argument will be unchanged.  The
  // combined graph is represented in the parameter ret.
  bool aggregate(struct sampleInterval *ret);
  
  // won't really get changed until current aggregation interval is aggregated
  void changeAggIntervalWidth(timeLength _aggIntervalWidth) {
    assert(_aggIntervalWidth > timeLength::Zero());
    newAggIntervalWidth = _aggIntervalWidth;
  }

  timeStamp  getCurIntvlStart() const {  return curIntvlStart;  }
  timeStamp  getCurIntvlEnd()   const {  return curIntvlEnd;    }
  timeLength getAggIntvlWidth() const {  return aggIntervalWidth; }
  pdSample   getInitialActualValue() const { return initActualVal; }
  
 private:
  void setCurIntvlStart(timeStamp t) {  
    curIntvlStart    = t;  
    curIntvlEnd      = curIntvlStart + newAggIntervalWidth;
    aggIntervalWidth = newAggIntervalWidth;
  }
  timeStamp earliestCompInitialStartTime() const;
  void setupIntvlTimes();
  bool allCompsReadyToReceiveSamples() const;    // for addSamplePt
  bool allCompsCompleteForInterval() const;
  void updateCompValues();
  void updateQueuedSamples();
  bool readyToAggregate();
  void calcInitialActualVal();
  void actuallyAggregate(struct sampleInterval *ret);
  void removeComponentsRequestedToRemove();

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

// need to be defined down here, since parentAggregator is after aggCompon.
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

