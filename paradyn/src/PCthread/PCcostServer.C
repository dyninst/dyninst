/*
 * PCcostServer.C
 * 
 * methods for the costServer class
 *
 * $Log: PCcostServer.C,v $
 * Revision 1.1  1996/05/02 19:48:16  karavan
 * a new class to cache predicted cost values, since these calls take so
 * darn long to get back from the daemons we seek to minimize them where
 * possible.
 *
 */

#include "PCintern.h"
#include "PCmetricInst.h"
#include "PCfilter.h"
#include "PCcostServer.h"

vector<costServerRec> costServer::costRecords;

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
