/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 * 
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */

/*
 * PCfilter.C
 *
 * Data filter class performs initial processing of raw DM data arriving 
 * in the Performance Consultant.  
 *
 * $Log: PCfilter.C,v $
 * Revision 1.3  1996/02/09 20:57:26  karavan
 * Added performanceConsultant::globalRawDataServer and
 * performanceConsultant::currentRawDataServer to streamline new data
 * storage.
 *
 * Revision 1.2  1996/02/08 19:52:41  karavan
 * changed performance consultant's use of tunable constants:  added 3 new
 * user-level TC's, PC_CPUThreshold, PC_IOThreshold, PC_SyncThreshold, which
 * are used for all hypotheses for the respective categories.  Also added
 * PC_useIndividualThresholds, which switches thresholds back to use hypothesis-
 * specific, rather than categorical, thresholds.
 *
 * Moved all TC initialization to PCconstants.C.
 *
 * Switched over to callbacks for TC value updates.
 *
 * Revision 1.1  1996/02/02 02:06:33  karavan
 * A baby Performance Consultant is born!
 *
 */

#include "PCfilter.h"
#include "PCintern.h"
#include "PCmetricInst.h"

perfStreamHandle filteredDataServer::pstream = 0;

ostream& operator <<(ostream &os, filter& f)
{
  const char *focname = dataMgr->getFocusNameFromMI(f.mi);
  const char *metname = dataMgr->getMetricNameFromMI(f.mi);
  os << "FILTER (mi=)" << f.mi << ":" << "metric: " << f.metric 
    << " " << metname << endl;
  os << " focus: " << f.foc << " " << focname << endl
    << "  workingValue: " << f.workingValue << endl
    << "  workingInterval: " << f.workingInterval << endl
      << "  cum average: " << (f.workingValue / f.workingInterval) << endl;
  
  os  << "  nextSendTime: " << f.nextSendTime << endl
    << "  lastDataSeen: " << f.lastDataSeen << endl
      << "  curr int start: " << f.partialIntervalStartTime <<endl
      << "  intervalLength: " << f.intervalLength << endl;
  return os;
}

filter::filter(filteredDataServer *keeper, 
	       metricInstanceHandle mih, metricHandle met, focus focs) 
: mi(mih), metric(met), foc (focs), intervalLength(0), nextSendTime(0.0),  
  lastDataSeen(0), partialIntervalStartTime(0.0), workingValue(0), 
  workingInterval(0), server(keeper)
{ 
  estimatedCost = 0.0;
  estimatedCost = dataMgr->getPredictedDataCost(foc, metric);
}

filter::~filter() {
  ;
} 

float
filter::getNewEstimatedCost()
{
  float oldcost = estimatedCost;
  estimatedCost = dataMgr->getPredictedDataCost (foc, metric);
  return estimatedCost;
}

void 
filter::updateNextSendTime(timeStamp startTime) 
{
  // each filter's interval size may get out of sync with the server 
  // for one send, so here we check if we need to resync.
  timeStamp newint = server->intervalSize;
  if (newint > intervalLength) {
    // intervalSize has changed since we last checked.  This means
    // this is the first piece of data since the fold occurred.
    intervalLength = newint;
    nextSendTime = startTime + newint;
    // ensure that the server always contains a valid send time 
    // for the current interval size.  
    if (nextSendTime > server->nextSendTime)
      server->nextSendTime = nextSendTime;
  } else {
    // need the loop here in case there's been an interruption in data 
    // values.
    while (nextSendTime < (startTime + newint))
      nextSendTime += newint;
  }
}

void filter::getInitialSendTime(timeStamp startTime)
{
  // set first value for intervalLength for this filter
  intervalLength = server->intervalSize;
  // the server gives us a valid send time for this interval size, although
  // it may be old.  we can catch up to startTime by simply adding 
  // intervalLength sized increments till first send will be a complete 
  // interval.
  nextSendTime = server->nextSendTime;
  while (nextSendTime < (startTime + intervalLength))
    nextSendTime += intervalLength;
}

