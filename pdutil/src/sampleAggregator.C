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

// $Id: sampleAggregator.C,v 1.13 2004/03/23 01:12:41 eli Exp $

#include <assert.h>
#include <math.h>
#include <iostream>
#include "pdutil/h/sampleAggregator.h"
#include "pdutil/h/pdDebugOstream.h"

unsigned enable_pd_sample_aggregate_debug = 0;

#if ENABLE_DEBUG_CERR == 1
#define aggu_cerr if (enable_pd_sample_aggregate_debug) cerr
#else
#define aggu_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

// Essentially, the aggregation is passed a set of graphs that represent the
// rate of change (ie. derivative) of the sample value and and an initial
// actual value of the graph.  The aggregation code calculates a (derivative)
// graph that represents the change in the (sum, max, or min) of the
// component graphs and an initial actual value for this combined graph.

// The set of graphs are broken up such that the sum, max, or min of the
// combined graph is calculated at specific intervals (ie. aggregation
// intervals).  This interval width can be changed (changeAggIntervalWidth())
// during the lifetime of the sampleAggregator object.  The individual sets
// of graphs are represented with aggComponent objects.  These aggComponents
// don't store the entire graph, but just the sampling data for the current
// interval and sampling data which is in the future compared to the current
// interval.

// When all the aggComponents have filled their intervals then the
// aggregation code can calculate a combined interval value.  From this
// combined graph, the derivative of the graph at this interval can be
// calculated.

// The initial start time needs to be set before values can be aggregated.
// Samples added through addSamplePt before initial start time set, will be
// queued.

void aggComponent::setInitialStartTime(timeStamp initialStTime) {
  assert(! isInitialStartTimeSet());

  // the addSamplePt function won't work if the lastProcessedSampleTime
  // isn't greater than the start of the interval
  if(startIntvl().isInitialized()) {
    if(! (initialStTime > startIntvl())) {
      cerr << "trying to start a component later in time (" << initialStTime
	   << ") than\nthe current interval start time (" << startIntvl()
	   << ").  Error.\n";
    }
    assert(initialStTime > startIntvl());
  }
  lastProcessedSampleTime = initialStTime;
}

// Just adds the samples to a priority queue sorted on timeOfSample.  Samples
// added for a particular aggComponent need to occur sequentially later in
// time.
void aggComponent::addSamplePt(timeStamp timeOfSample, pdSample value) {
  aggu_cerr << "addSamplePt- " << this << ", timeOfSample: " << timeOfSample
	    << ", lastProcessedSampleTime: " << lastProcessedSampleTime 
	    << ", value: " << value << "\n";
  assert(! hasFinished());  // can't add samples if it has finished already
  internalAddSamplePt(timeOfSample, value);
}

void aggComponent::internalAddSamplePt(timeStamp timeOfSample,pdSample value) {
  assert(timeOfSample > lastProcessedSampleTime);
  futureSamples.add(timeOfSample, value);
}


