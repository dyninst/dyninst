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
 * PCconstants.C
 *
 * All tunable Constants used by hypotheses.
 *
 * $Log: PCconstants.C,v $
 * Revision 1.7  1997/03/16 23:18:14  lzheng
 * Changes made for the value of observed cost
 *
 * Revision 1.6  1996/08/16 21:03:15  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.5  1996/07/22 18:56:20  karavan
 * part one of two-part commit
 *
 * Revision 1.4  1996/04/18 02:27:13  tamches
 * tunable "predictedCostLimit" renamed to "costLimit"; also, its initial
 * value now 0.2
 *
 * Revision 1.3  1996/03/18 07:10:28  karavan
 * added new tunable constant, PCcollectInstrTimings.
 *
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
void TCcollectInstrTimingsCB (bool newval)
{
  performanceConsultant::collectInstrTimings = newval;
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

  floatInitializer = 1.5;
  tunableConstantRegistry::createFloatTunableConstant
    ("costLimit",
     "Max allowable perturbation of the application, as a fraction of the program's CPU time",
     TCpredictedCostLimitCB, // callback routine
     userConstant,
     floatInitializer, // initial value
     1.0,  // min
     20.0); // max
  TCpredictedCostLimitCB(floatInitializer);

  floatInitializer = 0.05;
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

  boolInitializer = false;
  tunableConstantRegistry::createBoolTunableConstant 
    ("PCcollectInstrTimings", 
     "Time all instrumentation requests and save results in file TESTresult.out",
     TCcollectInstrTimingsCB, 
     developerConstant, 
     boolInitializer);
  TCcollectInstrTimingsCB (boolInitializer);

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
