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
 * PCdata.C
 *
 * dataSubscriber and dataProvider base classes
 *
 * $Log: PCdata.C,v $
 * Revision 1.5  1996/05/08 07:35:01  karavan
 * Changed enable data calls to be fully asynchronous within the performance consultant.
 *
 * some changes to cost handling, with additional limit on number of outstanding enable requests.
 *
 * Revision 1.4  1996/05/06 04:34:57  karavan
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
 * Revision 1.3  1996/05/02 19:46:26  karavan
 * changed predicted data cost to be fully asynchronous within the pc.
 *
 * added predicted cost server which caches predicted cost values, minimizing
 * the number of calls to the data manager.
 *
 * added new batch version of ui->DAGconfigNode
 *
 * added hysteresis factor to cost threshold
 *
 * eliminated calls to dm->enable wherever possible
 *
 * general cleanup
 *
 * Revision 1.2  1996/05/01 14:06:53  naim
 * Multiples changes in PC to make call to requestNodeInfoCallback async.
 * (UI<->PC). I also added some debugging information - naim
 *
 * Revision 1.1  1996/02/02  02:06:29  karavan
 * A baby Performance Consultant is born!
 *
 */

#include "PCintern.h"
#include "PCdata.h"

dataProvider::dataProvider() : 
estimatedCost(0.0), numConsumers(0) 
{
 ;
}

dataProvider::~dataProvider()
{
  ;
}

void
dataProvider::sendUpdatedEstimatedCost(float costDiff)
{
  unsigned size = allConsumers.size();
  for (unsigned i = 0; i < size; i++) {
    if (allConsumers[i] != NULL) 
      allConsumers[i] -> updateEstimatedCost (costDiff);
  }
}

//
// send data to each subscribed consumer
//
void
dataProvider::sendValue(PCmetDataID which, sampleValue newval, timeStamp begin, 
			timeStamp end, sampleValue norm)
{
  unsigned size = allConsumers.size();
#ifdef MYPCDEBUG
  if (size > 500) printf("================> allConsumers.size() = %d\n",size);
#endif
  for (unsigned i = 0; i < size; i++) {
    if (allConsumers[i] != NULL)
      (allConsumers[i])->newData (which, newval, begin, end, norm);
  }
}

void 
dataProvider::sendEnableReply (unsigned token1, unsigned token2, unsigned token3,
			       bool successful)
{
  unsigned size = allConsumers.size();
  for (unsigned i = 0; i < size; i++) {
    if (allConsumers[i] != NULL)
      (allConsumers[i])->enableReply (token1, token2, token3, successful);
  }
}
  
void 
dataProvider::addConsumer(dataSubscriber *consumer) 
{  
  bool added = false;
  // first check for open slot in existing consumers array
  // we don't care about order or indexed lookup for consumers cause 
  // there's so few per filter
  unsigned sz = allConsumers.size();
  for (unsigned i = 0; i < sz; i++) {
    if (allConsumers[i] == NULL) {
      allConsumers[i] = consumer;
      added = true;
      break;
    }
  }
  if (!added) {
    // grow array to open up more slots then use one
    allConsumers += consumer;
  }
  numConsumers++;
}

int
dataProvider::rmConsumer(dataSubscriber *consumer) 
{
  // we re-use slots in the consumer array (see addConsumer) so just 
  // find this one and set it to NULL and we're done.
  // NOTE that we don't delete the filter itself even if no consumers
  // remain.  This way we have the old data in case a new consumer arrives
  // later.
  unsigned size = allConsumers.size();
  for (unsigned i = 0; i < size; i++) {
    if (allConsumers[i] == consumer) {
      allConsumers[i] = NULL;
      numConsumers--;
      break;
    }
  }
  return numConsumers;
}

