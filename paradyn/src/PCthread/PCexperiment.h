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
 * PCexperiment.h
 *
 * experiment class
 *
 * $Log: PCexperiment.h,v $
 * Revision 1.7  1996/08/16 07:07:34  karavan
 * minor code cleanup
 *
 * Revision 1.6  1996/07/22 18:55:39  karavan
 * part one of two-part commit for new PC functionality of restarting searches.
 *
 * Revision 1.5  1996/05/15 04:35:09  karavan
 * bug fixes: changed pendingCost pendingSearches and numexperiments to
 * break down by phase type, so starting a new current phase updates these
 * totals correctly; fixed error in estimated cost propagation.
 *
 * Revision 1.4  1996/05/08 07:35:09  karavan
 * Changed enable data calls to be fully asynchronous within the performance consultant.
 *
 * some changes to cost handling, with additional limit on number of outstanding enable requests.
 *
 *
 * Revision 1.1  1996/02/02 02:07:25  karavan
 * A baby Performance Consultant is born!
 *
 */

#ifndef PCEXPER_H
#define PCEXPER_H

#include "PCintern.h"
#include "PCwhy.h"
#include "PCmetricInst.h"
#include "PCdata.h"

/******************************************************
Experiment 
========== 
Each node of the Search History Graph represents
a single experiment.  An experiment tests the value of a PCmetric for a
particular focus over some time interval.

*******************************************************/

class filter;
class PCsearch;

class experiment : public dataSubscriber
{
  friend ostream& operator <<(ostream &os, experiment& ex);
 public:
  experiment(hypothesis *whyowhy, focus whereowhere, bool persist, 
	     searchHistoryNode *papa, PCsearch *srch, bool amFlag, bool *err);
  ~experiment();
  //
  // subscribe/unsubscribe to flow of data which keeps 
  // experiment running
  bool start();
  void halt();
  // 
  // experiment status methods
  int getCurrentStatus() 
    {if (status) return 1; else return 0;}
  testResult getCurrentConclusion (){return currentConclusion;}
  timeStamp getTimeTrueFalse () {return timeTrueFalse;}
  sampleValue getCurrentValue () {return currentValue;}
  timeStamp getStartTime () {return startTime;}
  timeStamp getEndTime () {return endTime;}
  float getEstimatedCost() {return estimatedCost;}
  void findOutCost();  // make async requests to get cost and store it
  //
  // this call invoked by PCmetricInst to notify experiment of new 
  // values  
  void newData(PCmetDataID, float, double, double, float);
  void enableReply (unsigned token1, unsigned token2, unsigned token3,
		    bool successful);
  //
  // return cost value either in response to initialization async request
  // or when a resource update has resulted in a change to the specific 
  // metric-focus pairs being used by this experiment.
  void updateEstimatedCost(float costDiff); 
 private:
  // a true/false guess has held for the appropriate minimum interval
  void drawConclusion(testResult newConclusion);

  hypothesis *why;
  focus where;
  PCmetInstHandle pcmih;
  bool persistent;    // true means keep testing even if false
  PCsearch *mamaSearch;
  searchHistoryNode *papaNode;
  float estimatedCost;
  //
  // ## current test status information ##
  //
  // status is active (true) if currently subscribed to PCmetric data; 
  // inactive (false) otherwise
  bool status;    
  // true/false/unknown.  true/false conclusion can only be reached after
  // the correct minimum interval worth of data has been observed.
  testResult currentConclusion;      
  // unlike conclusion, guess will always be true or false for an active 
  // experiment which has received at least one piece of data.  A guess 
  // which remains the same for the proper interval results in a conclusion.
  testResult currentGuess;           // not long enough yet to say fer sure
  // how long has this guess held? (sum of intervals used in guess, not 
  // necessarily consecutive)
  timeStamp timeTrueFalse;      
  // this is value of PC metric minus threshold; only the sign has 
  // meaning, magnitudes mean varying things across different experiments.
  sampleValue currentValue;
  // always contains the correct hysteresis parameter to use for next 
  // evaluation.  1-hysparam if been true; else 1+hysparam
  float hysConstant;     
  // time of first data received
  timeStamp startTime;      
  // time of most recent data received
  timeStamp endTime;        
  // true if minObs time has passed
  bool minObservationFlag;  
  // use this to flag user changes to thresholds during a search
  sampleValue lastThreshold;
};

ostream& operator <<(ostream &os, experiment& ex);
        
#endif



