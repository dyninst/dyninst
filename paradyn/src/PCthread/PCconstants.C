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
 * PCconstants.C
 *
 * All tunable Constants used by hypotheses.
 *
 * $Log: PCconstants.C,v $
 * Revision 1.2  1996/02/08 19:52:34  karavan
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
 * Revision 1.1  1996/02/02 02:06:28  karavan
 * A baby Performance Consultant is born!
 *
 */

#include "PCintern.h"

//
// registered TC callbacks
//
void TChysRangeCB (float newval)
{
  performanceConsultant::hysteresisRange = newval;
}
void TCpredictedCostLimitCB (float newval)
{
  performanceConsultant::predictedCostLimit = newval;
}
void TCminObservationTimeCB (float newval)
{
  performanceConsultant::minObservationTime = newval;
}
void TCsufficientTimeCB (float newval)
{
  performanceConsultant::sufficientTime = newval;
}
void TCprintDataTraceCB (bool newval)
{
  performanceConsultant::printDataTrace = newval;
}
void TCprintTestResultsCB (bool newval)
{
  performanceConsultant::printTestResults = newval;
}
void TCprintSearchChangesCB (bool newval)
{
  performanceConsultant::printSearchChanges = newval;
}
void TCprintDataCollectionCB (bool newval)
{
  performanceConsultant::printDataCollection = newval;
}
void TCuseIndividualThresholdsCB (bool newval)
{
  performanceConsultant::useIndividualThresholds = newval;
}

