/*
 * Copyright (c) 1996-2000 Barton P. Miller
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

// $Id: PCcostServer.C,v 1.4 2002/12/20 07:50:02 jaw Exp $
// methods for the costServer class

#include "PCintern.h"
#include "PCmetricInst.h"
#include "PCfilter.h"
#include "PCcostServer.h"

pdvector<costServerRec> costServer::costRecords;

void
costServer::getPredictedCost (metricHandle m, focus f, PCmetricInst *me)
{
  int index = getIndex (m, f);
  if (index < 0) {
    // no existing record for this metric focus pair 
    costServerRec newrec;
    newrec.met = m;
    newrec.foc = f;
    newrec.cost = 0.0;
    newrec.pending += me;
    unsigned token = costRecords.size();
    costRecords += newrec;
    dataMgr->getPredictedDataCost(performanceConsultant::pstream,m,f,token);
  } else {
    // predicted cost already cached or requested for this mf pair
    costServerRec *curr = &(costRecords[index]);
    if (curr->pending.size() > 0) 
      // outstanding request to dm
      curr->pending += me;
    else
      me->updateEstimatedCost(curr->cost);
  }
}

void 
costServer::newPredictedCostData (unsigned token, float predCost)
{
  costServerRec *curr = &(costRecords[token]);
  unsigned psz = curr->pending.size();
  for (unsigned i = 0; i < psz; i++) {
    (curr->pending[i])->updateEstimatedCost(predCost);
  }
  curr->pending.resize(0);
  curr->cost = predCost;
}

//** don't forget to change this!!!
int 
costServer::getIndex (metricHandle mID, focus fID) 
{
  int retVal = -1;
  unsigned csz = costRecords.size();
  for (unsigned j = 0; j < csz; j++) {
    if (costRecords[j].met == mID)
      if (costRecords[j].foc == fID) {
	retVal = j;
	break;
      }
  }
  return retVal;
}
