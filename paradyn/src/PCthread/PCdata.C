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
 * Revision 1.1  1996/02/02 02:06:29  karavan
 * A baby Performance Consultant is born!
 *
 */

#include "PCintern.h"
#include "PCdata.h"

dataProvider::dataProvider() : 
estimatedCost(0.0), numConsumers(0) 
{
  allConsumers.resize(initialConsumerListSize);
  unsigned size = allConsumers.size();
  for (unsigned i = 0; i < size; i++) 
    allConsumers[i] = NULL;
}

dataProvider::~dataProvider()
{
  ;
}

//** in progress
void
dataProvider::sendUpdatedEstimatedCost(float costDiff)
{
  for (unsigned i = 0; i < allConsumers.size(); i++) {
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
  for (unsigned i = 0; i < size; i++) {
    if (allConsumers[i] != NULL)
      (allConsumers[i])->newData (which, newval, begin, end, norm);
  }
}

void 
dataProvider::addConsumer(dataSubscriber *consumer) 
{  
  bool added = false;
  // first check for open slot in existing consumers array
  // we don't care about order or indexed lookup for consumers cause 
  // there's so few per filter
  for (unsigned i = 0; i < allConsumers.size(); i++) {
    if (allConsumers[i] == NULL) {
      allConsumers[i] = consumer;
      added = true;
      break;
    }
  }
  if (!added) {
    // grow array to open up more slots then use one
    unsigned oldsize = allConsumers.size();
    allConsumers.resize(oldsize * 2);
    allConsumers[oldsize] = consumer;
    unsigned newsize = allConsumers.size();
    for (unsigned k = oldsize+1; k < newsize; k++)
      allConsumers[k] = NULL;
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