void initPCconstants ()
{
  float floatInitializer;
  bool boolInitializer;

  // 
  // user-level TCs used by the PC
  //

  floatInitializer = 20.0;
  tunableConstantRegistry::createFloatTunableConstant
    ("predictedCostLimit",
     "Max. allowable perturbation of the application.",
     TCpredictedCostLimitCB, // callback routine
     userConstant,
     floatInitializer, // initial value
     0.0,  // min
     100.0); // max
  TCpredictedCostLimitCB(floatInitializer);

  floatInitializer = 0.15;
  tunableConstantRegistry::createFloatTunableConstant
    ("hysteresisRange",
     "Fraction above and below threshold that a test should use.",
     TChysRangeCB, // callback
     developerConstant,
     floatInitializer, // initial
     0.0,  // min
     1.0);  // max
  TChysRangeCB (floatInitializer);  //initial value

  floatInitializer = 1.0;
  tunableConstantRegistry::createFloatTunableConstant
    ("minObservationTime",
     "min. time (in seconds) to wait after changing inst to start try hypotheses.",
     TCminObservationTimeCB, // callback
     userConstant,
     floatInitializer, // initial
     0.0, // min
     60.0); // max
  TCminObservationTimeCB(floatInitializer);

  floatInitializer = 6.0;
  tunableConstantRegistry::createFloatTunableConstant      
    ("sufficientTime",
     "How long to wait (in seconds) before we can conclude a hypothesis is false.",
     TCsufficientTimeCB,
     userConstant,
     floatInitializer, // initial
     0.0, // min
     1000.0); // max
  TCsufficientTimeCB(floatInitializer);

  //
  // debug printing
  //

  boolInitializer = false;
  tunableConstantRegistry::createBoolTunableConstant 
    ("PCprintDataTrace", 
     "Trace Data Movement from Arrival at PC thread to arrival at Experiment.",
     TCprintDataTraceCB,
     developerConstant, 
     boolInitializer);
  TCprintDataTraceCB (boolInitializer);

  boolInitializer = false;
  tunableConstantRegistry::createBoolTunableConstant 
    ("PCprintTestResults", 
     "Print details of each experiment comparison.",
     TCprintTestResultsCB, 
     developerConstant, 
     boolInitializer);
  TCprintTestResultsCB (boolInitializer);

  boolInitializer = false;
  tunableConstantRegistry::createBoolTunableConstant 
    ("PCprintSearchChanges", 
     "Print detail of search graph expansion and all node status changes.",
     TCprintSearchChangesCB,
     developerConstant, 
     boolInitializer);
  TCprintSearchChangesCB (boolInitializer);

  boolInitializer = false;
  tunableConstantRegistry::createBoolTunableConstant 
    ("PCprintDataCollection", 
     "Trace DM Data enables, disables, plus receipt of DM data.",
     TCprintDataCollectionCB, 
     developerConstant, 
     boolInitializer);
  TCprintDataCollectionCB (boolInitializer);

  //
  // thresholds used in experiments
  //

  //
  // first, general, default thresholds
  //
  boolInitializer = false;
  tunableConstantRegistry::createBoolTunableConstant 
    ("PCuseIndividualThresholds", 
     "Use individually defined thresholds for all PC hypotheses.",
     TCuseIndividualThresholdsCB, 
     developerConstant, 
     boolInitializer);
  TCuseIndividualThresholdsCB (boolInitializer);

  floatInitializer = 0.20;
  tunableConstantRegistry::createFloatTunableConstant 
    ("PC_SyncThreshold", 
     "Threshold to use while testing Synchronization Hypotheses", 
     (void *)NULL,
     userConstant,  
     floatInitializer, 0.0, 1.0);

  floatInitializer = 0.30;
  tunableConstantRegistry::createFloatTunableConstant
    ("PC_CPUThreshold", 
     "Threshold to use while testing CPUBound Hypothesis", 
     (void *)NULL, 
     userConstant, 
     floatInitializer, 0, 1.00);

  floatInitializer = 0.20;
  tunableConstantRegistry::createFloatTunableConstant
    ("PC_IOThreshold", 
     "Threshold to use while testing IO Hypotheses", 
     (void *)NULL, 
     userConstant, 
     floatInitializer, 0.0, 1.0);

  //
  // individual thresholds (will be defined in PCL eventually)
  //

  // 20% sync is considered high.
  //const float highSyncThreshold	= 0.20;
  tunableConstantRegistry::createFloatTunableConstant 
    ("highSyncThreshold", 
     "highSyncThreshold", 
     (void *) NULL, 
     developerConstant,  
     0.20, 0.10, 0.30);

  // 75% of time in CPU imples cpu time should be checked.
  //const float highCPUtoSyncRatioThreshold	= 0.60;
  tunableConstantRegistry::createFloatTunableConstant
    ("highCPUtoSyncRatioThreshold", 
     "highCPUtoSyncRatioThreshold", 
     (void *) NULL, 
     developerConstant, 
     0.25, 0, 1.00);

  // time to aquire an free lock (in usec).
  //const lockOverhead = 10;
  tunableConstantRegistry::createFloatTunableConstant
    ("lockOverhead", 
     "lockOverhead",
     (void *)NULL, 
     developerConstant, 
     10, 0, 20);

  // critical section should be larger than 4 null locks 
  //const float minLockSize = 4.00;
  tunableConstantRegistry::createFloatTunableConstant
    ("minLockSize", 
     "minLockSize", 
     (void *)NULL,
     developerConstant, 
     4.00, 3.00, 5.00);

  // 10-50% seems right
  //const float highIOthreshold = 0.10;
  tunableConstantRegistry::createFloatTunableConstant
    ("highIOthreshold", 
     "highIOthreshold", 
     (void *)NULL, 
     developerConstant, 
     0.10, 0.05, 0.15);

  // physical disk blcoks
  //const int diskBlockSize = 4096;
  tunableConstantRegistry::createFloatTunableConstant
    ("diskBlockSize", 
     "diskBlockSize", 
     (void *)NULL, 
     developerConstant, 
     4096, 0, 8192);

  // 50% of io delay is seek we call it seek bound.
  //const float seekBoundThreshold = 0.50;
  tunableConstantRegistry::createFloatTunableConstant
    ("seekBoundThreshold", 
     "seekBoundThreshold", 
     (void *)NULL,
     developerConstant, 
     0.50, 0.40, 0.60);

}
