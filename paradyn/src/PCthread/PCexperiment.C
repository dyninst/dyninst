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

/*
 * The experiment class methods.
 * $Id: PCexperiment.C,v 1.23 2003/05/23 07:27:43 pcroth Exp $
 */

#include "PCintern.h"
#include "PCexperiment.h"
#include "PCsearch.h"
//**
timeLength experiment::PCminTimeToFalse = timeLength(20, timeUnit::sec());
timeLength experiment::PCminTimeToTrue = timeLength(20, timeUnit::sec());


ostream& operator <<(ostream &os, experiment& ex)
{
  os << ex.getStartTime() << " " << ex.getEndTime() << 
    " " << ex.getCurrentValue() << " " <<  ex.getLastThreshold() <<
     " # experiment start end value threshold" << endl;
    
  return os;
}

void 
experiment::drawConclusion (testResult newConclusion)
{
  currentConclusion = newConclusion;
  papaNode->changeTruth(currentConclusion);
#ifdef PCDEBUG
  // debug printing
  if (performanceConsultant::printSearchChanges) {
    cout << "CONCLUDE for "<< why->getName() << endl
      << "          focus = <" << dataMgr->getFocusNameFromHandle(where) 
	<< ">" << endl
	  << "          time = " << endTime << endl
	    << "         concl= " << currentConclusion << endl;
  }
#endif
}

void 
experiment::updateEstimatedCost(float costDiff) 
{
  estimatedCost += costDiff;
  papaNode->estimatedCostNotification();
}

void 
experiment::newData(PCmetDataID, pdRate val, relTimeStamp start, 
		    relTimeStamp end)
{
  pdRate thresh;
  if (performanceConsultant::useIndividualThresholds)
    thresh = why->getThreshold (why->indivThresholdNm.c_str(), where);
  else
    thresh = why->getThreshold (why->groupThresholdNm.c_str(), where);
   

  // update currentValue and adjustedValue
  currentValue = val;
  adjustedValue = val - (thresh * hysConstant);

  endTime = end;
  if (startTime < relTimeStamp::Zero())    // this is first value
    startTime = start;


  // If the pc is currently paused, don't evaluate the data recieved
  if (mamaSearch->paused()) {
    return;
  }


  // evaluate result
  bool newGuess;
  if (why->compOp == gt)
    newGuess = (adjustedValue > pdRate::Zero());
  else
    newGuess = (adjustedValue < pdRate::Zero());

#ifdef PCDEBUG
  // debug printing
  if (performanceConsultant::printTestResults) {
  cout << "TESTEVAL for "<< why->getName() << endl 
    << "            focus = <" 
      << dataMgr->getFocusNameFromHandle(where) << ">" 
      << endl
      << "            time = " << endTime << "  pauseNorm = " 
	<< timeNormalizer << "  thresh = " << thresh << "  hys = " 
	  << hysConstant << endl;
    if (why->compOp == gt) {
      cout << "             " << val << " >? " 
	<< (thresh*timeNormalizer * hysConstant)
	<< " evals to " << newGuess << endl; 
    } else {
      cout << "             " << val << " <? " 
        << (thresh*timeNormalizer * hysConstant)
	<< " evals to " << newGuess << endl; 
    }
  } // end debug print
#endif

  // compare newGuess 
  if (newGuess == true) {
    if ((currentGuess == tfalse) || (currentGuess == tunknown)) {
      // this test differs from last result
      currentGuess = ttrue;
      // change hysteresis parameter to reflect true
      hysConstant = 1 - performanceConsultant::hysteresisRange;
      // interval for this guess equals interval for this one test
      timeTrueFalse = end - start;
    } else { 
      // this test result same as previous, so add interval to previous
      // total.  
      timeTrueFalse += end - start;
      if ((thresh < lastThreshold) && (currentConclusion == currentGuess)) {
	// threshold has been changed so reevaluate existing false children
	papaNode->retestAllChildren();
      }
    }
    if ((thresh != lastThreshold) && (currentConclusion == currentGuess)) {
      // threshold has been changed so reevaluate existing false children
      papaNode->retestAllChildren();
    }
    if ((timeTrueFalse >= PCminTimeToTrue) 
	&& (currentConclusion != currentGuess)) {
      drawConclusion (ttrue);
    }
  } else {
    // here because newConclusion is false
    if ((currentGuess == ttrue) || (currentGuess == tunknown)) {
      // this test differs from most recent test
      currentGuess = tfalse;
      // change hysteresis parameter to reflect false
      hysConstant = 1 + performanceConsultant::hysteresisRange;
      // time for this guess equals interval for this test
      timeTrueFalse = end - start;
    } else {
      // new test result same as previous
      timeTrueFalse += end - start;
    }
    if ((timeTrueFalse >= PCminTimeToFalse) 
	&& (currentConclusion != currentGuess)) {
      drawConclusion(tfalse);
    }
  }
  lastThreshold = thresh;
}

