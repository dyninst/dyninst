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
 * The specific metric and hypothesis definitions which will eventually 
 * be parsed from a configuration file.
 * $Id: PCrules.C,v 1.52 2003/07/15 22:45:51 schendel Exp $
 */

#include "PCintern.h"
#include "PCwhy.h"
#include "PCmetric.h"

typedef pdvector<pdstring*> stringList;

//** temporary home of <gasp> globals
whyAxis *PCWhyAxis = new whyAxis();
hypothesis *const topLevelHypothesis = PCWhyAxis->getRoot();

//
// general metric evaluation functions
//
pdRate
CostTrackerEval (focus, const pdRate *data, int dataSize)
{
  assert (dataSize == 3);
  if (data[1] <= data[2])
    return data[0]/data[1];
  else
    return data[0]/data[2];
}

pdRate
DivideEval (focus, const pdRate *data, int dataSize)
{
  assert (dataSize == 2);
  if (data[1] > pdRate::Zero())
    return data[0]/data[1];
  else
    return pdRate::Zero();
}

pdRate
MultiplyEval (focus, const pdRate *data, int dataSize)
{
  assert (dataSize == 2);
  return data[0] * data[1];
}

pdRate
AddEval (focus, const pdRate *data, int dataSize)
{
  assert (dataSize >= 2);
  const pdRate *curr = data;
  pdRate ansr = pdRate::Zero();
  for (int i = 0; i < dataSize; i++) {
    ansr = ansr + *curr;
    curr++;
  }
  return ansr;
}

pdRate
SubtractEval (focus, const pdRate *data, int dataSize)
{
  assert (dataSize == 2);
  return data[0] - data[1];
}

//
// The actual metrics.
//

