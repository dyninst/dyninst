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

// $Id: PCdata.h,v 1.10 2003/05/21 18:21:17 pcroth Exp $
// dataSubscriber and dataProvider base classes

#ifndef pc_data_h
#define pc_data_h

#include "PCintern.h"
#include "common/h/Time.h"
#include "pdutil/h/pdRate.h"

// all consumer lists grow as needed; this is only a start value
const unsigned initialConsumerListSize = 5;

/*
   note:  each filter may be a dataSubscriber, dataProvider, or both
   data flow in the current PC:
        DM ----> PCfilter ----> PCmetricInst ---> experiment 
      PCfilter: dataProvider
      PCmetricInst: dataSubscriber, dataProvider
      experiment: dataSubscriber
*/

class dataSubscriber {
 public:
  dataSubscriber() {;}
  virtual ~dataSubscriber() {;}
  virtual void newData(PCmetDataID, pdRate, relTimeStamp, relTimeStamp) = 0; 
  virtual void updateEstimatedCost(float) = 0;
  virtual void enableReply (unsigned, unsigned, unsigned, bool, string = "") = 0;
};

class dataProvider {
public:
  dataProvider();
  virtual ~dataProvider();
  float getEstimatedCost () {return estimatedCost;}
  // returns new number of subscribers, including new request
  int addConsumer(dataSubscriber*);
  // returns remaining number of subscribers after deletion
  int rmConsumer(dataSubscriber*);
  int getNumConsumers () {return numConsumers;}
  void sendValue(PCmetDataID, pdRate, relTimeStamp, relTimeStamp);
  void sendEnableReply (unsigned, unsigned, unsigned, bool, string = "");
protected:
  void sendUpdatedEstimatedCost(float costDiff);
  pdvector<dataSubscriber*> allConsumers;
  float estimatedCost;
  int numConsumers;
};

#endif
