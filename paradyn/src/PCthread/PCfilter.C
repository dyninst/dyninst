/*
 * Copyright (c) 1996-1998 Barton P. Miller
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

/*
 * $Id: PCfilter.C,v 1.41 2003/05/09 20:12:58 pcroth Exp $    
 */

#include "PCfilter.h"
#include "PCintern.h"
#include "PCmetricInst.h"
#include "dataManager.thread.h"
#include <math.h> // fabs

#include "../DMthread/DMmetric.h"  // delete this after the completed convers.


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
     << "  cum average: " << pdRate(f.workingValue, f.workingInterval) << endl;
  
  os  << "  nextSendTime: " << f.nextSendTime << endl
      << "  lastDataSeen: " << f.lastDataSeen << endl
      << "  curr int start: " << f.partialIntervalStartTime <<endl
      << "  intervalLength: " << f.intervalLength << endl;
  return os;
}

filter::filter(filteredDataServer *keeper, 
	       metricHandle met, focus focs, 
	       bool cf) 
: intervalLength(timeLength::Zero()), nextSendTime(relTimeStamp::Zero()),  
  lastDataSeen(relTimeStamp::Zero()), 
  partialIntervalStartTime(relTimeStamp::Zero()), 
  workingValue(pdSample::Zero()), workingInterval(timeLength::Zero()), mi(0), 
  metric(met), foc (focs), status(Inactive), costFlag(cf), server(keeper)
{
  ;
}
 
void 
filter::updateNextSendTime(relTimeStamp startTime) 
{
  // each filter's interval size may get out of sync with the server 
  // for one send, so here we check if we need to resync.
  timeLength newint = server->intervalSize;
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
	//
	// note that we check for closeness of the proposed nextSendTime 
	// with the startTime + newint, to avoid situations where floating
	// point roundoff error has produced a value that "should be" the
	// same, but isn't.  If we don't have this check, on some processors
	// (e.g., some x86) we see the performance consultant search stall
	// as unaligned interval endpoints cause data to be thrown away
    while ( (nextSendTime < (startTime + newint)) && 
	    (abs(nextSendTime - (startTime + newint)) > 
	     (timeLength::us()*100)))
	{
      nextSendTime += newint;
	}
  }
}

void filter::getInitialSendTime(relTimeStamp startTime)
{
  // set first value for intervalLength for this filter
  intervalLength = server->intervalSize;
  // the server gives us a valid send time for this interval size, although
  // it may be old.  we can catch up to startTime by simply adding 
  // intervalLength sized increments till first send will be a complete 
  // interval.
  nextSendTime = server->nextSendTime;

	//
	// note that we check for closeness of the proposed nextSendTime 
	// with the startTime + newint, to avoid situations where floating
	// point roundoff error has produced a value that "should be" the
	// same, but isn't.  If we don't have this check, on some processors
	// (e.g., some x86) we see the performance consultant search stall
	// as unaligned interval endpoints cause data to be thrown away
  while ( (nextSendTime < (startTime + intervalLength)) &&
	  (abs(nextSendTime - (startTime + intervalLength)) > 
	   (timeLength::us()*100)))
  {
    nextSendTime += intervalLength;
  }
}

void filter::wakeUp()
{
  server->makeEnableDataRequest (metric, foc);
  status = ActivationRequestPending;
}
  
