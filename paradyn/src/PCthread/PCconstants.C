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
 * Revision 1.1  1996/02/02 02:06:28  karavan
 * A baby Performance Consultant is born!
 *
 */

#include "PCintern.h"

void initPCconstants ()
{
  // ** debug printing TCs
  // debug printing
    tunableConstantRegistry::createBoolTunableConstant 
      ("PCprintDataTrace", 
       "Trace Data Movement from Arrival at PC thread to arrival at Experiment.",
       (void *)NULL, developerConstant, false);

    tunableConstantRegistry::createBoolTunableConstant 
      ("PCprintTestResults", 
       "Print details of each experiment comparison.",
       (void *)NULL, developerConstant, false);

    tunableConstantRegistry::createBoolTunableConstant 
      ("PCprintSearchChanges", 
       "Print detail of search graph expansion and all node status changes.",
       (void *)NULL, developerConstant, false);

    tunableConstantRegistry::createBoolTunableConstant 
      ("PCprintDataCollection", 
       "Trace DM Data enables, disables, plus receipt of DM data.",
       (void *)NULL, developerConstant, false);


  // ** here there be dummies **
  // this one always true
    tunableConstantRegistry::createFloatTunableConstant 
      ("pauseTimeLimit", "pauseTimeLimit", (void *)NULL, 
       developerConstant, 0.2, 0.0, 1.0);

    // **here there be real ones **
  // 20% sync is considered high.
  //const float highSyncThreshold	= 0.20;
  tunableConstantRegistry::createFloatTunableConstant 
    ("highSyncThreshold", "highSyncThreshold", (void *) NULL, 
     developerConstant,  0.20, 0.10, 0.30);

  //const float highInstOverheadThreshold	= 0.20;
  tunableConstantRegistry::createFloatTunableConstant
    ("highInstOverheadThreshold", "highInstOverheadThreshold",
     (void *)NULL, developerConstant, 0.20, 0.10, 0.30);

  // 75% of time in CPU imples cpu time should be checked.
  //const float highCPUtoSyncRatioThreshold	= 0.60;
  tunableConstantRegistry::createFloatTunableConstant
    ("highCPUtoSyncRatioThreshold", "highCPUtoSyncRatioThreshold", 
     (void *) NULL, developerConstant, 0.25, 0, 1.00);

  // time to aquire an free lock (in usec).
  //const lockOverhead = 10;
  tunableConstantRegistry::createFloatTunableConstant
    ("lockOverhead", "lockOverhead",
     (void *)NULL, developerConstant, 10, 0, 20);

  // critical section should be larger than 4 null locks 
  //const float minLockSize = 4.00;
  tunableConstantRegistry::createFloatTunableConstant
    ("minLockSize", "minLockSize", (void *)NULL,
     developerConstant, 4.00, 3.00, 5.00);

  // 10-50% seems right
  //const float highIOthreshold = 0.10;
  tunableConstantRegistry::createFloatTunableConstant
    ("highIOthreshold", "highIOthreshold", (void *)NULL, 
     developerConstant, 0.10, 0.05, 0.15);

  // physical disk blcoks
  //const int diskBlockSize = 4096;
  tunableConstantRegistry::createFloatTunableConstant
    ("diskBlockSize", "diskBlockSize", 
     (void *)NULL, developerConstant, 4096, 0, 8192);

  // 50% of io delay is seek we call it seek bound.
  //const float seekBoundThreshold = 0.50;
  tunableConstantRegistry::createFloatTunableConstant
    ("seekBoundThreshold", "seekBoundThreshold", (void *)NULL,
     developerConstant, 0.50, 0.40, 0.60);

}
