/*
 * PCcostServer.h
 * 
 * a static class which manages predicted cost data for the performance 
 * consultant.
 * 
 * $Log: PCcostServer.h,v $
 * Revision 1.1  1996/05/02 19:48:17  karavan
 * a new class to cache predicted cost values, since these calls take so
 * darn long to get back from the daemons we seek to minimize them where
 * possible.
 *
 */

#include "PCintern.h"

struct costServerRecStr {
  vector<PCmetricInst*> pending;
  metricHandle met;
  focus foc;
  float cost;
};
typedef struct costServerRecStr costServerRec;

class costServer {
public:
  static void getPredictedCost (metricHandle, focus, PCmetricInst*);
  static void newPredictedCostData (unsigned, float); 
private:
  static int getIndex (metricHandle, focus);
  static vector<costServerRec> costRecords;
};

