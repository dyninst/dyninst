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
 * PCdata.h
 *
 * dataSubscriber and dataProvider base classes
 *
 * $Log: PCdata.h,v $
 * Revision 1.2  1996/05/08 07:35:05  karavan
 * Changed enable data calls to be fully asynchronous within the performance consultant.
 *
 * some changes to cost handling, with additional limit on number of outstanding enable requests.
 *
 * Revision 1.1  1996/02/02 02:07:21  karavan
 * A baby Performance Consultant is born!
 *
 */

#ifndef pc_data_h
#define pc_data_h

#include "PCintern.h"

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
  virtual void newData(PCmetDataID, sampleValue, timeStamp, timeStamp, 
		       sampleValue) = 0; 
  virtual void updateEstimatedCost(float) = 0;
  virtual void enableReply (unsigned, unsigned, unsigned, bool) = 0;
};

class dataProvider {
public:
  dataProvider();
  virtual ~dataProvider();
  float getEstimatedCost () {return estimatedCost;}
  void addConsumer(dataSubscriber*);
  // returns remaining number of subscribers after deletion
  int rmConsumer(dataSubscriber*);
  void sendValue(PCmetDataID, sampleValue, timeStamp, timeStamp, sampleValue);
  void sendEnableReply (unsigned, unsigned, unsigned, bool);
protected:
  void sendUpdatedEstimatedCost(float costDiff);
  vector<dataSubscriber*> allConsumers;
  float estimatedCost;
  int numConsumers;
};

#endif