void initPCmetrics()
{
  // note: the PCmetric constructor stores a copy of the address in 
  // its dictionary, accessed by string name
  PCmetric *temp;
  metNameFocus specs[10];

  if(performanceConsultant::useCallGraphSearch)
    specs[0].mname = "cpu_inclusive";
  else 
    specs[0].mname = "cpu";

  specs[0].whichFocus = cf;
  specs[0].ft = averaging;
  specs[1].mname = "active_processes";
  specs[1].whichFocus = cf;
  specs[1].ft = averaging;
  temp = new PCmetric ("NormalizedCPUtime", specs, 2, NULL, 
		       DivideEval, 1);
#ifdef PCDEBUG
  if (performanceConsultant::printSearchChanges)
    cout << "PCmetric " << temp->getName() << " created." << endl;
#endif

  if(performanceConsultant::useCallGraphSearch)
    specs[0].mname = "cpu_inclusive";
  else
    specs[0].mname = "cpu";
  specs[0].whichFocus = cf;
  specs[0].ft = averaging;
  temp = new PCmetric ("nonNormalizedCPUtime", specs, 1, NULL, NULL, 1);
#ifdef PCDEBUG
  if (performanceConsultant::printSearchChanges)
    cout << "PCmetric " << temp->getName() << " created." << endl;
#endif

  if(performanceConsultant::useCallGraphSearch)
    specs[0].mname = "sync_wait_inclusive";
  else
    specs[0].mname = "sync_wait";
  specs[0].whichFocus = cf;
  specs[0].ft = averaging;
  specs[1].mname = "active_processes";
  specs[1].whichFocus = cf;
  specs[1].ft = averaging;
  temp = new PCmetric ("NormSyncToCPURatio", specs, 2, NULL, DivideEval, 1);
#ifdef PCDEBUG
  if (performanceConsultant::printSearchChanges)
    cout << "PCmetric " << temp->getName() << " created." << endl;
#endif

  
  if(performanceConsultant::useCallGraphSearch)
    specs[0].mname = "sync_wait_inclusive";
  else
    specs[0].mname = "sync_wait";
  specs[0].whichFocus = cf;
  specs[0].ft = averaging;
  temp = new PCmetric ("nonNormSyncToCPURatio", specs, 1, NULL, NULL, 1);
#ifdef PCDEBUG
  if (performanceConsultant::printSearchChanges)
    cout << "PCmetric " << temp->getName() << " created." << endl;
#endif

  specs[0].mname = "io_bytes";
  specs[0].whichFocus = cf;
  specs[0].ft = averaging;
  specs[1].mname = "io_ops";
  specs[1].whichFocus = cf;
  specs[1].ft = averaging;
  temp = new PCmetric ("IOAvgSize", specs, 2, NULL, DivideEval, 1);
#ifdef PCDEBUG
  if (performanceConsultant::printSearchChanges)
    cout << "PCmetric " << temp->getName() << " created." << endl;
#endif

  specs[0].mname = "locks_held";
  specs[0].whichFocus = cf;
  specs[0].ft = averaging;
  specs[1].mname = "sync_ops";
  specs[1].whichFocus = cf;
  specs[1].ft = averaging;
  temp = new PCmetric ("SyncRegionSize", specs, 2, NULL, DivideEval, 1);
#ifdef PCDEBUG
  if (performanceConsultant::printSearchChanges)
    cout << "PCmetric " << temp->getName() << " created." << endl;
#endif

  if(performanceConsultant::useCallGraphSearch)
    specs[0].mname = "io_wait_inclusive";
  else 
    specs[0].mname = "io_wait";
  specs[0].whichFocus = cf;
  specs[0].ft = averaging;
  temp = new PCmetric ("IoWait", specs, 1, NULL, NULL, 1);
#ifdef PCDEBUG
  if (performanceConsultant::printSearchChanges)
    cout << "PCmetric " << temp->getName() << " created." << endl;
#endif

  specs[0].mname = "observed_cost";
  specs[0].whichFocus = tlf;
  specs[0].ft = nonfiltering;
  specs[1].mname = "number_of_cpus";
  specs[1].whichFocus = tlf;
  specs[1].ft = averaging;
  specs[2].mname = "active_processes";
  specs[2].whichFocus = tlf;
  specs[2].ft = averaging;
  temp = new PCmetric("normSmoothCost", specs, 3, NULL, CostTrackerEval, 0);
#ifdef PCDEBUG
  if (performanceConsultant::printSearchChanges)
    cout << "PCmetric " << temp->getName() << " created." << endl;
#endif
//****** testers ******
  // metrics with single dm metric, with or without pause time
  specs[0].mname = "active_processes";
  specs[0].whichFocus = tlf;
  specs[0].ft = averaging;
  // 0 at end means without PauseTime 
  // **this is just a test but should 
  // be some way to catch these more generally
  temp = new PCmetric( "PauseTime", specs, 1, NULL, NULL, 0);
#ifdef PCDEBUG
  cout << "PCmetric " << temp->getName() << " created." << endl;
#endif
}

pdRate defaultGetThresholdFunc (const char *tname, focus)
{
  // get tunable constant value
  tunableFloatConstant threshtc = 
    tunableConstantRegistry::findFloatTunableConstant(tname);
  return pdRate(threshtc.getValue());
}

pdRate getOneThresholdFunc (const char *tname, focus)
{
  // get tunable constant value
  tunableFloatConstant threshtc = 
    tunableConstantRegistry::findFloatTunableConstant(tname);
  return pdRate(0.5);
}

pdRate getThreeThresholdFunc (const char *tname, focus)
{
  // get tunable constant value
  tunableFloatConstant threshtc = 
    tunableConstantRegistry::findFloatTunableConstant(tname);
  return pdRate(3.0);
}

pdRate SyncRegionGetThresholdFunc (const char *, focus)
{
  // get tunable constant value
  tunableFloatConstant threshtc = 
    tunableConstantRegistry::findFloatTunableConstant("lockOverhead");
  tunableFloatConstant threshtc2 = 
    tunableConstantRegistry::findFloatTunableConstant("minLockSize");

  return pdRate(threshtc.getValue() * threshtc2.getValue());
}

