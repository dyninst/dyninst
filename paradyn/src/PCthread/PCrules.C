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
 * PCrules.C
 *
 * The specific metric and hypothesis definitions which will eventually 
 *  be parsed from a configuration file.
 *
 * $Log: PCrules.C,v $
 * Revision 1.30  1996/02/02 02:06:46  karavan
 * A baby Performance Consultant is born!
 *
 */

#include "PCintern.h"
#include "PCwhy.h"
#include "PCmetric.h"

typedef vector<string*> stringList;

//** temporary home of <gasp> globals
whyAxis *PCWhyAxis = new whyAxis();
hypothesis *const topLevelHypothesis = PCWhyAxis->getRoot();

// metric-specific evaluation functions
sampleValue 
DivideEval (focus, sampleValue *data, int dataSize)
{
  assert (dataSize == 3);
  if (data[2] > 0.0)
    return data[1]/data[2];
  else
    return 0.0;
}

sampleValue 
MultiplyEval (focus, sampleValue *data, int dataSize)
{
  assert (dataSize == 3);
  return data[1] * data[2];
}

sampleValue 
AddEval (focus, sampleValue *data, int dataSize)
{
  assert (dataSize == 3);
  return data[1] + data[2];
}

sampleValue 
SubtractEval (focus, sampleValue *data, int dataSize)
{
  assert (dataSize == 3);
  return data[1] - data[2];
}

//
// The actual metrics.
//

void initPCmetrics()
{
  PCmetric *temp;
  metNameFocus *specs = new metNameFocus;
  metNameFocus *specs2 = new metNameFocus[2];

  // debug printing
  tunableBooleanConstant prtc = 
    tunableConstantRegistry::findBoolTunableConstant("PCprintSearchChanges");

  specs->mname = "observed_cost";
  specs->whichFocus = cf;
  temp = new PCmetric ("ObservedCost", specs, 1, NULL, NULL, 1);
  if (prtc.getValue())
    cout << "PCmetric " << temp->getName() << " created." << endl;

  specs2[0].mname = "cpu";
  specs2[0].whichFocus = cf;
  specs2[1].mname = "active_processes";
  specs2[1].whichFocus = tlf;
  temp = new PCmetric ("NormalizedCPUtime", specs2, 2, NULL, 
		       DivideEval, 1);
  if (prtc.getValue())
    cout << "PCmetric " << temp->getName() << " created." << endl;

  specs2[0].mname = "sync_wait";
  specs2[0].whichFocus = cf;
  specs2[1].mname = "active_processes";
  specs2[1].whichFocus = tlf;
  temp = new PCmetric ("SyncToCPURatio", specs2, 2, NULL, DivideEval, 1);
  if (prtc.getValue())
    cout << "PCmetric " << temp->getName() << " created." << endl;

  specs2[0].mname = "io_ops";
  specs2[0].whichFocus = cf;
  specs2[1].mname = "io_bytes";
  specs2[1].whichFocus = cf;
  temp = new PCmetric ("IOAvgSize", specs2, 2, NULL, DivideEval, 1);
  if (prtc.getValue())
    cout << "PCmetric " << temp->getName() << " created." << endl;

  specs2[0].mname = "locks_held";
  specs2[0].whichFocus = cf;
  specs2[1].mname = "sync_ops";
  specs2[1].whichFocus = cf;
  temp = new PCmetric ("SyncRegionSize", specs2, 2, NULL, DivideEval, 1);
  if (prtc.getValue())
    cout << "PCmetric " << temp->getName() << " created." << endl;

  specs->mname = "io_wait";
  specs->whichFocus = cf;
  temp = new PCmetric ("IoWait", specs, 1, NULL, NULL, 1);
  if (prtc.getValue())
    cout << "PCmetric " << temp->getName() << " created." << endl;


}

sampleValue defaultGetThresholdFunc (const char *tname, focus)
{
  // get tunable constant value
  tunableFloatConstant threshtc = 
    tunableConstantRegistry::findFloatTunableConstant(tname);
  return threshtc.getValue();
}

sampleValue SyncRegionGetThresholdFunc (const char *, focus)
{
  // get tunable constant value
  tunableFloatConstant threshtc = 
    tunableConstantRegistry::findFloatTunableConstant("lockOverhead");
  tunableFloatConstant threshtc2 = 
    tunableConstantRegistry::findFloatTunableConstant("minLockSize");

  return (threshtc.getValue() * threshtc2.getValue());
}

void initPChypos()
{
  bool flag;

  flag = PCWhyAxis->
    addHypothesis ("ExcessiveSyncWaitingTime", (const char *)NULL, 
		   "SyncToCPURatio",
		   "highSyncThreshold", 
		   defaultGetThresholdFunc, 
		   gt, (void *)NULL, (stringList *)NULL);

  if (!flag)
    cout << "hypothesis constructor failed for ExcessiveSyncWaitingTime" 
      << endl;

  flag = PCWhyAxis->
    addHypothesis ("SyncRegionTooSmall", (const char *)NULL, "SyncRegionSize",
		   "",
		   SyncRegionGetThresholdFunc,
		   lt, (void *)NULL, (stringList *)NULL);
  if (!flag)
    cout << "hypothesis constructor failed for SyncRegionTooSmall" << endl;

  flag = PCWhyAxis->
    addHypothesis ("ExcessiveIOBlockingTime", (const char *)NULL, "IoWait",
		   "highIOthreshold", 
		   defaultGetThresholdFunc, 
		   gt, (void *)NULL, (stringList *)NULL);

  if (!flag)
    cout << "hypothesis constructor failed for ExcessiveIOBlockingTime" << endl;

  flag = PCWhyAxis->
    addHypothesis ("TooManySmallIOOps", "ExcessiveIOBlockingTime",
		   "IOAvgSize",
		   "diskBlockSize", 
		   defaultGetThresholdFunc, 
		   gt, (void *)NULL, (stringList *)NULL);

  if (!flag)
    cout << "hypothesis constructor failed for TooManySmallIOOps" << endl;
  string *plum = new string ("/SyncObject");
  stringList plumList;
  plumList += plum;
  plum = new string ("/Process");
  plumList += plum;
  flag = PCWhyAxis->
    addHypothesis ("CPUbound", (const char *)NULL, 
		   "NormalizedCPUtime",
		   "highCPUtoSyncRatioThreshold", 
		   defaultGetThresholdFunc, 
		   gt, (void *)NULL, &plumList);

  if (!flag)
    cout << "hypothesis constructor failed for normCPUtimeTester" << endl;


}