// Actually process a sample, sample should have been shifted off of queue
void aggComponent::processSamplePt(timeStamp timeOfSample, pdSample value) {
  aggu_cerr << "processSamplePt\n";
  aggu_cerr << "timeOfSample: " << timeOfSample << "\n";
  aggu_cerr << "lastProcessedSampleTime: " << lastProcessedSampleTime << "\n";
  aggu_cerr << "startIntvl: " << startIntvl();
  aggu_cerr << "endIntvl: " << endIntvl();
  aggu_cerr << "value: " << value << "\n";

  assert(timeOfSample > startIntvl());
  assert(readyToProcessSamples());

  // curIntvlFilled() is always false at any of this function's callsites --
  // therefore, the following assert merely doubles the number of
  // times that we call curIntvlFilled.  Since the DM spends more time
  // in curIntvlFilled than in any other function (about 33%), I'm
  // excising this waste   -- willb 3/2003

  // assert(! curIntvlFilled());

  assert(timeOfSample > lastProcessedSampleTime);

  timeStamp rightTimeMark = earlier(timeOfSample, endIntvl());
  aggu_cerr << "rightTimeMark: " << rightTimeMark;
  timeStamp leftTimeMark  = later(lastProcessedSampleTime, startIntvl());
  aggu_cerr << "leftTimeMark: " << leftTimeMark << "\n";
  timeLength timeInCurIntvl = rightTimeMark - leftTimeMark;
  timeLength timeSinceLastSample = timeOfSample - lastProcessedSampleTime;
  aggu_cerr << "timeInCurIntvl: " << timeInCurIntvl << "\n";
  aggu_cerr << "timeSinceLastSampl: " << timeSinceLastSample << "\n";
  double pcntInCurIntvl = timeInCurIntvl / timeSinceLastSample;
  aggu_cerr << "pcntInCurIntvl: " << pcntInCurIntvl;
  pdSample addToCur = pcntInCurIntvl * value;
  aggu_cerr << "addToCur: " << addToCur << "\n";
  curIntvlVal += addToCur;
  // the leftover is the amount in later intervals
  pdSample addToLeftOver = value - addToCur;
  aggu_cerr << "addToLeftOver: " << addToLeftOver << "\n";

  // save the remainder of the sample that was after the current interval
  if(timeOfSample > endIntvl())
    internalAddSamplePt(timeOfSample, addToLeftOver);

  lastProcessedSampleTime = rightTimeMark;
}

// will attempt to update the aggComponents with the queued samples
void aggComponent::updateCurIntvlWithQueuedSamples() {
  aggu_cerr <<"------------   updateCurIntvlWithQueuedSamples  ---------------\n";
  aggu_cerr << "this: " << this << ", curIntvlVal: " << curIntvlVal << "\n";
  aggu_cerr << "curIntvlFilled: " << curIntvlFilled() << ", ";
  aggu_cerr << "futureSamples.size: " << futureSamples.size() << "\n";
  aggu_cerr << "endIntvl: " << endIntvl() << ", ";
  aggu_cerr << "lastProcessedSampleTime: " << lastProcessedSampleTime << "\n";
  while(!curIntvlFilled() && futureSamples.size()>0) {
    timeStamp frontPtTime = futureSamples.peek_first_key();
    pdSample frontPtVal = futureSamples.peek_first_data();
    futureSamples.delete_first();
    processSamplePt(frontPtTime, frontPtVal);
  }
  aggu_cerr << "curIntvlVal: " << curIntvlVal << "\n";
  aggu_cerr <<"^^^^^^^^^^^^   updateCurIntvlWithQueuedSamples  ^^^^^^^^^^^^^^^\n";
}

// indicates whether the aggComponent has received sampling data for the
// entire current interval
bool aggComponent::curIntvlFilled()  const {
  if(isInitialStartTimeSet() && filledUpto(endIntvl()))
    return true;
  else
    return false;
}

ostream& operator<<(ostream&s, const aggComponent &info) {
  s << "[aggComp- " << ", lastP.SampleTime: " << info.lastProcessedSampleTime
    << ", curIntvlVal: " << info.curIntvlVal << ", " 
    << info.futureSamples.size() << " future samples" << ", parentAggregator: "
    << static_cast<const void*>(&info.parentAggregator) << ", requestRemove: "
    << info._requestRemove << "]";
  return s;
}

aggComponent *sampleAggregator::newComponent() {
  aggComponent *comp = new aggComponent(*this);
  componentBuf.push_back(comp);
  bCachedAllStartTimesReceived = false;
  return comp;
}

// I consider a component complete for an interval if it has either
// filled the interval with samples or else it's last sample was at the
// beginning of the interval and it's marked for removal
bool sampleAggregator::allCompsCompleteForInterval() const {
  for(unsigned i=0; i<componentBuf.size(); i++) {
    aggComponent *curComp = componentBuf[i];
    bool compDone = false;  // the component is done for this interval
    if(curComp->hasFinished())
      compDone = true;
    if(curComp->isRemoveRequested() && curComp->filledUpto(curIntvlStart))
      compDone = true;
    bool compIntvlComplete = false;
    if(curComp->curIntvlFilled() || compDone)
      compIntvlComplete = true;
    aggu_cerr << "  (" << i << ") for comp: " << curComp << ", intvlFilled: " 
          << curComp->curIntvlFilled() << ", done: " << compDone << "\n";
    if(! compIntvlComplete)  return false;
  }
  return true;
}

