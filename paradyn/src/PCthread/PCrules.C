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
 * Revision 1.35  1996/05/06 04:35:23  karavan
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
 * Revision 1.34  1996/04/30 06:27:05  karavan
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
 * Revision 1.33  1996/03/18 07:13:07  karavan
 * Switched over to cost model for controlling extent of search.
 *
 * Added new TC PCcollectInstrTimings.
 *
 * Revision 1.32  1996/02/22 18:26:14  karavan
 * added some plums.
 *
 * Revision 1.31  1996/02/08 19:52:46  karavan
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
CostTrackerEval (focus, sampleValue *data, int dataSize)
{
  assert (dataSize == 3);
  if (data[1] <= data[2])
    return data[0]/data[1];
  else
    return data[0]/data[2];
}

sampleValue 
DivideEval (focus, sampleValue *data, int dataSize)
{
  assert (dataSize >= 2);
  if (dataSize == 3) {
    if (data[2] > 0.0)
      return data[1]/data[2];
    else
      return 0.0;
  } else {
    if (data[1] > 0.0)
      return data[0]/data[1];
    else
      return 0.0;
  }
}

sampleValue 
MultiplyEval (focus, sampleValue *data, int dataSize)
{
  assert (dataSize >= 2);
  if (dataSize == 3) {
    return data[1] * data[2];
  } else {
    return data[0] * data[1];
  }
}

sampleValue 
AddEval (focus, sampleValue *data, int dataSize)
{
  assert (dataSize >= 2);
  if (dataSize == 3) {
    return data[1] + data[2];
  } else {
    return data[0] + data[1];
  }
}

sampleValue 
SubtractEval (focus, sampleValue *data, int dataSize)
{
  assert (dataSize >= 2);
  if (dataSize == 3) {
    return data[1] - data[2];
  } else {
    return data[0] - data[1];
  }
}

//
// The actual metrics.
//

