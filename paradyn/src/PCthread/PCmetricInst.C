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

// $Id: PCmetricInst.C,v 1.23 2003/05/21 18:21:17 pcroth Exp $
// The PCmetricInst class and the PCmetricInstServer methods.

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


ostream& operator <<(ostream &os, PCmetricInst &pcm)
{
  const char *fname = dataMgr->getFocusNameFromHandle(pcm.foc);
  os << "PCMETRICINST " << pcm.met->getName() << ":" << endl
    << " focus: " << pcm.foc << " " << fname << endl;
  os << " start=" << pcm.startTime << " end=" << pcm.endTime 
    << " DataStatus=" << pcm.DataStatus << " AllDataReady= " << pcm.AllDataReady
      << "EnableStatus=" << pcm.EnableStatus << "TimesAligned: " 
	<< pcm.TimesAligned << endl;
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

ostream& operator <<(ostream &os, PCInterval &i)
{
  os << "Value: " << i.value << "\tStart: " << i.start << "\tEnd: " 
    << i.end << endl;
  return os;
}

PCmetricInst::PCmetricInst (PCmetric *pcMet, focus f, 
			    filteredDataServer *db, bool *err):
  foc(f), met(pcMet), currentValue(pdRate::Zero()), costEstimate(0.0), 
  numCostEstimates(0), startTime(-1,timeUnit::sec()),
  endTime(relTimeStamp::Zero()), totalTime(timeLength::Zero()), 
  AllDataReady(0), EnableStatus(0), DataStatus(0), AllCurrentValues(NULL), 
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
  newRec.active = false;
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

float
PCmetricInst::getEstimatedCost(dataSubscriber *sub)
{
  if (numCostEstimates == numInPorts) {
    // cost has already been calculated for this pcmi
    return costEstimate;
  } else {
    costWaitList += sub;
    unsigned sz = AllData.size();
    for (unsigned i = 0; i < sz; i++) {
      costServer::getPredictedCost(AllData[i].met, AllData[i].foc, this);
    }
    return -1;
  }
}

void
PCmetricInst::sendInitialCostEstimate (float costEstimate)
{
  unsigned sz = costWaitList.size();
  for (unsigned i = 0; i < sz; i++)
    costWaitList[i]-> updateEstimatedCost (costEstimate);
  costWaitList.resize(0);
}

void 
PCmetricInst::updateEstimatedCost(float costDiff)
{
#ifdef PCDEBUG
  cout << "$$ EstimatedCostData received: " << costDiff << " for met:"
    << met->getName() << foc << " foc: " << endl;
  cout << "                      new tot: " << costEstimate+costDiff << endl;
#endif
  costEstimate += costDiff;
  numCostEstimates++;
  if (numCostEstimates == numInPorts)
    sendInitialCostEstimate (costEstimate);
  else if (numCostEstimates > numInPorts)
    sendUpdatedEstimatedCost(costEstimate);
}

void 
PCmetricInst::enableReply (unsigned token1, unsigned token2, unsigned token3,
				bool successful, string msg)
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
	AllData[i].active = true;
	break;
      }
    }
    if (EnableStatus == AllDataReady) {
      sendEnableReply (0, 0, 0, true);
      active = true;
    }
  } else {
    // ** need to cancel other subscriptions!
    sendEnableReply (0, 0, 0, false, msg);
  }
}

void
PCmetricInst::activate()
{
  if (active) return;

  inPort *curr;
  AllDataReady = 0;
  // important!!! AllDataReady must be complete before we enter the 
  // data subscription loop.  Why?  it may be tested as part of call to 
  // db->addSubscription.  AllCurrentValues must be complete before 
  // we enter the data subscription loop.  Why?  Data may start arriving
  // as soon as the subscription is requested.
  for (int j = 0; j < numInPorts; j++) {
    AllDataReady = (AllDataReady << 1) | 1;
  }
  if (AllCurrentValues != NULL) 
    delete AllCurrentValues;
  AllCurrentValues = new pdRate[numInPorts];
  for (int k = 0; k < numInPorts; k++) {
    AllCurrentValues[k] = pdRate::Zero();
  }
  for (int i = 0; i < numInPorts; i++) {
    curr = &(AllData[i]);
    db->addSubscription (this, curr->met, curr->foc, curr->ft, 
				    costFlag); 
    // ** this must change when metrics using eg "all children" 
    // ** are implemented, to simply eliminate the single unsubscribable
    // ** met-focus pair
    //if (newnumPorts == 0) {
    //sendEnableReply (0, 0, 0, false);
    //return; }
  }
}