void sampleAggregator::updateCompValues() {
  for(unsigned i=0; i<componentBuf.size(); i++) {
    componentBuf[i]->updateActualValWithIntvlVal();
  }
}

void sampleAggregator::updateQueuedSamples() {
  for(unsigned i=0; i<componentBuf.size(); i++) {
    componentBuf[i]->updateCurIntvlWithQueuedSamples();
  }
}

bool sampleAggregator::allCompsReadyToReceiveSamples() const {
  aggu_cerr << "allCompInitialStartTimesReceived()\n";
  if(bCachedAllStartTimesReceived == true) return true;

  bool allCompReady = true;
  for(unsigned i=0; i<componentBuf.size(); i++) {
    aggu_cerr << "  comp[" << i << "]: " 
	      << componentBuf[i]->isReadyToReceiveSamples() << "\n";
    if(! componentBuf[i]->isReadyToReceiveSamples()) {
      allCompReady = false;
      break;
    }
  }
  bCachedAllStartTimesReceived = allCompReady;
  aggu_cerr << "  returning " << allCompReady << "\n";
  return allCompReady;
}

// returns the component start time that is the earliest out of all
// of the components
// 
timeStamp sampleAggregator::earliestCompInitialStartTime() const {
  timeStamp earliestTime = timeStamp::tsFarOffTime();
  for(unsigned i=0; i<componentBuf.size(); i++) {
    timeStamp stTime = componentBuf[i]->getInitialStartTime();
    if(! stTime.isInitialized()) 
      continue;
    if(stTime < earliestTime)
      earliestTime = stTime;
  }
  return earliestTime;
}

void sampleAggregator::setupIntvlTimes() {
  assert(! curIntvlStart.isInitialized());
  aggu_cerr << "setupIntvlTimes()\n";

  timeStamp earliestStartTime = earliestCompInitialStartTime();
  setCurIntvlStart(earliestStartTime);
  aggu_cerr << "Setting earliest time to: " << earliestStartTime << "\n";
}

bool sampleAggregator::readyToAggregate() {
  // the order that the following occurs is very specific
  // don't change this sequence without thinking through how it'll work

  if(numComponents() == 0) {
    aggu_cerr << "numComponents == 0, returning\n";
    return false;
  }

  // one example when this can occur is when a metricInstance component has
  // been "added" by the front-end but the front-end hasn't yet received a
  // message from the daemon that the daemon has created the metricInstance.
  if(! allCompsReadyToReceiveSamples()) {
    aggu_cerr << "! allCompsReadyToReceiveSamples, returning\n";
    return false;
  }

  // set up the start & end time of the current interval if not set up
  if(! curIntvlStart.isInitialized())
    setupIntvlTimes();

  // now start updating the components' current values with any queued
  // samples
  updateQueuedSamples();

  // this can return false when either a component hasn't received samples
  // yet that fill up the interval or else a component is marked for removal
  // but it hasn't filled up the interval completely
  if(! allCompsCompleteForInterval()) {
    aggu_cerr << "! all components complete for interval, returning\n";
    return false;
  }

  return true;
}

// attempt to aggregate the constituent graphs (ie. intervals)
bool sampleAggregator::aggregate(struct sampleInterval *ret) {
  aggu_cerr << "aggregate- start: " << curIntvlStart << ", end: " 
	    << curIntvlEnd << "\n";

  if(! readyToAggregate())
    return false;

  if(getInitialActualValue().isNaN()) {
    // sets initiActualVal and lastActualVal
    calcInitialActualVal();
  }
  actuallyAggregate(ret);
  return true;
}

