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
 * PCmetricInst.C
 * 
 * The PCmetricInst class and the PCmetricInstServer methods.
 * 
 * $Log: PCmetricInst.C,v $
 * Revision 1.12  1996/05/15 04:35:13  karavan
 * bug fixes: changed pendingCost pendingSearches and numexperiments to
 * break down by phase type, so starting a new current phase updates these
 * totals correctly; fixed error in estimated cost propagation.
 *
 * Revision 1.11  1996/05/08 07:35:17  karavan
 * Changed enable data calls to be fully asynchronous within the performance consultant.
 *
 * some changes to cost handling, with additional limit on number of outstanding enable requests.
 *
 * Revision 1.10  1996/05/06 04:35:16  karavan
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
 * Revision 1.9  1996/05/02 19:46:41  karavan
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
 * Revision 1.8  1996/05/01 14:06:59  naim
 * Multiples changes in PC to make call to requestNodeInfoCallback async.
 * (UI<->PC). I also added some debugging information - naim
 *
 * Revision 1.7  1996/04/30  06:27:00  karavan
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
 * Revision 1.6  1996/04/13 04:43:03  karavan
 * bug fix to AlignTimes
 *
 * Revision 1.5  1996/04/07 21:29:37  karavan
 * split up search ready queue into two, one global one current, and moved to
 * round robin queue removal.
 *
 * eliminated startSearch(), combined functionality into activateSearch().  All
 * search requests are for a specific phase id.
 *
 * changed dataMgr->enableDataCollection2 to take phaseID argument, with needed
 * changes internal to PC to track phaseID, to avoid enable requests being handled
 * for incorrect current phase.
 *
 * added update of display when phase ends, so all nodes changed to inactive display
 * style.
 *
 * Revision 1.4  1996/03/18 07:13:04  karavan
 * Switched over to cost model for controlling extent of search.
 *
 * Added new TC PCcollectInstrTimings.
 *
 * Revision 1.3  1996/02/22 18:32:00  karavan
 * Changed data storage from queue to circular buffer
 *
 * Revision 1.2  1996/02/08 19:52:44  karavan
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
 * Revision 1.1  1996/02/02 02:06:42  karavan
 * A baby Performance Consultant is born!
 *
 */

/*
 ****** important********
 here we assume that data continues flowing for all enabled metric/focus
 pairs unless (a) some drastic error renders a met/focus pair uncollectible or
 (b) a process exits.  In other words, if data is not received for a given 
 metric instance for some relatively short period of time, we will mark that 
 PC metric as uncollectible and stop testing the subscribing experiment(s)
*/

#include "PCintern.h"
#include "../DMthread/DMinclude.h"
#include "PCmetric.h"
#include "PCexperiment.h"
#include "PCmetricInst.h"
#include "PCcostServer.h"

typedef experiment* PCmiSubscriber;

#ifdef MYPCDEBUG
extern double TESTgetTime();
#endif

ostream& operator <<(ostream &os, PCmetricInst &pcm)
{
  const char *fname = dataMgr->getFocusNameFromHandle(pcm.foc);
  os << "PCMETRICINST " << pcm.met->getName() << ":" << endl
    << " focus: " << pcm.foc << " " << fname << endl;
  os << " start: " << pcm.startTime << " end: " << pcm.endTime 
    << " DataStatus: " << pcm.DataStatus << " AllDataReady: " << pcm.AllDataReady
      << "TimesAligned: " << pcm.TimesAligned << endl;
  inPort *currPort;
  for (int i = 0; i < pcm.numInPorts; i++) {
    currPort = &(pcm.AllData[i]);
    os << " Port# " << currPort->portID << " mih: " << currPort->mih 
      << " mh: " << currPort->met << " foc: " << currPort->foc 
	<< " size: " << (currPort->indataQ).getSize() << endl;
    (currPort->indataQ).print();
  }
  return os;
}

ostream& operator <<(ostream &os, Interval &i)
{
  os << "Value: " << i.value << "\tStart: " << i.start << "\tEnd: " 
    << i.end << endl;
  return os;
}

PCmetricInst::PCmetricInst (PCmetric *pcMet, focus f, 
			    filteredDataServer *db, bool *err):
foc(f), met(pcMet), currentValue(0.0), costEstimate(0.0), 
numCostEstimates(0), startTime(-1), endTime(0.0),
totalTime (0.0), AllDataReady(0), EnableStatus(0), DataStatus(0), AllCurrentValues(NULL), 
TimesAligned(0), active(false), costFlag(*err), db(db)
{
  assert (pcMet);

  // how many met-foc pairs for this PCmetric?
  numInPorts = pcMet->DMmetrics->size();
  if (pcMet->InstWithPause) {
    // default for PCmetrics is to subscribe to pause time as well
    numInPorts++;
  }
  int pauseOffset = 0;
  inPort newRec;
  newRec.mih = 0;
  newRec.ft = averaging;
  if (pcMet->InstWithPause) {
    // create pause_time queue as 0;
    newRec.met = performanceConsultant::normalMetric;
    newRec.foc = topLevelFocus;
    newRec.portID = 0;
    AllData += newRec;
    pauseOffset = 1;
  } 
  // create remaining queues
  PCMetInfo *currInfo;
  for (int j = 0; j < numInPorts-pauseOffset; j++) {
    currInfo = (*(pcMet->DMmetrics))[j]; 
    if (currInfo->fType == tlf)
      newRec.foc = topLevelFocus;
    else
      newRec.foc = f;
    newRec.met = currInfo->mh;
    newRec.ft = currInfo->ft;
    newRec.portID = j+pauseOffset;
    AllData += newRec;
  }
//** future feature
//  if (pcMet->setup != NULL)
//    pcMet->setup(foc);
}