void initPChypos()
{
  bool flag;
  pdstring *plum;
  stringList plumList;

  plum = new pdstring ("/Machine");
  plumList += plum;
  flag = PCWhyAxis->
    addHypothesis ("ExcessiveSyncWaitingTime", (const char *)NULL, 
		   "NormSyncToCPURatio",
		   "nonNormSyncToCPURatio",
		   "highSyncThreshold", 
		   "PC_SyncThreshold",
		   defaultGetThresholdFunc, 
		   gt, whyAndWhere, NULL, &plumList, NULL);

  if (!flag)
    cout << "hypothesis constructor failed for ExcessiveSyncWaitingTime" 
      << endl;

/*
  flag = PCWhyAxis->
    addHypothesis ("SyncRegionTooSmall", (const char *)NULL, "SyncRegionSize",
		   "",
		   "",
		   "PC_SyncThreshold",
		   SyncRegionGetThresholdFunc,
		   lt, whyOnly, NULL, &plumList, NULL);
  if (!flag)
    cout << "hypothesis constructor failed for SyncRegionTooSmall" << endl;
*/
  plum = new pdstring ("/SyncObject");
  stringList plumList2;
  plumList2 += plum;
  flag = PCWhyAxis->
    addHypothesis ("ExcessiveIOBlockingTime", (const char *)NULL, "IoWait",
		   "", "highIOthreshold", 
		   "PC_IOThreshold",
		   defaultGetThresholdFunc, 
		   gt, whereBeforeWhy, NULL, &plumList2, NULL);

  if (!flag)
    cout << "hypothesis constructor failed for ExcessiveIOBlockingTime" << endl;

  flag = PCWhyAxis->
    addHypothesis ("TooManySmallIOOps", "ExcessiveIOBlockingTime",
		   "IOAvgSize", "",
		   "diskBlockSize", 
		   "PC_IOOpThreshold",
		   defaultGetThresholdFunc, 
		   lt, whyAndWhere, NULL, &plumList2, NULL);

  if (!flag)
    cout << "hypothesis constructor failed for TooManySmallIOOps" << endl;
  plum = new pdstring ("/SyncObject");
  stringList plumList3;
  stringList suppress;
  plumList3 += plum;
  //plum = new pdstring ("/Process");
  //plumList3 += plum;
  //plum = new pdstring  ("/Code/anneal.c");
  //suppress += plum;
  flag = PCWhyAxis->
    addHypothesis ("CPUbound", (const char *)NULL, 
		   "NormalizedCPUtime",
		   "nonNormalizedCPUtime",
		   "highCPUtoSyncRatioThreshold",
		   "PC_CPUThreshold",
		   defaultGetThresholdFunc,
		   gt, whyAndWhere, NULL, &plumList3, NULL);
		   //gt, NULL, &plumList3, &suppress);

  if (!flag)
    cout << "hypothesis constructor failed for normCPUtimeTester" << endl;

/*
    // **** the test hypotheses  *****
  flag = PCWhyAxis->
    addHypothesis("tooMuchPauseTime", (const char *)NULL, 
		  "PauseTime",
		  "PauseTime",
                  "highCPUtoSyncRatioThreshold",
		  "PC_CPUThreshold",
		  getOneThresholdFunc, 
		  gt, whereOnly, NULL, &plumList3, NULL);
  // ** how to handle this case?
  if (!flag)
    cout << "hypothesis constructor failed for PauseTime" << endl;

  flag = PCWhyAxis->
    addHypothesis("WAYtooMuchPauseTime", "tooMuchPauseTime", 
		  "PauseTime",
		  "PauseTime",
                  "highCPUtoSyncRatioThreshold",
		  "PC_CPUThreshold",
		  getThreeThresholdFunc, 
		  gt, whyOnly, NULL, &plumList3, NULL);
  // ** how to handle this case?
  if (!flag)
    cout << "hypothesis constructor failed for PauseTime" << endl;

  flag = PCWhyAxis->
    addHypothesis("TeenytooMuchPauseTime", "tooMuchPauseTime", 
		  "PauseTime",
		  "PauseTime",
                  "highCPUtoSyncRatioThreshold",
		  "PC_CPUThreshold",
		  getOneThresholdFunc, 
		  gt, whyOnly, NULL, &plumList3, NULL);
  // ** how to handle this case?
  if (!flag)
    cout << "hypothesis constructor failed for PauseTime" << endl;
*/
}
