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
 * Revision 1.27  1996/07/23 20:28:01  karavan
 * second part of two-part commit.
 *
 * implements new search strategy which retests false nodes under certain
 * circumstances.
 *
 * change in handling of high-cost nodes blocking the ready queue.
 *
 * code cleanup.
 *
 * Revision 1.26  1996/07/22 18:55:40  karavan
 * part one of two-part commit for new PC functionality of restarting searches.
 *
 * Revision 1.25  1996/05/15 04:42:02  karavan
 * oops! removed debugging print!
 *
 * Revision 1.24  1996/05/15 04:35:11  karavan
 * bug fixes: changed pendingCost pendingSearches and numexperiments to
 * break down by phase type, so starting a new current phase updates these
 * totals correctly; fixed error in estimated cost propagation.
 *
 * Revision 1.23  1996/05/11 01:58:01  karavan
 * fixed bug in PendingCost calculation.
 *
 * Revision 1.22  1996/05/08 13:37:09  naim
 * Minor changes to debugging information - naim
 *
 * Revision 1.21  1996/05/08  07:35:11  karavan
 * Changed enable data calls to be fully asynchronous within the performance consultant.
 *
 * some changes to cost handling, with additional limit on number of outstanding enable requests.
 *
 * Revision 1.19  1996/05/06 04:35:07  karavan
 * Bug fix for asynchronous predicted cost changes.
 *
 * added new function find() to template classes dictionary_hash and
 * dictionary_lite.
 *
 * changed filteredDataServer::DataFilters to dictionary_lite
 *
 * changed normalized hypotheses to use activeProcesses:cf rather than
 * activeProcesses:tlf
 *
 * code cleanup
 *
 * Revision 1.18  1996/05/02 19:46:33  karavan
 * changed predicted data cost to be fully asynchronous within the pc.
 *
 * added predicted cost server which caches predicted cost values, minimizing
 * the number of calls to the data manager.
 *
 * added new batch version of ui->DAGconfigNode
 *
 * added hysteresis factor to cost threshold
 *
 * eliminated calls to dm->enable wherever possible
 *
 * general cleanup
 *
 * Revision 1.17  1996/05/01 18:38:22  newhall
 * bug fix in enable data response
 *
 * Revision 1.16  1996/05/01  18:11:43  newhall
 * fixed some purify errors, added clientId parameter to predicted cost calls
 *
 * Revision 1.15  1996/05/01  16:05:00  naim
 * More debugging stuff to PCfilter.C - naim
 *
 * Revision 1.14  1996/05/01  14:06:55  naim
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
	       metricHandle met, focus focs, 
	       bool cf) 
