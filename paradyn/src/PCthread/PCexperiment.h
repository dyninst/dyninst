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
 * experiment class
 * $Id: PCexperiment.h,v 1.14 2003/07/15 22:45:45 schendel Exp $
 */

#ifndef PCEXPER_H
#define PCEXPER_H

#include "PCintern.h"
#include "PCwhy.h"
#include "PCmetricInst.h"
#include "PCdata.h"
#include "common/h/Time.h"
#include "pdutil/h/pdRate.h"

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
  timeLength getTimeTrueFalse () {return timeTrueFalse;}
  pdRate getCurrentValue () {return currentValue;}
  pdRate getAdjustedValue () {return adjustedValue;}
  pdRate getLastThreshold () {return lastThreshold;}
  float getHysConstant () {return hysConstant;}
  relTimeStamp getStartTime () {return startTime;}
  relTimeStamp getEndTime () {return endTime;}
  float getEstimatedCost() {return estimatedCost;}
  void findOutCost();  // make async requests to get cost and store it
  //
  // this call invoked by PCmetricInst to notify experiment of new 
  // values  
  void newData(PCmetDataID, pdRate, relTimeStamp, relTimeStamp);
  void enableReply (unsigned token1, unsigned token2, unsigned token3,
		    bool successful, bool deferred = false, pdstring msg = "");
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
  timeLength timeTrueFalse;      
  // this is the (unadjusted) value of the PC metric itself
  pdRate currentValue;
  // this is value of PC metric minus threshold; only the sign has 
  // meaning, magnitudes mean varying things across different experiments.
  pdRate adjustedValue;
  // always contains the correct hysteresis parameter to use for next 
  // evaluation.  1-hysparam if been true; else 1+hysparam
  float hysConstant;     
  // time of first data received
  relTimeStamp startTime;      
  // time of most recent data received
  relTimeStamp endTime;        
  // true if minObs time has passed
  bool minObservationFlag;  
  // use this to flag user changes to thresholds during a search
  pdRate lastThreshold;
  static timeLength PCminTimeToFalse;
  static timeLength PCminTimeToTrue;
};

ostream& operator <<(ostream &os, experiment& ex);
        
#endif



