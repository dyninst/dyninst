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
 * Revision 1.14  1996/05/01 14:06:55  naim
 * Multiples changes in PC to make call to requestNodeInfoCallback async.
 * (UI<->PC). I also added some debugging information - naim
 *
 * Revision 1.13  1996/04/30  18:56:57  newhall
 * changes to support the asynchrounous enable data calls to the DM
 * this code contains a kludge to make the PC wait for the DM's async response
 *
 * Revision 1.12  1996/04/30  06:26:49  karavan
 * change PC pause function so cost-related metric instances aren't disabled
 * if another phase is running.
 *
 * fixed bug in search node activation code.
 *
 * added change to treat activeProcesses metric differently in all PCmetrics
 * in which it is used; checks for refinement along process hierarchy and
 * if there is one, uses value "1" instead of enabling activeProcesses metric.
 *
 * changed costTracker:  we now use min of active Processes and number of
 * cpus, instead of just number of cpus; also now we average only across
 * time intervals rather than cumulative average.
 *
 * Revision 1.11  1996/04/24 15:01:19  naim
 * Minor change to print message - naim
 *
 * Revision 1.10  1996/04/22  17:59:23  newhall
 * added comments, minor change to getPredictedDataCostAsync
 *
 * Revision 1.9  1996/04/21  21:45:38  newhall
 * changed getPredictedDataCostAsync
 *
 * Revision 1.8  1996/04/18  22:01:52  naim
 * Changes to make getPredictedDataCost asynchronous - naim
 *
 * Revision 1.7  1996/04/07  21:24:36  karavan
 * added phaseID parameter to dataMgr->enableDataCollection2.
 *
 * Revision 1.6  1996/03/18 07:10:35  karavan
 * added new tunable constant, PCcollectInstrTimings.
 *
 * Revision 1.5  1996/03/05 16:12:55  naim
 * Minor changes for debugging purposes - naim
 *
 * Revision 1.4  1996/02/22  18:30:01  karavan
 * bug fix to PC pause/resume so only filters active at time of pause
 * resubscribe to data
 *
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
#include "dataManager.thread.h"

#ifdef MYPCDEBUG
#include <sys/time.h>
double TESTgetTime()
{
  double now;
  struct timeval tv;
  gettimeofday(&tv,NULL);
  now = (double) tv.tv_sec + ((double)tv.tv_usec/(double)1000000.0);
  return(now);
}
static double enableTotTime=0.0,enableWorstTime=0.0;
static int enableCounter=0;
#endif

extern performanceConsultant *pc;
extern float getPredictedDataCostAsync(perfStreamHandle, resourceListHandle, 
				       metricHandle);
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
	       metricInstanceHandle mih, metricHandle met, focus focs, 
	       bool cf) 
: intervalLength(0), nextSendTime(0.0),  
  lastDataSeen(0), partialIntervalStartTime(0.0), workingValue(0), 
  workingInterval(0), mi(mih), metric(met), foc (focs), 
  estimatedCost(0.0), costFlag(cf), server(keeper)
{
  ;
}
 
float
filter::getNewEstimatedCost()
{
  estimatedCost = getPredictedDataCostAsync(filteredDataServer::pstream,
					    foc, metric);
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
avgFilter::newData(sampleValue newVal, timeStamp start, timeStamp end)
{
#ifdef MYPCDEBUG
    double t1,t2,t3,t4;
    t1=TESTgetTime();
#endif
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
#ifdef MYPCDEBUG
    t3=TESTgetTime();
#endif
    sendValue(mi, workingValue/workingInterval, partialIntervalStartTime, 
              nextSendTime, 0);
#ifdef MYPCDEBUG
    t4=TESTgetTime();
#endif
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

#ifdef MYPCDEBUG
    t2=TESTgetTime();
    if ((t2-t1) > 1.0) {
      printf("********** filter::newData took %5.2f seconds, sendValue took %5.2f seconds\n",t2-t1,t4-t3);
    }
#endif
}

void
valFilter::newData(sampleValue newVal, timeStamp start, timeStamp end)
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
    // new data came along, it is discarded since it may no longer 
    // be current. 
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
  // currInterval will be 0 so workingValue and workingInterval will both
  // be set to 0.
  workingValue = newVal * currInterval;
  workingInterval = currInterval;
  lastDataSeen = end;
  
#ifdef PCDEBUG
  // debug printing
  if (performanceConsultant::printDataTrace) {
    cout << *this;
  }
#endif
}

