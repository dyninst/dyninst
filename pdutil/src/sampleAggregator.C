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

// $Id: sampleAggregator.C,v 1.2 2001/08/28 02:50:46 schendel Exp $

#include <assert.h>
#include <math.h>
#include <iostream.h>
#include "pdutil/h/sampleAggregator.h"
#include "pdutil/h/pdDebugOstream.h"


#ifdef AGGREGATE_DEBUG
pdDebug_ostream aggu_cerr(cerr, true);
#else
pdDebug_ostream aggu_cerr(cerr, false);
#endif

const timeStamp  sampleAggregator::DONTWAITTOAGGREGATE(timeStamp::ts1970());
timeLength sampleAggregator::aggDelayTime;

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

// used when need to adjust start of component to a more convenient time
void aggComponent::resetInitialStartTime(timeStamp initialStTime) {
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

void aggComponent::addSamplePt(timeStamp timeOfSample, pdSample value) {
  aggu_cerr << "addSamplePt- " << this << ", timeOfSample: " << timeOfSample
	    << ", lastProcessedSampleTime: " << lastProcessedSampleTime 
	    << ", value: " << value << "\n";
  assert(timeOfSample > lastProcessedSampleTime);
  futureSamples.add(timeOfSample, value);
}

void aggComponent::processSamplePt(timeStamp timeOfSample, pdSample value) {
  aggu_cerr << "processSamplePt\n";
  aggu_cerr << "timeOfSample: " << timeOfSample << "\n";
  aggu_cerr << "lastProcessedSampleTime: " << lastProcessedSampleTime << "\n";
  aggu_cerr << "startIntvl: " << startIntvl();
  aggu_cerr << "endIntvl: " << endIntvl();
  aggu_cerr << "value: " << value << "\n";

  assert(timeOfSample > startIntvl());
  assert(readyToProcessSamples());
  assert(! curIntvlFilled());
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
  pdSample addToLeftOver = value - addToCur;
  aggu_cerr << "addToLeftOver: " << addToLeftOver << "\n";
  if(addToLeftOver > pdSample::Zero()) {
    // if there's value leftOver (for the next interval) than the timeOfSample
    // better be later in time than the endIntvl() time
    assert(timeOfSample > endIntvl() && rightTimeMark==endIntvl());
    addSamplePt(timeOfSample, addToLeftOver);
  }
  lastProcessedSampleTime = rightTimeMark;
}

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

// We want to consider that the interval is filled if it has been marked for
// removal and it has received atleast one sample in this interval.  We need
// to do this so that all the components can be considered to have filled
// intervals (even those requested for removal) and thus aggregation can
// occur even with components that are requested for removal.  We consider
// the case that the lastProcessedSampleTime = startIntvl as a filled
// interval since the times of its samples could be in line with the parent
// sampleAggregator's interval boundary times, and it could have been 
// aggregated in the last interval
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

// returns true if start and end time of intervals was successfully set up
void sampleAggregator::tryToSetupIntvls() {
  assert(! curIntvlStart.isInitialized());
  aggu_cerr << "tryToSetupIntvls()\n";

  timeStamp earliestStartTime = earliestCompInitialStartTime();
  setCurIntvlStart(earliestStartTime);
  aggu_cerr << "Setting earliest time to: " << earliestStartTime << "\n";
}

bool sampleAggregator::readyToAggregate(timeStamp /* curTime */) {
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

  // make sure all the start & end time of the current interval is set up

  if(! curIntvlStart.isInitialized())
    tryToSetupIntvls();

  // we want to wait a certain amount of time until we do aggregation (even
  // if it seems like we're all ready to go) because a new component
  // (ie. daemon/metric-focus pair) could start with a start time in the
  // current interval.  If we start aggregating right away, then we've moved
  // the current interval down to far in time, past the "new" components
  // time.  I think this race condition is possible because the time
  // adjustment factor between the daemon and the front-end can only be so
  // accurate.  I haven't seen any of these particular race conditions
  // recently so we'll investigate this further if we start seeing further
  // problems.  If the bug occurs, it should be triggered in the assert in
  // setInitialStartTime.
  /*
  if(curTime != DONTWAITTOAGGREGATE) {
    timeLength timeWaitedSoFar = curTime - curIntvlEnd;
    if(timeWaitedSoFar < getAggDelayTime()) {
      aggu_cerr << "timeWaitedSoFar: " << timeWaitedSoFar << " < "
		<< "aggDelayTime: " << getAggDelayTime() << ", returning\n";
      //      return false;
    }
  }
  */

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

// the aggregation is passed a set of graphs that represent the rate of
// change (ie. derivative) of the sample value and and an initial actual
// value of the graph.  The aggregation code calculates a (derivative) graph
// that represents the change in the (sum, max, or min) of the individual
// graphs and an initial actual value for this combined graph.

bool sampleAggregator::aggregate(struct sampleInterval *ret, 
				 timeStamp curTime) {
  aggu_cerr << "aggregate- start: " << curIntvlStart << ", end: " 
	    << curIntvlEnd << "\n";

  if(! readyToAggregate(curTime))
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
    } else if(aggOp == aggMax) {
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
  for(unsigned i=0; i<componentBuf.size(); i++) {
    aggComponent *curComp = componentBuf[i];
    if(curComp->isRemoveRequested() && curComp->numQueuedSamples()==0) {
      aggu_cerr << "removing aggComp: " << curComp << "\n";
      componentBuf.erase(i);
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