void sampleAggregator::calcInitialActualVal() {
  pdSample aggInitActualVal;
  for(unsigned i=0; i<componentBuf.size(); i++) {
    aggComponent &curComp = *componentBuf[i];
    pdSample compInitActualVal = curComp.getInitialActualValue();
    if(i==0) {
      aggInitActualVal = compInitActualVal;
    } else if(aggOp == aggSum) {
      aggInitActualVal += compInitActualVal;
    } else if(aggOp == aggMax) {
      if(compInitActualVal > aggInitActualVal)
	aggInitActualVal = compInitActualVal;
    } else if(aggOp == aggMin) {
      if(compInitActualVal < aggInitActualVal)
	aggInitActualVal = compInitActualVal;
    }
  }
  initActualVal = aggInitActualVal;
  lastActualVal = aggInitActualVal;
  aggu_cerr << "  setting initActualVal to " << initActualVal << "\n";
}

void sampleAggregator::actuallyAggregate(struct sampleInterval *ret) {
  pdSample aggActualVal;
  for(unsigned i=0; i<componentBuf.size(); i++) {
    aggComponent &curComp = *componentBuf[i];
    
    pdSample compActualVal = curComp.getCurActualVal();
    if(i==0) {
      aggActualVal = compActualVal;
    } else if(aggOp == aggSum) {
      aggActualVal += compActualVal;
    } else if(aggOp == aggMax) {
      // For aggMax and aggMin, we need to look at the actual value since the
      // max/min of the interval (ie. the delta) isn't the max/min of the
      // actual value, which is what we want.
      // Remember the point in time is actually at the end of the intervals
      // since we've waited until the intervals are filled.
      if(compActualVal > aggActualVal)  aggActualVal = compActualVal;
    } else if(aggOp == aggMin) {
      if(compActualVal < aggActualVal)  aggActualVal = compActualVal;   
    }
  }
  aggu_cerr << "  calculated aggActualVal: " << aggActualVal 
	    << "lastActualVal: " << lastActualVal << ", initActVal: "
	    << initActualVal << "\n";

  pdSample aggIntvlVal = aggActualVal - lastActualVal;

  removeComponentsRequestedToRemove();
  assert(curIntvlEnd > curIntvlStart);
  (*ret).start = curIntvlStart;
  (*ret).end = curIntvlEnd;
  (*ret).value = aggIntvlVal;

  aggu_cerr << "aggreg- st: " << curIntvlStart << ", end: " << curIntvlEnd << "\n";
  aggu_cerr << "             value(interval): " << aggIntvlVal 
	    << ",  (aggActualVal: " << aggActualVal << ")\n";
  setCurIntvlStart(curIntvlEnd);  // curIntvlEnd = curIntvlStart + intvlWidth
  updateCompValues();
  lastActualVal = aggActualVal;
}

void sampleAggregator::removeComponentsRequestedToRemove() {
   pdvector<aggComponent *>::iterator iter = componentBuf.end();
   while(iter != componentBuf.begin()) {
      iter--;
      aggComponent *curComp = (*iter);
      if(curComp->isRemoveRequested() && curComp->numQueuedSamples()==0) {
         aggu_cerr << "removing aggComp: " << curComp << "\n";
         componentBuf.erase(iter);
         delete curComp;
      }
   }
}

ostream& operator<<(ostream&s, const sampleAggregator &ag) {
  const char *aggStr[] = { "aggSum", "aggMin", "aggMax", "aggAvg" };

  s << "------  sampleAggregator  --------------------------------------\n";
  s << "[aggOp: " << aggStr[ag.aggOp] << ", numComponents: " 
    << ag.numComponents() << ", curIntvlStart: " << ag.curIntvlStart 
    << ", curIntvlEnd: " << ag.curIntvlEnd << ", aggIntervalWidth: " 
    << ag.aggIntervalWidth << ", newAggIntervalWidth: " 
    << ag.newAggIntervalWidth << "]";
  return s;
}