void initPCmetrics()
{
  // note: the PCmetric constructor stores a copy of the address in 
  // its dictionary, accessed by string name
  PCmetric *temp;
  metNameFocus *specs = new metNameFocus;
  metNameFocus *specs2 = new metNameFocus[2];
  metNameFocus *specs3 = new metNameFocus[3];

  specs2[0].mname = "cpu";
  specs2[0].whichFocus = cf;
  specs2[0].ft = averaging;
  specs2[1].mname = "active_processes";
  specs2[1].whichFocus = cf;
  specs2[1].ft = averaging;
  temp = new PCmetric ("NormalizedCPUtime", specs2, 2, NULL, 
		       DivideEval, 1);
  if (performanceConsultant::printSearchChanges)
    cout << "PCmetric " << temp->getName() << " created." << endl;

  specs[0].mname = "cpu";
  specs[0].whichFocus = cf;
  specs[0].ft = averaging;
  temp = new PCmetric ("nonNormalizedCPUtime", specs, 1, NULL, NULL, 1);
  if (performanceConsultant::printSearchChanges)
    cout << "PCmetric " << temp->getName() << " created." << endl;
  
  specs2[0].mname = "sync_wait";
  specs2[0].whichFocus = cf;
  specs2[0].ft = averaging;
  specs2[1].mname = "active_processes";
  specs2[1].whichFocus = cf;
  specs2[1].ft = averaging;
  temp = new PCmetric ("NormSyncToCPURatio", specs2, 2, NULL, DivideEval, 1);
  if (performanceConsultant::printSearchChanges)
    cout << "PCmetric " << temp->getName() << " created." << endl;

  specs[0].mname = "sync_wait";
  specs[0].whichFocus = cf;
  specs[0].ft = averaging;
  temp = new PCmetric ("nonNormSyncToCPURatio", specs, 1, NULL, NULL, 1);
  if (performanceConsultant::printSearchChanges)
    cout << "PCmetric " << temp->getName() << " created." << endl;

  specs2[0].mname = "io_ops";
  specs2[0].whichFocus = cf;
  specs2[0].ft = averaging;
  specs2[1].mname = "io_bytes";
  specs2[1].whichFocus = cf;
  specs2[1].ft = averaging;
  temp = new PCmetric ("IOAvgSize", specs2, 2, NULL, DivideEval, 1);
  if (performanceConsultant::printSearchChanges)
    cout << "PCmetric " << temp->getName() << " created." << endl;

  specs2[0].mname = "locks_held";
  specs2[0].whichFocus = cf;
  specs2[0].ft = averaging;
  specs2[1].mname = "sync_ops";
  specs2[1].whichFocus = cf;
  specs2[1].ft = averaging;
  temp = new PCmetric ("SyncRegionSize", specs2, 2, NULL, DivideEval, 1);
  if (performanceConsultant::printSearchChanges)
    cout << "PCmetric " << temp->getName() << " created." << endl;

  specs->mname = "io_wait";
  specs->whichFocus = cf;
  specs->ft = averaging;
  temp = new PCmetric ("IoWait", specs, 1, NULL, NULL, 1);
  if (performanceConsultant::printSearchChanges)
    cout << "PCmetric " << temp->getName() << " created." << endl;

  specs3[0].mname = "smooth_obs_cost";
  specs3[0].whichFocus = tlf;
  specs3[0].ft = nonfiltering;
  specs3[1].mname = "number_of_cpus";
  specs3[1].whichFocus = tlf;
  specs3[1].ft = averaging;
  specs3[2].mname = "active_processes";
  specs3[2].whichFocus = tlf;
  specs3[2].ft = averaging;
  temp = new PCmetric("normSmoothCost", specs3, 3, NULL, CostTrackerEval, 0);
  if (performanceConsultant::printSearchChanges)
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
  string *plum;
  stringList plumList;

  plum = new string ("/Machine");
  plumList += plum;
  flag = PCWhyAxis->
    addHypothesis ("ExcessiveSyncWaitingTime", (const char *)NULL, 
		   "NormSyncToCPURatio",
		   "nonNormSyncToCPURatio",
		   "highSyncThreshold", 
		   "PC_SyncThreshold",
		   defaultGetThresholdFunc, 
		   gt, (void *)NULL, &plumList);

  if (!flag)
    cout << "hypothesis constructor failed for ExcessiveSyncWaitingTime" 
      << endl;

  flag = PCWhyAxis->
    addHypothesis ("SyncRegionTooSmall", (const char *)NULL, "SyncRegionSize",
		   "",
		   "",
		   "PC_SyncThreshold",
		   SyncRegionGetThresholdFunc,
		   lt, (void *)NULL, &plumList);
  if (!flag)
    cout << "hypothesis constructor failed for SyncRegionTooSmall" << endl;

  plum = new string ("/SyncObject");
  stringList plumList2;
  plumList2 += plum;
  flag = PCWhyAxis->
    addHypothesis ("ExcessiveIOBlockingTime", (const char *)NULL, "IoWait",
		   "", "highIOthreshold", 
		   "PC_IOThreshold",
		   defaultGetThresholdFunc, 
		   gt, (void *)NULL, &plumList2);

  if (!flag)
    cout << "hypothesis constructor failed for ExcessiveIOBlockingTime" << endl;

  flag = PCWhyAxis->
    addHypothesis ("TooManySmallIOOps", "ExcessiveIOBlockingTime",
		   "IOAvgSize", "",
		   "diskBlockSize", 
		   "PC_IOThreshold",
		   defaultGetThresholdFunc, 
		   gt, (void *)NULL, &plumList2);

  if (!flag)
    cout << "hypothesis constructor failed for TooManySmallIOOps" << endl;
  plum = new string ("/SyncObject");
  stringList plumList3;
  plumList3 += plum;
  plum = new string ("/Process");
  plumList3 += plum;
  flag = PCWhyAxis->
    addHypothesis ("CPUbound", (const char *)NULL, 
		   "NormalizedCPUtime",
		   "nonNormalizedCPUtime",
		   "highCPUtoSyncRatioThreshold",
		   "PC_CPUThreshold",
		   defaultGetThresholdFunc, 
		   gt, (void *)NULL, &plumList3);

  if (!flag)
    cout << "hypothesis constructor failed for normCPUtimeTester" << endl;

}