void
PCmetricInst::deactivate()
{
  for (int j = 0; j < numInPorts; j++) {
    if (AllData[j].active) {
      db->endSubscription (this, AllData[j].mih);
      AllData[j].active = false;
    } else {
      // data requested but confirmation not yet received
      db->cancelSubRequest (this, AllData[j].met, AllData[j].foc);
    }
  }
  active = false;
  EnableStatus = 0;
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
PCmetricInst::newData (metricInstanceHandle whichData, pdRate newVal, 
		       relTimeStamp start, relTimeStamp end)
{
  // don't use data until all ports successfully enabled
  if (EnableStatus != AllDataReady) return;

  // adjust metric start time if this is first data received
  if (startTime < relTimeStamp::Zero())
    startTime = start;

  bool found = false;
  bool queueGrew;
  PCInterval newInterval;
  newInterval.value = newVal;
  newInterval.start = start;
  newInterval.end = end;

  unsigned portNum = 0;
  // find queue for this metricInstanceHandle
  for (unsigned k = 0; k < AllData.size(); k++)
    if (AllData[k].mih == whichData) {
      queueGrew = (AllData[k].indataQ).add(&newInterval);
      portNum = AllData[k].portID;
      found = true;
      break;
    }
  assert (found);

  // update data ready
  this->setDataReady (portNum);

#ifdef PCDEBUG
    // debug printing
    if (performanceConsultant::printDataTrace 
	|| performanceConsultant::printDataCollection) {
      cout << *this << endl;
    }
#endif

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
    PCInterval thisInt;
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
    pdRate newguy;
    int numPs = numInPorts;
    pdRate *allvalues = AllCurrentValues;
    if (met->InstWithPause) {
      allvalues++;
      numPs--;
    }
    if (met->calc != NULL)
      newguy = met->calc(foc, allvalues, numPs);
    else
      // ** (if calc == NULL, just return single value)
      newguy = AllCurrentValues[numInPorts-1];    

    // notify all consumers of this PCmi of the new value
#ifdef MYPCDEBUG
    timeStamp t1,t2;
    t1 = getCurrentTime();
#endif
    sendValue (0, newguy, start, end);
#ifdef MYPCDEBUG
    t2 = getCurrentTime();
    if ((t2 - t1) > timeLength::sec())
      cerr << "-------------> sendValue in PCmetricInst took time: " << t2-t1 
	   << "\n";
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
  relTimeStamp intervalEnd;
  const PCInterval *thisInt;
  PCInterval toss;

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

void 
PCmetricInst::endSubscription(dataSubscriber *sub)
{
  int numLeft = rmConsumer (sub);
  if (numLeft == 0) {
    // leave the PCmetricInst but stop the flow of data to it
    //** flush queues??
    deactivate();
  }
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
PCmetricInstServer::createPcmi(PCmetric *pcm,
			       focus f,
			       bool *errFlag)
{
  PCmetricInst *newpcmi = NULL;
  // PCmetric instance may already exist

  unsigned sz = AllData.size();
  for (unsigned i = 0; i < sz; i++) {
    if (((AllData[i]).f == f) && ((AllData[i]).pcm == pcm)) {
      newpcmi = (AllData[i]).pcmi;
      break;
    }
  }
  if (newpcmi == NULL) {
    newpcmi = new PCmetricInst(pcm, f, datasource, errFlag);
    PCMRec tmpRec (pcm, f, newpcmi);
    AllData += tmpRec;
  }
  return (PCmetInstHandle) newpcmi;
}