//
// constructor sets err to true if unable to construct this experiment,
// which only happens if subscription to pcmetric fails. 
//
experiment::experiment(hypothesis *whyowhy, focus whereowhere, 
		       bool persist, searchHistoryNode *papa,
		       PCsearch *srch, bool amFlag, bool *err):
why(whyowhy), where(whereowhere), persistent(persist), mamaSearch(srch),
papaNode(papa),  estimatedCost(0.0), status(false), 
currentConclusion(tunknown), currentGuess(tunknown), 
timeTrueFalse(timeLength::Zero()), currentValue(pdRate::Zero()), 
adjustedValue(pdRate::Zero()), startTime(-1, timeUnit::sec()), 
endTime(relTimeStamp::Zero()), minObservationFlag(false), 
lastThreshold(pdRate::Zero())
{
  PCmetricInstServer *db = mamaSearch->getDatabase();
  assert (db);
  bool errf = false;
  // **here, need to check if pause_time if so, set flag to true instead
  PCmetric *mymet = why->getPcMet(amFlag);
  assert (mymet);
  pcmih = db->createPcmi (mymet, where, &errf);
  // error here if pcmetric couldn't be enabled 
  *err = ((pcmih == NULL) || errf);
  if (*err) return;
  // hysteresis parameter:
  // we want to avoid bad behavior for results which hover right around 
  // the true/false threshold, so we multiply by a small hysteresis 
  // parameter, just under 1 if this test was previously true, just over 
  // 1 if new or previously false, and close values won't oscillate 
  // between true/false.  The hysteresis parameter itself is a tunable 
  // constant.
  hysConstant = 1 + performanceConsultant::hysteresisRange;
  
#ifdef PCDEBUG
  // debug print 
  if (performanceConsultant::printTestResults) {
    cout << "EXP added:" << endl;
    cout << *this;
  }
#endif
}

void
experiment::findOutCost()
{
  float currEstimate = pcmih->getEstimatedCost(this);
  if (currEstimate >= 0) 
    updateEstimatedCost(currEstimate);
}

experiment::~experiment()
{
  ;
}  

// here's where we actually start/halt the flow of data to this experiment.
// note that a single experiment may be started and halted many times 
// over its lifetime

bool
experiment::start()
{
  if (status) return true;
  assert(pcmih);
  pcmih->addSubscription((PCmetSubscriber)this);
  papaNode->addActiveSearch();   // update active search count
  status = true;
  currentConclusion = tunknown;
  minObservationFlag = false;
  timeTrueFalse = timeLength::Zero();
  //** need to distinguish here if PCmetricInst already running!!
  return false;
}

void
experiment::halt ()
{
  if (status) {
    mamaSearch->decrNumActiveExperiments();
    // to stop experiment, just turn off flow of data at the source
    pcmih->endSubscription ((PCmetSubscriber)this);
    status = false;
  }
}

void 
experiment::enableReply (unsigned, unsigned, unsigned,
                            bool successful, bool deferred, string msg)
{
#ifdef PCDEBUG
    // debug printing
  if (successful && performanceConsultant::printDataTrace) {
    cout << "EXP started:" << endl;
    cout << *pcmih << endl;
  }
#endif
  papaNode->enableReply(successful, deferred, msg);
}

