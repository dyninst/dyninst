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

// $Id: PCdata.C,v 1.11 2003/05/21 18:21:17 pcroth Exp $
// dataSubscriber and dataProvider base classes

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
dataProvider::sendValue(PCmetDataID which, pdRate newval, relTimeStamp begin,
			relTimeStamp end)
{
  unsigned size = allConsumers.size();
#ifdef MYPCDEBUG
  if (size > 500) printf("================> allConsumers.size() = %d\n",size);
#endif
  for (unsigned i = 0; i < size; i++) {
    if (allConsumers[i] != NULL)
      (allConsumers[i])->newData (which, newval, begin, end);
  }
}

void 
dataProvider::sendEnableReply (unsigned token1, unsigned token2, unsigned token3,
			       bool successful, string msg )
{
  unsigned size = allConsumers.size();
  for (unsigned i = 0; i < size; i++) {
    if (allConsumers[i] != NULL)
      (allConsumers[i])->enableReply (token1, token2, token3, successful, msg);
  }
}
  
int
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
  return numConsumers;
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

