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
 * PCrules.C
 *
 * The specific metric and hypothesis definitions which will eventually 
 *  be parsed from a configuration file.
 *
 * $Log: PCrules.C,v $
 * Revision 1.40  1996/08/16 21:03:40  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.39  1996/07/23 20:28:04  karavan
 * second part of two-part commit.
 *
 * implements new search strategy which retests false nodes under certain
 * circumstances.
 *
 * change in handling of high-cost nodes blocking the ready queue.
 *
 * code cleanup.
 *
 * Revision 1.38  1996/07/22 18:56:21  karavan
 * part one of two-part commit
 *
 * Revision 1.37  1996/05/17 16:19:49  karavan
 * commented out debug print
 *
 * Revision 1.36  1996/05/08 07:35:23  karavan
 * Changed enable data calls to be fully asynchronous within the performance consultant.
 *
 * some changes to cost handling, with additional limit on number of outstanding enable requests.
 *
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
 */

#include "PCintern.h"
#include "PCwhy.h"
#include "PCmetric.h"

typedef vector<string*> stringList;

//** temporary home of <gasp> globals
whyAxis *PCWhyAxis = new whyAxis();
hypothesis *const topLevelHypothesis = PCWhyAxis->getRoot();

//
// general metric evaluation functions
//
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
  assert (dataSize == 2);
  if (data[1] > 0.0)
    return data[0]/data[1];
  else
    return 0.0;
}

sampleValue 
MultiplyEval (focus, sampleValue *data, int dataSize)
{
  assert (dataSize == 2);
  return data[0] * data[1];
}

sampleValue 
AddEval (focus, sampleValue *data, int dataSize)
{
  assert (dataSize >= 2);
  sampleValue *curr = data;
  sampleValue ansr = 0.0;
  for (int i = 0; i < dataSize; i++) {
    ansr = ansr + *curr;
    curr++;
  }
  return ansr;
}

sampleValue 
SubtractEval (focus, sampleValue *data, int dataSize)
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

  specs[0].mname = "cpu";
  specs[0].whichFocus = cf;
  specs[0].ft = averaging;
  temp = new PCmetric ("nonNormalizedCPUtime", specs, 1, NULL, NULL, 1);
#ifdef PCDEBUG
  if (performanceConsultant::printSearchChanges)
    cout << "PCmetric " << temp->getName() << " created." << endl;
#endif

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

  specs[0].mname = "sync_wait";
  specs[0].whichFocus = cf;
  specs[0].ft = averaging;
  temp = new PCmetric ("nonNormSyncToCPURatio", specs, 1, NULL, NULL, 1);
#ifdef PCDEBUG
  if (performanceConsultant::printSearchChanges)
    cout << "PCmetric " << temp->getName() << " created." << endl;
#endif

  specs[0].mname = "io_ops";
  specs[0].whichFocus = cf;
  specs[0].ft = averaging;
  specs[1].mname = "io_bytes";
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
		   gt, (void *)NULL, &plumList, NULL);

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
		   lt, (void *)NULL, &plumList, NULL);
  if (!flag)
    cout << "hypothesis constructor failed for SyncRegionTooSmall" << endl;
*/
  plum = new string ("/SyncObject");
  stringList plumList2;
  plumList2 += plum;
  flag = PCWhyAxis->
    addHypothesis ("ExcessiveIOBlockingTime", (const char *)NULL, "IoWait",
		   "", "highIOthreshold", 
		   "PC_IOThreshold",
		   defaultGetThresholdFunc, 
		   gt, (void *)NULL, &plumList2, NULL);

  if (!flag)
    cout << "hypothesis constructor failed for ExcessiveIOBlockingTime" << endl;

  flag = PCWhyAxis->
    addHypothesis ("TooManySmallIOOps", "ExcessiveIOBlockingTime",
		   "IOAvgSize", "",
		   "diskBlockSize", 
		   "PC_IOThreshold",
		   defaultGetThresholdFunc, 
		   gt, (void *)NULL, &plumList2, NULL);

  if (!flag)
    cout << "hypothesis constructor failed for TooManySmallIOOps" << endl;
  plum = new string ("/SyncObject");
  stringList plumList3;
  stringList suppress;
  plumList3 += plum;
  plum = new string ("/Process");
  plumList3 += plum;
  //plum = new string  ("/Code/anneal.c");
  //suppress += plum;
  flag = PCWhyAxis->
    addHypothesis ("CPUbound", (const char *)NULL, 
		   "NormalizedCPUtime",
		   "nonNormalizedCPUtime",
		   "highCPUtoSyncRatioThreshold",
		   "PC_CPUThreshold",
		   defaultGetThresholdFunc,
		   gt, (void *)NULL, &plumList3, NULL);
		   //gt, (void *)NULL, &plumList3, &suppress);

  if (!flag)
    cout << "hypothesis constructor failed for normCPUtimeTester" << endl;

}
