/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: PCmetric.C,v 1.38 2004/03/23 01:12:28 eli Exp $
// The PCmetric class implements a limited subset of derived metrics.

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

dictionary_hash<pdstring, PCmetric*> PCmetric::AllPCmetrics(pdstring::hash);


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
  DMmetrics = new pdvector <PCMetInfo*> (dataSpecsSize); 
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


