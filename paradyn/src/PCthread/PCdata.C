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
 * PCdata.C
 *
 * dataSubscriber and dataProvider base classes
 *
 * $Log: PCdata.C,v $
 * Revision 1.7  1996/08/16 21:03:20  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.6  1996/07/22 18:55:36  karavan
 * part one of two-part commit for new PC functionality of restarting searches.
 *
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