void
filter::newData(sampleValue newVal, timeStamp start, timeStamp end)
{
  // lack of forward progress signals a serious internal data handling error
  // in paradyn DM and will absolutely break the PC
  assert (end > lastDataSeen);

  // first, make adjustments to nextSendTime, partialIntervalStartTime,
  // and/or start to handle some special cases
  if (nextSendTime == 0) {
    // CASE 1: this is first data received since this filter was created.
    // We compute nextSendTime, the first possible send, and initialize 
    // partial interval to begin at time "start".
    getInitialSendTime(start);  
    partialIntervalStartTime = nextSendTime - intervalLength;
  } else if (start > lastDataSeen)  {
    // CASE 2: this is first data value after a gap in data.
    // We re-compute nextSendTime and reset partial interval (if any)
    // to begin at time "start".  if there was partial data before this 
    // new data came along, it is already included in the running 
    // average, so we're not discarding it, we're just not sending it
    // separately, since we never send partial intervals.
    updateNextSendTime(start);
    partialIntervalStartTime = nextSendTime - intervalLength;
  } else if (start < lastDataSeen) {
    // CASE 3: partially duplicate data after a fold
    // we've already seen data for a piece of this interval, so we reset 
    // the start time to use the new portion and ignore the rest
    start = lastDataSeen;
  }
  if (end > nextSendTime) {
  // CASE 4: first data value after a fold
  // call to updateNextSendTime will correctly adjust intervalSize
  // we roll back nextSendTime to last send, then recompute nextSendTime.
    nextSendTime = partialIntervalStartTime;
    updateNextSendTime (partialIntervalStartTime);
  }

  // now we update our running average, and send a data value to subscribers
  // if this datum fills an interval.
  // Note: because intervalLength is always set to 
  //       MAX(requested value, bucket size)
  // we never overlap more than one interval with a single datum

  timeStamp currInterval = end - start;

 if (end >= nextSendTime) {
    // PART A: if we have a full interval of data, split off piece of 
    // data which is within interval (if needed) and send.
    timeStamp pieceOfInterval = nextSendTime - start;
    workingValue += newVal * (pieceOfInterval);
    workingInterval += pieceOfInterval;
    sendValue(mi, workingValue/workingInterval, partialIntervalStartTime, 
              nextSendTime, 0);
#ifdef PCDEBUG
    // debug printing
    if (performanceConsultant::printDataTrace) {
      cout << "FILTER SEND mi=" << mi << " mh=" << metric << " foc=" << foc 
	<< " value=" << workingValue/workingInterval 
	  << "interval=" << start << " to " << end << endl;
    }
#endif

    // its important to update intervalLength only after we send a sample;
    // part of what this filtering accomplishes is keeping intervals correctly
    // aligned in spite of the unaligned data the data manager sends 
    // around a fold.  If you're not clear on what happens around a fold,
    // go find out before you try to change this code!!
    partialIntervalStartTime = nextSendTime;
    updateNextSendTime (nextSendTime);
    // adjust currInterval to contain only portion of time not just sent,   
    // since we already added the other portion to workingInterval above
    currInterval = currInterval - pieceOfInterval;     
  }
  // PART B: either the new value received did not complete an interval, 
  // or it did and there may be some remainder data.  If no remainder data,
  // currInterval will be 0 so the next two instructions have no effect.
  workingValue += newVal * currInterval;
  workingInterval += currInterval;
  lastDataSeen = end;
  
#ifdef PCDEBUG
  // debug printing
  if (performanceConsultant::printDataTrace) {
    cout << *this;
  }
#endif
}

filteredDataServer::filteredDataServer(phaseType pt)
: nextSendTime(0.0), pht(pt),  
  DataFilters(filteredDataServer::fdid_hash),
  DataByMetFocus(filteredDataServer::fdid_hash)
{
  if (pt == GlobalPhase) {
    currentBinSize = dataMgr->getGlobalBucketWidth();
    performanceConsultant::globalRawDataServer = this;
  } else {
    currentBinSize = dataMgr->getCurrentBucketWidth();
    performanceConsultant::currentRawDataServer = this;
  }
  timeStamp minGranularity = performanceConsultant::minObservationTime/4.0;
  // ensure that starting interval size is an even multiple of dm's 
  // bucket width, for efficiency:  once bucket width passes minimum,
  // we will send on each piece of data as it comes rather than splitting
  // it across intervals each time.
  timeStamp binFactor = currentBinSize;
  while (binFactor < minGranularity) {
    binFactor *= 2;
  }
  // adjusted minGranularity, or binsize, whichever's bigger
  intervalSize = binFactor;
}

//
// dm has doubled the bin size for the data histogram.  server needs the 
// actual size to convert data values to intervals.  Also, we adjust
// intervalSize if needed to ensure intervalSize = MAX(minInterval, binSize)
//
void
filteredDataServer::newBinSize (timeStamp bs)
{
  currentBinSize = bs;
  if (bs > intervalSize)
    intervalSize = bs;
}

