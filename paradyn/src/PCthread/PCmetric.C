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
 * PCmetric.C
 * 
 * The PCmetric class implements a limited subset of derived metrics.
 * 
 * $Log: PCmetric.C,v $
 * Revision 1.33  1996/04/30 06:26:57  karavan
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
 * Revision 1.32  1996/02/22 18:30:36  karavan
 * cleanup after dataMgr calls; explicitly cast all NULLs to keep
 * AIX happy
 *
 * Revision 1.31  1996/02/02 02:06:40  karavan
 * A baby Performance Consultant is born!
 *
 */

/*
 ****** important********
 here we assume that data continues flowing for all enabled metric/focus
 pairs unless (a) some drastic error renders a met/focus pair uncollectible or
 (b) a process exits.  In other words, if data is not received for a given 
 metric instance for some relatively short period of time, we will mark that 
 PC metric as uncollectible and stop testing the subscribing experiment(s)
*/

#include "PCintern.h"
#include "../DMthread/DMinclude.h"
#include "PCmetric.h"

dictionary_hash<string, PCmetric*> PCmetric::AllPCmetrics(string::hash);


PCmetric::PCmetric (char *DMmetName, focusType ftype, bool *success):
ft(ftype)
{
  metricHandle *tmpmh = dataMgr->findMetric(DMmetName);
  if (tmpmh) {
    mh = *tmpmh;
    delete tmpmh;
    metName = DMmetName;
    calc = (evalPCmetricInstFunc) NULL;
    setup = (initPCmetricInstFunc) NULL;
    *success = true;
    AllPCmetrics[metName] = this;
  } else {
    *success = false;
  }
}

PCmetric::PCmetric (const char *thisName, 
		    metNameFocus *dataSpecs, int dataSpecsSize, 
		    initPCmetricInstFunc setupFun, 
		    evalPCmetricInstFunc calcFun,
		    bool withPause):
		    metName(thisName), InstWithPause(withPause),
		    setup(setupFun), calc(calcFun)
{
  PCMetInfo *newEntry;
  metricHandle *mh;
  DMmetrics = new vector <PCMetInfo*> (dataSpecsSize); 
  for (int i = 0; i < dataSpecsSize; i++) {
    mh = dataMgr->findMetric(dataSpecs[i].mname);
    if (!mh) {
#ifdef PCDEBUG
      cout << "PCmetric can't find metric: " << dataSpecs[i].mname << endl;
#endif
      return;
    }
    newEntry = new PCMetInfo;
    newEntry->mh = *mh;
    delete mh;
    newEntry->fType = dataSpecs[i].whichFocus;
    newEntry->ft = dataSpecs[i].ft;
    (*DMmetrics)[i] = newEntry;
  }
  AllPCmetrics[metName] = this;
}