filteredDataServer::filteredDataServer(unsigned phID)
: nextSendTime(0.0), 
  DataFilters(filteredDataServer::fdid_hash),
  DataByMetFocus(filteredDataServer::fdid_hash)
{
  dmPhaseID = phID - 1;
  if (phID == GlobalPhaseID) {
    phType = GlobalPhase;
    currentBinSize = dataMgr->getGlobalBucketWidth();
    performanceConsultant::globalRawDataServer = this;
  } else {
    phType = CurrentPhase;
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
  metricInstanceHandle *curr;
#ifdef MYPCDEBUG
  double t1,t2;
#endif
  for (unsigned i = 0; i < AllDataFilters.size(); i++) {
    if (AllDataFilters[i]->pausable()) {
#ifdef MYPCDEBUG
      if (performanceConsultant::collectInstrTimings) {
	t1=TESTgetTime(); 
      }
#endif
      
      vector<metricRLType> *request = new vector<metricRLType>;
      metricRLType request_entry(AllDataFilters[i]->getMetric(),
		                 AllDataFilters[i]->getFocus());
      *request += request_entry;
      assert(request->size() == 1);
      // make async request to enable data
      dataMgr->enableDataRequest2(filteredDataServer::pstream,request,
				  0,phType,dmPhaseID,1,0);

      // KLUDGE wait for DM's async response
      bool ready=false;
      vector<metricInstInfo> *response;
      // wait for response from DM
      while(!ready){
	  T_dataManager::msg_buf buffer;
	  T_dataManager::message_tags waitTag;
	  tag_t tag = T_dataManager::enableDataCallback_REQ;
	  int from = msg_poll(&tag, true);
	  assert(from != THR_ERR);
	  if (dataMgr->isValidTag((T_dataManager::message_tags)tag)) {
	      waitTag = dataMgr->waitLoop(true,
			(T_dataManager::message_tags)tag,&buffer);
              if(waitTag == T_dataManager::enableDataCallback_REQ){
		  ready = true;
		  response = buffer.enableDataCallback_call.response;
		  buffer.enableDataCallback_call.response = 0;
	      }
	      else {
		  cout << "error PC wait data enable resp:tag invalid" << endl;
		  assert(0);
	      }
	  }
	  else{
	      cout << "error PC wait data enable resp:tag invalid" << endl;
	      assert(0);
	  }
      } // while(!ready)
      curr = 0;
      // if this MI was successfully enabled
      if(response && (*response)[0].successfully_enabled) {
	  curr = new metricInstanceHandle;
	  *curr = (*response)[0].mi_id; 
      }

#ifdef MYPCDEBUG
      // -------------------------- PCDEBUG ------------------
      if (performanceConsultant::collectInstrTimings) {
        t2=TESTgetTime();
        enableTotTime += t2-t1;
        enableCounter++;
        if ((t2-t1) > enableWorstTime) enableWorstTime = t2-t1;
        if ((t2-t1) > 1.0) 
	  printf("=-=-=-=> PCfilter 1, enableDataRequest2 took %5.2f secs, avg=%5.2f, worst=%5.2f\n",t2-t1,enableTotTime/enableCounter,enableWorstTime); 
      }
      // -------------------------- PCDEBUG ------------------
#endif
      delete curr;
    }
  }
}

//
// stop all data flow to PC for a PC-pause
//
void
filteredDataServer::unsubscribeAllData() 
{
  for (unsigned i = 0; i < AllDataFilters.size(); i++) {
    if (AllDataFilters[i]->pausable()) {
#ifdef MYPCDEBUG
      double t1,t2;
      if (performanceConsultant::collectInstrTimings) {
	t1=TESTgetTime(); 
      }
#endif
      dataMgr->disableDataCollection(filteredDataServer::pstream, 
				     AllDataFilters[i]->getMI(), phType);
#ifdef MYPCDEBUG
      // -------------------------- PCDEBUG ------------------
      if (performanceConsultant::collectInstrTimings) {
        t2=TESTgetTime();
        if ((t2-t1) > 1.0) 
	  printf("==> TEST <== PCfilter 1, disableDataCollection took %5.2f secs\n",t2-t1); 
      }
      // -------------------------- PCDEBUG ------------------
#endif
    }
  }
}

filteredDataServer::~filteredDataServer ()
{
  unsubscribeAllData();
  for (unsigned i = 0; i < AllDataFilters.size(); i++) {
#ifdef MYPCDEBUG
    if (!AllDataFilters[i]) printf("++++++++++++++ AllDataFilters[%d] is NULL\n",i);
#endif
    delete AllDataFilters[i];
  }
}

fdsDataID
filteredDataServer::addSubscription(fdsSubscriber sub,
				    metricHandle mh,
				    focus f,
				    filterType ft,
				    bool *flag)
{
  filter *subfilter;
  metricInstanceHandle *index;
  metricInstanceHandle indexCopy;
#ifdef MYPCDEBUG
  double t1,t2;
#endif
  // does filter already exist?
#ifdef MYPCDEBUG
  if (performanceConsultant::collectInstrTimings) {
    t1=TESTgetTime(); 
  }
#endif

      vector<metricRLType> *request = new vector<metricRLType>;
      metricRLType request_entry(mh,f);
      *request += request_entry;
      assert(request->size() == 1);
      // make async request to enable data
      dataMgr->enableDataRequest2(filteredDataServer::pstream,request,
				  0,phType,dmPhaseID,1,0);

      // KLUDGE wait for DM's async response
      bool ready=false;
      vector<metricInstInfo> *response;

      // wait for response from DM
      while(!ready){
	  T_dataManager::msg_buf buffer;
	  T_dataManager::message_tags waitTag;
	  tag_t tag = T_dataManager::enableDataCallback_REQ;
	  int from = msg_poll(&tag, true);
	  assert(from != THR_ERR);
	  if (dataMgr->isValidTag((T_dataManager::message_tags)tag)) {
	      waitTag = dataMgr->waitLoop(true,
			(T_dataManager::message_tags)tag,&buffer);
              if(waitTag == T_dataManager::enableDataCallback_REQ){
		  ready = true;
		  response = buffer.enableDataCallback_call.response;
		  buffer.enableDataCallback_call.response = 0;
	      }
	      else {
		  cout << "error PC wait data enable resp:tag invalid" << endl;
		  assert(0);
	      }
	  }
	  else{
	      cout << "error PC wait data enable resp:tag invalid" << endl;
	      assert(0);
	  }
      } // while(!ready)
      index = 0;
      // if this MI was successfully enabled
      if(response && (*response)[0].successfully_enabled) {
	  index = new metricInstanceHandle;
	  *index = (*response)[0].mi_id; 
      }

#ifdef MYPCDEBUG
  if (performanceConsultant::collectInstrTimings) {
    t2=TESTgetTime();
    enableTotTime += t2-t1;
    enableCounter++;
    if ((t2-t1) > enableWorstTime) enableWorstTime = t2-t1;
    if ((t2-t1) > 1.0)
      printf("=-=-=-=> PCfilter 2, enableDataRequest2 took %5.2f secs, avg=%5.2f, worst=%5.2f\n",t2-t1,enableTotTime/enableCounter,enableWorstTime);
  }
#endif
  if (index == NULL) {
    // unable to collect this data
    *flag = true;
    return 0;
  }
  indexCopy = *index;
  delete index;
  if (!DataFilters.defines((fdsDataID) indexCopy)) {
    if (ft == nonfiltering)
      subfilter = new valFilter (this, indexCopy, mh, f, *flag);
    else
      subfilter = new avgFilter (this, indexCopy, mh, f, *flag);
    AllDataFilters += subfilter;
    DataFilters[(fdsDataID) indexCopy] = subfilter;
  } else
    subfilter = DataFilters[indexCopy];
  // add subscriber
  subfilter->addConsumer (sub);
#ifdef PCDEBUG
  // ---------------------  debug printing ----------------------------
  if (performanceConsultant::printDataCollection) {
    cout << "FDS: " << sub << " subscribed to " << indexCopy << " met=" 
      << dataMgr->getMetricNameFromMI(indexCopy) << " methandle=" << mh << endl
	<< "foc=" << dataMgr->getFocusNameFromMI(indexCopy) << endl;
  }
#endif

#ifdef MYPCDEBUG
  if (performanceConsultant::collectInstrTimings) {
    if ((t2-t1) > 1.0) 
      printf("==> TEST <== Metric name = %s, Focus name = %s\n",dataMgr->getMetricNameFromMI(indexCopy),dataMgr->getFocusNameFromMI(indexCopy)); 
  }
  // ---------------------  debug printing  ----------------------------
#endif
  *flag = false;
  return indexCopy;
}

float
filteredDataServer::getEstimatedCost(metricHandle mh,
				     focus f)
{
  return(getPredictedDataCostAsync(filteredDataServer::pstream,f, mh));
}

float
filteredDataServer::getEstimatedCost(fdsDataID mih)
{
  if (DataFilters.defines(mih))
    return (DataFilters[mih]->getNewEstimatedCost());
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
#ifdef MYPCDEBUG
      double t1,t2;
      if (performanceConsultant::collectInstrTimings) {
	t1=TESTgetTime();
      }
#endif
      dataMgr->clearPersistentData(subID);
      dataMgr->disableDataCollection (pstream, subID, phType);
#ifdef MYPCDEBUG
      if (performanceConsultant::collectInstrTimings) {
        t2=TESTgetTime();
        if ((t2-t1) > 1.0) 
	  printf("==> TEST <== PCfilter 2, disableDataCollection took %5.2f secs\n",t2-t1); 
      }
#endif
    }
  }
#ifdef PCDEBUG
  // debug printing
  if (performanceConsultant::printDataCollection) {
    cout << "FDS: subscription ended: " << subID << "numLeft=" << subsLeft 
         << endl; 
  }
#endif
}

void
filteredDataServer::newData (metricInstanceHandle mih, 
				 sampleValue value, 
				 int bucketNumber)
{
  if ( DataFilters.defines ((fdsDataID) mih)) {
    // convert data to start and end based on bin
    timeStamp start = currentBinSize * bucketNumber;
    timeStamp end = currentBinSize * (bucketNumber + 1);
    DataFilters[(fdsDataID) mih]-> newData(value, start, end);
  } 

#ifdef PCDEBUG
  else {
    cout << "FDS unexpected data, mi handle = " << mih << endl;
  }
#endif

}