//
// restart PC after PC-pause (not application-level pause)
//
void
filteredDataServer::resubscribeAllData() 
{
  for (unsigned i = 0; i < AllDataFilters.size(); i++) {
    dataMgr->enableDataCollection2(filteredDataServer::pstream, 
				   AllDataFilters[i]->getFocus(), 
				   AllDataFilters[i]->getMetric(),
				   pht, 1, 0);
  }
}

//
// stop all data flow to PC for a PC-pause
//
void
filteredDataServer::unsubscribeAllData() 
{
  for (unsigned i = 0; i < AllDataFilters.size(); i++) {
    dataMgr->disableDataCollection(filteredDataServer::pstream, 
				   AllDataFilters[i]->getMI(), pht);
  }
}

filteredDataServer::~filteredDataServer ()
{
  unsubscribeAllData();
  for (unsigned i = 0; i < AllDataFilters.size(); i++) {
    delete AllDataFilters[i];
  }
}

fdsDataID
filteredDataServer::addSubscription(fdsSubscriber sub,
				    metricHandle mh,
				    focus f,
				    bool *errFlag)
{
  filter *subfilter;
  metricInstanceHandle *index;
  metricInstanceHandle indexCopy;
  *errFlag = false;
  // does filter already exist?
  index = dataMgr->enableDataCollection2 (filteredDataServer::pstream, 
					  f, mh, pht, 1, 0);
  if (index == NULL) {
    // unable to collect this data
    *errFlag = true;
    return 0;
  }
  indexCopy = *index;
  delete index;
  if (!DataFilters.defines((fdsDataID) indexCopy)) {
    subfilter = new filter (this, indexCopy, mh, f);
    AllDataFilters += subfilter;
    DataFilters[(fdsDataID) indexCopy] = subfilter;
  } else
    subfilter = DataFilters[indexCopy];
  // add subscriber
  subfilter->addConsumer (sub);
#ifdef PCDEBUG
  // debug printing
  if (performanceConsultant::printDataCollection) {
    cout << "FDS: " << sub << " subscribed to " << indexCopy << " met=" 
      << dataMgr->getMetricNameFromMI(indexCopy) << " methandle=" << mh << endl
	<< "foc=" << dataMgr->getFocusNameFromMI(indexCopy) << endl;
  }
#endif
  return indexCopy;
}

float
filteredDataServer::getEstimatedCost(metricHandle mh,
				     focus f)
{
  return dataMgr->getPredictedDataCost (f, mh);
}

float
filteredDataServer::getEstimatedCost(fdsDataID mih)
{
  if (DataFilters.defines(mih))
    return (DataFilters[mih]->getEstimatedCost());
  else
    return 0.0;
}

// all filters live until end of Search, although no subscribers may remain
// if subscriber count goes to 0, disable raw data
//
void 
filteredDataServer::endSubscription(fdsSubscriber sub, 
				    fdsDataID subID)
{
  // find filter by subID
  int subsLeft;
  if (DataFilters.defines((unsigned)subID)) { 
    subsLeft = DataFilters[(unsigned)subID]->rmConsumer(sub);
    if (subsLeft == 0) {
      dataMgr->clearPersistentData(subID);
      dataMgr->disableDataCollection (pstream, subID, pht);
    }
  }
#ifdef PCDEBUG
  // debug printing
  if (performanceConsultant::printDataCollection) {
    cout << "FDS: subscription ended: " << subID << "numLeft=" << subsLeft
      << endl << " met=" << dataMgr->getMetricNameFromMI(subID) << endl;
    cout << "foc=" << dataMgr->getFocusNameFromMI(subID) << endl;
  }
#endif
}

void
filteredDataServer::newData (metricInstanceHandle mih, 
				 sampleValue value, 
				 int bucketNumber)
{
  //** handle error
  if ( DataFilters.defines ((fdsDataID) mih)) {
    // convert data to start and end based on bin
    timeStamp start = currentBinSize * bucketNumber;
    timeStamp end = currentBinSize * (bucketNumber + 1);
    //** question here multiply value by bucketWidth as in oldPC/PCmain.C??   
    //**     sampleValue total = value*PCbucketWidth;
    DataFilters[(fdsDataID) mih]-> newData(value, start, end);
  } 

#ifdef PCDEBUG
  //** handle error
  else {
    cout << "FDS unexpected data, mi handle = " << mih << endl;
  }
#endif

}
