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
 * experiment.C
 * 
 * The experiment class methods.
 * 
 * $Log: PCexperiment.C,v $
 * Revision 1.15  1996/08/16 21:03:22  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.14  1996/07/24 20:10:39  karavan
 * Fixed error in numActiveExperiments calculation; numActiveCurrentExperiments
 * now zero'd at phase boundary.
 *
 * Revision 1.13  1996/07/23 20:27:59  karavan
 * second part of two-part commit.
 *
 * implements new search strategy which retests false nodes under certain
 * circumstances.
 *
 * change in handling of high-cost nodes blocking the ready queue.
 *
 * code cleanup.
 *
 * Revision 1.12  1996/07/22 18:55:38  karavan
 * part one of two-part commit for new PC functionality of restarting searches.
 *
 * Revision 1.11  1996/05/15 04:35:07  karavan
 * bug fixes: changed pendingCost pendingSearches and numexperiments to
 * break down by phase type, so starting a new current phase updates these
 * totals correctly; fixed error in estimated cost propagation.
 *
 * Revision 1.10  1996/05/11 01:57:58  karavan
 * fixed bug in PendingCost calculation.
 *
 * Revision 1.9  1996/05/08 07:35:07  karavan
 * Changed enable data calls to be fully asynchronous within the performance consultant.
 *
 * some changes to cost handling, with additional limit on number of outstanding enable requests.
 *
 * Revision 1.8  1996/05/06 04:35:04  karavan
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
 * Revision 1.7  1996/05/02 19:46:29  karavan
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
 * Revision 1.6  1996/04/30 06:26:46  karavan
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
 * Revision 1.5  1996/04/07 21:29:31  karavan
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
 * Revision 1.4  1996/02/22 20:02:20  karavan
 * fixed bug introduced by bug fix.
 *
 * Revision 1.3  1996/02/22 18:29:24  karavan
 * changed min time to conclusion to 10 (temporary)
 *
 * changed debug print calls from dataMgr->getFocusName to
 * dataMgr->getFocusNameFromHandle
 *
 * Revision 1.2  1996/02/08 19:52:39  karavan
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
 * Revision 1.1  1996/02/02 02:06:32  karavan
 * A baby Performance Consultant is born!
 *
 */

#include "PCintern.h"
#include "PCexperiment.h"
#include "PCsearch.h"
//**
#define PCminTimeToFalse 20
#define PCminTimeToTrue 20


ostream& operator <<(ostream &os, experiment& ex)
{
  os << "experiment: " << ex.pcmih <<  endl;
    
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
experiment::newData(PCmetDataID, float val, double start, double end, 
		    float pauseTime)
{
  sampleValue thresh;
  if (performanceConsultant::useIndividualThresholds)
    thresh = why->getThreshold (why->indivThresholdNm.string_of(), where);
  else
    thresh = why->getThreshold (why->groupThresholdNm.string_of(), where);
    
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
currentConclusion(tunknown), currentGuess(tunknown), timeTrueFalse(0), 
currentValue(0.0), startTime(-1), endTime(0), minObservationFlag(false),
lastThreshold(0.0)

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
  sampleValue currEstimate = pcmih->getEstimatedCost(this);
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
  timeTrueFalse = 0;
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
experiment::enableReply (unsigned, unsigned, unsigned, bool successful)
{
#ifdef PCDEBUG
    // debug printing
  if (successful && performanceConsultant::printDataTrace) {
    cout << "EXP started:" << endl;
    cout << *pcmih << endl;
  }
#endif
  papaNode->enableReply(successful);
}