: intervalLength(0), nextSendTime(0.0),  
  lastDataSeen(0), partialIntervalStartTime(0.0), workingValue(0), 
  workingInterval(0), mi(0), metric(met), foc (focs), 
  active(false), costFlag(cf), server(keeper)
{
  ;
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

void filter::wakeUp()
{
  server->makeEnableDataRequest (metric, foc, this);
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
  DataFilters(filteredDataServer::fdid_hash)
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
  for (unsigned i = 0; i < AllDataFilters.size(); i++) {
    if (AllDataFilters[i]->pausable()) {
      makeEnableDataRequest (AllDataFilters[i]->getMetric(),
			     AllDataFilters[i]->getFocus(),
			     AllDataFilters[i]);
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
      dataMgr->disableDataCollection(performanceConsultant::pstream, 
				     AllDataFilters[i]->getMI(), phType);
    }
  }
}

filteredDataServer::~filteredDataServer ()
{
  unsubscribeAllData();
  for (unsigned i = 0; i < AllDataFilters.size(); i++) {
    delete AllDataFilters[i];
  }
}

void
filteredDataServer::newDataEnabled(vector<metricInstInfo> *newlyEnabled)
{
  //** cache focus name here??
  filter *curr = NULL;
  metricInstInfo *miicurr;
  unsigned nesz = newlyEnabled->size();
  for (unsigned i = 0; i < nesz; i++) {
    miicurr = &((*newlyEnabled)[i]);

#ifdef PCDEBUG
    cout << "enable REPLY for m=" << miicurr->m_id << " f=" << miicurr->r_id 
      << " mi=" << miicurr->mi_id << endl;
#endif

    // first check if this mihandle already in use; if so its a bad internal error
    //**
    bool beenEnabled = DataFilters.find(miicurr->mi_id, curr);

    if (beenEnabled && curr) {
      // this request was part of a PC pause; there is no pending
      // record and metricInstanceHandle is unchanged
      curr->sendEnableReply(miicurr->m_id, miicurr->r_id, 
			    miicurr->mi_id, miicurr->successfully_enabled);
      return;
    } 
    bool pendingRecFound = false;
    for (unsigned j = 0; j < Pendings.size(); j++) {
      if ((Pendings[j].mh == miicurr->m_id) && (Pendings[j].f == miicurr->r_id)
	  && Pendings[j].fil) {
	pendingRecFound = true;
	curr = Pendings[j].fil;
	if (miicurr->successfully_enabled) {
	  curr->mi = miicurr->mi_id;
	  DataFilters[(fdsDataID) curr->mi] = curr;
	  Pendings[j].fil = curr;
	  miIndex[curr->metric] += Pendings[j];
	  curr->sendEnableReply(miicurr->m_id, miicurr->r_id, miicurr->mi_id, true);
	  } else { 
	    // enable failed and it has never succeeded; trash this filter
	    curr->sendEnableReply(miicurr->m_id, miicurr->r_id, 0, false);
	  }
	// clear out this record for reuse
	Pendings[j].fil = NULL;
	Pendings[j].mh = 0;
	Pendings[j].f = 0;
	break;
      }
    } // for j < Pendings.size()

    if (!pendingRecFound) {
      // this enable request was cancelled while it was being handled by the dm
      // so we need to send back an explicit cancel request
      if (phType == GlobalPhase)
	dataMgr->disableDataAndClearPersistentData
	  (performanceConsultant::pstream, miicurr->mi_id, phType, true, false);
      else
	dataMgr->disableDataAndClearPersistentData
	  (performanceConsultant::pstream, miicurr->mi_id, phType, false, true);

#ifdef PCDEBUG
      cout << "PCdisableData: m=" << miicurr->m_id << " f=" << miicurr->r_id 
	<< " mi=" << miicurr->mi_id << endl;
#endif
    }
  }
#ifdef PCDEBUG
  if (performanceConsultant::printDataCollection) 
    printPendings();
#endif
}
	
void 
filteredDataServer::makeEnableDataRequest (metricHandle met,
					   focus foc, filter *subfilter)
{
  // record the dm enable request
  bool roomAtTheInn = false;
  fmf newPending (met, foc, subfilter);
  for (unsigned j = 0; j < Pendings.size(); j++) {
    if (Pendings[j].fil == NULL) {
      Pendings[j] = newPending;
      roomAtTheInn = true;
      break;
    }
  }
  if (!roomAtTheInn) {
    Pendings += newPending;
  }
#ifdef PCDEBUG
  if (performanceConsultant::printDataCollection)
    printPendings();
#endif

  vector<metricRLType> *request = new vector<metricRLType>;
  metricRLType request_entry(met, foc);
  *request += request_entry;
  assert(request->size() == 1);
  
  // make async request to enable data
  unsigned myPhaseID = getPCphaseID();
  if (phType == GlobalPhase)
    dataMgr->enableDataRequest2(performanceConsultant::pstream, request, 
				myPhaseID, phType, dmPhaseID, 1, 0, 0);
  else
    dataMgr->enableDataRequest2(performanceConsultant::pstream, request, 
				myPhaseID, phType, dmPhaseID, 0, 0, 1);
#ifdef PCDEBUG
  // ---------------------  debug printing ----------------------------
  if (performanceConsultant::printDataCollection) {
    cout << "FDS: subscribed to "  
      << " methandle=" << met 
	<< " foc= " << "fochandle=" << foc << endl;
  }
#endif
}

void
filteredDataServer::printPendings() 
{
  cout << "Pending Enables:" << endl;
  cout << "=============== " << endl;
  for (unsigned k = 0; k < Pendings.size(); k++) {
    if (Pendings[k].fil) {
      cout << " mh:" << Pendings[k].mh << " f:" << Pendings[k].f;
      if (Pendings[k].fil == NULL)
	cout << " fil = NULL" << endl;
      else 
	cout << " fill non-NULL" << endl;
    }
  }
}


filter *
filteredDataServer::findFilter(metricHandle mh, focus f)
{
  unsigned sz = (miIndex[mh]).size();
  // is there already a filter for this met/focus pair?
  for (unsigned i = 0; i < sz; i++) {
    if ((miIndex[mh])[i].f == f) {
      return (miIndex[mh])[i].fil;
    }
  }
  return (filter *)NULL;
}

void
filteredDataServer::addSubscription(fdsSubscriber sub,
				    metricHandle mh,
				    focus f,
				    filterType ft,
				    bool flag)
{
  // is there already a filter for this met/focus pair?
  filter *subfilter = findFilter(mh, f);

  if (subfilter) {
    if (flag) subfilter->setcostFlag();
    // add subscriber to filter, which already exists
    if ( (subfilter->addConsumer (sub)) > 1) {      
      // filter is already active
      subfilter->sendEnableReply(mh, f, subfilter->getMI(), true);
    } else {
      subfilter->wakeUp();
    }
    return;
  }

  // is there already a pending enable request for this met/focus pair?
  for (unsigned k = 0; k < Pendings.size(); k++) {
    if ((Pendings[k].mh == mh) && (Pendings[k].f == f) && Pendings[k].fil) {
      subfilter = Pendings[k].fil;
      subfilter->addConsumer(sub);
      return;
    }
  }
 
  // this is first request received for this met focus pair; construct new filter
  if (ft == nonfiltering)
    subfilter = new valFilter (this, mh, f, flag);
  else
    subfilter = new avgFilter (this, mh, f, flag);
  AllDataFilters += subfilter;
  subfilter->addConsumer(sub);
  subfilter->wakeUp();
}

void 
filteredDataServer::cancelSubRequest (fdsSubscriber sub, metricHandle met, focus foc)
{
  // find the pending enable request for this met/focus pair
  for (unsigned k = 0; k < Pendings.size(); k++) {
    if ((Pendings[k].mh == met) && (Pendings[k].f == foc) && Pendings[k].fil) {
      filter *subfilter = Pendings[k].fil;
      if (subfilter->numConsumers == 1) {
	// this was the only pending request; delete it
	// clear out this record for reuse
	Pendings[k].fil = NULL;
	Pendings[k].mh = 0;
	Pendings[k].f = 0;
      }
      subfilter->rmConsumer(sub);
      break;
    }
  }
}
 
void
filteredDataServer::inActivateFilter (filter *fil)
{
  // ask dm to disable metric/focus pair 
  if (phType == GlobalPhase)
    dataMgr->disableDataAndClearPersistentData
      (performanceConsultant::pstream, fil->getMI(), phType, true, false);
  else
    dataMgr->disableDataAndClearPersistentData
      (performanceConsultant::pstream, fil->getMI(), phType, false, true);

  // update server indices
  fil->inactivate();
  DataFilters[fil->getMI()] = NULL;
  
#ifdef PCDEBUG
  cout << "PCdisableData: m=" << fil->getMetric() << " f=" << fil->getFocus() 
    << " mi=" << fil->getMI() << endl;
#endif
}
  
// 
// all filters live until end of Search, although no subscribers may remain
// if subscriber count goes to 0, disable raw data
//
void 
filteredDataServer::endSubscription(fdsSubscriber sub, metricHandle met, focus foc)
{
  filter *curr = findFilter (met, foc);
  if (!curr) 
    // invalid request 
    return;
  int subsLeft = curr->rmConsumer(sub);
  if (subsLeft == 0) {
    // we just removed the only subscriber
    inActivateFilter(curr);
  }
#ifdef PCDEBUG
  if (performanceConsultant::printDataCollection) {
    cout << "FDS: subscription ended: " << curr->getMI() << "numLeft=" << subsLeft 
         << endl; 
  }
#endif
}
  
void 
filteredDataServer::endSubscription(fdsSubscriber sub, 
				    fdsDataID subID)
{
  // find filter by subID
  int subsLeft;
  filter *curr;
  bool fndflag = DataFilters.find((unsigned)subID, curr);
  if (!fndflag) return;
  subsLeft = curr->rmConsumer(sub);
  if (subsLeft == 0) {
    // we just removed the only subscriber
    inActivateFilter(curr);
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
  filter *curr;
  bool fndflag = DataFilters.find(mih, curr);
  if (fndflag && curr) {
    // convert data to start and end based on bin
    timeStamp start = currentBinSize * bucketNumber;
    timeStamp end = currentBinSize * (bucketNumber + 1);
    curr->newData(value, start, end);
  } 

#ifdef PCDEBUG
  else {
    cout << "FDS unexpected data, mi handle = " << mih << endl;
  }
#endif
  //DataFilters.printStats();

}