void
PCmetricInst::getEstimatedCost()
{
  unsigned sz = AllData.size();
  for (unsigned i = 0; i < sz; i++) {
    costServer::getPredictedCost(AllData[i].met, AllData[i].foc, this);
  }
}
void 
PCmetricInst::enableReply (unsigned token1, unsigned token2, unsigned token3,
				bool successful)
{
  if (EnableStatus == AllDataReady)
    // this is a duplicate; drop it on the floor
    return;

  if (successful) {
    for (unsigned i = 0; i < AllData.size(); i++) {
      if ((AllData[i].met == (metricHandle)token1) && 
	  (AllData[i].foc == (focus)token2)) {
	setEnableReady(i);
	AllData[i].mih = (metricInstanceHandle)token3;
	break;
      }
    }
    if (EnableStatus == AllDataReady) {
      sendEnableReply (0, 0, 0, true);
      active = true;
    }
  } else {
    // ** need to cancel other subscriptions!
    sendEnableReply (0, 0, 0, false);
  }
}

void
PCmetricInst::activate()
{
  if (active) return;

  inPort *curr;
  unsigned nextPortId = 0;
  AllDataReady = 0;
  // important!!! AllDataReady must be complete before we enter the 
  // second for loop.  Why?  it may be tested as part of call to 
  // db->addSubscription.
  for (int j = 0; j < numInPorts; j++) {
    AllDataReady = (AllDataReady << 1) | 1;
  }
  for (int i = 0; i < numInPorts; i++) {
    curr = &(AllData[i]);
    curr->portID = nextPortId++;
    db->addSubscription (this, curr->met, curr->foc, curr->ft, 
				    costFlag); 
    // ** this must change when metrics using eg "all children" 
    // ** are implemented, to simply eliminate the single unsubscribable
    // ** met-focus pair
    //if (newnumPorts == 0) {
    //sendEnableReply (0, 0, 0, false);
    //return; }
  }
  if (AllCurrentValues != NULL) 
    delete AllCurrentValues;
  AllCurrentValues = new sampleValue[numInPorts];
  for (int k = 0; k < numInPorts; k++) {
    AllCurrentValues[k] = 0.0;
  }
}

void
PCmetricInst::deactivate()
{
  for (int j = 0; j < numInPorts; j++) {
    db->endSubscription (this, AllData[j].mih);
  }
  active = false;
}

PCmetricInst::~PCmetricInst()
{
  delete AllCurrentValues;
}  

void
PCmetricInst::setDataReady (int portnum)
{
  unsigned mask = 1 << portnum;
  DataStatus = DataStatus | mask;
}
void 
PCmetricInst::clearDataReady (int portnum)
{
  unsigned mask = 1 << portnum;
  DataStatus = DataStatus ^ mask;
}

void
PCmetricInst::setEnableReady (int portnum)
{
  unsigned mask = 1 << portnum;
  EnableStatus = EnableStatus | mask;
}
void 
PCmetricInst::clearEnableReady (int portnum)
{
  unsigned mask = 1 << portnum;
  EnableStatus = EnableStatus ^ mask;
}

void
PCmetricInst::newData (metricInstanceHandle whichData, sampleValue newVal, 
		       timeStamp start, timeStamp end, sampleValue)
{
  unsigned portNum;
  // adjust metric start time if this is first data received
  if (startTime < 0)
    startTime = start;

  bool found = false;
  bool queueGrew;
  Interval newInterval;
  newInterval.value = newVal;
  newInterval.start = start;
  newInterval.end = end;

  // find queue for this metricInstanceHandle
  for (unsigned k = 0; k < AllData.size(); k++)
    if (AllData[k].mih == whichData) {
      queueGrew = (AllData[k].indataQ).add(&newInterval);
      portNum = AllData[k].portID;
      found = true;
      break;
    }
  if (!found) {
    bool notfound = true;
  }
  assert (found);

  // update data ready
  this->setDataReady (portNum);

#ifdef PCDEBUG
    // debug printing
    if (performanceConsultant::printDataTrace) {
      cout << *this << endl;
    }
#endif

    //**
    if (performanceConsultant::printDataCollection) {
      cout << *this << endl;
    }

  // check all data ready, if so, compute new value
  if (DataStatus == AllDataReady) {

    // this is where to check for time alignment. 
    if (!TimesAligned) {
      TimesAligned = this->alignTimes();
      if (! TimesAligned)
	return;
    }

    // if we reach this point, then we have new time-aligned piece of 
    // data for everything
    inPort *curr;
    Interval thisInt;
    for (int m = 0; m < numInPorts; m++) {
      curr = &(AllData[m]);
      thisInt = (curr->indataQ).remove ();
      // reset DataStatus
      if ((curr->indataQ).isEmpty()) {
	clearDataReady(curr->portID);
      }	     
      AllCurrentValues[m] = thisInt.value;
    }
    // reset TimesAligned
    TimesAligned = 0;
    endTime = end;
    sampleValue newguy;
    if (met->calc != NULL)
      newguy = met->calc(foc, AllCurrentValues, numInPorts);
    else
      // (if calc == NULL, just return single value)
      newguy = AllCurrentValues[numInPorts-1];    

    sampleValue pauseNorm = 0.0;
    if (met->InstWithPause)
      pauseNorm = AllCurrentValues[0];

    // notify all consumers of this PCmi of the new value
#ifdef MYPCDEBUG
    double t1,t2;
    t1=TESTgetTime();
#endif
    sendValue (0, newguy, start, end, pauseNorm);
#ifdef MYPCDEBUG
    t2=TESTgetTime();
    if ((t2-t1) > 1.0)
      printf("-------------> sendValue in PCmetricInst took %5.2f seconds\n",t2-t1);
#endif
  }
}

