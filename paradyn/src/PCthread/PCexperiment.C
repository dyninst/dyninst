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
 * experiment.C
 * 
 * The experiment class methods.
 * 
 * $Log: PCexperiment.C,v $
 * Revision 1.1  1996/02/02 02:06:32  karavan
 * A baby Performance Consultant is born!
 *
 */

#include "PCintern.h"
#include "PCexperiment.h"
#include "PCsearch.h"

//**
#define PCminTimeToFalse 40
#define PCminTimeToTrue 20


ostream& operator <<(ostream &os, experiment& ex)
{
  os << "experiment " << ex.pcmih << ":" << "\n" 
    << "/tcost:/t" << ex.estimatedCost << "\n"
      << endl;
    
  return os;
}

void 
experiment::drawConclusion (testResult newConclusion)
{
  currentConclusion = newConclusion;
  papaNode->changeTruth(currentConclusion);
#ifdef PCDEBUG
  // debug printing
  tunableBooleanConstant prtc = 
    tunableConstantRegistry::findBoolTunableConstant("PCprintSearchChanges");
  if (prtc.getValue()) {
    const vector<resourceHandle> *longname = dataMgr->getResourceHandles(where);
    cout << "CONCLUDE for "<< why->getName() << endl
      << "          focus = <" << dataMgr->getFocusName(longname) 
	<< ">" << endl
	<< "          time = " << endTime << endl
	  << "         concl= " << currentConclusion << endl;
  }
#endif
}

void 
experiment::newData(PCmetDataID, float val, double start, double end, 
		    float pauseTime)
{
  sampleValue thresh = why->getThreshold (why->thresholdNm.string_of(), where);

  // adjust for pause time: 
  // when we compare the pcmetric to the threshold, we need to normalize 
  // e.g., comparing io time to threshold we want io time only for the 
  // time the application was active.  the actual dm metric will have 0's
  // for all pause time (well for everything except of course the pause_time
  // metric itself!!) so the average value for the metric will not be the 
  // rate we need.  We normalize by comparing the pcmetric to the threshold
  // multiplied by percent of time active.
  float timeNormalizer = 1 - pauseTime;
  // I'm not sure why pauseTime would ever be negative or greater than 1,
  // but I picked up this correction from the previous pc code.
  if (timeNormalizer < 0.0)
    timeNormalizer = 0.00001;
  if (timeNormalizer > 1.0)
    timeNormalizer = 0.99999;

  // update currentValue
  currentValue = val - (thresh * timeNormalizer * hysConstant);
  endTime = end;
  if (startTime < 0)    // this is first value
    startTime = start;

  // evaluate result
  bool newGuess;
  if (why->compOp == gt)
    newGuess = currentValue > 0;
  else
    newGuess = currentValue < 0;

#ifdef PCDEBUG
  // debug printing
  tunableBooleanConstant prtc = 
    tunableConstantRegistry::findBoolTunableConstant("PCprintTestResults");
  if (prtc.getValue()) {
  const vector<resourceHandle> *longname = dataMgr->getResourceHandles(where);
  cout << "TESTEVAL for "<< why->getName() << endl 
    << "            focus = <" << dataMgr->getFocusName(longname) << ">" 
      << endl
      << "            time = " << endTime << endl;
    if (why->compOp == gt) {
      cout << "             " << val << " >? " 
	<< (thresh*timeNormalizer * hysConstant)
	<< " evals to " << newGuess << endl; 
    } else {
      cout << val << " <? " << (thresh*timeNormalizer * hysConstant)
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
      tunableFloatConstant hysRange = 
	tunableConstantRegistry::findFloatTunableConstant("hysteresisRange");
      hysConstant = 1 - hysRange.getValue();
      // interval for this guess equals interval for this one test
      timeTrueFalse = end - start;
    } else { 
      // this test result same as previous, so add interval to previous
      // total.  
      timeTrueFalse += end - start;
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
      tunableFloatConstant hysRange = 
	tunableConstantRegistry::findFloatTunableConstant("hysteresisRange");
      hysConstant = 1 + hysRange.getValue();
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
}

//
// constructor sets err to true if unable to construct this experiment,
// which only happens if subscription to pcmetric fails. 
//
experiment::experiment(hypothesis *whyowhy, focus whereowhere, 
		       bool persist, searchHistoryNode *papa,
		       PCsearch *srch, bool *err):
why(whyowhy), where(whereowhere), persistent(persist), mamaSearch(srch),
papaNode(papa),  estimatedCost(0.0), status(false), 
currentConclusion(tunknown), currentGuess(tunknown), timeTrueFalse(0), 
currentValue(0.0), startTime(-1), endTime(0), minObservationFlag(false)

{
  PCmetricInstServer *db = mamaSearch->getDatabase();
  assert (db);
  bool errf = false;
  // **here, need to check if pause_time if so, set flag to true instead
  pcmih = db->addSubscription ((PCmetSubscriber)this, why->getPcMet(), 
			       where, &errf);
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
  tunableFloatConstant hysRange = 
    tunableConstantRegistry::findFloatTunableConstant("hysteresisRange");
  hysConstant = 1 + hysRange.getValue();
  
  // get an initial estimate of the cost
  estimatedCost = pcmih->getEstimatedCost();
#ifdef PCDEBUG
  // debug print 
  tunableBooleanConstant prtc = 
    tunableConstantRegistry::findBoolTunableConstant("PCprintTestResults");
  if (prtc.getValue()) {
    cout << "EXP added:" << endl;
    cout << *this;
  }
#endif
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
  assert(pcmih);
  if (status)
    return true;
  if (pcmih->activate()) {
    status = true;
#ifdef PCDEBUG
    // debug printing
    tunableBooleanConstant prtc = 
      tunableConstantRegistry::findBoolTunableConstant("PCprintDataTrace");
    if (prtc.getValue()) {
      cout << "EXP started:" << endl;
      cout << *pcmih << endl;
    }
#endif
    return true;
  } else {
    return false;
  }
}

void
experiment::halt ()
{
  // to stop experiment, just turn off flow of data at the source
  (mamaSearch->getDatabase())->endSubscription ((PCmetSubscriber)this, pcmih);
  status = false;
}