void
avgFilter::newData(pdRate newVal, relTimeStamp start, relTimeStamp end)
{
#ifdef MYPCDEBUG
    timeStamp t1,t2,t3,t4;
    t1 = getCurrentTime();
#endif
  // lack of forward progress signals a serious internal data handling error
  // in paradyn DM and will absolutely break the PC
  assert (end > lastDataSeen);

  // first, make adjustments to nextSendTime, partialIntervalStartTime,
  // and/or start to handle some special cases
  if (nextSendTime == relTimeStamp::Zero()) {
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

  timeLength currInterval = end - start;

 if (end >= nextSendTime) {
    // PART A: if we have a full interval of data, split off piece of 
    // data which is within interval (if needed) and send.
    timeLength pieceOfInterval = nextSendTime - start;
    workingValue += newVal * pieceOfInterval;
    workingInterval += pieceOfInterval;
#ifdef MYPCDEBUG
    t3 = getCurrentTime();
#endif
    pdRate curRate(workingValue, workingInterval);
    sendValue(mi, curRate, partialIntervalStartTime, nextSendTime);
#ifdef MYPCDEBUG
    t4 = getCurrentTime();
#endif
#ifdef PCDEBUG
    // debug printing
    if (performanceConsultant::printDataTrace) {
      cout << "FILTER SEND mi=" << mi << " mh=" << metric << " foc=" << foc 
	   << " value=" << curRate << " interval=" << start << " to " << end 
	   << endl;
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
    t2 = getCurrentTime();
    if ((t2-t1) > timeLength::sec()) {
      cerr << "********** filter::newData took " << t2-t1 
	   << " , sendValue took " << t4-t3 << "\n";
    }
#endif
}

void
valFilter::newData(pdRate newVal, relTimeStamp start, relTimeStamp end)
{
  // lack of forward progress signals a serious internal data handling error
  // in paradyn DM and will absolutely break the PC
  assert (end > lastDataSeen);
  // first, make adjustments to nextSendTime, partialIntervalStartTime,
  // and/or start to handle some special cases
  if (nextSendTime == relTimeStamp::Zero()) {
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

  timeLength currInterval = end - start;

 if (end >= nextSendTime) {
    // PART A: if we have a full interval of data, split off piece of 
    // data which is within interval (if needed) and send.
    timeLength pieceOfInterval = nextSendTime - start;
    workingValue += newVal * pieceOfInterval;
    workingInterval += pieceOfInterval;
    pdRate curValue(workingValue, workingInterval);
    sendValue(mi, curValue, partialIntervalStartTime, nextSendTime);
#ifdef PCDEBUG
    // debug printing
    if (performanceConsultant::printDataTrace) {
      cout << "FILTER SEND mi=" << mi << " mh=" << metric << " foc=" << foc 
	   << " value=" << curValue 
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
: nextSendTime(relTimeStamp::Zero()),
  DataFilters(filteredDataServer::fdid_hash)
{
  dmPhaseID = phID - 1;
  if (phID == GlobalPhaseID) {
    phType = GlobalPhase;
    dataMgr->getGlobalBucketWidth(&currentBinSize);
    performanceConsultant::globalRawDataServer = this;
  } else {
    phType = CurrentPhase;
    dataMgr->getCurrentBucketWidth(&currentBinSize);
    performanceConsultant::currentRawDataServer = this;
  }
  minGranularity = performanceConsultant::minObservationTime / 4.0;
  // ensure that starting interval size is an even multiple of dm's 
  // bucket width, for efficiency:  once bucket width passes minimum,
  // we will send on each piece of data as it comes rather than splitting
  // it across intervals each time.
  timeLength binFactor = currentBinSize;
  while (binFactor < minGranularity) {
    binFactor *= 2;
  }
  // adjusted minGranularity, or binsize, whichever's bigger
  intervalSize = binFactor;
  miIndex = new dictionary_hash<focus, filter*>*[performanceConsultant::numMetrics];
  for (unsigned j = 0; j < performanceConsultant::numMetrics; j++)
    miIndex[j] = new dictionary_hash<focus, filter*> 
      (filteredDataServer::fdid_hash);
}

//
// dm has doubled the bin size for the data histogram.  server needs the 
// actual size to convert data values to intervals.  Also, we adjust
// intervalSize if needed to ensure intervalSize = MAX(minInterval, binSize)
//
void
filteredDataServer::newBinSize (timeLength bs)
{
  currentBinSize = bs;
  if (bs > intervalSize)
    intervalSize = bs;
}


//
// stop all data flow to PC
//
void
filteredDataServer::unsubscribeAllData() 
{
  for (unsigned i = 0; i < AllDataFilters.size(); i++) {
    if (AllDataFilters[i]->pausable()) {
      dataMgr->disableDataCollection(performanceConsultant::pstream, 0, 
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
filteredDataServer::newDataEnabled(pdvector<metricInstInfo> *newlyEnabled)
{
  //** cache focus name here??
  filter *curr = NULL;
  metricInstInfo *miicurr;
  unsigned nesz = newlyEnabled->size();
  for (unsigned i = 0; i < nesz; i++) {
    miicurr = &((*newlyEnabled)[i]);

#ifdef PCDEBUG
    cout << "enable REPLY for m=" << miicurr->m_id << " f=" << miicurr->r_id 
      << " mi=" << miicurr->mi_id  
      << "  enabled=" << miicurr->successfully_enabled << endl;
#endif

    bool beenEnabled = DataFilters.find(miicurr->mi_id, curr);

    if (beenEnabled && curr) {
      // this request was part of a PC pause; there is no pending
      // record and metricInstanceHandle is unchanged
      curr->sendEnableReply(miicurr->m_id, miicurr->r_id, 
			    miicurr->mi_id, miicurr->successfully_enabled);
      return;
    } 
    // mihandle not in use; get filter for this mh/f pair
    filter *curr = findFilter(miicurr->m_id, miicurr->r_id);
    if (!curr) {
      //**
      //cout << "UH-OH, FILTER NOT FOUND! mh=" << miicurr->m_id << " f=" 
	//<< miicurr->r_id << endl;
      return;
    }
    if (curr->status != filter::ActivationRequestPending) {
      // this enable request was cancelled while it was being handled by the dm
      // so we need to send back an explicit cancel request
      if (phType == GlobalPhase)
	dataMgr->disableDataAndClearPersistentData
	  (performanceConsultant::pstream,0,miicurr->mi_id,phType,true,false);
      else
	dataMgr->disableDataAndClearPersistentData
	  (performanceConsultant::pstream,0,miicurr->mi_id,phType,false,true);

#ifdef PCDEBUG
      cout << "PCdisableData: m=" << miicurr->m_id << " f=" << miicurr->r_id 
	<< " mi=" << miicurr->mi_id << endl;
#endif
    } else {
      if (miicurr->successfully_enabled) {
	curr->mi = miicurr->mi_id;
	DataFilters[(fdsDataID) curr->mi] = curr;
	curr->status = filter::Active;
	curr->sendEnableReply(miicurr->m_id, miicurr->r_id, miicurr->mi_id, true);
      } else { 
	// enable failed 
	//**
	curr->sendEnableReply(miicurr->m_id, miicurr->r_id, 0, false);
	curr->status = filter::Inactive;
      }
    }
  }

#ifdef PCDEBUG
  if (performanceConsultant::printDataCollection) 
    printPendings();
    ;
#endif
}

void filteredDataServer::setCurActualValue(metricInstanceHandle mih, 
					   pdSample v)
{
  filter *curr;
  bool fndflag = DataFilters.find(mih, curr);
  assert(fndflag && curr);
  curr->setCurActualValue(v);
}
	
void 
filteredDataServer::makeEnableDataRequest (metricHandle met,
					   focus foc)
{

#ifdef PCDEBUG
  if (performanceConsultant::printDataCollection)
    printPendings();
#endif

  pdvector<metricRLType> *request = new pdvector<metricRLType>;
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
  for (unsigned k = 0; k < AllDataFilters.size(); k++) {
    filter *curr = AllDataFilters[k];
    if (curr && curr->isPending()) {
      cout << " mh:" << curr->metric << " f:" << curr->foc;
    }
  }
}


filter *
filteredDataServer::findFilter(metricHandle mh, focus f)
{
  filter *fil;
  dictionary_hash<focus, filter*> *curr = miIndex[mh];
  bool fndflag = curr->find(f, fil);
  if (!fndflag) return (filter *)NULL;
  return fil;
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

    if (subfilter->status == filter::Active) {
      subfilter->addConsumer (sub);
      subfilter->sendEnableReply(mh, f, subfilter->getMI(), true);
    } else if (subfilter->status == filter::Inactive) {
      subfilter->addConsumer (sub);
      subfilter->wakeUp();
    } else if (subfilter->status == filter::ActivationRequestPending) {
      subfilter->addConsumer(sub);
    }
    return;
  }

  // this is first request received for this met focus pair; construct new filter
  if (ft == nonfiltering)
    subfilter = new valFilter (this, mh, f, flag);
  else
    subfilter = new avgFilter (this, mh, f, flag);
  AllDataFilters += subfilter;
  (*(miIndex[mh])) [f] = subfilter;  
  subfilter->addConsumer(sub);
  subfilter->wakeUp();
}

void 
filteredDataServer::cancelSubRequest (fdsSubscriber sub, metricHandle met, focus foc)
{
  // find the pending enable request for this met/focus pair
  filter *subfilter = findFilter(met, foc);
  assert (subfilter);

  if (subfilter->numConsumers == 1) {
    // this was the only pending request; change status
    subfilter->status = filter::Inactive;
  }
  subfilter->rmConsumer(sub);
}
 
void
filteredDataServer::inActivateFilter (filter *fil)
{
  // ask dm to disable metric/focus pair 
  if (phType == GlobalPhase)
    dataMgr->disableDataAndClearPersistentData
      (performanceConsultant::pstream,0, fil->getMI(), phType, true, false);
  else
    dataMgr->disableDataAndClearPersistentData
      (performanceConsultant::pstream,0, fil->getMI(), phType, false, true);

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
filteredDataServer::newData (metricInstanceHandle mih, pdSample deltaVal, 
			     int bucketNumber, phaseType ptype)
{
  filter *curr;
  bool fndflag = DataFilters.find(mih, curr);
  if (fndflag && curr) {
    // convert data to start and end based on bin
    relTimeStamp start = currentBinSize * bucketNumber;
    relTimeStamp end = currentBinSize * (bucketNumber + 1);
    pdRate newValue;

    // the PC needs the actual value of the SampledFunction metrics
    // (eg. number_of_cpus, active_processes, etc.)  the rate is used for the
    // "time", EventCounter metrics (cputime, etc.)  currently, considering
    // holding actual values of SampledFunctions in a pdRate object even
    // though they aren't rates, but actual values.  The PC was previously
    // written so that sample values (being rates or actual values) were just
    // doubles.  It would be nice if this inconsistency was fixed.
    metricHandle* hmet = dataMgr->getMetric( mih );
    if( hmet == NULL )
    {
        // we failed to find the metric
        // this may be because we have just defined a new phase,
        // and the metric instance named by mih is no longer relevant.
        return;
    }
    assert( hmet != NULL );
    T_dyninstRPC::metricInfo* pmi = dataMgr->getMetricInfo( *hmet );
    assert( pmi != NULL );

    // I want to elinate these different metric types.  It's just
    // unnecessary.  I'll have the visis and the PC use the sample values in
    // whatever way they want, whether actual value or the sample rate.  In
    // the PC, we can indicate what format (whether actual or delta value)
    // when we define a PCmetricInst in PCrules.C::initPCmetrics().  Or get
    // this "default" data format from the metric definition in the pcl file.

    // We're keeping a running current actual value of each metric.  In
    // essence, were integrating (summing) the change in sample value as we
    // go along.  The actual value (as opposed to the delta value) is used by
    // metrics such as num_of_cpus, active_processes
    if( pmi->style == SampledFunction ) {
      pdSample actVal = curr->getCurActualValue();
      if(actVal.isNaN())
        newValue.setNaN();
      else {
        newValue = pdRate(static_cast<double>(actVal.getValue()));
        curr->incCurActualValue(deltaVal);
      }
    } else {
      if(deltaVal.isNaN())
        newValue.setNaN();
      else {
        // get the bucket width
        // because we may be making this call in the temporal
        // vacinity of the start of a new phase, where the mih
        // is still valid but the metric has no data, be sure to 
        // deal with the situation
        timeLength* bucketWidth = dataMgr->getBucketWidth( mih, ptype );
        if( bucketWidth != NULL )
        {
            newValue = pdRate(deltaVal, *bucketWidth);
            delete bucketWidth;
        }
        else
        {
            timeLength curBucketWidth = timeLength::Zero();
            dataMgr->getCurrentBucketWidth( &curBucketWidth );            
            newValue = pdRate( deltaVal, curBucketWidth );
        }
      }
    }
    curr->newData(newValue, start, end);

    // cleanup
    delete hmet;
    delete pmi;
  } 

#ifdef PCDEBUG
  else {
    cout << "FDS unexpected data, mi handle = " << mih << endl;
  }
#endif
  //DataFilters.printStats();

}