// returns true if all in ports have at head of queue data for the 
// same interval end; false means still waiting for some data.
// we don't care about interval start; for each DMmetric we are using 
// the maximum possible data for the most accurate estimate.
bool
PCmetricInst::alignTimes()
{
  bool allLinedUp = true;
  bool needSecondPass = false;
  timeStamp intervalEnd;
  const Interval *thisInt;
  Interval toss;

  thisInt = (AllData[0].indataQ).peek ();
  assert (thisInt);
  intervalEnd = thisInt->end;

  for (int i = 1; i < numInPorts; i++) {
      thisInt = (AllData[i].indataQ).peek ();
      if (thisInt->end > intervalEnd) {
	// we're changing intervalEnd to later time and we've already
	// processed some queues, so we will need a second pass to 
	// reprocess those
	intervalEnd = thisInt->end;
	needSecondPass = true;
      } else {
	while (!(AllData[i].indataQ).isEmpty() 
	       && (thisInt->end < intervalEnd)) {
	  // toss old data like smelly garbage
	  toss = (AllData[i].indataQ).remove ();
	  thisInt = (AllData[i].indataQ).peek ();
	}
	if ((AllData[i].indataQ).isEmpty())  {
	  // we ran out of data before we got this one aligned; align 
	  // whatever else possible but return false
	  allLinedUp = false;
	  this->clearDataReady(i);
	}
      }
    }
  if (needSecondPass && allLinedUp) 
    for (int j = 0; j < numInPorts; j++) {
      thisInt = (AllData[j].indataQ).peek ();
      while ((!(AllData[j].indataQ).isEmpty()) &&
	     (thisInt->end < intervalEnd)) {
	// toss old data like smelly garbage
	toss = (AllData[j].indataQ).remove ();
      	thisInt = (AllData[j].indataQ).peek ();
      }
      if ((AllData[j].indataQ).isEmpty()) {
	// we ran out of data before we got this one aligned; align 
	// whatever else possible but return false
	allLinedUp = false;
	this->clearDataReady(j);
      }
    }
#ifdef PCDEBUG
  // debug printing
  if (performanceConsultant::printDataCollection) {
    cout << "alignTimes returns " << allLinedUp << " interval ends at " 
      << intervalEnd << " DataStatus=" << DataStatus << endl;
  }
#endif
  return allLinedUp;
}

PCmetricInstServer::PCmetricInstServer (unsigned phaseID) 
{  
    datasource = new filteredDataServer(phaseID);
    if (phaseID == GlobalPhaseID)
        performanceConsultant::globalPCMetricServer = this;
 }

PCmetricInstServer::~PCmetricInstServer ()
{
  delete datasource;
}

PCmetInstHandle
PCmetricInstServer::addSubscription(dataSubscriber *sub,
				    PCmetric *pcm,
				    focus f,
				    bool *errFlag)
{
  PCmetricInst *newsub = NULL;
  // PCmetric instance may already exist

  unsigned sz = AllData.size();
  for (unsigned i = 0; i < sz; i++) {
    if (((AllData[i]).f == f) && ((AllData[i]).pcm == pcm)) {
      newsub = (AllData[i]).pcmi;
      break;
    }
  }
  if (newsub == NULL) {
    newsub = new PCmetricInst(pcm, f, datasource, errFlag);
    PCMRec tmpRec (pcm, f, newsub);
    AllData += tmpRec;
  }
  newsub->addConsumer(sub);
  return (PCmetInstHandle) newsub;
}

void 
PCmetricInstServer::endSubscription(dataSubscriber *sub, 
				    PCmetInstHandle id)
{
  int numLeft = id->rmConsumer (sub);
  if (numLeft == 0) {
    // leave the PCmetricInst but stop the flow of data to it
    //** flush queues??
    id->deactivate();
  }
}
